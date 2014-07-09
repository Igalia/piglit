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
 * @file roundmode-getintegerv.c
 *
 * Tests that the floating point rounding mode doesn't impact
 * glGetIntegerv's rounding behavior.
 *
 * From the GL 2.1 specification, page 248 (page 262 of the PDF):
 *
 *     "If a Get command is issued that returns value types different
 *      from the type of the value being obtained, a type conversion
 *      is performed...  If GetIntegerv (or any of the Get commands
 *      below) is called, a boolean value is interpreted as either 1
 *      or 0, and a floating-point value is rounded to the nearest
 *      integer, unless the value is an RGBA color component, a
 *      DepthRange value, a depth buffer clear value, or a normal
 *      coordinate."
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

	glFogf(GL_FOG_START, val);
	glGetIntegerv(GL_FOG_START, &out);

	if (out != expect) {
		printf("Set fog start to %.1f, expected %d, got %d\n",
		       val, expect, out);
		return false;
	} else {
		printf("Set fog start to %.1f, got %d\n", val, out);
		return true;
	}
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
	pass = test(-2.2, -2) && pass;
	pass = test(-2.8, -3) && pass;

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

