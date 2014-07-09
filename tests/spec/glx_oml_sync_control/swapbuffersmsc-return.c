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
 *
 * Authors:
 *    Eric Anholt <eric@anholt.net>
 *
 */

/** @file swapbuffersmsc-return.c
 *
 * Test that glXSwapBuffersMscOML() returns a correct sbc value.
 *
 * Catches a bug in the X Server when a swap interval of 0 is used.
 */

#include "piglit-util-gl.h"
#include "piglit-glx-util.h"
#include "common.h"

int piglit_width = 50, piglit_height = 50;

int swap_interval = -1;

enum piglit_result
draw(Display *dpy)
{
	/* Fill the variables that will be returned as out values with
	 * junk to better detect failure there.
	 */
	int64_t start_ust = 0xd0, start_msc = 0xd0, start_sbc = 0xd0;
	int64_t next_sbc;
	bool pass = true;
	int i;

#if defined(GLX_MESA_swap_control)
	if (swap_interval != -1) {
		PFNGLXSWAPINTERVALMESAPROC pglXSwapIntervalMESA;

		printf("Testing with swap interval %d\n", swap_interval);

		piglit_require_glx_extension(dpy, "GLX_MESA_swap_control");
		pglXSwapIntervalMESA = (PFNGLXSWAPINTERVALMESAPROC)
			glXGetProcAddressARB((const GLubyte *)
					     "glXSwapIntervalMESA");
		pglXSwapIntervalMESA(swap_interval);
	} else {
		printf("Testing with default swap interval\n");
	}
#else
	printf("Testing with default swap interval\n");
#endif

	glXGetSyncValuesOML(dpy, win, &start_ust, &start_msc, &start_sbc);
	if (start_sbc != 0) {
		fprintf(stderr,
			"Initial SBC for the window should be 0, was %lld\n",
			(long long)start_sbc);
		piglit_report_result(PIGLIT_FAIL);
	}
	next_sbc = start_sbc + 1;

	for (i = 0; i < 3; i++) {
		int64_t ret_sbc;

		glClearColor(0.0, 1.0, 0.0, 0.0);
		glClear(GL_COLOR_BUFFER_BIT);

		ret_sbc = glXSwapBuffersMscOML(dpy, win, 0, 1, 0);

		if (ret_sbc != next_sbc) {
			printf("Frame %d: sbc was %lld, should be %lld\n",
			       i,
			       (long long)ret_sbc,
			       (long long)next_sbc);
			pass = false;
		}

		next_sbc++;
	}

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);

	/* UNREACHED */
	return PIGLIT_FAIL;
}

int
main(int argc, char **argv)
{
	if (argc >= 2) {
		swap_interval = atoi(argv[1]);
	}

	piglit_oml_sync_control_test_run(draw);

	return 0;
}
