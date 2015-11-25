/*
 * Copyright (c) 2015 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
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
 * @file draw-arrays-shared-binding.c
 *
 * Test creates a VAO and tests sharing binding point with 2 enabled vertex
 * attribute arrays, one used for vertices and another for output colors.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_core_version = 31;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE;
PIGLIT_GL_TEST_CONFIG_END

static const char vs_text[] =
	"attribute vec4 vertex;\n"
	"attribute vec4 colors;\n"
	"varying vec4 color;\n"
	"void main() {\n"
		"gl_Position = vertex;\n"
		"color = colors;\n"
	"}";

static const char fs_text[] =
	"varying vec4 color;\n"
	"void main() {\n"
		"gl_FragColor = color;\n"
	"}";

static const GLfloat rect[] = {
	-1.0f,  1.0f, 0.0f,
	1.0f,  1.0f, 0.0f,
	-1.0f, -1.0f, 0.0f,
	1.0f, -1.0f, 0.0f,
};

GLuint indirect_data[] = {
	4, 1, 0, 0
};

GLuint vao;

enum piglit_result
piglit_display(void)
{
	bool pass = true;
	unsigned i;
	GLfloat colors[ARRAY_SIZE(rect)];

	/* Resulting colors should match given vertices clamped. */
	for (i = 0; i < ARRAY_SIZE(rect); i++)
		colors[i] = CLAMP(rect[i], 0.0, 1.0);

	glBindVertexArray(vao);
	glDrawArraysIndirect(GL_TRIANGLE_STRIP, 0);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	piglit_present_results();

	/* Probe each corner. */
	pass &= piglit_probe_pixel_rgb(0, piglit_height -1, colors);
	pass &= piglit_probe_pixel_rgb(piglit_width - 1, piglit_height - 1, &colors[3]);
	pass &= piglit_probe_pixel_rgb(0, 0, &colors[6]);
	pass &= piglit_probe_pixel_rgb(piglit_width - 1, 0, &colors[9]);

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	GLuint prog, vbo, indirect;

	piglit_require_GLSL();
	piglit_require_extension("GL_ARB_draw_indirect");

	prog = piglit_build_simple_program(vs_text, fs_text);
	glUseProgram(prog);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &indirect);
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indirect);
	glBufferData(GL_DRAW_INDIRECT_BUFFER, sizeof(indirect_data), indirect_data, GL_STATIC_DRAW);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(rect), rect, GL_STATIC_DRAW);

	/* Enable 2 vertex attrib arrays. */
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

	/* Associate both arrays with binding 0. */
	glVertexAttribBinding(0, 0);
	glVertexAttribBinding(1, 0);

	glBindVertexArray(0);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);
}
