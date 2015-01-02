/*
 * Copyright (C) 2015 Ilia Mirkin
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
 * \file dlist.c
 *
 * Test that glPolygonOffsetClampEXT works inside of a call list. See
 * draw.c for testing technique comments.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 21;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DEPTH | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

GLint prog, color;

enum piglit_result
piglit_display(void)
{
	static const float blue[4] = {0, 0, 1, 1};
	static const float red[4] = {1, 0, 0, 1};
	static const float green[4] = {0, 1, 0, 1};

	GLuint list;
	bool passa = true, passb = true;

	glUseProgram(prog);

	glViewport(0, 0, piglit_width, piglit_height);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_POLYGON_OFFSET_FILL);

	glClearColor(0, 0, 1, 1);
	glClearDepth(0.5);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	list = glGenLists(2);

	/* Draw red rectangle that slopes between 1 and 0.1. Use a
	 * polygon offset with a high factor but small clamp
	 */
	glNewList(list, GL_COMPILE_AND_EXECUTE);
	glPolygonOffsetClampEXT(-1000, 0, -0.05);
	glUniform4fv(color, 1, red);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glEndList();

	if (!piglit_probe_rect_rgba(0, 0, piglit_width, piglit_height, blue)) {
		printf("  FAIL: red rect peeks over blue rect\n");
		passa = false;
	}

	/* And now set the clamp such that all parts of the polygon
	 * can pass the depth test.
	 */
	glNewList(list + 1, GL_COMPILE_AND_EXECUTE);
	glPolygonOffsetClampEXT(-1000, 0, -0.51);
	glUniform4fv(color, 1, green);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glEndList();

	if (!piglit_probe_rect_rgba(0, 0, piglit_width, piglit_height, green)) {
		printf("  FAIL: green rect does not cover blue rect\n");
		passa = false;
	}

	piglit_report_subtest_result(passa ? PIGLIT_PASS : PIGLIT_FAIL,
				     "compile and execute");
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glCallList(list);
	if (!piglit_probe_rect_rgba(0, 0, piglit_width, piglit_height, blue)) {
		printf("  FAIL: red rect peeks over blue rect\n");
		passb = false;
	}

	glCallList(list + 1);
	if (!piglit_probe_rect_rgba(0, 0, piglit_width, piglit_height, green)) {
		printf("  FAIL: green rect does not cover blue rect\n");
		passb = false;
	}

	piglit_report_subtest_result(passb ? PIGLIT_PASS : PIGLIT_FAIL,
				     "call");

	piglit_present_results();

	return (passa && passb) ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	static const float verts[4][4] = {
		/* x   y   z   w */
		{ -1, -1, 1.0, 1 },
		{  1, -1, 1.0, 1 },
		{ -1,  1, 0.1, 1 },
		{  1,  1, 0.1, 1 }
	};

	GLuint bo;

	piglit_require_extension("GL_EXT_polygon_offset_clamp");

	prog = piglit_build_simple_program(
			"#version 120\n"
			"void main() { gl_Position = gl_Vertex; }\n",

			"#version 120\n"
			"uniform vec4 color;\n"
			"void main() { gl_FragColor = color; }\n");
	color = glGetUniformLocation(prog, "color");

	glEnableVertexAttribArray(0);
	glGenBuffers(1, &bo);
	glBindBuffer(GL_ARRAY_BUFFER, bo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid const *)0);
}
