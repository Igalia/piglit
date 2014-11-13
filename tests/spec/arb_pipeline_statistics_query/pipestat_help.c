/*
 * Copyright © 2014 Intel Corporation
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

/** \file pipestat_help.c
 *
 * Helper library for the pipeline statistics tests
 */

#include <stdint.h>
#include <inttypes.h>
#include "piglit-util-gl.h"
#include "pipestat_help.h"

void
do_query_init(struct query *queries, const int count)
{
	int i;

	/* Some of the token require more than just having the extension, but
	 * all require at least having the extension.
	 */
	piglit_require_extension("GL_ARB_pipeline_statistics_query");

	for (i = 0; i < count; i++) {
		GLint bits;
		glGetQueryiv(queries[i].query, GL_QUERY_COUNTER_BITS, &bits);
		if (bits == 0) {
			printf("%s is unsupported.\n", queries[i].name);
			piglit_report_result(PIGLIT_SKIP);
		}
	}


	for (i = 0; i < count; i++) {
		glGenQueries(1, &queries[i].obj);
		if (glGetError() != GL_NO_ERROR)
			piglit_report_result(PIGLIT_FAIL);
	}
}

enum piglit_result
do_query_func(const struct query *queries, const int count,
	      void (*draw)(void))
{
	const float green[] = {0, 1, 0, 0};
	int i;

	glClearColor(1.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);
	if (piglit_get_gl_version() <= 30)
		glColor4fv(green);

	for (i = 0; i < count; i++)
		begin_query(&queries[i]);

	draw();

	for (i = 0; i < count; i++)
		end_query(&queries[i]);

	for (i = 0; i < count; i++) {
		const struct query *q = &queries[i];
		GLuint64 max = q->max != 0 ? q->max : q->min;
		GLuint64 params;

		glGetQueryObjectui64v(queries[i].obj, GL_QUERY_RESULT, &params);
		if (q->min > params || max < params) {
			fprintf(stderr,
					"%s value was invalid.\n  Expected: %" PRIu64 " - %" PRIu64 "\n  Observed: %" PRIu64 "\n",
					q->name, q->min, max, params);
			piglit_report_result(PIGLIT_FAIL);
		}
	}

	return PIGLIT_PASS;
}

static void
default_draw(void)
{
	piglit_draw_rect(-1, -1, 2, 2);
}

enum piglit_result
do_query(const struct query *queries, const int count)
{
	return do_query_func(queries, count, default_draw);
}
