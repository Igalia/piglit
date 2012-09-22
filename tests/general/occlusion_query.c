/*
 * Copyright © 2009 Intel Corporation
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
 */

/**
 * \file occlusion_query.c
 * Simple test for GL_ARB_occlusion_query.
 */

#include "piglit-util-gl-common.h"

PIGLIT_GL_TEST_MAIN(
    180 /*window_width*/,
    100 /*window_height*/,
    PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_DEPTH)

#define MAX_QUERIES 5
static GLuint occ_queries[MAX_QUERIES];
static PFNGLGENQUERIESPROC gen_queries = NULL;
static PFNGLBEGINQUERYPROC begin_query = NULL;
static PFNGLENDQUERYPROC end_query = NULL;
static PFNGLGETQUERYIVPROC get_queryiv = NULL;
static PFNGLGETQUERYOBJECTIVPROC get_query_objectiv = NULL;


static void draw_box(float x, float y, float z, float w, float h)
{
	glBegin(GL_QUADS);
	glVertex3f(x, y, z);
	glVertex3f(x + w, y, z);
	glVertex3f(x + w, y + h, z);
	glVertex3f(x, y + h, z);
	glEnd();
}


static int check_result(GLint passed, GLint expected)
{
	printf("samples passed = %d, expected = %d\n", passed, expected);
	return (expected == passed) ? 1 : 0;
}


static int do_test(float x, int all_at_once)
{
	static const struct {
		float x;
		float y;
		float z;
		float w;
		float h;
		int expected;
		GLubyte color[3];
	} tests[MAX_QUERIES] = {
		{
			25.0f, 25.0f, 0.2f, 20.0f, 20.0f, 20 * 20,
			{ 0x00, 0xff, 0x00 }
		},
		{
			45.0f, 45.0f, -0.2f, 20.0f, 20.0f, 0,
			{ 0x00, 0x7f, 0xf0 }
		},
		{
			10.0f, 10.0f, -0.3f, 75.0f, 75.0f,
			(75 * 75) - (55 * 55),
			{ 0x00, 0x00, 0xff }
		},
		{
			20.0f, 20.0f, -0.1f, 55.0f, 55.0f, 0,
			{ 0x7f, 0x7f, 0x00 }
		},
		{
			50.0f, 25.0f, 0.2f, 20.0f, 20.0f, 20 * 20,
			{ 0x00, 0x7f, 0xf0 }
		},
	};
	GLint passed;
	int test_pass = 1;
	unsigned i;


	/* Draw an initial red box that is 2500 pixels.  All of the occlusion
	 * query measurements are relative to this box.
	 */
	glColor3ub(0xff, 0x00, 0x00);
	draw_box(x + 20.0f, 20.0f, 0.0f, 55.0f, 55.0f);

	for (i = 0; i < MAX_QUERIES; i++) {
		(*begin_query)(GL_SAMPLES_PASSED, occ_queries[i]);
		glColor3ubv(tests[i].color);
		draw_box(x + tests[i].x, tests[i].y, tests[i].z,
			 tests[i].w, tests[i].h);
		(*end_query)(GL_SAMPLES_PASSED);

		if (! all_at_once) {
			(*get_query_objectiv)(occ_queries[i],
					      GL_QUERY_RESULT, &passed);
			test_pass &= check_result(passed, tests[i].expected);
		}
	}


	if (all_at_once) {
		for (i = 0; i < MAX_QUERIES; i++) {
			(*get_query_objectiv)(occ_queries[i], GL_QUERY_RESULT,
					      &passed);
			test_pass &= check_result(passed, tests[i].expected);
		}
	}

	printf("\n");
	return test_pass;
}

enum piglit_result
piglit_display(void)
{
	int test_pass;

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	test_pass = do_test(0.0f, 0);
	test_pass &= do_test(85.0f, 1);

	piglit_present_results();

	return test_pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	GLint query_bits;

	glClearColor(0.0, 0.2, 0.3, 0.0);
	glClearDepth(1.0);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);


	if (piglit_get_gl_version() >= 15) {
		gen_queries = glGenQueries;
		begin_query = glBeginQuery;
		end_query = glEndQuery;
		get_queryiv = glGetQueryiv;
		get_query_objectiv = glGetQueryObjectiv;
	} else if (piglit_is_extension_supported("GL_ARB_occlusion_query")) {
		gen_queries = glGenQueriesARB;
		begin_query = glBeginQueryARB;
		end_query = glEndQueryARB;
		get_queryiv = glGetQueryivARB;
		get_query_objectiv = glGetQueryObjectivARB;
	} else {
		piglit_report_result(PIGLIT_SKIP);
	}


	/* It is legal for a driver to support the query API but not have
	 * any query bits.  I wonder how many applications actually check for
	 * this case...
	 */
	(*get_queryiv)(GL_SAMPLES_PASSED, GL_QUERY_COUNTER_BITS,
		       & query_bits);
	if (query_bits == 0) {
		piglit_report_result(PIGLIT_SKIP);
	}


	(*gen_queries)(MAX_QUERIES, occ_queries);
}
