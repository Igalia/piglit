/*
 * Copyright © 2014 The TOVA Company
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
 * \file timing.c
 * Validates that OML Sync Control implementation actually syncs to vertical
 * retrace
 */

#include "piglit-util-gl.h"
#include "piglit-glx-util.h"
#include "common.h"
/*
 * TODO: varying MSC deltas enumerated as arguments
 * TODO: As a different test, create two drawables and verify they have
 *       independent SBC
 */
static bool fullscreen;
static bool use_swapbuffers = true;
static int64_t target_msc_delta;
static int64_t divisor;
static const int64_t msc_remainder = 0;
static const unsigned int loops = 10;

struct stats {
	unsigned int n;
	double mean;
	double M2;
};

static void update_stats(struct stats *stats, double val) {
	double delta = val - stats->mean;
	stats->n += 1;
	stats->mean += delta / stats->n;
	stats->M2 += delta * (val - stats->mean);
}

static double get_stddev(struct stats *stats) {
	return sqrt(stats->M2 / (stats->n - 1));
}

static enum piglit_result
draw(Display *dpy)
{
	enum piglit_result result = PIGLIT_PASS;
	int64_t last_ust = 0xd0, last_msc = 0xd0, last_sbc = 0xd0;
	int64_t last_timestamp = -1;
	struct stats msc_wallclock_duration_stats = {};
	struct stats msc_ust_duration_stats = {};
	double expected_msc_wallclock_duration = 0.0;
	int32_t rate_num, rate_den;
	unsigned int i;

	if (!glXGetSyncValuesOML(dpy, win, &last_ust, &last_msc, &last_sbc)) {
		fprintf(stderr, "Initial glXGetSyncValuesOML failed\n");
		return PIGLIT_FAIL;
	}

	/* Check that the window is fresh */
	if (last_sbc != 0) {
		fprintf(stderr, "Initial SBC for the window should be 0, was "
			"%" PRId64 "\n",
			last_sbc);
		piglit_merge_result(&result, PIGLIT_WARN);
	}

	if (!glXGetMscRateOML(dpy, win, &rate_num, &rate_den)) {
		fprintf(stderr,
			"glXGetMscRateOML failed, can't test MSC duration\n");
		piglit_merge_result(&result, PIGLIT_WARN);
	} else {
		expected_msc_wallclock_duration = 1e6 * rate_den / rate_num;
	}

	piglit_set_timeout(5, PIGLIT_FAIL);


	for (i = 0; i < loops; i++) {
		int64_t new_ust = 0xd0, new_msc = 0xd0, new_sbc = 0xd0;
		int64_t check_ust = 0xd0, check_msc = 0xd0, check_sbc = 0xd0;
		int64_t new_timestamp;
		int64_t expected_msc, target_sbc;
		int64_t target_msc = 0;

		if (target_msc_delta) {
			target_msc = last_msc + target_msc_delta;
		}

		if (use_swapbuffers) {
			glClearColor(0.0, 1.0, 0.0, 0.0);
			glClear(GL_COLOR_BUFFER_BIT);

			target_sbc = glXSwapBuffersMscOML(dpy, win,
							  target_msc, divisor,
							  msc_remainder);
			if (target_sbc <= 0) {
				fprintf(stderr, "SwapBuffersMscOML failed\n");
				return PIGLIT_FAIL;
			}
			if (target_sbc != last_sbc + 1) {
				fprintf(stderr,
					"glXSwapBuffersMscOML calculated the"
					" wrong target sbc: expected %"PRId64
					" but got %"PRId64"\n",
					last_sbc + 1, target_sbc);
				result = PIGLIT_FAIL;
			}

			if (!glXWaitForSbcOML(dpy, win, target_sbc,
					      &new_ust, &new_msc, &new_sbc))
			{
				fprintf(stderr, "glXWaitForSbcOML failed\n");
				result = PIGLIT_FAIL;
			}
		} else {
			target_sbc = last_sbc;

			if (!glXWaitForMscOML(dpy, win, target_msc, divisor,
					      msc_remainder, &new_ust,
					      &new_msc, &new_sbc))
			{
				fprintf(stderr, "glXWaitForSbcOML failed\n");
				result = PIGLIT_FAIL;
			}
		}
		new_timestamp = piglit_get_microseconds();

		if (!glXGetSyncValuesOML(dpy, win,
				&check_ust, &check_msc, &check_sbc))
		{
			fprintf(stderr, "Follow-up GetSyncValuesOML failed\n");
			return PIGLIT_FAIL;
		}

		if (new_ust < last_ust) {
			fprintf(stderr, "iteration %u: non-monotonic UST went "
				"backward by %"PRId64" during Wait\n",
				i, last_ust - new_ust);
			result = PIGLIT_FAIL;
			/* Wait returned something bogus, but GetSyncValues
			 * usually works, so try evaluating the rest of the
			 * tests using the check values. */
			new_ust = check_ust;
		}

		if (check_ust < new_ust) {
			fprintf(stderr, "iteration %u: non-monotonic UST went "
				"backward by %"PRId64" across GetSyncValues\n",
				i, last_ust - check_ust);
			result = PIGLIT_FAIL;
		}

		if (new_msc < last_msc) {
			fprintf(stderr, "iteration %u: non-monotonic MSC went "
				"backward by %"PRId64" during Wait\n",
				i, last_msc - new_msc);
			result = PIGLIT_FAIL;
			/* Wait returned something bogus, but GetSyncValues
			 * usually works, so try evaluating the rest of the
			 * tests using the check values. */
			new_msc = check_msc;
		}

		if (check_msc < new_msc) {
			fprintf(stderr, "iteration %u: non-monotonic MSC went "
				"backward by %"PRId64" across GetSyncValues\n",
				i, last_msc - check_msc);
			result = PIGLIT_FAIL;
		}

		if (new_sbc != target_sbc) {
			fprintf(stderr, "iteration %u: Wait should have "
				"returned at SBC %"PRId64" but returned at "
				"%"PRId64"\n",
				i, target_sbc, new_sbc);
			result = PIGLIT_FAIL;
		}

		if (check_sbc != new_sbc) {
			fprintf(stderr, "iteration %u: GetSyncValues "
				"returned SBC %"PRId64" but Wait returned "
				"%"PRId64"\n",
				i, check_sbc, new_sbc);
			result = PIGLIT_FAIL;
		}

		if (new_msc > last_msc) {
			int64_t delta_msc = new_msc - last_msc;
			update_stats(&msc_ust_duration_stats,
					(new_ust - last_ust) / delta_msc);

			if (last_timestamp >= 0) {
				if (new_timestamp < 0) {
					fprintf(stderr,
						"no monotonic clock\n");
					piglit_merge_result(&result,
						PIGLIT_WARN);
				} else {
					update_stats(
						&msc_wallclock_duration_stats,
						(new_timestamp - last_timestamp)
							/ delta_msc);
				}
			}
		}

		expected_msc = target_msc;
		if (!target_msc) {
			/* If there is a divisor, the expected MSC is the
			 * next MSC after last_msc such that
			 * MSC % divisor == remainder
			 */
			int64_t last_remainder = last_msc % divisor;
			expected_msc = last_msc - last_remainder + msc_remainder;
			if (expected_msc <= last_msc)
				expected_msc += divisor;
		}

		if (new_msc < expected_msc) {
			fprintf(stderr, "iteration %u woke up %"PRId64
				" MSCs early\n",
				i, expected_msc - new_msc);
			result = PIGLIT_FAIL;
		}

		if (new_msc > expected_msc) {
			fprintf(stderr, "iteration %u woke up %"PRId64
				" MSCs later than expected\n",
				i, new_msc - expected_msc);
			piglit_merge_result(&result, PIGLIT_WARN);
		}

		if (new_msc % divisor != msc_remainder) {
			fprintf(stderr, "iteration %u woke up at wrong MSC"
				" remainder %"PRId64", not requested remainder"
				" %"PRId64"\n",
				i, new_msc % divisor, msc_remainder);
			result = PIGLIT_FAIL;
		}

		last_ust = new_ust;
		last_msc = new_msc;
		last_sbc = new_sbc;
		last_timestamp = new_timestamp;
	}

	if (msc_ust_duration_stats.n < 2) {
		fprintf(stderr, "Not enough UST timing samples\n");
		piglit_merge_result(&result, PIGLIT_WARN);
	} else if (expected_msc_wallclock_duration > 0.0) {
		double apparent_ust_rate = msc_ust_duration_stats.mean /
			                   expected_msc_wallclock_duration;
		if (get_stddev(&msc_ust_duration_stats) /
		    apparent_ust_rate > 100)
		{
			fprintf(stderr, "UST duration per MSC is surprisingly"
				" variable (stddev %f USTs), but then it only"
				" has to be monotonic\n",
				get_stddev(&msc_ust_duration_stats));
			piglit_merge_result(&result, PIGLIT_WARN);
		}
	}

	if (msc_wallclock_duration_stats.n < 2) {
		fprintf(stderr, "Not enough wallclock timing samples\n");
		piglit_merge_result(&result, PIGLIT_WARN);
	} else if (get_stddev(&msc_wallclock_duration_stats) > 1000) {
		fprintf(stderr, "Wallclock time between MSCs has stddev > 1ms"
			" (%fus), driver is probably not syncing to"
			" vblank\n",
			get_stddev(&msc_wallclock_duration_stats));
		result = PIGLIT_FAIL;
	} else if (expected_msc_wallclock_duration > 0.0) {
		if (fabs(expected_msc_wallclock_duration -
			 msc_wallclock_duration_stats.mean) > 50)
		{
			fprintf(stderr, "Wallclock time between MSCs %fus"
				" does not match glXGetMscRateOML %fus\n",
				msc_wallclock_duration_stats.mean,
				expected_msc_wallclock_duration);
			result = PIGLIT_FAIL;
		}
	}

	return result;
}

