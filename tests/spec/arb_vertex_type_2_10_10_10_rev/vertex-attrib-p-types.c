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

/**
 * Test that VertexAttribP*() must use types INT_2_10_10_10_REV or
 * UNSIGNED_INT_2_10_10_10_REV
 *
 * Section 2.7(Vertex Specification) of GL3.3 core spec says:
 * "The type parameter must be INT_2_10_10_10_REV or
 *  UNSIGNED_INT_2_10_10_10_REV, specifying signed or unsigned data
 *  respectively."
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_core_version = 31;
	config.supports_gl_compat_version = 20;

PIGLIT_GL_TEST_CONFIG_END

void
piglit_init(int argc, char **argv)
{
	bool pass = true;
	int i;
	GLuint x = 21;
	GLenum valid_types[] = {
		GL_UNSIGNED_INT_2_10_10_10_REV,
		GL_INT_2_10_10_10_REV
	};
	GLenum invalid_types[] = {
		GL_BYTE, GL_SHORT, GL_INT, GL_FLOAT, GL_HALF_FLOAT, GL_DOUBLE,
		GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT
	};

	if(piglit_get_gl_version() < 33)
		piglit_require_extension("GL_ARB_vertex_type_2_10_10_10_rev");

	for (i = 0; i < ARRAY_SIZE(valid_types); i++) {
		glVertexAttribP1ui(0, valid_types[i], true, x);
		pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

		glVertexAttribP1uiv(0, valid_types[i], true, &x);
		pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
	}

	for (i = 0; i < ARRAY_SIZE(invalid_types); i++) {
		glVertexAttribP1ui(0, invalid_types[i], true, x);
		pass = piglit_check_gl_error(GL_INVALID_ENUM) && pass;

		glVertexAttribP1uiv(0, invalid_types[i], true, &x);
		pass = piglit_check_gl_error(GL_INVALID_ENUM) && pass;
	}

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result
piglit_display(void)
{
	/* UNREACHED */
	return PIGLIT_FAIL;
}
