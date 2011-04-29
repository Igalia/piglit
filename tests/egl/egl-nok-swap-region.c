/*
 * Copyright © 2010 Intel Corporation
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
 * Author: Kristian Høgsberg <krh@bitplanet.net>
 */

/** @file egl-nok-swap-region.c
 *
 * Test EGL_NOK_swap_region.
 */

#include "piglit-util.h"
#include "egl-util.h"

#ifdef EGL_NOK_swap_region

const char *extensions[] = { "EGL_NOK_swap_region", NULL };

static enum piglit_result
draw(struct egl_state *state)
{
	EGLint rects[] = { 
		10, 10, 10, 10, 
		20, 20, 20, 10,
		40, 30, 10, 20,
		50, 50, 10, 10
	};
	PFNEGLSWAPBUFFERSREGIONNOK swap_buffers_region;
	float red[] = { 1.0, 0.0, 0.0, 1.0};
	int i;

	swap_buffers_region = (PFNEGLSWAPBUFFERSREGIONNOK) 
		eglGetProcAddress("eglSwapBuffersRegionNOK");

	if (swap_buffers_region == NULL) {
		fprintf(stderr, "could not getproc eglSwapBuffersRegionNOK");
		piglit_report_result(PIGLIT_PASS);
	}

	/* Clear background to green */
	glClearColor(0.0, 1.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	eglSwapBuffers(state->egl_dpy, state->surf);
	glClearColor(1.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	swap_buffers_region(state->egl_dpy, state->surf, 4, rects);

	for (i = 0; i < 16; i += 4)
		if (!piglit_probe_pixel_rgba(rects[i] + 5,
					     rects[i + 1] + 5, red))
			return PIGLIT_FAIL;

	return PIGLIT_PASS;
}

static const struct egl_test test = {
	.extensions = extensions,
	.draw = draw
};

int
main(int argc, char *argv[])
{
	return egl_util_run(&test, argc, argv);
}

#else

int
main(int argc, char *argv[])
{
	piglit_report_result(PIGLIT_SKIP);

	return 0;
}

#endif
