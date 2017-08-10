/*
 * Copyright © 2016 Intel Corporation
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
 */

/**
 * \file max-vertex-attrib.c
 *
 * Test setting vertex attrib value of GL_MAX_VERTEX_ATTRIBS attrib
 * for the New Procedures and Functions defined by the
 * GL_ARB_vertex_attrib_64bit extension.
 *
 * Note that this test is based on glsl-max-vertex-attrib.c which is
 * already testing the already previously existing vertex attrib
 * methods.
 *
 * Queries the value for GL_MAX_VERTEX_ATTRIBS and uses that as index
 * to set a value. GL specification states that GL_INVALID_VALUE
 * should occur if index >= GL_MAX_VERTEX_ATTRIBS.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 32;
	config.supports_gl_core_version = 32;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;
	config.khr_no_error_support = PIGLIT_HAS_ERRORS;
PIGLIT_GL_TEST_CONFIG_END

static int test = 0;

#define CHECK_GL_INVALID_VALUE \
	if (glGetError() != GL_INVALID_VALUE) return PIGLIT_FAIL; \
	else printf("max-vertex-attrib test %d passed\n", ++test);

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

static GLboolean
run_test(void)
{
	GLdouble doublev[] = { 1.0, 1.0, 1.0, 1.0 };
	GLdouble quad[] = { -1.0, 1.0, 1.0, 1.0, -1.0, -1.0, 1.0, -1.0 };

	int maxAttribCount;

	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxAttribCount);

	glVertexAttribL1d(maxAttribCount, doublev[0]);
	CHECK_GL_INVALID_VALUE;

	glVertexAttribL2d(maxAttribCount, doublev[0], doublev[1]);
	CHECK_GL_INVALID_VALUE;

	glVertexAttribL3d(maxAttribCount, doublev[0], doublev[1], doublev[2]);
	CHECK_GL_INVALID_VALUE;

	glVertexAttribL4d(maxAttribCount, doublev[0], doublev[1], doublev[2],
			 doublev[3]);
	CHECK_GL_INVALID_VALUE;

	glVertexAttribL1dv(maxAttribCount, doublev);
	CHECK_GL_INVALID_VALUE;

	glVertexAttribL2dv(maxAttribCount, doublev);
	CHECK_GL_INVALID_VALUE;

	glVertexAttribL3dv(maxAttribCount, doublev);
	CHECK_GL_INVALID_VALUE;

	glVertexAttribL4dv(maxAttribCount, doublev);
	CHECK_GL_INVALID_VALUE;

	glVertexAttribLPointer(maxAttribCount, 2, GL_DOUBLE, 0, quad);
	CHECK_GL_INVALID_VALUE;

	glGetVertexAttribLdv(maxAttribCount, GL_CURRENT_VERTEX_ATTRIB, doublev);
	CHECK_GL_INVALID_VALUE;

	if (piglit_is_extension_supported("GL_EXT_direct_state_access")) {
		unsigned int vaobj;

		glGenVertexArrays(1, &vaobj);
		glBindVertexArray(vaobj);

		glVertexArrayVertexAttribLOffsetEXT(vaobj, 0, maxAttribCount, 3,
						    GL_DOUBLE, 0, 0);
		glDeleteVertexArrays(1, &vaobj);
		CHECK_GL_INVALID_VALUE;
	}

	return PIGLIT_PASS;
}

void piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_ARB_vertex_attrib_64bit");

	piglit_report_result(run_test());
}
