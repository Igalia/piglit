/*
 * Copyright (c) 2014 Intel Corporation
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
 * @file vao-broken-attrib.c
 *
 * Tests that in the core profile, drawing with a vertex attrib enabled but not
 * sourced from a buffer does not crash the driver. Exercises a bug in i965.
 *
 * The GL 4.5 spec says:
 *	"If any enabled array’s buffer binding is zero when DrawArrays or
 *	one of the other drawing commands defined in section 10.4 is called,
 *	the result is undefined.""
 *
 * Note that this crash is reasonable (but unfortunate) in versions of GL which
 * permit vertex attributes to be sourced from client memory.
 */

#include "piglit-util-gl.h"
#include "piglit-matrix.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_core_version = 31;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;
PIGLIT_GL_TEST_CONFIG_END


enum piglit_result
piglit_display(void)
{
	/* unreached */
	return PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	GLuint vao;
	GLuint prog = piglit_build_simple_program(
			"#version 130\n in vec4 x; void main() { gl_Position = x; }",
			"#version 130\n void main() { gl_FragColor = vec4(0); }");
	glUseProgram(prog);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	/* Enable this array but don't source it from anywhere. */
	glEnableVertexAttribArray(0);

	/* Result of this draw is undefined, but shouldn't crash! */
	glDrawArrays(GL_TRIANGLES, 0, 3);

	piglit_report_result(PIGLIT_PASS);
}
