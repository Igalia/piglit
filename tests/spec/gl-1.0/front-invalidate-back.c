/*
 * Copyright © 2013 Intel Corporation
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/** @file fbo-front-invalidate-back.c
 *
 * This test validates a corner case in the Intel mesa driver: if GL
 * calls that require access to the front buffer are followed by GL
 * calls that don't require access to the front buffer, and an
 * invalidate event is received from the server in between, then
 * before the driver responds to the invalidate event by requesting a
 * new back buffer, it needs to flush the pending front buffer
 * rendering.  Otherwise the front buffer rendering will be lost.
 *
 * Unfortunately, we can't force the server to send an invalidate
 * event to the driver at a specific time; however, thanks to a quirk
 * in the driver, we can simulate one using a sequence of
 * glDrawBuffer() calls.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.requires_displayed_window = true;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

void
piglit_init(int argc, char **argv)
{
}

enum piglit_result
piglit_display(void)
{
	const float green[] = {0.0f, 1.0f, 0.0f};
	bool pass;

	/* Do some rendering that requires access to the front buffer
	 * (clear it to green).
	 */
	glDrawBuffer(GL_FRONT);
	glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	/* Simulate an invalidate event using a sequence of
	 * glDrawBuffer() calls.  This takes advantage of a quirk of
	 * the Intel driver: when glDrawBuffer() is used to switch
	 * from back buffer to front buffer rendering, the driver acts
	 * as though a buffer invalidate event has occurred, so that
	 * when the next draw operation occurs, it will be sure to
	 * pick up a valid front buffer.  Therefore, we can simulate
	 * an invalidate event by switching to GL_BACK and then to
	 * GL_FRONT again.
	 */
	glDrawBuffer(GL_BACK);
	glDrawBuffer(GL_FRONT);

	/* Do some rendering that doesn't require access to the front
	 * buffer (clear the back buffer to red).  Note:
	 * gl_ReadBuffer(GL_BACK) ensures that the driver doesn't try
	 * to maintain access to the front buffer.
	 */
	glReadBuffer(GL_BACK);
	glDrawBuffer(GL_BACK);
	glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	/* Check that the front buffer rendering was not lost. */
	glReadBuffer(GL_FRONT);
	pass = piglit_probe_rect_rgb(0, 0, piglit_width, piglit_height,
				     green);

	/* Since we don't do a buffer swap, flush to make sure
	 * rendering gets to the screen.
	 */
	glFlush();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
