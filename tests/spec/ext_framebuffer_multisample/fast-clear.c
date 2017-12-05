/*
 * Copyright © 2015 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/**
 * \file fast-clear.c
 *
 * Clears a multisample texture with various formats to various
 * different colors and then samples from it in a shader to ensure
 * that the expected color is returned. This includes verifying that
 * when there are components missing they are overriden to the right
 * value (such as GL_RED should report 0 for green and blue and 1 for
 * the alpha). The main reason to do this is that the i965 driver has
 * various different code paths to implement a fast clear optimisation
 * and the path taken depends on the color chosen to a certain
 * degree.
 *
 * The test can take the following additional arguments:
 *
 *  enable-fb-srgb: This will cause it to enable GL_FRAMEBUFFER_SRGB
 *    before clearing the buffer so that it can test that the color
 *    gets correctly converted to SRGB before being stored in the
 *    color buffer.
 *  single-sample: A single sample texture will be created instead.
 */

#include "piglit-util-gl.h"
#include "../../fbo/fbo-formats.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 21;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

static const char
vertex_source[] =
	"attribute vec4 piglit_vertex;\n"
	"\n"
	"void\n"
	"main()\n"
	"{\n"
	"	 gl_Position = piglit_vertex;\n"
	"}\n";

static const char
fragment_source_float[] =
	"#version 130\n"
	"%s\n"
	"\n"
	"uniform %s tex;\n"
	"\n"
	"void\n"
	"main()\n"
	"{\n"
	"	 gl_FragColor = texelFetch(tex, ivec2(0), 0);\n"
	"}\n";

static const char
fragment_source_int[] =
	"#version 130\n"
	"%s\n"
	"\n"
	"uniform i%s tex;\n"
	"\n"
	"void\n"
	"main()\n"
	"{\n"
	"	 gl_FragColor = vec4(texelFetch(tex, ivec2(0), 0));\n"
	"}\n";

static const char
fragment_source_uint[] =
	"#version 130\n"
	"%s\n"
	"\n"
	"uniform u%s tex;\n"
	"\n"
	"void\n"
	"main()\n"
	"{\n"
	"	 gl_FragColor = vec4(texelFetch(tex, ivec2(0), 0));\n"
	"}\n";

static const struct test_desc *
test_set = &test_sets[0];

static const float
clear_colors[][4] = {
	{ 0.0f, 0.0f, 0.0f, 0.0f },
	{ 1.0f, 1.0f, 1.0f, 1.0f },
	{ 0.0f, 0.0f, 1.0f, 0.0f },
	{ 1.0f, 0.0f, 0.0f, 1.0f },

	{ 0.25f, 0.5f, 0.75f, 1.0f },
	{ 0.75f, 0.5f, 0.25f, 0.0f },
	{ 0.5f, 0.25f, 0.75f, 0.5f },

	{ 2.0f, 3.0f, 0.5f, 1.0f },
	{ -2.0f, 0.0f, 0.25f, 1.0f },
	{ -0.5f, 0.0f, 0.25f, 1.0f },
};

struct component_sizes {
	int r, g, b;
	int a;
	int l;
	int i;
};

static GLuint prog_float, prog_int, prog_uint;
static GLuint result_fbo;
static bool enable_fb_srgb = false;
static bool single_sample = false;
static int num_samples = 2;

static void
convert_srgb_color(const struct format_desc *format,
		   float *color)
{
	int i;

	/* If the texture is not an sRGB format then no conversion is
	 * needed regardless of the sRGB settings.
	 */
	if (strstr(format->name, "SRGB") == NULL &&
	    strstr(format->name, "SLUMINANCE") == NULL)
		return;

	/* If GL_FRAMEBUFFER_SRGB was enabled when we did the clear
	 * then the clear color would have been converted to SRGB
	 * before being written. When it is sampled it will be
	 * converted back to linear. The two conversions cancel each
	 * other out so we don't need to do anything.
	 */
	if (enable_fb_srgb)
		return;

	/* Otherwise we need to compensate for the color being
	 * converted to linear when sampled.
	 */
	for (i = 0; i < 3; i++)
		color[i] = piglit_srgb_to_linear(color[i]);
}

static int
clamp_signed(int value, int size)
{
	return CLAMP(value,
		     -1 << (size - 1),
		     (int) (UINT32_MAX >> (33 - size)));
}

static unsigned int
clamp_unsigned(int value, int size)
{
	if (value < 0)
		return 0;
	return CLAMP((unsigned int) value, 0, UINT32_MAX >> (32 - size));
}

