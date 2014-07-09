/* Copyright © 2012 Intel Corporation
 * Copyright © 2014 Advanced Micro Devices, Inc.
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
#include "minmax-test.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	bool pass = true;
	float green[] = {0, 1, 0, 0};
	float clear[] = {0, 0, 0, 0};

	glClear(GL_COLOR_BUFFER_BIT);

	glEdgeFlag(GL_TRUE);

	glBegin(GL_POLYGON);
	glVertex2f(1.5, 1.5);
	glVertex2f(5.5, 1.5);
	glVertex2f(5.5, 5.5);
	glVertex2f(1.5, 5.5);
	glEnd();

	/* This prevents merging the 2 glBegin/End parts and turning the edge
	 * flag into a per-vertex attrib. */
	glFlush();

	glEdgeFlag(GL_FALSE);

	glBegin(GL_POLYGON);
	glVertex2f(11.5, 1.5);
	glVertex2f(15.5, 1.5);
	glVertex2f(15.5, 5.5);
	glVertex2f(11.5, 5.5);
	glEnd();

	pass = piglit_probe_pixel_rgba(3, 1, green) && pass;
	pass = piglit_probe_pixel_rgba(3, 5, green) && pass;
	pass = piglit_probe_pixel_rgba(1, 3, green) && pass;
	pass = piglit_probe_pixel_rgba(5, 3, green) && pass;

	pass = piglit_probe_pixel_rgba(13, 1, clear) && pass;
	pass = piglit_probe_pixel_rgba(13, 5, clear) && pass;
	pass = piglit_probe_pixel_rgba(11, 3, clear) && pass;
	pass = piglit_probe_pixel_rgba(15, 3, clear) && pass;

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	piglit_ortho_projection(piglit_width, piglit_height, false);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glColor4f(0, 1, 0, 0);
}
