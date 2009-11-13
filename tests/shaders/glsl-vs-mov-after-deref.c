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

/** @file glsl-vs-mov-after-deref.c
 *
 * Tests moving of a temporary array dereference result.  Catches a
 * regression introduced in the Mesa optimizer.
 */

#include "piglit-util.h"

int piglit_width = 100, piglit_height = 100;
int piglit_window_mode = GLUT_RGB | GLUT_DOUBLE;

static GLint prog;
static int index_location;

enum piglit_result
piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	static const float green[] = {0.0, 1.0, 0.0, 0.0};

	glClearColor(0.5, 0.5, 0.5, 0.5);
	glClear(GL_COLOR_BUFFER_BIT);

	glUniform1i(index_location, 3);
	piglit_draw_rect(10, 10, 10, 10);

	pass &= piglit_probe_pixel_rgb(15, 15, green);

	glutSwapBuffers();

	return pass ? PIGLIT_SUCCESS : PIGLIT_FAILURE;
}

void piglit_init(int argc, char **argv)
{
	GLint vs, fs;

	if (!GLEW_VERSION_2_0) {
		printf("Requires OpenGL 2.0\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	vs = piglit_compile_shader(GL_VERTEX_SHADER,
				   SOURCE_DIR "tests/shaders/glsl-vs-mov-after-deref.vert");
	fs = piglit_compile_shader(GL_FRAGMENT_SHADER,
				   SOURCE_DIR "tests/shaders/glsl-vs-mov-after-deref.frag");

	prog = piglit_link_simple_program(vs, fs);

	index_location = glGetUniformLocation(prog, "index");

	glUseProgram(prog);
}
