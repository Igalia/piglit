/*
 * Copyright © 2014 VMware, Inc.
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
 * @file swapbuffers-behavior.c
 *
 * Test/check behavior of SwapBuffers.  In some environments, SwapBuffers
 * just copies the back buffer to the front.  Other times it's a true swap.
 * This test just checks and reports which behavior is found.
 */

#include "piglit-util-gl.h"


PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;
	config.requires_displayed_window = true;
PIGLIT_GL_TEST_CONFIG_END


static const float blue[4] = { 0.0f, 0.0f, 1.0f, 0.0f };
static const float green[4] = { 0.0f, 1.0f, 0.0f, 0.0f };


static bool
match(const float c1[4], const float c2[4])
{
	return (c1[0] == c2[0] &&
		c1[1] == c2[1] &&
		c1[2] == c2[2] &&
		c1[3] == c2[3]);
}


enum piglit_result
piglit_display(void)
{
	GLfloat color[4];

	glViewport(0, 0, piglit_width, piglit_height);

	/* Clear back buffer to green */
	glDrawBuffer(GL_BACK);
	glClearColor(green[0], green[1], green[2], green[3]);
	glClear(GL_COLOR_BUFFER_BIT);

        /* First swap */
	piglit_swap_buffers();

        /* Front buffer sanity-check */
	glReadBuffer(GL_FRONT);
	if (!piglit_probe_rect_rgb_silent(0, 0, piglit_width, piglit_height,
					  green)) {
		printf("SwapBuffers apparently failed!\n");
                return PIGLIT_FAIL;
	}

	/* Read back buffer */
	glReadBuffer(GL_BACK);
	if (!piglit_probe_rect_rgb_silent(0, 0, piglit_width, piglit_height,
					  green)) {
		printf("After 1st swap, back buffer is no longer green.\n");
	}

	/* Clear front buffer to blue */
	glDrawBuffer(GL_FRONT);
	glClearColor(blue[0], blue[1], blue[2], blue[3]);
	glClear(GL_COLOR_BUFFER_BIT);

        /* Second swap */
	piglit_swap_buffers();

	/* Read back buffer */
	glReadBuffer(GL_BACK);
	glReadPixels(piglit_width/2, piglit_height/2, 1, 1, GL_RGBA, GL_FLOAT,
		     color);
	if (match(color, green)) {
	        printf("After 2nd swap: back buffer is green. "
		       "SwapBuffers is a back-to-front copy.\n");
	}
	else if (match(color, blue)) {
	        printf("After 2nd swap: back buffer is blue. "
		       "SwapBuffers is a true swap.\n");
	}
	else {
		printf("Back color: %g, %g, %g, %g. Back buffer undefined.\n",
		       color[0], color[1], color[2], color[3]);
	}

	fflush(stdout);

	return PIGLIT_PASS;
}


void
piglit_init(int argc, char **argv)
{
	/* nothing */
}
