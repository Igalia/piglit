/*
 * Copyright © 2009 Intel Corporation
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
 *
 * Authors:
 *    Eric Anholt <eric@anholt.net>
 *
 */

/** @file glsl-getactiveuniform-count.c
 *
 * Tests that glGetActiveUniform() has the uniform's string length correctly
 * reflected in GL_ACTIVE_UNIFORM_MAX_LENGTH and the *length outvalue.
 */

#include "piglit-util.h"

int piglit_width = 100, piglit_height = 100;
int piglit_window_mode = GLUT_RGB | GLUT_DOUBLE;

static GLint prog;

enum piglit_result
piglit_display(void)
{
	/* unreached */
	return PIGLIT_FAILURE;
}

void
piglit_init(int argc, char **argv)
{
	GLboolean pass = GL_TRUE;
	GLint vs, fs, len, ret_len;
	GLenum type;
	char *name;
	GLsizei size;

	if (!GLEW_VERSION_2_0) {
		printf("Requires OpenGL 2.0\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	vs = piglit_compile_shader(GL_VERTEX_SHADER,
				   "shaders/glsl-getactiveuniform-length.vert");
	fs = piglit_compile_shader(GL_FRAGMENT_SHADER,
				   "shaders/glsl-color.frag");

	prog = piglit_link_simple_program(vs, fs);

	/* From page 80 (page 88 of the PDF) of the OpenGL 2.1 specification:
	 *
	 *     The actual number of characters written into name,
	 *     excluding the null terminator, is returned in length.
	 *
	 * This describes the length outvalue of glGetActiveUniform().
	 * GL_ACTIVE_UNIFORM_MAX_LENGTH on the following page just
	 * says:
	 *
	 *     The length of the longest uniform name in program is
	 *     given by ACTIVE UNIFORM MAX LENGTH, which can be
	 *     queried with GetProgramiv (see section 6.1.14).
	 */

	glGetProgramiv(prog, GL_ACTIVE_UNIFORM_MAX_LENGTH, &len);
	if (len != strlen("color")) {
		printf("Unexpected max active uniform length "
		       "(saw %d, expected %d)\n", len, strlen("color"));
		pass = GL_FALSE;
	}

	/* From page 80 (page 88 of the PDF) of the OpenGL 2.1 specification:
	 *
	 *     The actual number of characters written into name,
	 *     excluding the null terminator, is returned in length.
	 */
	name = malloc(len + 1);
	glGetActiveUniform(prog, 0, len + 20, &ret_len, &size, &type, name);
	if (ret_len != len) {
		printf("Unexpected active uniform length "
		       "(saw %d, expected %d) for \"%s\"\n",
		       ret_len, len, name);
		pass = GL_FALSE;
	}


	if (pass)
		piglit_report_result(PIGLIT_SUCCESS);
	else
		piglit_report_result(PIGLIT_FAILURE);
}
