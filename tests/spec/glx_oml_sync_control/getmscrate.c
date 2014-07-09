/*
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
 * \file getmscrate.c
 * Verifies that glXGetMscRateOML returns sensible data.
 */

#include "piglit-util-gl.h"
#include "piglit-glx-util.h"
#include "common.h"

int piglit_width = 50, piglit_height = 50;

enum piglit_result
draw(Display *dpy)
{
	int32_t numerator = 0xDEADBEEF;
	int32_t denominator = 0xDEADBEEF;
	bool pass = true;

	if (!glXGetMscRateOML(dpy, win, &numerator, &denominator)) {
		printf("glXGetMscRateOML returned failure.\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	if (numerator == 0xDEADBEEF) {
		printf("glXGetMscRateOML did not write numerator.");
		pass = false;
	} else if (numerator <= 0) {
		printf("Numerator <= 0: %d\n", numerator);
		pass = false;
	}

	if (denominator == 0xDEADBEEF) {
		printf("glXGetMscRateOML did not write denominator.");
		pass = false;
	} else if (denominator <= 0) {
		printf("Denominator <= 0: %d\n", denominator);
		pass = false;
	}

	/* The GLX_OML_sync_control spec says:
	 *
	 *     "If the MSC rate in Hertz is an integer, then <denominator>
	 *     will be 1 and <numerator> will be the MSC rate."
	 */
	if (denominator != 1 && (numerator % denominator == 0)) {
		printf("Numerator should be %d and denominator should be 1,\n"
		       "but are %d and %d instead.\n",
		       numerator / denominator,
		       numerator, denominator);
		pass = false;
	}

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);

	/* UNREACHED */
	return PIGLIT_FAIL;
}

int
main(int argc, char **argv)
{
	piglit_oml_sync_control_test_run(draw);

	return 0;
}
