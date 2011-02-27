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
#include "piglit-util.h"

int piglit_width = 100, piglit_height = 100;
int piglit_window_mode = GLUT_RGB | GLUT_DOUBLE;

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

static const GLfloat black[4] = { 0.0, 0.0, 0.0, 1.0 };
static const GLfloat white[4] = { 1.0, 1.0, 1.0, 1.0 };
static const GLfloat red[4]   = { 1.0, 0.0, 0.0, 1.0 };
static const GLfloat green[4] = { 0.0, 1.0, 0.0, 1.0 };
static const GLfloat blue[4]  = { 0.0, 0.0, 1.0, 1.0 };

static GLint prog;

enum piglit_result
piglit_display(void)
{
	GLboolean pass = GL_TRUE;

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

	glutSwapBuffers();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	GLuint vs;
	GLuint fs;
	GLint loc;
	GLint tex;

	piglit_require_GLSL();

	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vs_text);
	fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, fs_text);
	prog = piglit_link_simple_program(vs, fs);

	piglit_BindAttribLocation(prog, 0, "vertex");
	piglit_BindAttribLocation(prog, 1, "textureCoord");

	piglit_LinkProgram(prog);
	piglit_link_check_status(prog);

	piglit_UseProgram(prog);

	loc = piglit_GetUniformLocation(prog, "colorMatrix");
	piglit_UniformMatrix4fv(loc, 1, GL_FALSE, identity_matrix);

	loc = piglit_GetUniformLocation(prog, "texture");
	piglit_Uniform1i(loc, 0);

	glClearColor(0.2, 0.2, 0.2, 1.0);

	piglit_VertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
				   2 * sizeof(GLfloat), vertex);
	piglit_VertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
				   2 * sizeof(GLfloat), tex_coord);

	piglit_EnableVertexAttribArray(0);
	piglit_EnableVertexAttribArray(1);

	tex = piglit_rgbw_texture(GL_RGBA8, 64, 64, GL_FALSE, GL_TRUE);
}
