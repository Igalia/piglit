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
 */

/**
 * \file render.c
 *
 * Simple test for getting query results with GL_ARB_occlusion_query2.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_DEPTH;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	GLuint oq[3];
	GLint result[3] = {2, 2, 2};
	bool pass = true;
	int i;

	glEnable(GL_DEPTH_TEST);

	glClearDepth(1.0);
	glClearColor(0.5, 0.5, 0.5, 0.5);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glGenQueries(3, oq);
	glDeleteQueries(3, oq);

	glColor4f(0.0, 1.0, 0.0, 0.0);
	glBeginQuery(GL_ANY_SAMPLES_PASSED, oq[0]);
	piglit_draw_rect_z(0.5, -1, -1, 2, 2);
	glEndQuery(GL_ANY_SAMPLES_PASSED);

	glBeginQuery(GL_ANY_SAMPLES_PASSED, oq[1]);
	glEndQuery(GL_ANY_SAMPLES_PASSED);

	glColor4f(1.0, 0.0, 0.0, 0.0);
	glBeginQuery(GL_ANY_SAMPLES_PASSED, oq[2]);
	piglit_draw_rect_z(0.75, -0.5, -0.5, 1.0, 1.0);
	glEndQuery(GL_ANY_SAMPLES_PASSED);

	piglit_present_results();

	for (i = 0; i < 3; i++) {
		glGetQueryObjectiv(oq[i], GL_QUERY_RESULT, &result[i]);
	}

	if (result[0] != GL_TRUE) {
		fprintf(stderr, "GL_ANY_SAMPLES_PASSED with passed fragments returned %d\n",
			result[0]);
		pass = false;
	}

	if (result[1] != GL_FALSE) {
		fprintf(stderr, "GL_ANY_SAMPLES_PASSED with no rendering returned %d\n",
			result[1]);
		pass = false;
	}

	if (result[2] != GL_FALSE) {
		fprintf(stderr, "GL_ANY_SAMPLES_PASSED with occluded rendering returned %d\n",
			result[2]);
		pass = false;
	}

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_ARB_occlusion_query2");
}
