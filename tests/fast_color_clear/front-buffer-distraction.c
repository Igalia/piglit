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

/**
 * \file front-buffer-distraction.c
 *
 * Test that a fast color clear of the back buffer is properly
 * resolved even if we try to "distract" the implementation by forcing
 * a fast color clear resolve in the front buffer.  This verifies that
 * either (a) fast color clears are independently tracked between the
 * front and back buffers, or (b) the implementation resolves fast
 * clears before switching from back buffer rendering to front buffer
 * rendering.
 *
 * The test operates by fast clearing the back buffer, then fast
 * clearing the front buffer, then reading from the front buffer
 * (forcing a front buffer resolve), then reading from the back buffer
 * (forcing a back buffer resolve).
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 11;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;
PIGLIT_GL_TEST_CONFIG_END


void
piglit_init(int argc, char **argv)
{
}

enum piglit_result
piglit_display(void)
{
	bool pass = true;
	static const GLfloat green[] = { 0.0, 1.0, 0.0, 1.0 };
	static const GLfloat red[] = { 1.0, 0.0, 0.0, 1.0 };

	/* Clear the back buffer to green */
	glClearColor(0, 1, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	/* Clear the front buffer to red */
	glDrawBuffer(GL_FRONT);
	glClearColor(1, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);
	glDrawBuffer(GL_BACK);

	/* Read from the front buffer and make sure that it's red */
	glReadBuffer(GL_FRONT);
	pass = piglit_probe_rect_rgba(0, 0, piglit_width,
				      piglit_height, red) && pass;
	glReadBuffer(GL_BACK);

	/* Read from the back buffer and make sure that it's green */
	pass = piglit_probe_rect_rgba(0, 0, piglit_width,
				      piglit_height, green) && pass;

	if (!piglit_check_gl_error(GL_NO_ERROR))
		pass = false;

	piglit_present_results();
	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
