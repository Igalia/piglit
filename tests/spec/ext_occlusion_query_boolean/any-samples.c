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
 * Tests GL_EXT_occlusion_query_boolean extension. Test does not to cover
 * the whole API as that is tested throughly by existing arb_occlusion_query
 * and arb_occlusion_query2 tests. Main objective is to test that boolean
 * query works on OpenGL ES 2.0.
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
	GLuint query, samples;
	GLint current;

	glGenQueriesEXT(1, &query);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	glBeginQueryEXT(GL_ANY_SAMPLES_PASSED_EXT, query);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	if (!glIsQueryEXT(query))
		piglit_report_result(PIGLIT_FAIL);

	glGetQueryivEXT(GL_ANY_SAMPLES_PASSED_EXT, GL_CURRENT_QUERY_EXT,
			&current);

	if (current != query)
		piglit_report_result(PIGLIT_FAIL);

	/* 0x8864 equals to ARB_occlusion_query GL_QUERY_COUNTER_BITS_ARB */
	glGetQueryivEXT(GL_ANY_SAMPLES_PASSED_EXT, 0x8864, &current);

	/* The error INVALID_ENUM is generated if GetQueryivEXT is called where
         * <pname> is not CURRENT_QUERY_EXT.
         */
	if (!piglit_check_gl_error(GL_INVALID_ENUM))
		piglit_report_result(PIGLIT_FAIL);

	GLint prog = piglit_build_simple_program(vs_source, fs_source);
	glUseProgram(prog);

	piglit_draw_rect(-1, -1, 2, 2);

	glDeleteProgram(prog);

	glEndQueryEXT(GL_ANY_SAMPLES_PASSED_EXT);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	glGetQueryObjectuivEXT(query, GL_QUERY_RESULT_EXT, &samples);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	if (samples != 1) {
		piglit_report_result(PIGLIT_FAIL);
	}

	glDeleteQueriesEXT(1, &query);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	return PIGLIT_PASS;
}

void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_EXT_occlusion_query_boolean");
}
