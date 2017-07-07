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
 * Basic WGL sanity check: create a window, context, clear window to green.
 *
 * Authors:
 *    Brian Paul
 */

#include <assert.h>
#include <stdio.h>
#include "piglit-util-gl.h"
#include "piglit-wgl-util.h"

HWND hWnd;
HGLRC ctx;


enum piglit_result
draw(void)
{
	static const float green[4] = { 0, 1, 0, 1 };
	bool pass = true;

	glClearColor(0, 1, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	if (!piglit_probe_rect_rgba(0, 0, piglit_width, piglit_height,
				   green)) {
		pass = false;
	}

	SwapBuffers(GetDC(hWnd));

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


int
main(int argc, char **argv)
{
	int i;

	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-auto") == 0) {
			piglit_automatic = 1;
			break;
		}
	}

	hWnd = piglit_get_wgl_window();
	assert(hWnd);

	ctx = piglit_get_wgl_context(hWnd);
	assert(ctx);

	if (!wglMakeCurrent(GetDC(hWnd), ctx)) {
		fprintf(stderr, "wglMakeCurrent failed\n");
		return 0;
	}

	piglit_dispatch_default_init(PIGLIT_DISPATCH_GL);

	piglit_wgl_event_loop(draw);

	return 0;
}
