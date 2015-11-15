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
 */

#include "piglit-util-gl.h"
#include "../../fbo/fbo-formats.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 21;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

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
	"#extension GL_ARB_texture_multisample : require\n"
	"\n"
	"uniform sampler2DMS tex;\n"
	"\n"
	"void\n"
	"main()\n"
	"{\n"
	"	 gl_FragColor = texelFetch(tex, ivec2(0), 0);\n"
	"}\n";

static const char
fragment_source_int[] =
	"#version 130\n"
	"#extension GL_ARB_texture_multisample : require\n"
	"\n"
	"uniform isampler2DMS tex;\n"
	"\n"
	"void\n"
	"main()\n"
	"{\n"
	"	 gl_FragColor = vec4(texelFetch(tex, ivec2(0), 0)) / 127.0;\n"
	"}\n";

static const char
fragment_source_uint[] =
	"#version 130\n"
	"#extension GL_ARB_texture_multisample : require\n"
	"\n"
	"uniform usampler2DMS tex;\n"
	"\n"
	"void\n"
	"main()\n"
	"{\n"
	"	 gl_FragColor = vec4(texelFetch(tex, ivec2(0), 0)) / 255.0;\n"
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
};

static GLuint prog_float, prog_int, prog_uint;

static enum piglit_result
test_color(GLuint fbo,
	   int offset,
	   const struct format_desc *format,
	   GLenum clear_type,
	   const float *clear_color)
{
	float expected_color[4];
	float alpha_override;
	int i;

	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	switch (clear_type) {
	case GL_INT: {
		GLint clear_color_int[4] = {
			clear_color[0] * 127,
			clear_color[1] * 127,
			clear_color[2] * 127,
			clear_color[3] * 127
		};
		if (prog_int == 0)
			return PIGLIT_SKIP;
		glUseProgram(prog_int);
		glClearBufferiv(GL_COLOR,
				0, /* draw buffer */
				clear_color_int);
		alpha_override = 1.0f / 127.0f;
		break;
	}
	case GL_UNSIGNED_INT: {
		GLint clear_color_uint[4] = {
			clear_color[0] * 255,
			clear_color[1] * 255,
			clear_color[2] * 255,
			clear_color[3] * 255
		};
		if (prog_uint == 0)
			return PIGLIT_SKIP;
		glUseProgram(prog_uint);
		glClearBufferiv(GL_COLOR,
				0, /* draw buffer */
				clear_color_uint);
		alpha_override = 1.0f / 255.0f;
		break;
	}
	default:
		glUseProgram(prog_float);
		glClearColor(clear_color[0],
			     clear_color[1],
			     clear_color[2],
			     clear_color[3]);
		glClear(GL_COLOR_BUFFER_BIT);
		alpha_override = 1.0f;
		break;
	}

	memcpy(expected_color, clear_color, sizeof expected_color);

	switch (format->base_internal_format) {
	case GL_ALPHA:
		expected_color[0] = 0.0f;
		expected_color[1] = 0.0f;
		expected_color[2] = 0.0f;
		break;
	case GL_INTENSITY:
		expected_color[0] = clear_color[0];
		expected_color[1] = clear_color[0];
		expected_color[2] = clear_color[0];
		expected_color[3] = clear_color[0];
		break;
	case GL_LUMINANCE:
		expected_color[1] = clear_color[0];
		expected_color[2] = clear_color[0];
		expected_color[3] = alpha_override;
		break;
	case GL_LUMINANCE_ALPHA:
		expected_color[1] = clear_color[0];
		expected_color[2] = clear_color[0];
		break;
	case GL_RED:
		expected_color[1] = 0.0f;
		expected_color[2] = 0.0f;
		expected_color[3] = alpha_override;
		break;
	case GL_RG:
		expected_color[2] = 0.0f;
		expected_color[3] = alpha_override;
		break;
	case GL_RGB:
		expected_color[3] = alpha_override;
		break;
	}

	if (strstr(format->name, "SRGB") ||
	    strstr(format->name, "SLUMINANCE")) {
		for (i = 0; i < 3; i++) {
			expected_color[i] =
				piglit_srgb_to_linear(expected_color[i]);
		}
	}

	glBindFramebuffer(GL_FRAMEBUFFER, piglit_winsys_fbo);
	piglit_draw_rect(offset * 16 * 2.0f / piglit_width - 1.0f,
			 -1.0f,
			 16 * 2.0f / piglit_width,
			 16 * 2.0f / piglit_height);
	return piglit_probe_rect_rgba(offset * 16, 0, 16, 16,
				      expected_color) ?
		PIGLIT_PASS :
		PIGLIT_FAIL;
}

