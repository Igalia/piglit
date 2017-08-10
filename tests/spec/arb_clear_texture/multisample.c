/*
 * Copyright (c) 2014 Intel Corporation
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

/** @file multisample.c
 *
 * A test of using glClearTexSubImage to clear sub-regions of a
 * multisample texture. An 8x8 multisample texture is created with 4
 * samples. The whole texture is cleared using glClearTexImage and
 * then two sub-regions are cleared using glClearTexSubImage. One
 * region is using NULL for the data and the other is using a known
 * value. The texture is then drawn at 16x16 so that every sample of
 * every texel can be drawn using special shader. The values are then
 * compared to check that all of the samples are cleared.
 */

#define TEX_WIDTH 8
#define TEX_HEIGHT 8
#define TEX_SAMPLES 4

#define VALUE_CLEAR_X 2
#define VALUE_CLEAR_Y 4
#define VALUE_CLEAR_WIDTH 3
#define VALUE_CLEAR_HEIGHT 2

#define ZERO_CLEAR_X 5
#define ZERO_CLEAR_Y 1
#define ZERO_CLEAR_WIDTH 2
#define ZERO_CLEAR_HEIGHT 3

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 21;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

static const float red[4] = {1.0, 0.0, 0.0, 1.0};
static const float green[4] = {0.0, 1.0, 0.0, 1.0};
static const float black[4] = {0.0, 0.0, 0.0, 1.0};

static GLuint
create_texture(void)
{
	GLuint tex;

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, tex);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE,
				TEX_SAMPLES,
				GL_RGBA,
				TEX_WIDTH, TEX_HEIGHT,
				GL_FALSE /* fixedsamplelocations */);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	return tex;
}

static void
clear_texture(GLuint tex)
{
	/* Clear the entire texture to red */
	glClearTexImage(tex,
			0, /* level */
			GL_RGBA,
			GL_FLOAT,
			red);

	/* Clear two other sub-regions */

	glClearTexSubImage(tex,
			   0, /* level */
			   ZERO_CLEAR_X, ZERO_CLEAR_Y, 0,
			   ZERO_CLEAR_WIDTH, ZERO_CLEAR_HEIGHT, 1,
			   GL_RGBA,
			   GL_UNSIGNED_BYTE,
			   NULL);

	glClearTexSubImage(tex,
			   0, /* level */
			   VALUE_CLEAR_X, VALUE_CLEAR_Y, 0,
			   VALUE_CLEAR_WIDTH, VALUE_CLEAR_HEIGHT, 1,
			   GL_RGBA,
			   GL_FLOAT,
			   green);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);
}

static void
init_program(void)
{
	GLuint prog;
	GLuint uniform;

	static const char vs_source[] =
		"#version 130\n"
		"in vec2 piglit_vertex;\n"
		"uniform vec2 fb_size;\n"
		"out vec2 sample_coord;\n"
		"\n"
		"void main()\n"
		"{\n"
		"        gl_Position = vec4(piglit_vertex * 2.0 /\n"
		"                           fb_size - 1.0,\n"
		"                           0.0, 1.0);\n"
		"        sample_coord = piglit_vertex;\n"
		"}\n";
	static const char fs_source[] =
		"#version 130\n"
		"#extension GL_ARB_texture_multisample : enable\n"
		"uniform sampler2DMS tex;\n"
		"in vec2 sample_coord;\n"
		"\n"
		"void main()\n"
		"{\n"
		"        ivec2 isample_coord = ivec2(sample_coord);\n"
		"        ivec2 tex_coord = isample_coord / 2;\n"
		"        int sample = ((isample_coord.x & 1) * 2 +\n"
		"                      (isample_coord.y & 1));\n"
		"        gl_FragColor = texelFetch(tex, tex_coord, sample);\n"
		"}\n";

	prog = piglit_build_simple_program(vs_source, fs_source);

	glUseProgram(prog);

	uniform = glGetUniformLocation(prog, "tex");
	glUniform1i(uniform, 0);

	uniform = glGetUniformLocation(prog, "fb_size");
	glUniform2f(uniform, piglit_width, piglit_height);
}

