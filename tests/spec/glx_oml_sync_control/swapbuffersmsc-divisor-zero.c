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

/** @file swapbuffersmsc-divisor-zero.c
 *
 * Test that when the divsior is zero in glXSwapBuffersMscOML, the
 * target MSC is reached.
 */

#include "piglit-util-gl.h"
#include "piglit-glx-util.h"
#include "common.h"

int piglit_width = 50, piglit_height = 50;

enum piglit_result
draw(Display *dpy)
{
	/* Fill the variables that will be returned as out values with
	 * junk to better detect failure there.
	 */
	int64_t start_ust = 0xd0, start_msc = 0xd0, start_sbc = 0xd0;
	int64_t swapped_ust = 0xd0, swapped_msc = 0xd0, swapped_sbc = 0xd0;
	int64_t current_ust = 0xd0, current_msc = 0xd0, current_sbc = 0xd0;
	int64_t target_msc, outstanding_sbc;
	bool already_wrapped = false;

	glXGetSyncValuesOML(dpy, win, &start_ust, &start_msc, &start_sbc);
	if (start_sbc != 0) {
		fprintf(stderr,
			"Initial SBC for the window should be 0, was %lld\n",
			(long long)start_sbc);
		piglit_report_result(PIGLIT_FAIL);
	}
	outstanding_sbc = start_sbc;

wrap:
	glClearColor(0.0, 1.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);

	/* Queue a swap for 5 frames from when we started. */
	target_msc = start_msc + 5;
	glXSwapBuffersMscOML(dpy, win, target_msc, 0, 0);
	outstanding_sbc++;

	/* Wait for that swap. */
	glXWaitForSbcOML(dpy, win, outstanding_sbc,
			 &swapped_ust, &swapped_msc, &swapped_sbc);
	if (swapped_sbc != outstanding_sbc) {
		fprintf(stderr,
			"glXWaitForSbcOML() returned SBC %lld, "
			"should be %lld\n",
			(long long)swapped_sbc, (long long)outstanding_sbc);
		piglit_report_result(PIGLIT_FAIL);
	}

	glXGetSyncValuesOML(dpy, win,
			    &current_ust, &current_msc, &current_sbc);
	if (current_sbc != outstanding_sbc) {
		fprintf(stderr,
			"glXGetSyncValuesOML() returned SBC %lld, "
			"should be %lld\n",
			(long long)current_sbc, (long long)outstanding_sbc);
		piglit_report_result(PIGLIT_FAIL);
	}

	if (current_msc < start_msc) {
		/* The MSC counter wrapped. Try the test again.  But
		 * it definitely won't wrap this time.
		 */
		if (already_wrapped) {
			fprintf(stderr,
				"Wrapped MSC twice!\n"
				"Second time: %lld -> %lld\n",
				(long long)start_msc,
				(long long)current_msc);
			piglit_report_result(PIGLIT_FAIL);
		}

		glXGetSyncValuesOML(dpy, win,
				    &start_ust, &start_msc, &start_sbc);
		already_wrapped = true;
		goto wrap;
	}

	if (swapped_msc < target_msc) {
		fprintf(stderr,
			"glXWaitForSbcOML() returned MSC %lld, "
			"should be at least %lld\n",
			(long long)swapped_msc,
			(long long)target_msc);
		piglit_report_result(PIGLIT_FAIL);
	}

	if (current_msc < target_msc ||
	    current_msc < swapped_msc) {
		fprintf(stderr,
			"glXGetSyncValuesMsc() returned MSC %lld, "
			"should be at least swap target msc (%lld) "
			"and last swap MSC (%lld)\n",
			(long long)current_msc,
			(long long)target_msc,
			(long long)swapped_msc);
		piglit_report_result(PIGLIT_FAIL);
	}

	piglit_report_result(PIGLIT_PASS);

	/* UNREACHED */
	return PIGLIT_FAIL;
}

int
main(int argc, char **argv)
{
	piglit_oml_sync_control_test_run(false, draw);

	return 0;
}
