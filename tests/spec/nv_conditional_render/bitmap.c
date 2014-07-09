/*
 * Copyright © 2011 Intel Corporation
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

/**
 * @file bitmap.c
 *
 * Tests that conditional rendering appropriately affects glBitmap().
 *
 * From the NV_conditional_render spec:
 *
 *     "If the result (SAMPLES_PASSED) of the query is zero, all
 *      rendering commands between BeginConditionalRenderNV and the
 *      corresponding EndConditionalRenderNV are discarded.  In this
 *      case, Begin, End, all vertex array commands performing an
 *      implicit Begin and End, DrawPixels (section 3.6), Bitmap
 *      (section 3.7), Clear (section 4.2.3), Accum (section 4.2.4),
 *      CopyPixels (section 4.3.3), EvalMesh1, and EvalMesh2 (section
 *      5.1) have no effect."
 */

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	bool pass = true;
	float green[4] = {0.0, 1.0, 0.0, 0.0};
	void *buf;
	GLuint q;

	glClearColor(0.5, 0.5, 0.5, 0.5);
	glClear(GL_COLOR_BUFFER_BIT);

	/* Massively overallocate the bitmap, and set it to all on */
	buf = malloc(piglit_width * piglit_height);
	memset(buf, 0xff, piglit_width * piglit_height);

	glGenQueries(1, &q);

	/* Generate query pass: draw top half of screen. */
	glColor4f(0.0, 1.0, 0.0, 0.0);
	glBeginQuery(GL_SAMPLES_PASSED, q);
	piglit_draw_rect(-1, 0, 2, 1);
	glEndQuery(GL_SAMPLES_PASSED);

	/* Conditional render that should draw the whole screen. */
	glBeginConditionalRenderNV(q, GL_QUERY_WAIT_NV);
	glRasterPos2i(-1, -1);
	glBitmap(piglit_width, piglit_height, 0, 0, 0, 0, buf);
	glEndConditionalRenderNV();

	/* Generate query fail */
	glBeginQuery(GL_SAMPLES_PASSED, q);
	glEndQuery(GL_SAMPLES_PASSED);

	/* Conditional render that should not draw full screen. */
	glBeginConditionalRenderNV(q, GL_QUERY_WAIT_NV);
	glColor4f(1.0, 0.0, 0.0, 0.0);
	glRasterPos2i(-1, -1);
	glBitmap(piglit_width, piglit_height, 0, 0, 0, 0, buf);
	glEndConditionalRenderNV();

	pass = piglit_probe_rect_rgba(0, 0, piglit_width, piglit_height,
				      green);

	piglit_present_results();

	glDeleteQueries(1, &q);
	free(buf);

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	piglit_require_gl_version(20);

	piglit_require_extension("GL_NV_conditional_render");
}
