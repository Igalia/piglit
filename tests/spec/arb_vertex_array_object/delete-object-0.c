/*
 * Copyright © 2018 Intel Corporation
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
 * @file delete-object-0.c
 *
 * The OpenGL 4.6 Core Profile specification (dated May 14, 2018) says:
 *
 *    Unused names in arrays that have been marked as used for the purposes of
 *    GenVertexArrays are marked as unused again.  Unused names in arrays are
 *    silently ignored, as is the value zero.
 *
 * This test verifies the no errors are generated for 0 or for names from
 * glGenVertexArrays that have not been bound.
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
	GLuint id[3] = { 0, 0, 0 };
	bool pass = true;

	piglit_require_gl_version(15);
	piglit_require_extension("GL_ARB_vertex_array_object");

	/* After this call to Gen, the id array will contain { 0, id1, id2 }. */
	glGenVertexArrays(2, &id[1]);
	glBindVertexArray(id[1]);
	glBindVertexArray(0);

	glDeleteVertexArrays(3, id);

	pass = piglit_check_gl_error(GL_NO_ERROR);

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
