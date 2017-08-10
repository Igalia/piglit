/*
 * Copyright © 2012 Marek Olšák <maraeo@gmail.com>
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

#include "piglit-util-gl.h"

#include <inttypes.h>  /* for PRIu64 macro */
#ifdef HAVE_UNISTD_H
#include <unistd.h>    /* for usleep */
#endif

/* GL_TIMESTAMP isn't expected to be reliable for measuring long durations and
 * although the ARB_timer_query spec doesn't stipulate what kind of drifting
 * from wall clock time is acceptable, we at least want a sanity check that
 * things look reasonable...
 */
#define DRIFT_NS_PER_SEC_THRESHOLD	3000000

/**
 * @file timestamp-get.c
 *
 * Test that GL_TIMESTAMP obtained via glGet and glQuery returns roughly
 * the same value, and that durations measured via GL_TIMESTAMP have
 * nanosecond units.
 */

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

static GLint64
get_gpu_time_via_query(GLuint q)
{
	GLint64 time;

	glQueryCounter(q, GL_TIMESTAMP);
	glGetQueryObjecti64v(q, GL_QUERY_RESULT, &time);
	return time;
}

static GLint64
get_gpu_time_via_get(GLuint q)
{
	GLint64 time;

	glGetInteger64v(GL_TIMESTAMP, &time);
	return time;
}

static void
validate_times(GLint64 t1, GLint64 t2, GLint64 tolerance)
{
	if (t1 > t2) {
		printf("old time = %" PRIu64 " us\n", (uint64_t) t1 / 1000);
		printf("new time = %" PRIu64 " us\n", (uint64_t) t2 / 1000);
		puts("old time > new time");
		piglit_report_result(PIGLIT_FAIL);
	}

	/* the tolerance of 1 milisecond seems to be sufficient */
	if (t2 - t1 > tolerance) {
		printf("time 1 = %" PRIu64 " us\n", (uint64_t) t1 / 1000);
		printf("time 2 = %" PRIu64 " us\n", (uint64_t) t2 / 1000);
		puts("too big difference");
		piglit_report_result(PIGLIT_FAIL);
	}
}

static void
validate_delta(GLint64 gl_ts1, GLint64 gl_ts2, GLint64 cpu_delay_ns)
{
	GLint64 gl_ts_delta = gl_ts2 - gl_ts1;
	int64_t drift = llabs(cpu_delay_ns - gl_ts_delta);
	int64_t drift_per_sec = drift * 1000000000LL /  cpu_delay_ns;

	/* XXX: technically we shouldn't be as strict about drift when the gpu
	 * clock is running fast and the duration is longer than expected,
	 * because we can't easily exclude other factors like OS scheduling
	 * affecting the measurements. For now though we don't take this into
	 * account.
	 */
	if (drift_per_sec > DRIFT_NS_PER_SEC_THRESHOLD) {
		printf("GL_TIMESTAMP 1 = %" PRId64 " us\n", gl_ts1 / 1000);
		printf("GL_TIMESTAMP 2 = %" PRId64 " us\n", gl_ts2 / 1000);
		printf("delta  = %" PRId64 " us (expect >= %"PRId64" us)\n",
		       gl_ts_delta / 1000, cpu_delay_ns / 1000);

		piglit_loge("GL_TIMESTAMP drift of %" PRId64 " ns/sec, greater than %" PRId64 " ns/sec",
			    drift_per_sec,
			    (int64_t)DRIFT_NS_PER_SEC_THRESHOLD);
		piglit_report_result(PIGLIT_FAIL);
	} else
		printf("GL_TIMESTAMP drift of approx. %" PRId64 " ns/sec\n",
		       drift_per_sec);
}

enum piglit_result
piglit_display(void)
{
	GLint64 t1, t2;
	GLint64 query_overhead, get_overhead, tolerance;
	GLuint q;
	int64_t delay;

	glGenQueries(1, &q);

	/* this creates the query in the driver */
	get_gpu_time_via_query(q);

	/* compute a reasonable tolerance based on driver overhead */
	t1 = piglit_time_get_nano();
	get_gpu_time_via_query(q);
	query_overhead = piglit_time_get_nano() - t1;

	t1 = piglit_time_get_nano();
	get_gpu_time_via_get(q);
	get_overhead = piglit_time_get_nano() - t1;

	printf("glGet overhead: %" PRIu64 " us\n", (uint64_t) get_overhead / 1000);
	printf("glQuery overhead: %" PRIu64 " us\n", (uint64_t) query_overhead / 1000);

	/* minimum tolerance is 3 ms */
	tolerance = query_overhead + get_overhead + 3000000;

	/* do testing */
	puts("Test: first glQuery, then glGet");
	t1 = get_gpu_time_via_query(q);
	t2 = get_gpu_time_via_get(q);
	validate_times(t1, t2, tolerance);

	usleep(10000);

	puts("Test: first glGet, then glQuery");
	t1 = get_gpu_time_via_get(q);
	t2 = get_gpu_time_via_query(q);
	validate_times(t1, t2, tolerance);

	puts("Test: wall clock time via glQuery");
	t1 = get_gpu_time_via_query(q);
	delay = piglit_delay_ns(1000000000);
	t2 = get_gpu_time_via_query(q);
	validate_delta(t1, t2, delay);

	puts("Test: wall clock time via glGet");
	t1 = get_gpu_time_via_get(q);
	delay = piglit_delay_ns(1000000000);
	t2 = get_gpu_time_via_get(q);
	validate_delta(t1, t2, delay);

	glDeleteQueries(1, &q);

	return PIGLIT_PASS;
}

void
piglit_init(int argc, char **argv)
{
	piglit_automatic = true;

	piglit_require_gl_version(20);

	piglit_require_extension("GL_ARB_timer_query");
}