static enum piglit_result
test_color(GLuint test_fbo,
	   int offset,
	   const struct format_desc *format,
	   GLenum clear_type,
	   const struct component_sizes *sizes,
	   const float *clear_color)
{
	float expected_color[4];
	int i;

	glBindFramebuffer(GL_FRAMEBUFFER, test_fbo);

	if (enable_fb_srgb)
		glEnable(GL_FRAMEBUFFER_SRGB);

	memcpy(expected_color, clear_color, sizeof expected_color);

	switch (clear_type) {
	case GL_INT: {
		GLint clear_color_int[4];
		for (i = 0; i < 4; i++) {
			expected_color[i] *= 127;
			clear_color_int[i] = expected_color[i];
		}
		if (prog_int == 0)
			return PIGLIT_SKIP;
		glUseProgram(prog_int);
		glClearBufferiv(GL_COLOR,
				0, /* draw buffer */
				clear_color_int);
		break;
	}
	case GL_UNSIGNED_INT: {
		GLuint clear_color_uint[4];
		for (i = 0; i < 4; i++) {
			expected_color[i] *= 255;
			clear_color_uint[i] = MAX2(expected_color[i], 0);
		}
		if (prog_uint == 0)
			return PIGLIT_SKIP;
		glUseProgram(prog_uint);
		glClearBufferuiv(GL_COLOR,
				 0, /* draw buffer */
				 clear_color_uint);
		break;
	}
	default:
		glUseProgram(prog_float);
		glClearColor(clear_color[0],
			     clear_color[1],
			     clear_color[2],
			     clear_color[3]);
		glClear(GL_COLOR_BUFFER_BIT);
		break;
	}

	if (enable_fb_srgb)
		glDisable(GL_FRAMEBUFFER_SRGB);

	switch (format->base_internal_format) {
	case GL_ALPHA:
		expected_color[0] = 0.0f;
		expected_color[1] = 0.0f;
		expected_color[2] = 0.0f;
		break;
	case GL_INTENSITY:
		expected_color[1] = expected_color[0];
		expected_color[2] = expected_color[0];
		expected_color[3] = expected_color[0];
		break;
	case GL_LUMINANCE:
		expected_color[1] = expected_color[0];
		expected_color[2] = expected_color[0];
		expected_color[3] = 1.0f;
		break;
	case GL_LUMINANCE_ALPHA:
		expected_color[1] = expected_color[0];
		expected_color[2] = expected_color[0];
		break;
	case GL_RED:
		expected_color[1] = 0.0f;
		expected_color[2] = 0.0f;
		expected_color[3] = 1.0f;
		break;
	case GL_RG:
		expected_color[2] = 0.0f;
		expected_color[3] = 1.0f;
		break;
	case GL_RGB:
		expected_color[3] = 1.0f;
		break;
	}

	convert_srgb_color(format, expected_color);

	if (clear_type == GL_UNSIGNED_NORMALIZED) {
		for (i = 0; i < 4; i++) {
			expected_color[i] =
				CLAMP(expected_color[i], 0.0f, 1.0f);
		}
	} else if (clear_type == GL_SIGNED_NORMALIZED) {
		for (i = 0; i < 4; i++) {
			expected_color[i] =
				CLAMP(expected_color[i], -1.0f, 1.0f);
		}
	} else if (clear_type == GL_INT) {
		/* The clear colors are multiplied by 127 for integer
		 * formats so some of them will be large values. The
		 * GL spec states that out-of-range integer values
		 * written to the framebuffer will be clamped so we
		 * need to replicate this in the expected values. For
		 * example, the -2.0 color will be set to -254, and
		 * this will be clamped to -128 for an 8-bit integer
		 * surface.
		 */
		expected_color[0] = clamp_signed(expected_color[0], sizes->r);
		expected_color[1] = clamp_signed(expected_color[1], sizes->g);
		expected_color[2] = clamp_signed(expected_color[2], sizes->b);
		expected_color[3] = clamp_signed(expected_color[3], sizes->a);
	} else if (clear_type == GL_UNSIGNED_INT) {
		expected_color[0] = clamp_unsigned(expected_color[0], sizes->r);
		expected_color[1] = clamp_unsigned(expected_color[1], sizes->g);
		expected_color[2] = clamp_unsigned(expected_color[2], sizes->b);
		expected_color[3] = clamp_unsigned(expected_color[3], sizes->a);
	} else if (test_sets[test_index].format == ext_packed_float) {
		/* These formats can't store negative values */
		for (i = 0; i < 4; i++)
			expected_color[i] = MAX2(expected_color[i], 0.0f);
	}

	/* Display something on the winsys FBO just so that something
	 * will be shown. This isn't used for the test results because
	 * the winsys buffer is a normalised format and some of the
	 * values we want to detect will be out of the range [0,1].
	 */
	glBindFramebuffer(GL_FRAMEBUFFER, piglit_winsys_fbo);
	piglit_draw_rect(offset * 16 * 2.0f / piglit_width - 1.0f,
			 -1.0f,
			 16 * 2.0f / piglit_width,
			 16 * 2.0f / piglit_height);

	glBindFramebuffer(GL_FRAMEBUFFER, result_fbo);
	piglit_draw_rect(-1, -1, 2, 2);

	return piglit_probe_rect_rgba(0, 0, 1, 1, expected_color) ?
		PIGLIT_PASS :
		PIGLIT_FAIL;
}

