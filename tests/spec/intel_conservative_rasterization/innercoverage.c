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


/** @file innercoverage.c
 *
 * Verifies that the inner_coverage layout qualifier works when
 * GL_INTEL_conservative_rasterization is enabled.
 */

#include "piglit-util-gl.h"
#include "piglit-matrix.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

#if defined(PIGLIT_USE_OPENGL)
	config.supports_gl_core_version = 42;
#elif defined(PIGLIT_USE_OPENGL_ES3)
	config.supports_gl_es_version = 31;
#endif

	config.window_width = 400;
	config.window_height = 400;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	enum piglit_result result = PIGLIT_PASS;

	/* The following fragment programs render the opaque red color
	 * only when dispatched with a gl_SampleMaskIn empty. It
	 * should therefore expose the effect of the inner_coverage
	 * (i.e. no color with this qualifier and a triangle outline
	 * without it). */
	GLuint inner_prog = piglit_build_simple_program(
#if defined(PIGLIT_USE_OPENGL)
		"#version 420\n"
#elif defined(PIGLIT_USE_OPENGL_ES3)
		"#version 310 es\n"
#endif
		"in vec4 piglit_vertex;\n"
		"void main()\n"
		"{\n"
		"  gl_Position = piglit_vertex;\n"
		"}\n",
#if defined(PIGLIT_USE_OPENGL)
		"#version 420\n"
		"#extension GL_INTEL_conservative_rasterization: enable\n"
#elif defined(PIGLIT_USE_OPENGL_ES3)
		"#version 310 es\n"
		"#extension GL_OES_sample_variables: enable\n"
		"#extension GL_INTEL_conservative_rasterization: enable\n"
		"precision highp float;\n"
#endif
		"layout(inner_coverage) in;\n"
		"out vec4 color;\n"
		"void main()\n"
		"{\n"
		"  float one = 1.0 - float(gl_SampleMaskIn[0]);\n"
		"  color = vec4(one, 0.0, 0.0, one);\n"
		"}\n");
	if (!inner_prog)
		piglit_report_result(PIGLIT_FAIL);

	GLuint conservative_prog = piglit_build_simple_program(
#if defined(PIGLIT_USE_OPENGL)
		"#version 420\n"
#elif defined(PIGLIT_USE_OPENGL_ES3)
		"#version 310 es\n"
#endif
		"in vec4 piglit_vertex;\n"
		"void main()\n"
		"{\n"
		"  gl_Position = piglit_vertex;\n"
		"}\n",
#if defined(PIGLIT_USE_OPENGL)
		"#version 420\n"
		"#extension GL_INTEL_conservative_rasterization: enable\n"
#elif defined(PIGLIT_USE_OPENGL_ES3)
		"#version 310 es\n"
		"#extension GL_OES_sample_variables: enable\n"
		"#extension GL_INTEL_conservative_rasterization: enable\n"
		"precision highp float;\n"
#endif
		"out vec4 color;\n"
		"void main()\n"
		"{\n"
		"  float one = 1.0 - float(gl_SampleMaskIn[0]);\n"
		"  color = vec4(one, 0.0, 0.0, one);\n"
		"}\n");
	if (!conservative_prog)
		piglit_report_result(PIGLIT_FAIL);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
	glViewport(0, 0, piglit_width, piglit_height);

	glClearColor(0.0, 0.0, 0.0, 0.0);

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	GLuint vbo;
	float delta = 1.01 / piglit_width;
	float vertices[3][2] = {
		{ -0.5, -1 + delta, },
		{ 0, 0.8, },
		{ 0.5, -1 + delta, },
	};
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), NULL);
	glEnableVertexAttribArray(0);

	glEnable(GL_CONSERVATIVE_RASTERIZATION_INTEL);

	glUseProgram(conservative_prog);
	glClear(GL_COLOR_BUFFER_BIT);
	glDrawArrays(GL_TRIANGLES, 0, 3);

	if(!piglit_check_gl_error(GL_NO_ERROR))
		return PIGLIT_FAIL;

	piglit_present_results();

	const float conservative_expected[] = { 0.0, 0.0, 0.0, 0.0 };
	if (!piglit_probe_pixel_rgba(piglit_width / 2, 0, conservative_expected))
		result = PIGLIT_FAIL;

	glUseProgram(inner_prog);
	glClear(GL_COLOR_BUFFER_BIT);
	glDrawArrays(GL_TRIANGLES, 0, 3);

	if(!piglit_check_gl_error(GL_NO_ERROR))
		result = PIGLIT_FAIL;

	piglit_present_results();

	const float inner_expected[] = { 1.0, 0.0, 0.0, 1.0 };
	if (!piglit_probe_pixel_rgba(piglit_width / 2, 0, inner_expected))
		result = PIGLIT_FAIL;

	return result;
}

void piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_INTEL_conservative_rasterization");
}
