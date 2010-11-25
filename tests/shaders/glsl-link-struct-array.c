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

#include "piglit-util.h"

int piglit_width = 100, piglit_height = 100;
int piglit_window_mode = GLUT_RGB | GLUT_DOUBLE;

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAILURE;
}

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
	GLint frag;
	GLint prog;
	GLboolean ok;

	if (!GLEW_VERSION_2_0) {
		printf("Requires OpenGL 2.0\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	frag =
		piglit_compile_shader(GL_FRAGMENT_SHADER,
				      "shaders/glsl-link-struct-array.frag");
	prog = glCreateProgram();
	glAttachShader(prog, frag);
	glLinkProgram(prog);

	ok = link_check_status(prog);
	piglit_report_result((ok) ? PIGLIT_SUCCESS : PIGLIT_FAILURE);
}

