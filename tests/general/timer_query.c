/*
 * Copyright © 2009 Intel Corporation
 * Copyright © 2010 Mathias Fröhlich
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
 *    Ian Romanick <ian.d.romanick@intel.com>
 *    Mathias Fröhlich <m.froehlich@web.de>
 */

/**
 * \file timer_query.c
 * Simple test for GL_EXT_timer_query.
 */

#include "piglit-util.h"

int piglit_width = 180, piglit_height = 100;
int piglit_window_mode = GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH;

static GLuint timer_query;
static PFNGLGENQUERIESPROC gen_queries = NULL;
static PFNGLBEGINQUERYPROC begin_query = NULL;
static PFNGLENDQUERYPROC end_query = NULL;
static PFNGLGETQUERYIVPROC get_queryiv = NULL;
static PFNGLGETQUERYOBJECTIVPROC get_query_objectiv = NULL;
static PFNGLGETQUERYOBJECTI64VPROC get_query_objecti64v = NULL;
static PFNGLGETQUERYOBJECTUI64VPROC get_query_objectui64v = NULL;

void
piglit_init(int argc, char **argv)
{
	GLint query_bits;

	piglit_require_extension("GL_EXT_timer_query");

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	gen_queries = GLEW_GET_FUN(__glewGenQueriesARB);
	begin_query = GLEW_GET_FUN(__glewBeginQueryARB);
	end_query = GLEW_GET_FUN(__glewEndQueryARB);
	get_queryiv = GLEW_GET_FUN(__glewGetQueryivARB);
	get_query_objectiv = GLEW_GET_FUN(__glewGetQueryObjectivARB);
	get_query_objecti64v = GLEW_GET_FUN(__glewGetQueryObjecti64vEXT);
	get_query_objectui64v = GLEW_GET_FUN(__glewGetQueryObjectui64vEXT);

	/* It is legal for a driver to support the query API but not have
	 * any query bits.  I wonder how many applications actually check for
	 * this case...
	 */
	(*get_queryiv)(GL_TIME_ELAPSED, GL_QUERY_COUNTER_BITS, &query_bits);
	if (query_bits == 0) {
		piglit_report_result(PIGLIT_SKIP);
	}

	(*gen_queries)(1, &timer_query);
}

enum piglit_result
piglit_display(void)
{
	GLint available = 0;
	GLint nsecs;
	GLint64 nsecs64;
	GLuint64 nsecs64u;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/* Start a query */
	(*begin_query)(GL_TIME_ELAPSED_EXT, timer_query);

	/* Paint something */
	glColor3ub(0xff, 0xff, 0xff);

	glBegin(GL_QUADS);
	glVertex3f(0, 0, 0);
	glVertex3f(piglit_width, 0, 0);
	glVertex3f(piglit_width, piglit_height, 0);
	glVertex3f(0, piglit_height, 0);
	glEnd();

	/* Stop a query */
	(*end_query)(GL_TIME_ELAPSED_EXT);

	/* In this case poll until available */
	while (!available)
		(*get_query_objectiv)(timer_query, GL_QUERY_RESULT_AVAILABLE, &available);

	/* Get the result */
	(*get_query_objectiv)(timer_query, GL_QUERY_RESULT, &nsecs);
	(*get_query_objecti64v)(timer_query, GL_QUERY_RESULT, &nsecs64);
	(*get_query_objectui64v)(timer_query, GL_QUERY_RESULT, &nsecs64u);

	if ((nsecs & 0xffffffff) != (nsecs64 & 0xffffffff)) {
		printf("timer_query: 32 and 64-bit results differ!\n");
		return PIGLIT_FAILURE;
	}

	if ((nsecs & 0xffffffff) != (nsecs64u & 0xffffffff)) {
		printf("timer_query: 32 and 64-bit unsigned results differ!\n");
		return PIGLIT_FAILURE;
	}

	/*printf("nsecs = %d %ld\n", nsecs, (long int) nsecs64);*/

	glutSwapBuffers();

	return PIGLIT_SUCCESS;
}
