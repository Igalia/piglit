/*
 * Copyright © 2011 Intel Corporation
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

/**
 * \file glsl-fs-color-matrix.c
 * Transform the color value read from a texture by a matrix, keeping its alpha
 *
 * The shader in this test is fairly terrible (calling texture2D twice with
 * the same texture coordinate), but it reproduces a bug in the Mesa i915
 * driver.  See Meego bug #13005 (https://bugs.meego.com/show_bug.cgi?id=13005).
 */
#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

const char *vs_text =
	"attribute vec4 vertex;\n"
	"attribute vec2 textureCoord;\n"
	"varying   vec2 coord;\n"
	"void main(void)\n"
	"{\n"
	"  gl_Position = vertex;\n"
	"  coord = textureCoord;\n"
	"}\n";

const char *fs_text =
	"uniform sampler2D  texture;\n"
	"uniform mat4 colorMatrix;\n"
	"varying vec2 coord;\n"
	"void main(void)\n"
	"{\n"
	"  vec4 color = vec4(texture2D(texture, coord.st).rgb, 1.0);\n"
	"  color = colorMatrix * color;\n"
	"  gl_FragColor = vec4(color.rgb, texture2D(texture, coord.st).a);\n"
	"}\n";

static const GLfloat identity_matrix[] = {
	0.0, 0.0, 1.0, 0.0,
	1.0, 0.0, 0.0, 0.0,
	0.0, 1.0, 0.0, 0.0,
	0.0, 0.0, 0.0, 1.0,
};

static const GLfloat vertex[] = {
	-1.0, -1.0,
	+1.0, -1.0,
	+1.0,  1.0,
	-1.0,  1.0,
};

static const GLfloat tex_coord[] = {
	0.0, 0.0,
	1.0, 0.0,
	1.0, 1.0,
	0.0, 1.0,
};

static const GLfloat white[4] = { 1.0, 1.0, 1.0, 1.0 };
static const GLfloat red[4]   = { 1.0, 0.0, 0.0, 1.0 };
static const GLfloat green[4] = { 0.0, 1.0, 0.0, 1.0 };
static const GLfloat blue[4]  = { 0.0, 0.0, 1.0, 1.0 };

static GLint prog;

enum piglit_result
piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	GLuint tex;

	tex = piglit_rgbw_texture(GL_RGBA8, 64, 64, GL_FALSE, GL_TRUE,
				  GL_UNSIGNED_NORMALIZED);

	glClear(GL_COLOR_BUFFER_BIT);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	pass = piglit_probe_pixel_rgb(1 * piglit_width / 3,
				      1 * piglit_width / 3,
				      blue);
	pass = piglit_probe_pixel_rgb(2 * piglit_width / 3,
				      1 * piglit_width / 3,
				      red)
		&& pass;
	pass = piglit_probe_pixel_rgb(1 * piglit_width / 3,
				      2 * piglit_width / 3,
				      green)
		&& pass;
	pass = piglit_probe_pixel_rgb(2 * piglit_width / 3,
				      2 * piglit_width / 3,
				      white)
		&& pass;

	piglit_present_results();

	glDeleteTextures(1, &tex);

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	GLuint vs;
	GLuint fs;
	GLint loc;
	GLboolean ok;

	piglit_require_GLSL();

	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vs_text);
	fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, fs_text);
	prog = piglit_link_simple_program(vs, fs);

	glBindAttribLocation(prog, 0, "vertex");
	glBindAttribLocation(prog, 1, "textureCoord");

	glLinkProgram(prog);
	ok = piglit_link_check_status(prog);
	if (!ok)
		piglit_report_result(PIGLIT_FAIL);

	glUseProgram(prog);

	loc = glGetUniformLocation(prog, "colorMatrix");
	glUniformMatrix4fv(loc, 1, GL_FALSE, identity_matrix);

	loc = glGetUniformLocation(prog, "texture");
	glUniform1i(loc, 0);

	glClearColor(0.2, 0.2, 0.2, 1.0);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
				   2 * sizeof(GLfloat), vertex);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
				   2 * sizeof(GLfloat), tex_coord);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
}
