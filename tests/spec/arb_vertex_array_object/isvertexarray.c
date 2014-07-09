/*
 * Copyright © 2012 Intel Corporation
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

#include "piglit-util-gl.h"

/**
 * @file isvertexarray.c
 *
 * Tests basic API support for glIsVertexArray().
 *
 * From the ARB_vertex_array_object spec:
 *
 *     "The command
 *
 *         void GenVertexArrays(sizei n, uint *arrays);
 *
 *      returns <n> previous unused vertex array object names in <arrays>. These
 *      names are marked as used, for the purposes of GenVertexArrays only, but
 *      they acquire array state only when they are first bound.
 *
 *      [...]
 *
 *      A vertex array object is created by binding a name returned by
 *      GenVertexArrays with the command
 *
 *         void BindVertexArray(uint array);"
 *
 * The APPLE_vertex_array_object spec contains similar wording.
 */

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	/* UNREACHED */
	return PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	GLuint id;
	GLboolean apple = GL_FALSE;

	if (argc == 2 && strcmp(argv[1], "apple") == 0) {
		printf("apple\n");
		apple = GL_TRUE;
	}

	piglit_require_gl_version(15);
	if (apple)
		piglit_require_extension("GL_APPLE_vertex_array_object");
	else
		piglit_require_extension("GL_ARB_vertex_array_object");

	glGenVertexArrays(1, &id);

	if (glIsVertexArray(id)) {
		fprintf(stderr, "id recognized incorrectly as a vertex array object.\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	if (apple)
		glBindVertexArrayAPPLE(id);
	else
		glBindVertexArray(id);

	if (!glIsVertexArray(id)) {
		fprintf(stderr, "id not recognized correctly as a vertex array object.\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	piglit_report_result(PIGLIT_PASS);
}
