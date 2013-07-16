/* Copyright © 2013 Intel Corporation
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

/** @file vertex-array-bgra.c
 *
 * Section 2.8(Vertex Arrays) From GL spec 3.2 core (GL_ARB_vertex_array_bgra):
 *
 * The error INVALID_VALUE is generated if size is specified with a value other
 * than that indicated in the table(GL 3.2 2.8), if size is BGRA and type is not
 * UNSIGNED_BYTE, or by VertexAttribPointer if size is BGRA and normalized is 
 * FALSE.
 */

#include "piglit-util-gl-common.h"

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
	bool pass = true;
	piglit_require_extension("GL_ARB_vertex_array_bgra");

	glEnableVertexAttribArray(0);

	/* Test when size == GL_BGRA && normalized == GL_TRUE;
	 * should generate GL_NO_ERROR
	 */
	glVertexAttribPointer(0, GL_BGRA, GL_UNSIGNED_BYTE, GL_TRUE, 0, 0);
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	/* Test when size == GL_BGRA && normalized == GL_FALSE;
	 * should generate GL_INVALID_VALUE
	 */
	glVertexAttribPointer(0, GL_BGRA, GL_UNSIGNED_BYTE, GL_FALSE, 0, 0);
	pass = piglit_check_gl_error(GL_INVALID_VALUE) && pass;


	/* Test when size == GL_BGRA && type != GL_UNSIGNED_BYTE;
	 * should generate GL_INVALID_VALUE
	 */
	glVertexAttribPointer(0, GL_BGRA, GL_BYTE, GL_TRUE, 0, 0);
	pass = piglit_check_gl_error(GL_INVALID_VALUE) && pass;	

	glVertexAttribPointer(0, GL_BGRA, GL_SHORT, GL_TRUE, 0, 0);
	pass = piglit_check_gl_error(GL_INVALID_VALUE) && pass;

	glVertexAttribPointer(0, GL_BGRA, GL_UNSIGNED_SHORT, GL_TRUE, 0, 0);
	pass = piglit_check_gl_error(GL_INVALID_VALUE) && pass;

	glVertexAttribPointer(0, GL_BGRA, GL_INT, GL_TRUE, 0, 0);
	pass = piglit_check_gl_error(GL_INVALID_VALUE) && pass;

	glVertexAttribPointer(0, GL_BGRA, GL_UNSIGNED_INT, GL_TRUE, 0, 0);
	pass = piglit_check_gl_error(GL_INVALID_VALUE) && pass;

	glVertexAttribPointer(0, GL_BGRA, GL_HALF_FLOAT, GL_TRUE, 0, 0);
	pass = piglit_check_gl_error(GL_INVALID_VALUE) && pass;

	glVertexAttribPointer(0, GL_BGRA, GL_FLOAT, GL_TRUE, 0, 0);
	pass = piglit_check_gl_error(GL_INVALID_VALUE) && pass;

	glVertexAttribPointer(0, GL_BGRA, GL_DOUBLE, GL_TRUE, 0, 0);
	pass = piglit_check_gl_error(GL_INVALID_VALUE) && pass;

	glVertexAttribPointer(0, GL_BGRA, GL_FIXED, GL_TRUE, 0, 0);
	pass = piglit_check_gl_error(GL_INVALID_VALUE) && pass;

	glDisableVertexAttribArray(0);
	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
