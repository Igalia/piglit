/*
 * Copyright © 2016 Intel Corporation
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


/** @file invalid.c
 *
 * Verifies that with GL_INTEL_conservative_rasterization enabled, we
 * get specified errors when using invalid modes for that extension.
 */

#include "piglit-util-gl.h"
#include "piglit-matrix.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

#if defined(PIGLIT_USE_OPENGL)
	config.supports_gl_core_version = 42;
#elif defined(PIGLIT_USE_OPENGL_ES3)
	config.supports_gl_es_version = 31;
#endif

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

void piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_INTEL_conservative_rasterization");

	GLuint prog = piglit_build_simple_program(
#if defined(PIGLIT_USE_OPENGL)
		"#version 330\n"
#elif defined(PIGLIT_USE_OPENGL_ES3)
		"#version 310 es\n"
		"precision highp float;\n"
#endif
		"in vec4 piglit_vertex;\n"
		"void main()\n"
		"{\n"
		"  gl_Position = piglit_vertex;\n"
		"}\n",
#if defined(PIGLIT_USE_OPENGL)
		"#version 330\n"
#elif defined(PIGLIT_USE_OPENGL_ES3)
		"#version 310 es\n"
		"precision highp float;\n"
#endif
		"\n"
		"out vec4 color;\n"
		"\n"
		"void main()\n"
		"{\n"
		"  color = vec4(1.0, 0.0, 0.0, 1.0);\n"
		"}\n");
	if (!prog)
		piglit_report_result(PIGLIT_FAIL);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
	glUseProgram(prog);

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	GLuint vbo;
	float vertices[3][2] = {
		{ -0.5, -1, },
		{ 0, 0.8, },
		{ 0.5, -1, },
	};
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), NULL);
	glEnableVertexAttribArray(0);

	glEnable(GL_CONSERVATIVE_RASTERIZATION_INTEL);
#ifdef PIGLIT_USE_OPENGL
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
#endif
	glClear(GL_COLOR_BUFFER_BIT);

	glDrawArrays(GL_POINTS, 0, 3);
	if (!piglit_check_gl_error(GL_INVALID_OPERATION))
		piglit_report_result(PIGLIT_FAIL);

	glDrawArrays(GL_LINES, 0, 3);
	if (!piglit_check_gl_error(GL_INVALID_OPERATION))
		piglit_report_result(PIGLIT_FAIL);

#ifdef PIGLIT_USE_OPENGL
	glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
	glDrawArrays(GL_LINES, 0, 3);
	if (!piglit_check_gl_error(GL_INVALID_OPERATION))
		piglit_report_result(PIGLIT_FAIL);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDrawArrays(GL_LINES, 0, 3);
	if (!piglit_check_gl_error(GL_INVALID_OPERATION))
		piglit_report_result(PIGLIT_FAIL);
#endif

	piglit_report_result(PIGLIT_PASS);
}