static enum piglit_result
test_format(const struct format_desc *format)
{
	enum piglit_result result = PIGLIT_PASS;
	enum piglit_result color_result;
	struct component_sizes sizes;
	GLenum type_param;
	GLenum tex_target;
	GLenum tex_error;
	GLint type;
	GLuint tex;
	GLuint fbo;
	int i;

	if (format->internalformat == 3 || format->internalformat == 4)
		return PIGLIT_SKIP;

	/* Compressed formats aren't supported for multisampling */
	if (strstr("COMPRESSED", format->name))
		return PIGLIT_SKIP;

	printf("Testing %s\n", format->name);

	if (single_sample)
		tex_target = GL_TEXTURE_2D;
	else
		tex_target = GL_TEXTURE_2D_MULTISAMPLE;

	glGenTextures(1, &tex);
	glBindTexture(tex_target, tex);

	if (single_sample) {
		glTexParameteri(tex_target,
				GL_TEXTURE_MAG_FILTER,
				GL_NEAREST);
		glTexParameteri(tex_target,
				GL_TEXTURE_MIN_FILTER,
				GL_NEAREST);
		glTexParameteri(tex_target,
				GL_TEXTURE_MAX_LEVEL,
				0);
		/* The pitch of the texture needs to at least as wide
		 * as a tile and taller than 1 pixel so that it will
		 * be y-tiled in the i965 driver. Otherwise fast
		 * clears will be disabled and the test will be
		 * pointless.
		 */
		glTexImage2D(tex_target,
			     0, /* level */
			     format->internalformat,
			     128, 128, /* width/height */
			     0, /* border */
			     GL_RGBA, GL_UNSIGNED_BYTE,
			     NULL);
	} else {
		if (piglit_khr_no_error)
			return PIGLIT_SKIP;

		piglit_reset_gl_error();

		/* The size doesn't matter on the i965 driver for
		 * multisample surfaces because it will always
		 * allocate an MCS buffer and so it will always do
		 * fast clears.
		 */
		glTexImage2DMultisample(tex_target,
					num_samples,
					format->internalformat,
					1, 1, /* width/height */
					GL_FALSE /* fixed sample locations */);
		tex_error = glGetError();

		if (tex_error != GL_NO_ERROR) {
			glDeleteTextures(1, &tex);

			if (tex_error == GL_INVALID_ENUM) {
				/* You're only supposed to pass color
				 * renderable formats to
				 * glTexImage2DMultisample.
				 */
				printf("Format is not color renderable\n");
				return PIGLIT_SKIP;
			} else {
				printf("Unexpected GL error: %s 0x%x\n",
				       piglit_get_gl_error_name(tex_error),
				       tex_error);
				return PIGLIT_FAIL;
			}
		}
	}

	glGetTexLevelParameteriv(tex_target, 0,
				 GL_TEXTURE_LUMINANCE_SIZE, &sizes.l);
	glGetTexLevelParameteriv(tex_target, 0,
				 GL_TEXTURE_ALPHA_SIZE, &sizes.a);
	glGetTexLevelParameteriv(tex_target, 0,
				 GL_TEXTURE_INTENSITY_SIZE, &sizes.i);
	glGetTexLevelParameteriv(tex_target, 0,
				 GL_TEXTURE_RED_SIZE, &sizes.r);
	glGetTexLevelParameteriv(tex_target, 0,
				 GL_TEXTURE_GREEN_SIZE, &sizes.g);
	glGetTexLevelParameteriv(tex_target, 0,
				 GL_TEXTURE_BLUE_SIZE, &sizes.b);

	if (sizes.l > 0)
		type_param = GL_TEXTURE_LUMINANCE_TYPE;
	else if (sizes.i > 0)
		type_param = GL_TEXTURE_INTENSITY_TYPE;
	else if (sizes.r > 0)
		type_param = GL_TEXTURE_RED_TYPE;
	else if (sizes.a > 0)
		type_param = GL_TEXTURE_ALPHA_TYPE;
	else {
		assert(0);
		type_param = GL_NONE;
	}
	glGetTexLevelParameteriv(tex_target,
				 0, /* level */
				 type_param,
				 &type);

	switch (format->base_internal_format) {
	case GL_ALPHA:
		sizes.r = sizes.g = sizes.b = 8;
		break;
	case GL_INTENSITY:
		sizes.r = sizes.g = sizes.b = sizes.a = sizes.i;
		break;
	case GL_LUMINANCE:
		sizes.r = sizes.g = sizes.b = sizes.l;
		sizes.a = 8;
		break;
	case GL_LUMINANCE_ALPHA:
		sizes.r = sizes.g = sizes.b = sizes.l;
		break;
	case GL_RED:
		sizes.g = sizes.b = sizes.a = 8;
		break;
	case GL_RG:
		sizes.b = sizes.a = 8;
		break;
	case GL_RGB:
		sizes.a = 8;
		break;
	}

	piglit_set_tolerance_for_bits(sizes.r, sizes.g, sizes.b, sizes.a);

	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER,
			       GL_COLOR_ATTACHMENT0,
			       tex_target,
			       tex,
			       0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) ==
	    GL_FRAMEBUFFER_COMPLETE) {
		for (i = 0; i < ARRAY_SIZE(clear_colors); i++) {
			color_result = test_color(fbo, i, format, type, &sizes,
						  clear_colors[i]);
			if (color_result == PIGLIT_SKIP) {
				if (result == PIGLIT_PASS)
					result = PIGLIT_SKIP;
				break;
			} else if (color_result == PIGLIT_FAIL) {
				result = PIGLIT_FAIL;
			}
		}
	} else {
		printf("FBO not complete\n");
		result = PIGLIT_SKIP;
	}

	glDeleteFramebuffers(1, &fbo);
	glDeleteTextures(1, &tex);

	return result;
}

