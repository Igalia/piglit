/*
 * Copyright (C) 2018 VMware, Inc.
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
 * Test glGetInteger/Float/Double/Booleanv with vertex array attributes.
 */

#include "piglit-util-gl.h"


PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 15;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;
PIGLIT_GL_TEST_CONFIG_END


static bool
test_get(GLenum pname, GLint expectedValue)
{
	GLint i;
	GLfloat f;
	GLdouble d;
	GLboolean b;
	bool pass = true;

	glGetIntegerv(pname, &i);
	glGetFloatv(pname, &f);
	glGetDoublev(pname, &d);
	glGetBooleanv(pname, &b);

	if (i != expectedValue) {
		printf("glGetIntegerv(%s) failed: expected %d, got %d\n",
		       piglit_get_gl_enum_name(pname),
		       expectedValue, i);
		pass = false;
	}

	if (f != (GLfloat) expectedValue) {
		printf("glGetFloatv(%s) failed: expected %f, got %f\n",
		       piglit_get_gl_enum_name(pname),
		       (GLfloat) expectedValue, f);
		pass = false;
	}

	if (d != (GLdouble) expectedValue) {
		printf("glGetDoublev(%s) failed: expected %f, got %f\n",
		       piglit_get_gl_enum_name(pname),
		       (GLdouble) expectedValue, f);
		pass = false;
	}

	if (b != (GLboolean) !!expectedValue) {
		printf("glGetBooleanv(%s) failed: expected %d, got %d\n",
		       piglit_get_gl_enum_name(pname),
		       !!expectedValue, b);
		pass = false;
	}

	return pass;
}


enum piglit_result
piglit_display(void)
{
	/* nothing */
	return PIGLIT_PASS;
}


void
piglit_init(int argc, char **argv)
{
	bool pass = true;
	GLubyte dummy[100];

	glVertexPointer(2, GL_FLOAT, 12, dummy);
	glNormalPointer(GL_FLOAT, 0, dummy);
	glColorPointer(4, GL_UNSIGNED_BYTE, 16, dummy);
	glSecondaryColorPointer(3, GL_SHORT, 32, dummy);
	glTexCoordPointer(3, GL_SHORT, 18, dummy);
	glEdgeFlagPointer(4, dummy);
	glIndexPointer(GL_SHORT, 10, dummy);
	glFogCoordPointer(GL_FLOAT, 8, dummy);

	pass = test_get(GL_VERTEX_ARRAY_SIZE, 2) && pass;
	pass = test_get(GL_VERTEX_ARRAY_TYPE, GL_FLOAT) && pass;
	pass = test_get(GL_VERTEX_ARRAY_STRIDE, 12) && pass;

	pass = test_get(GL_NORMAL_ARRAY_TYPE, GL_FLOAT) && pass;
	pass = test_get(GL_NORMAL_ARRAY_STRIDE, 0) && pass;

	pass = test_get(GL_COLOR_ARRAY_SIZE, 4) && pass;
	pass = test_get(GL_COLOR_ARRAY_TYPE, GL_UNSIGNED_BYTE) && pass;
	pass = test_get(GL_COLOR_ARRAY_STRIDE, 16) && pass;

	pass = test_get(GL_SECONDARY_COLOR_ARRAY_SIZE, 3) && pass;
	pass = test_get(GL_SECONDARY_COLOR_ARRAY_TYPE, GL_SHORT) && pass;
	pass = test_get(GL_SECONDARY_COLOR_ARRAY_STRIDE, 32) && pass;

	pass = test_get(GL_TEXTURE_COORD_ARRAY_SIZE, 3) && pass;
	pass = test_get(GL_TEXTURE_COORD_ARRAY_TYPE, GL_SHORT) && pass;
	pass = test_get(GL_TEXTURE_COORD_ARRAY_STRIDE, 18) && pass;

	pass = test_get(GL_EDGE_FLAG_ARRAY_STRIDE, 4) && pass;

	pass = test_get(GL_INDEX_ARRAY_TYPE, GL_SHORT) && pass;
	pass = test_get(GL_INDEX_ARRAY_STRIDE, 10) && pass;

	pass = test_get(GL_FOG_COORD_ARRAY_TYPE, GL_FLOAT) && pass;
	pass = test_get(GL_FOG_COORD_ARRAY_STRIDE, 8) && pass;

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
