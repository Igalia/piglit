/* Copyright © 2011-2012 Intel Corporation
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

/** @file minmax.c
 *
 * Test for the minimum maximum value in the GL_ARB_texture_buffer_object spec.
 */

#include "piglit-util.h"

int piglit_width = 32;
int piglit_height = 32;
int piglit_window_mode = GLUT_DOUBLE | GLUT_RGB | GLUT_ALPHA;

enum piglit_result
piglit_display(void)
{
	/* UNREACHED */
	return PIGLIT_FAIL;
}

static bool pass = true;

static void
min_test_i(GLenum token, GLint min, const char *name)
{
	GLint val = 0;

	glGetIntegerv(token, &val);

	if (val < min) {
		fprintf(stderr, "%-50s %8d %8d (ERROR)\n",
			name, min, val);
		pass = false;
	} else {
		printf("%-50s %8d %8d\n", name, min, val);
	}
}

#define MIN_INTEGER_TEST(token, min) min_test_i(token, min, #token)

void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_ARB_texture_buffer_object");

	printf("%-50s %8s %8s\n", "token", "minimum", "value");

	MIN_INTEGER_TEST(GL_MAX_TEXTURE_BUFFER_SIZE, 65536);

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