void
piglit_init(int argc, char **argv)
{
	GLint maxColorTextureSamples;

	/* glClearTexture is either in the GL_ARB_clear_texture
	 * extension or in core in GL 4.4
	 */
	if (piglit_get_gl_version() < 44 &&
	    !piglit_is_extension_supported("GL_ARB_clear_texture")) {
		printf("OpenGL 4.4 or GL_ARB_clear_texture is required.\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	piglit_require_extension("GL_ARB_texture_multisample");
	piglit_require_GLSL_version(130);

	/* We need to support multisample textures with at least 4
	 * samples */
	glGetIntegerv(GL_MAX_COLOR_TEXTURE_SAMPLES, &maxColorTextureSamples);
	if (maxColorTextureSamples < TEX_SAMPLES) {
		printf("At least %i texture samples are required\n",
		       TEX_SAMPLES);
		piglit_report_result(PIGLIT_SKIP);
	}

	init_program();
}

static void
draw_tex(GLuint tex)
{
	static const struct {
		float x, y;
	} attribs[] = {
		{ 0.0f, 0.0f },
		{ TEX_WIDTH * 2.0f, 0.0f },
		{ 0.0f, TEX_HEIGHT * 2.0f },
		{ TEX_WIDTH * 2, TEX_HEIGHT * 2.0f },
	};

	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, tex);

	glEnableVertexAttribArray(PIGLIT_ATTRIB_POS);
	glVertexAttribPointer(PIGLIT_ATTRIB_POS,
			      2, /* size */
			      GL_FLOAT,
			      GL_FALSE, /* normalized */
			      sizeof attribs[0],
			      &attribs[0].x);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glDisableVertexAttribArray(PIGLIT_ATTRIB_POS);
}

static bool
probe_tex(int x, int y, int width, int height)
{
	/* The texture coordinates are double because it was drawn a
	 * quadruple the size */
	return piglit_probe_rect_rgb(x * 2, y * 2, width * 2, height * 2, red);
}

enum piglit_result
piglit_display(void)
{
	bool pass = true;
	GLuint tex;

	tex = create_texture();

	clear_texture(tex);

	draw_tex(tex);

	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

	glDeleteTextures(1, &tex);

	pass &= piglit_probe_rect_rgb(VALUE_CLEAR_X * 2,
				      VALUE_CLEAR_Y * 2,
				      VALUE_CLEAR_WIDTH * 2,
				      VALUE_CLEAR_HEIGHT * 2,
				      green);

	pass &= piglit_probe_rect_rgb(ZERO_CLEAR_X * 2,
				      ZERO_CLEAR_Y * 2,
				      ZERO_CLEAR_WIDTH * 2,
				      ZERO_CLEAR_HEIGHT * 2,
				      black);

	/* Everything else should be red */

	pass &= probe_tex(0, 0,
			  TEX_WIDTH, ZERO_CLEAR_Y);
	pass &= probe_tex(0, ZERO_CLEAR_Y,
			  ZERO_CLEAR_X, ZERO_CLEAR_HEIGHT);
	pass &= probe_tex(ZERO_CLEAR_X + ZERO_CLEAR_WIDTH, ZERO_CLEAR_Y,
			  TEX_WIDTH - ZERO_CLEAR_X - ZERO_CLEAR_WIDTH,
			  ZERO_CLEAR_HEIGHT);
	pass &= probe_tex(0, VALUE_CLEAR_Y,
			  VALUE_CLEAR_X, VALUE_CLEAR_HEIGHT);
	pass &= probe_tex(VALUE_CLEAR_X + VALUE_CLEAR_WIDTH, VALUE_CLEAR_Y,
			  TEX_WIDTH - VALUE_CLEAR_X - VALUE_CLEAR_WIDTH,
			  VALUE_CLEAR_HEIGHT);
	pass &= probe_tex(0, VALUE_CLEAR_Y + VALUE_CLEAR_HEIGHT,
			  TEX_WIDTH,
			  TEX_HEIGHT - VALUE_CLEAR_Y - VALUE_CLEAR_HEIGHT);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
