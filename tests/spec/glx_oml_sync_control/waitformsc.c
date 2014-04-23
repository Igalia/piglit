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

/** @file waitformsc.c
 *
 * Test that glXWaitForMscOML() waits until both it and
 * glXGetSyncValuesOML() return a an msc that meet the target.
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
	int64_t wait_ust = 0xd0, wait_msc = 0xd0, wait_sbc = 0xd0;
	int64_t current_ust = 0xd0, current_msc = 0xd0, current_sbc = 0xd0;
	int64_t target_msc;
	bool already_wrapped = false;

wrap:
	glXGetSyncValuesOML(dpy, win, &start_ust, &start_msc, &start_sbc);

	/* Wait for the MSC to be at least equal to target,
	 * with no divisor trickery.
	 */
	target_msc = start_msc + 5;
	glXWaitForMscOML(dpy, win, target_msc, 0, 0,
			 &wait_ust, &wait_msc, &wait_sbc);

	glXGetSyncValuesOML(dpy, win,
			    &current_ust, &current_msc, &current_sbc);

	if (current_msc < target_msc) {
		/* The clock may have actually wrapped, in which case
		 * we need to try again because we're not doing
		 * wrapping math here for simplicity.
		 */
		if (!already_wrapped) {
			already_wrapped = true;
			goto wrap;
		}

		fprintf(stderr,
			"glXGetSyncValuesOML() returned msc of %lld, "
			"expected >= %lld\n",
			(long long)current_msc,
			(long long)target_msc);
	}

	if (wait_msc < target_msc) {
		fprintf(stderr,
			"glXWaitForMscOML() returned msc of %lld, "
			"expected >= %lld\n",
			(long long)wait_msc,
			(long long)target_msc);
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