enum piglit_result
piglit_display()
{
	return fbo_formats_display(test_format);
}

static GLuint
build_program(const char *fragment_source)
{
	GLint tex_location;
	GLuint prog;
	char *source;

	(void)!asprintf(&source,
		 fragment_source,
		 single_sample ?
		 "" :
		 "#extension GL_ARB_texture_multisample : require\n",
		 single_sample ? "sampler2D" : "sampler2DMS");

	prog = piglit_build_simple_program(vertex_source, source);

	free(source);

	glUseProgram(prog);
	tex_location = glGetUniformLocation(prog, "tex");
	glUniform1i(tex_location, 0);

	return prog;
}

void
piglit_init(int argc, char **argv)
{
	int test_set_index = 0;
	GLuint rb;
	int i;

	for (i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "enable-fb-srgb")) {
			enable_fb_srgb = true;
			piglit_require_extension("GL_ARB_framebuffer_sRGB");
		} else if (!strcmp(argv[i], "single-sample")) {
			single_sample = true;
		} else {
			test_set_index = fbo_lookup_test_set(argv[i]);
		}
	}

	if (!single_sample) {
		piglit_require_extension("GL_ARB_texture_multisample");
		/* Use the max number of samples for testing */
		glGetIntegerv(GL_MAX_COLOR_TEXTURE_SAMPLES, &num_samples);
		printf("Testing %d samples\n", num_samples);
	}

	piglit_require_extension("GL_ARB_texture_float");
	piglit_require_GLSL_version(130);

	test_set = test_set + test_set_index;

	fbo_formats_init_test_set(test_set_index,
				  GL_TRUE /* print_options */);

	/* Create a floating point fbo to store the result of
	 * sampling. It is only used to store a single color sampled
	 * from the texture so it doesn't need to bigger than 1x1.
	 */
	glGenFramebuffers(1, &result_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, result_fbo);
	glGenRenderbuffers(1, &rb);
	glBindRenderbuffer(GL_RENDERBUFFER, rb);
	glRenderbufferStorage(GL_RENDERBUFFER,
			      GL_RGBA32F,
			      1, 1 /* width/height */);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER,
				  GL_COLOR_ATTACHMENT0,
				  GL_RENDERBUFFER,
				  rb);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) !=
	    GL_FRAMEBUFFER_COMPLETE) {
		printf("Couldn't create RGBA32F FBO\n");
		piglit_report_result(PIGLIT_SKIP);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, piglit_winsys_fbo);

	prog_float = build_program(fragment_source_float);
	prog_int = build_program(fragment_source_int);
	prog_uint = build_program(fragment_source_uint);
}
