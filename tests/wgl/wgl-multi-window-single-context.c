/*
 * Copyright © 2017 VMware, Inc.
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

/*
 * Test rendering into multiple windows with one context.
 *
 * Authors:
 *    Brian Paul
 */

#include <assert.h>
#include <stdio.h>
#include "piglit-util-gl.h"
#include "piglit-wgl-util.h"


#define MAX_WINDOWS 8

static HWND win[MAX_WINDOWS];
static int num_windows = MAX_WINDOWS;
static HGLRC ctx;

static const float colors[MAX_WINDOWS][4] = {
	{1, 0, 0, 1},
	{0, 1, 0, 1},
	{0, 0, 1, 1},
	{0, 1, 1, 1},
	{1, 0, 1, 1},
	{1, 1, 0, 1},
	{1, 1, 1, 1},
	{.5, .5, .5, 1},
};


enum piglit_result
draw(void)
{
	int i;
	bool pass = true;

	/* draw colored quad in each window */
	for (i = 0; i < num_windows; i++) {
		wglMakeCurrent(GetDC(win[i]), ctx);

		glClear(GL_COLOR_BUFFER_BIT);
		glColor4fv(colors[i]);
		piglit_draw_rect(-1, -1, 2, 2);
	}

	/* probe windows */
	for (i = 0; i < num_windows; i++) {
		wglMakeCurrent(GetDC(win[i]), ctx);

		glReadBuffer(GL_BACK);
		/* only read back 20x20 region instead of
		 * piglit_width x piglit_height since Windows may
		 * resize our windows.
		 */
		int p = piglit_probe_rect_rgb(0, 0, 20, 20,
					      colors[i]);

		SwapBuffers(GetDC(win[i]));

		if (!p) {
			printf("Failed probe in window %d\n", i);
			pass = false;
		}
	}

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


int
main(int argc, char **argv)
{
	int i;

	piglit_width = 100;
	piglit_height = 100;

	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-auto") == 0) {
			piglit_automatic = 1;
			break;
		}
	}

	/* Create windows */
	for (i = 0; i < num_windows; i++) {
		win[i] = piglit_get_wgl_window();
		MoveWindow(win[i], i*130, 0,
			   piglit_width, piglit_height, FALSE);
		assert(win[i]);
	}

	ctx = piglit_get_wgl_context(win[0]);
	assert(ctx);

	if (!wglMakeCurrent(GetDC(win[0]), ctx)) {
		fprintf(stderr, "wglMakeCurrent failed\n");
		return 0;
	}

	piglit_dispatch_default_init(PIGLIT_DISPATCH_GL);

	piglit_wgl_event_loop(draw);

	return 0;
}
