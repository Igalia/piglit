/*
 * Copyright (c) 2012 The Khronos Group Inc.
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
 * Adapted for Piglit and OpenGL by:
 *   Ilia Mirkin
 */

/**
 * Test glVertexAtribNfv changes and redraws without changing any other state
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 20;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;
PIGLIT_GL_TEST_CONFIG_END

static const char *vs =
	"attribute vec2 vertex; \n"
	"attribute vec4 attr; \n"
	"varying vec4 color; \n"
	"void main() { \n"
	"  gl_Position = vec4(vertex, 0, 1); \n"
	"  color = attr; \n"
	"}\n";

static const char *fs =
	"varying vec4 color; \n"
	"void main() { gl_FragColor = color; } \n";

static GLint prog, attr;

enum piglit_result
piglit_display(void)
{
	float color[4][4] = {
		{1, 0, 0, 1},
		{0, 1, 0, 1},
		{0, 0, 1, 1},
		{1, 1, 1, 1},
	};
	bool pass = true;

	glViewport(0, 0, piglit_width, piglit_height);
	glClearColor(0.5, 0.5, 0.5, 0.5);
	glClear(GL_COLOR_BUFFER_BIT);

	glVertexAttrib1fv(attr, color[0]);
	glViewport(0, 0, piglit_width/2, piglit_height/2);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glVertexAttrib2fv(attr, color[1]);
	glViewport(0, piglit_height/2, piglit_width/2, piglit_height/2);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glVertexAttrib3fv(attr, color[2]);
	glViewport(piglit_width/2, 0, piglit_width/2, piglit_height/2);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glVertexAttrib4fv(attr, color[3]);
	glViewport(piglit_width/2, piglit_height/2,
		   piglit_width/2, piglit_height/2);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	pass &= piglit_probe_rect_rgba(0, 0,
				       piglit_width / 2, piglit_height / 2,
				       color[0]);
	pass &= piglit_probe_rect_rgba(0, piglit_height / 2,
				       piglit_width / 2, piglit_height / 2,
				       color[1]);
	pass &= piglit_probe_rect_rgba(piglit_width / 2, 0,
				       piglit_width / 2, piglit_height / 2,
				       color[2]);
	pass &= piglit_probe_rect_rgba(piglit_width / 2, piglit_height / 2,
				       piglit_width / 2, piglit_height / 2,
				       color[3]);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

static const float verts[4][2] = {
	{-1, -1},
	{ 1, -1},
	{-1,  1},
	{ 1,  1},
	};

void
piglit_init(int argc, char **argv)
{
	GLuint buf;
	GLint vertex;

	piglit_require_gl_version(20);

	prog = piglit_build_simple_program(vs, fs);
	if (!prog) {
		printf("Failed to compile/link program\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	glUseProgram(prog);

	attr = glGetAttribLocation(prog, "attr");
	vertex = glGetAttribLocation(prog, "vertex");

	glGenBuffers(1, &buf);
	glBindBuffer(GL_ARRAY_BUFFER, buf);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
	glVertexAttribPointer(vertex,
			      2, GL_FLOAT, GL_FALSE, 0, 0);

	glEnableVertexAttribArray(vertex);
}
