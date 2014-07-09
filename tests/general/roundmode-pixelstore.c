/*
 * Copyright © 2011 Intel Corporation
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
 * @file roundmode-pixelstore.c
 *
 * Tests that the floating point rounding mode doesn't impact
 * glPixelStore's rounding behavior.
 *
 * From the GL 2.1 specification, page 114 (page 128 of the PDF):
 *
 *     "The version of PixelStore that takes a floating-point value
 *      may be used to set any type of parameter; if the parameter is
 *      boolean, then it is set to FALSE if the passed value is 0.0
 *      and TRUE otherwise, while if the parameter is an integer, then
 *      the passed value is rounded to the nearest integer."
 */

#include "piglit-util-gl.h"

#include <fenv.h>

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	/* UNREACHED */
	return PIGLIT_FAIL;
}

static bool
test(float val, int expect)
{
	GLint out;

	glPixelStoref(GL_UNPACK_ROW_LENGTH, val);
	glGetIntegerv(GL_UNPACK_ROW_LENGTH, &out);

	if (out != expect) {
		printf("Set row length to %.1f, expected %d, got %d\n",
		       val, expect, out);
		return false;
	} else {
		printf("Set row length to %.1f and got %d\n", val, out);
	}

	return true;
}

void
piglit_init(int argc, char **argv)
{
	int ret;
	bool pass = true;
	ret = fesetround(FE_UPWARD);
	if (ret != 0) {
		printf("Couldn't set rounding mode\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	pass = test(2.2, 2) && pass;
	pass = test(2.8, 3) && pass;
	pass = test(-0.1, 0) && pass;
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	printf("Setting row length -0.9, and expecting error\n");
	glPixelStoref(GL_UNPACK_ROW_LENGTH, -0.9);
	pass = piglit_check_gl_error(GL_INVALID_VALUE) && pass;

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

