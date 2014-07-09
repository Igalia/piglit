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
#include <unistd.h>
#include <sys/time.h>

/**
 * @file timestamp-get.c
 *
 * Test that GL_TIMESTAMP obtained via glGet and glQuery returns roughly
 * the same value.
 */

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

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

static GLint64
get_cpu_time()
{
	struct timeval tv;

	gettimeofday(&tv, 0);
	return (GLint64)tv.tv_sec * 1000000000 + (GLint64)tv.tv_usec * 1000;
}

static void
validate_times(GLint64 t1, GLint64 t2, GLint64 tolerance)
{
	if (t1 > t2) {
		printf("old time = %llu us\n", (unsigned long long) t1 / 1000);
		printf("new time = %llu us\n", (unsigned long long) t2 / 1000);
		puts("old time > new time");
		piglit_report_result(PIGLIT_FAIL);
	}

	/* the tolerance of 1 milisecond seems to be sufficient */
	if (t2 - t1 > tolerance) {
		printf("time 1 = %llu us\n", (unsigned long long) t1 / 1000);
		printf("time 2 = %llu us\n", (unsigned long long) t2 / 1000);
		puts("too big difference");
		piglit_report_result(PIGLIT_FAIL);
	}
}

enum piglit_result
piglit_display(void)
{
	GLint64 t1, t2;
	GLint64 query_overhead, get_overhead, tolerance;
	GLuint q;

	glGenQueries(1, &q);

	/* this creates the query in the driver */
	get_gpu_time_via_query(q);

	/* compute a reasonable tolerance based on driver overhead */
	t1 = get_cpu_time();
	get_gpu_time_via_query(q);
	query_overhead = get_cpu_time() - t1;

	t1 = get_cpu_time();
	get_gpu_time_via_get(q);
	get_overhead = get_cpu_time() - t1;

	printf("glGet overhead: %llu us\n", (unsigned long long) get_overhead / 1000);
	printf("glQuery overhead: %llu us\n", (unsigned long long) query_overhead / 1000);

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
