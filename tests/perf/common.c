/*
 * Copyright (C) 2009  VMware, Inc.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * VMWARE BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/**
 * Common perf code.  This should be re-usable with other tests.
 */

#include "piglit-util-gl.h"
#include "common.h"

/** Return time in seconds */
static double
perf_get_time(void)
{
	return piglit_time_get_nano() * 0.000000001;
}

/**
 * Run function 'f' for enough iterations to reach a steady state.
 * Return the rate (iterations/second).
 */
double
perf_measure_rate(perf_rate_func f)
{
	const double minDuration = 0.5;
	double rate = 0.0, prevRate = 0.0;
	unsigned subiters;

	/* Compute initial number of iterations to try.
	 * If the test function is pretty slow this helps to avoid
	 * extraordinarily long run times.
	 */
	subiters = 2;
	{
		const double t0 = perf_get_time();
		double t1;
		do {
			f(subiters); /* call the rendering function */
			glFinish();
			t1 = perf_get_time();
			subiters *= 2;
		} while (t1 - t0 < 0.1 * minDuration);
	}
	/*perf_printf("initial subIters = %u\n", subiters);*/

	while (1) {
		const double t0 = perf_get_time();
		unsigned iters = 0;
		double t1;

		do {
			f(subiters); /* call the rendering function */
			glFinish();
			t1 = perf_get_time();
			iters += subiters;
		} while (t1 - t0 < minDuration);

		rate = iters / (t1 - t0);

		if (0)
			printf("prevRate %f  rate  %f  ratio %f  iters %u\n",
			       prevRate, rate, rate/prevRate, iters);

		/* Try and speed the search up by skipping a few steps: */
		if (rate > prevRate * 1.6)
			subiters *= 8;
		else if (rate > prevRate * 1.2)
			subiters *= 4;
		else if (rate > prevRate * 1.05)
			subiters *= 2;
		else
			break;

		prevRate = rate;
	}

	if (0)
		printf("%s returning iters %u  rate %f\n", __FUNCTION__, subiters, rate);
	return rate;
}
