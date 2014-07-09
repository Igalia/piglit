/*
 * Copyright © 2011 Henri Verbeet <hverbeet@gmail.com>
 * Copyright 2011 Red Hat, Inc.
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
 */

/** @file fbo-sys-sub-blit.c
 *
 * Test FBO blits involving a subset of the window-system buffer.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGB;
	config.requires_displayed_window = true;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result piglit_display(void)
{
	const float green[] = {0.0f, 1.0f, 0.0f};
	const float red[] = {1.0f, 0.0f, 0.0f};
	int w = piglit_width;
	int h = piglit_height;
	bool success = 1;

	glDrawBuffer(GL_BACK);

	/* paint the back buffer green */
	glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	/* blit back to front */
	glDrawBuffer(GL_FRONT);
	glReadBuffer(GL_BACK);
	glBlitFramebufferEXT(0, 0, w, h, 0, 0, w, h, GL_COLOR_BUFFER_BIT, GL_LINEAR);

	/* paint the back buffer red */
	glDrawBuffer(GL_BACK);
	glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	/* paint a square in the middle of the front buffer red */
	glDrawBuffer(GL_FRONT);
	glReadBuffer(GL_BACK);
	glBlitFramebufferEXT(w / 4, h / 4, 3 * w / 4, 3 * h / 4,
			     w / 4, h / 4, 3 * w / 4, 3 * h / 4,
			     GL_COLOR_BUFFER_BIT, GL_LINEAR);

	glReadBuffer(GL_FRONT);

	/* the middle should be red */
	success &= piglit_probe_pixel_rgb(w / 2, h / 2, red);

	/* the corners should be green */
	success &= piglit_probe_pixel_rgb(0, 0, green);
	success &= piglit_probe_pixel_rgb(w - 1, 0, green);
	success &= piglit_probe_pixel_rgb(0, h - 1, green);
	success &= piglit_probe_pixel_rgb(w - 1, h - 1, green);

	glFlush();

	return success ? PIGLIT_PASS : PIGLIT_FAIL;
}

void piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_EXT_framebuffer_object");
	piglit_require_extension("GL_EXT_framebuffer_blit");
}
