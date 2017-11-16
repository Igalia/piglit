/*
 * Copyright © 2017 Intel Corporation
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
 * @file
 * Tests GL_EXT_disjoint_timer_query extension. Test does not to cover
 * the whole API as that is tested throughly by existing query tests for
 * desktop GL. Main objective is to test that timer queries work on OpenGL
 * ES 2.0 and we can get GL_GPU_DISJOINT_EXT value from the driver.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_es_version = 20;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA;
PIGLIT_GL_TEST_CONFIG_END

static const char vs_source[] =
	"attribute vec2 piglit_vertex;\n"
	"\n"
	"void main()\n"
	"{\n"
	"	gl_Position = vec4(piglit_vertex, 0.0, 1.0);\n"
	"}\n";

static const char fs_source[] =
	"void main()\n"
	"{\n"
	"	gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);\n"
	"}\n";

enum piglit_result
piglit_display(void)
{
	GLuint query;
	GLint current, disjoint;
	GLuint64 time = 0;

	glGenQueriesEXT(1, &query);

	/* Clear disjoint error state. */
	glGetIntegerv(GL_GPU_DISJOINT_EXT, &disjoint);

	glBeginQueryEXT(GL_TIME_ELAPSED_EXT, query);

	/* "The error INVALID_OPERATION is generated if QueryCounterEXT is
	 * called on a query object that is already in use inside a
	 * BeginQueryEXT/EndQueryEXT."
	 */
	glQueryCounterEXT(query, GL_TIMESTAMP_EXT);

	if (!piglit_check_gl_error(GL_INVALID_OPERATION))
		piglit_report_result(PIGLIT_FAIL);

	if (!glIsQueryEXT(query))
		piglit_report_result(PIGLIT_FAIL);

	glGetQueryivEXT(GL_TIME_ELAPSED_EXT, GL_CURRENT_QUERY_EXT,
			&current);

	if (current != query)
		piglit_report_result(PIGLIT_FAIL);

	GLint prog = piglit_build_simple_program(vs_source, fs_source);
	glUseProgram(prog);

	piglit_draw_rect(-1, -1, 2, 2);

	glDeleteProgram(prog);

	glEndQueryEXT(GL_TIME_ELAPSED_EXT);

	glGetQueryObjectui64vEXT(query, GL_QUERY_RESULT_EXT, &time);

	/* Check if if disjoint operation occured. */
	glGetIntegerv(GL_GPU_DISJOINT_EXT, &disjoint);

	/* If no disjoint operation, then we should have a
	 * 'sensible value' in time, expecting more than 0.
	 */
	if (disjoint == 0 && time <= 0)
		piglit_report_result(PIGLIT_FAIL);

	glDeleteQueriesEXT(1, &query);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	return PIGLIT_PASS;
}

void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_EXT_disjoint_timer_query");
}