static unsigned int
parse_num_arg(int argc, char **argv, int j)
{
	char *ptr;
	unsigned int val;

	if (j >= argc) {
		fprintf(stderr, "%s requires an argument\n", argv[j - 1]);
		piglit_report_result(PIGLIT_FAIL);
	}

	val = strtoul(argv[j], &ptr, 0);
	if (!val || *ptr != '\0') {
		fprintf(stderr, "%s requires an argument\n", argv[j - 1]);
		piglit_report_result(PIGLIT_FAIL);
	}

	return val;
}

int
main(int argc, char **argv)
{
	int j;
	for (j = 1; j < argc; j++) {
		if (!strcmp(argv[j], "-fullscreen")) {
			fullscreen = true;
		} else if (!strcmp(argv[j], "-waitformsc")) {
			use_swapbuffers = false;
		} else if (!strcmp(argv[j], "-divisor")) {
			j++;
			divisor = parse_num_arg(argc, argv, j);
		} else if (!strcmp(argv[j], "-msc-delta")) {
			j++;
			target_msc_delta = parse_num_arg(argc, argv, j);
		} else if (!strcmp(argv[j], "-auto")) {
			piglit_automatic = true;
		} else {
			fprintf(stderr, "unsupported option %s\n", argv[j]);
			piglit_report_result(PIGLIT_FAIL);
		}
	}

	if (divisor && target_msc_delta) {
		fprintf(stderr, "this test doesn't support using both "
			"-divisor and -msc-delta\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	if (!use_swapbuffers && !divisor && !target_msc_delta) {
		fprintf(stderr, "when using -waitformsc, this test requires "
			"either -divisor or -msc-delta\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	if (!divisor) {
		divisor=1;
	}
	piglit_automatic = true;
	piglit_oml_sync_control_test_run(fullscreen, draw);

	return 0;
}