static enum piglit_result
test_format(const struct format_desc *format)
{
	enum piglit_result result = PIGLIT_PASS;
	enum piglit_result color_result;
	GLint l_size, i_size, r_size, g_size, b_size, a_size;
	GLenum type_param;
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

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, tex);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE,
				1, /* samples */
				format->internalformat,
				1, 1, /* width/height */
				GL_FALSE /* fixed sample locations */);

	glGetTexLevelParameteriv(GL_TEXTURE_2D_MULTISAMPLE, 0,
				 GL_TEXTURE_LUMINANCE_SIZE, &l_size);
	glGetTexLevelParameteriv(GL_TEXTURE_2D_MULTISAMPLE, 0,
				 GL_TEXTURE_ALPHA_SIZE, &a_size);
	glGetTexLevelParameteriv(GL_TEXTURE_2D_MULTISAMPLE, 0,
				 GL_TEXTURE_INTENSITY_SIZE, &i_size);
	glGetTexLevelParameteriv(GL_TEXTURE_2D_MULTISAMPLE, 0,
				 GL_TEXTURE_RED_SIZE, &r_size);
	glGetTexLevelParameteriv(GL_TEXTURE_2D_MULTISAMPLE, 0,
				 GL_TEXTURE_GREEN_SIZE, &g_size);
	glGetTexLevelParameteriv(GL_TEXTURE_2D_MULTISAMPLE, 0,
				 GL_TEXTURE_BLUE_SIZE, &b_size);

	if (l_size > 0)
		type_param = GL_TEXTURE_LUMINANCE_TYPE;
	else if (i_size > 0)
		type_param = GL_TEXTURE_INTENSITY_TYPE;
	else if (r_size > 0)
		type_param = GL_TEXTURE_RED_TYPE;
	else if (a_size > 0)
		type_param = GL_TEXTURE_ALPHA_TYPE;
	else {
		assert(0);
		type_param = GL_NONE;
	}
	glGetTexLevelParameteriv(GL_TEXTURE_2D_MULTISAMPLE,
				 0, /* level */
				 type_param,
				 &type);

	switch (format->base_internal_format) {
	case GL_ALPHA:
		r_size = g_size = b_size = 8;
		break;
	case GL_INTENSITY:
		r_size = g_size = b_size = a_size = i_size;
		break;
	case GL_LUMINANCE:
		r_size = g_size = b_size = l_size;
		a_size = 8;
		break;
	case GL_LUMINANCE_ALPHA:
		r_size = g_size = b_size = l_size;
		break;
	case GL_RED:
		g_size = b_size = a_size = 8;
		break;
	case GL_RG:
		b_size = a_size = 8;
		break;
	case GL_RGB:
		a_size = 8;
		break;
	}

	/* We can't measure more bits than what the winsys buffer has */
	r_size = MIN2(r_size, 8);
	g_size = MIN2(g_size, 8);
	b_size = MIN2(b_size, 8);
	a_size = MIN2(a_size, 8);

	piglit_set_tolerance_for_bits(r_size, g_size, b_size, a_size);

	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER,
			       GL_COLOR_ATTACHMENT0,
			       GL_TEXTURE_2D_MULTISAMPLE,
			       tex,
			       0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) ==
	    GL_FRAMEBUFFER_COMPLETE) {
		for (i = 0; i < ARRAY_SIZE(clear_colors); i++) {
			color_result = test_color(fbo, i, format, type,
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

	prog = piglit_build_simple_program(vertex_source, fragment_source);
	glUseProgram(prog);
	tex_location = glGetUniformLocation(prog, "tex");
	glUniform1i(tex_location, 0);

	return prog;
}

void
piglit_init(int argc, char **argv)
{
	int test_set_index = 0;
	int glsl_major, glsl_minor;
	bool es;

	if (argc >= 2)
		test_set_index = fbo_lookup_test_set(argv[1]);

	piglit_require_extension("GL_ARB_texture_multisample");

	test_set = test_set + test_set_index;

	fbo_formats_init_test_set(test_set_index,
				  GL_TRUE /* print_options */);

	prog_float = build_program(fragment_source_float);

	piglit_get_glsl_version(&es, &glsl_major, &glsl_minor);

	if (!es && (glsl_major > 1 || (glsl_major == 1 && glsl_minor >= 3))) {
		prog_int = build_program(fragment_source_int);
		prog_uint = build_program(fragment_source_uint);
	}
}
