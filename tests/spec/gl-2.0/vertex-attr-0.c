/*
 * Copyright (c) 2017 VMware, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and/or associated documentation files (the
 * "Materials"), to deal in the Materials without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Materials, and to
 * permit persons to whom the Materials are furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Materials.
 *
 * THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.
 *
 */

/**
 * Test glVertexAttrib(index=0).
 * Brian Paul
 * 22 Aug 2017
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 20;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;
PIGLIT_GL_TEST_CONFIG_END

static const char *vs =
	"attribute vec4 color_in; \n"
	"varying vec4 color; \n"
	"void main() { \n"
	"  gl_Position = gl_Vertex; \n"
	"  color = color_in; \n"
	"}\n";

static const char *fs =
	"varying vec4 color; \n"
	"void main() \n"
	"{ \n"
	"   gl_FragColor = color;\n"
	"} \n";

static GLint prog;


enum piglit_result
piglit_display(void)
{
	static const float color[4][4] = {
		{1, 0, 0, 1},
		{0, 1, 0, 1},
		{0, 0, 1, 1},
		{1, 1, 1, 1},
	};
	static const float verts[4][2] = {
		{-1, -1},
		{ 1, -1},
		{ 1,  1},
		{-1,  1},
	};
	GLint i, color_in;
	bool pass = true;

	glViewport(0, 0, piglit_width, piglit_height);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(prog);

	color_in = glGetAttribLocation(prog, "color_in");
	printf("color_in at locagion %d\n", color_in);

	/* draw quad */
	glBegin(GL_TRIANGLE_FAN);
	for (i = 0; i < 4; i++) {
		/* update generic attribute */
		glVertexAttrib4fv(color_in, color[i]);
		/* emit vertes */
		glVertexAttrib2fv(0, verts[i]);
	}
	glEnd();

	/* probe four corners */
	pass &= piglit_probe_pixel_rgba(0, 0, color[0]);
	pass &= piglit_probe_pixel_rgba(piglit_width - 1, 0, color[1]);
	pass &= piglit_probe_pixel_rgba(piglit_width - 1, piglit_height - 1,
					color[2]);
	pass &= piglit_probe_pixel_rgba(0, piglit_height -1, color[3]);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


void
piglit_init(int argc, char **argv)
{
	piglit_require_gl_version(20);

	prog = piglit_build_simple_program(vs, fs);
	if (!prog) {
		printf("Failed to compile/link program\n");
		piglit_report_result(PIGLIT_FAIL);
	}
}
