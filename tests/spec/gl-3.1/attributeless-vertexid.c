/* Copyright © 2014 Intel Corporation
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

/** @file attributeless-vertexid.c
 *
 * Test that rendering with no vertex attributes (but only using gl_VertexID)
 * works in the core profile.
 */

#include "piglit-util-gl-common.h"
#include "minmax-test.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_core_version = 31;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

const float red[] = { 1, 0, 0, 1 };

enum piglit_result
piglit_display(void)
{
	bool pass;

	glViewport(0, 0, piglit_width, piglit_height);
	glClearColor(0.2, 0.2, 0.2, 0.2);
	glClear(GL_COLOR_BUFFER_BIT);

	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	pass = piglit_probe_rect_rgba(0, 0, piglit_width, piglit_height, red);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	GLuint prog = piglit_build_simple_program(
		"#version 140\n"
		"const vec2 verts[4] = vec2[](\n"
		"	vec2(-1, 1),\n"
		"	vec2(-1,-1),\n"
		"	vec2( 1,-1),\n"
		"	vec2( 1, 1)\n"
		");\n"
		"void main() {\n"
		"	gl_Position = vec4(verts[gl_VertexID], 0, 1);\n"
		"}\n",

		"#version 140\n"
		"void main() {\n"
		"	gl_FragColor = vec4(1,0,0,1);\n"
		"}\n");

	GLuint vao;

	glUseProgram(prog);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
}
