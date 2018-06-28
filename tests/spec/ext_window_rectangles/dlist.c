/*
 * Copyright (C) 2016 Ilia Mirkin
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
 * Test that glWindowRectanglesEXT works inside of a call list. See
 * draw.c for testing technique comments.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 30;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

GLint prog, color;
GLuint fb;

enum piglit_result
piglit_display(void)
{
	static const float blue[4] = {0, 0, 1, 1};
	static const float green[4] = {0, 1, 0, 1};
	static const float red[4] = {1, 0, 0, 1};
	static const int rect[4] = {10, 10, 10, 10};

	GLuint list;
	bool passa = true, passb = true;

	glBindFramebuffer(GL_FRAMEBUFFER, fb);
	glUseProgram(prog);

	glViewport(0, 0, 20, 20);

	glClearColor(0, 0, 1, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	list = glGenLists(2);

	/* Try a single rect in exclusive mode */
	glNewList(list, GL_COMPILE_AND_EXECUTE);
	glWindowRectanglesEXT(GL_EXCLUSIVE_EXT, 1, rect);
	glUniform4fv(color, 1, green);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glEndList();

	if (!piglit_probe_rect_rgba(0, 0, 10, 10, green)) {
		printf("  FAIL: green color not filled in \n");
		passa = false;
	}
	if (!piglit_probe_rect_rgba(10, 10, 10, 10, blue)) {
		printf("  FAIL: green color fills in excluded area\n");
		passa = false;
	}

	/* And now in inclusive mode */
	glNewList(list + 1, GL_COMPILE_AND_EXECUTE);
	glWindowRectanglesEXT(GL_INCLUSIVE_EXT, 1, rect);
	glUniform4fv(color, 1, red);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glEndList();

	if (!piglit_probe_rect_rgba(0, 0, 10, 10, green)) {
		printf("  FAIL: green color overwritten\n");
		passa = false;
	}
	if (!piglit_probe_rect_rgba(10, 10, 10, 10, red)) {
		printf("  FAIL: red color not written to included area\n");
		passa = false;
	}

	piglit_report_subtest_result(passa ? PIGLIT_PASS : PIGLIT_FAIL,
				     "compile and execute");
	glClear(GL_COLOR_BUFFER_BIT);

	glCallList(list);
	if (!piglit_probe_rect_rgba(0, 0, 10, 10, green)) {
		printf("  FAIL: green color not filled in \n");
		passb = false;
	}
	if (!piglit_probe_rect_rgba(10, 10, 10, 10, blue)) {
		printf("  FAIL: green color fills in excluded area\n");
		passb = false;
	}

	glCallList(list + 1);
	if (!piglit_probe_rect_rgba(0, 0, 10, 10, green)) {
		printf("  FAIL: green color overwritten\n");
		passa = false;
	}
	if (!piglit_probe_rect_rgba(10, 10, 10, 10, red)) {
		printf("  FAIL: red color not written to included area\n");
		passa = false;
	}

	piglit_report_subtest_result(passb ? PIGLIT_PASS : PIGLIT_FAIL,
				     "call");

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
	glBlitFramebuffer(0, 0, 20, 20,
			  0, 0, piglit_width, piglit_height,
			  GL_COLOR_BUFFER_BIT, GL_NEAREST);

	piglit_present_results();

	return (passa && passb) ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	const char * subtests[] = { "compile and execute", "call", NULL };
	piglit_register_subtests(subtests);

	static const float verts[4][4] = {
		/* x   y   z   w */
		{ -1, -1, 1.0, 1 },
		{  1, -1, 1.0, 1 },
		{ -1,  1, 0.1, 1 },
		{  1,  1, 0.1, 1 }
	};

	GLuint bo, rb;

	piglit_require_extension("GL_EXT_window_rectangles");

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

	glGenFramebuffers(1, &fb);
	glBindFramebuffer(GL_FRAMEBUFFER, fb);
	glGenRenderbuffers(1, &rb);
	glBindRenderbuffer(GL_RENDERBUFFER, rb);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGB, 20, 20);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
				  GL_RENDERBUFFER, rb);
}
