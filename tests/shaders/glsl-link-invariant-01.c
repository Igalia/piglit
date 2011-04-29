/*
 * Copyright © 2010 Intel Corporation
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
 * \file glsl-link-invariant-01.c
 * C file derived from glsl-link-initializer-01.c
 *
 * Negative test for inconsistent invariant qualifier usage between vertex 
 * shaders.
 *
 * Both vertex shaders involved in this test have a varying float variable
 * with the same name. But the first shader declares it with the invariant
 * qualifier while the second not. 
 * The test verifies that linking the 2 shaders together results in an error, 
 * according to GLSL 1.20 section 4.3.6:
 * The type and presence of the invariant qualifiers of varying variables with
 * the same name declared in linked vertex and fragments shaders must match,
 * otherwise the link command will fail.
 *
 * \author Gordon Jin
 */

#include "piglit-util.h"

int piglit_width = 100, piglit_height = 100;
int piglit_window_mode = GLUT_RGB | GLUT_DOUBLE;

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

/**
 * Don't use piglit_link_check_status because it will log a message to stderr
 * when the link fails.  Since this test wants the link to fail, logging an
 * error message will cause the test to be listed as "warn" instead of "pass".
 */
GLboolean
link_check_status(GLint prog)
{
	GLint ok;

	glGetProgramiv(prog, GL_LINK_STATUS, &ok);
	if (!ok) {
		GLchar *info;
		GLint size;

		glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &size);
		info = malloc(size);

		glGetProgramInfoLog(prog, size, NULL, info);
		printf("Failed to link: %s\n", info);

		free(info);
	}

	return ok;
}

void piglit_init(int argc, char **argv)
{
	GLint vert[2];
	GLint prog;
	GLboolean ok;

	if (!GLEW_VERSION_2_0) {
		printf("Requires OpenGL 2.0\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	vert[0] =
		piglit_compile_shader(GL_VERTEX_SHADER,
				      "shaders/glsl-link-invariant-01a.vert");
	vert[1] =
		piglit_compile_shader(GL_VERTEX_SHADER,
				      "shaders/glsl-link-invariant-01b.vert");
	prog = glCreateProgram();
	glAttachShader(prog, vert[0]);
	glAttachShader(prog, vert[1]);
	glLinkProgram(prog);

	ok = link_check_status(prog);
	if (ok)
		fprintf(stderr,
			"Program should have failed linking, but "
			"it was (incorrectly) successful.\n");

	piglit_report_result((!ok) ? PIGLIT_PASS : PIGLIT_FAIL);
}

