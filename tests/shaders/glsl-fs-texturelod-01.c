/*
 * Copyright © 2010 Intel Corporation
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

#include "piglit-util.h"

/// \name GL State
/// \{
static GLuint texture_id;
static const GLuint texture_unit = 0;
static const int num_lod = 4;
static GLint lod_uniform;
/// \}

/// \name Colors
/// \{
static const int num_colors = 4;
static const float color_wheel[4][4] = {
	{1, 0, 0, 1}, // red
	{0, 1, 0, 1}, // green
	{0, 0, 1, 1}, // blue
	{1, 1, 1, 1}, // white
};
/// \}

/// \name Piglit State
/// \{
int piglit_width = 100;
int piglit_height = 100;
int piglit_window_mode = GLUT_RGB | GLUT_DOUBLE;
/// \}

static void
setup_mipmap_level(int lod, int width, int height)
{
	GLfloat *texture_data;
	const float *color;
	int i;
	const GLenum internal_format = GL_RGBA;

	texture_data = (GLfloat *) malloc(width * height * 4 * sizeof(GLfloat));
	assert(texture_data != NULL);

	color = color_wheel[lod % num_colors];

	for (i = 0; i < width * height; ++i) {
		texture_data[4 * i + 0] = color[0];
		texture_data[4 * i + 1] = color[1];
		texture_data[4 * i + 2] = color[2];
		texture_data[4 * i + 3] = color[3];
	}
	glTexImage2D(GL_TEXTURE_2D, lod, internal_format, width, height, 0,
				 GL_RGBA, GL_FLOAT, texture_data);

	free(texture_data);
}

static void
setup_texture()
{
	int lod;

	glGenTextures(1, &texture_id);
	glActiveTexture(GL_TEXTURE0 + texture_unit);
	glBindTexture(GL_TEXTURE_2D, texture_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);

	for (lod = 0; lod < num_lod; ++lod) {
		const int level_size = (2 * num_lod) >> lod;
		printf("Creating level %d at size %d\n", lod, level_size);
		setup_mipmap_level(lod, level_size, level_size);
	}

	glEnable(GL_TEXTURE_2D);
}

enum piglit_result
piglit_display()
{
	int lod;
	int x;
	GLboolean pass = GL_TRUE;

	glClearColor(0.4, 0.4, 0.4, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);

	for (lod = 0; lod < num_lod; ++lod) {
		x = 10 + lod * 20;
		glUniform1f(lod_uniform, lod);
		piglit_draw_rect(x, 10, 10, 10);

		pass &= piglit_probe_rect_rgba(x, 10, 10, 10,
				color_wheel[lod % num_colors]);
	}

	glutSwapBuffers();

	return pass ? PIGLIT_SUCCESS : PIGLIT_FAILURE;
}

void
piglit_init(int argc, char **argv)
{
	GLint vs, fs, prog, sampler_uniform;

	if (!GLEW_VERSION_2_0) {
		printf("Requires OpenGL 2.0\n");
		piglit_report_result(PIGLIT_SKIP);
	}
	if (!GLEW_ARB_shader_texture_lod) {
		printf("Requires extension GL_ARB_shader_texture_lod\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	setup_texture();

	// Compile and use program.
	vs = piglit_compile_shader(GL_VERTEX_SHADER,
			"shaders/glsl-fs-texturelod-01.vert");
	fs = piglit_compile_shader(GL_FRAGMENT_SHADER,
			"shaders/glsl-fs-texturelod-01.frag");
	prog = piglit_link_simple_program(vs, fs);
	glUseProgram(prog);

	// Setup uniforms.
	sampler_uniform = glGetUniformLocation(prog, "sampler");
	if (sampler_uniform == -1) {
		printf("error: Unable to get location of uniform 'sampler'\n");
		piglit_report_result(PIGLIT_FAILURE);
		return;
	}
	glUniform1i(sampler_uniform, texture_unit);
	lod_uniform = glGetUniformLocation(prog, "lod");
	if (lod_uniform == -1) {
		printf("error: Unable to get location of uniform 'lod'\n");
		piglit_report_result(PIGLIT_FAILURE);
		return;
	}
}
