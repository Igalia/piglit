/**
 * Copyright © 2013 Intel Corporation
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
 * Test that GL_DEPTH_CLAMP is a valid state
 *
 * Table 6.8 (Transformation state) of OpenGL 3.2 Core added DEPTH_CLAMP
 *
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 32;
	config.supports_gl_core_version = 32;

PIGLIT_GL_TEST_CONFIG_END

void
piglit_init(int argc, char **argv)
{
	bool pass = true;
	GLint i;
	GLfloat f;
	GLboolean b;
	GLdouble d;

	/* Check that GL_DEPTH_CLAMP was initialized to TRUE */
	if (glIsEnabled(GL_DEPTH_CLAMP)) {
		printf("GL_DEPTH_CLAMP was not initialized to FALSE\n");
		pass = false;
	}
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	/* Test that GL_DEPTH_CLAMP is enable/disabled correctly */
	glEnable(GL_DEPTH_CLAMP);
	if (!glIsEnabled(GL_DEPTH_CLAMP)) {
		printf("GL_DEPTH_CLAMP was not enabled properly\n");
		pass = false;
	}
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	glDisable(GL_DEPTH_CLAMP);
	if (glIsEnabled(GL_DEPTH_CLAMP)) {
		printf("GL_DEPTH_CLAMP was not disabled properly\n");
		pass = false;
	}
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	/* Test that GL_DEPTH_CLAMP disabled is returned from glGet calls */
	glGetIntegerv(GL_DEPTH_CLAMP, &i);
	if(i != 0) {
		printf("i expected to be 0, but returned %d\n", i);
		pass = false;
	}
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
	glGetFloatv(GL_DEPTH_CLAMP, &f);
	if(f != 0.0f) {
		printf("f expected to be 0.0, but returned %f\n", f);
		pass = false;
	}
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
	glGetBooleanv(GL_DEPTH_CLAMP, &b);
	if(b != GL_FALSE) {
		printf("b expected to be 0, but returned %d\n", (int)b);
		pass = false;
	}
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
	glGetDoublev(GL_DEPTH_CLAMP, &d);
	if(d != 0.0) {
		printf("d expected to be 0.0, but returned %f\n", d);
		pass = false;
	}
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	/* Test that GL_DEPTH_CLAMP enabled is returned from glGet calls */
	glEnable(GL_DEPTH_CLAMP);

	glGetIntegerv(GL_DEPTH_CLAMP, &i);
	if(i != 1) {
		printf("i expected to be 1, but returned %d\n", i);
		pass = false;
	}
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
	glGetFloatv(GL_DEPTH_CLAMP, &f);
	if(f != 1.0f) {
		printf("f expected to be 1.0, but returned %f\n", f);
		pass = false;
	}
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
	glGetBooleanv(GL_DEPTH_CLAMP, &b);
	if(b != GL_TRUE) {
		printf("b expected to be 1, but returned %d\n", (int)b);
		pass = false;
	}
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
	glGetDoublev(GL_DEPTH_CLAMP, &d);
	if(d != 1.0) {
		printf("d expected to be 1.0, but returned %f\n", d);
		pass = false;
	}
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}
