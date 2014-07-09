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

#include "piglit-util-gl.h"
#include "egl-util.h"

#ifdef EGL_NOK_swap_region

const char *extensions[] = { "EGL_NOK_swap_region", NULL };

static enum piglit_result
draw(struct egl_state *state)
{
	EGLint rects[] = { 
		10, 10, 10, 10, 
		20, 20, 20, 10, /* wide rect */
		40, 30, 10, 20, /* tall rect */
		50, 50, 10, 10
	};
	float green[] = { 0.0, 1.0, 0.0, 1.0};
	float red[] = { 1.0, 0.0, 0.0, 1.0};
	struct {
		int x, y;
		const float *expected;
	} probes[] = {
		{ 15, 15, red },
		{ 15, state->height - 15, green },

		{ 25, 25, red },
		{ 35, 25, red },
		{ 25, 35, green },
		{ 25, state->height - 25, green },

		{ 45, 35, red },
		{ 45, 45, red },
		{ 55, 35, green },
		{ 45, state->height - 35, green },

		{ 55, 55, red },
		{ 55, state->height - 55, green },
	};
	PFNEGLSWAPBUFFERSREGIONNOK swap_buffers_region;
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

	glFinish();

	for (i = 0; i < ARRAY_SIZE(probes); i++)
		if (!egl_probe_front_pixel_rgb(state,
					       probes[i].x,
					       probes[i].y,
					       probes[i].expected))
			return PIGLIT_FAIL;

	return PIGLIT_PASS;
}

int
main(int argc, char *argv[])
{
	struct egl_test test;

	egl_init_test(&test);
	test.extensions = extensions;
	test.draw = draw;

	if (egl_util_run(&test, argc, argv) != PIGLIT_PASS)
		return EXIT_FAILURE;
	return EXIT_SUCCESS;
}

#else

int
main(int argc, char *argv[])
{
	piglit_report_result(PIGLIT_SKIP);

	return 0;
}

#endif
