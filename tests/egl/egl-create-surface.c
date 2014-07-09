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
 *
 * Author: Kristian Høgsberg <krh@bitplanet.net>
 */

/** @file egl-create-surface.c
 *
 * Test EGLCreate*Surface behaviour.
 * EGL 1.4, section 3.5.1 "Creating On-Screen Rendering Surfaces":
 *
 *   On failure eglCreateWindowSurface returns EGL_NO_SURFACE.  [...]
 *   If there is already an EGLConfig associated with win (as a result
 *   of a previous eglCreateWindowSurface call), then an EGL_BAD_ALLOC
 *   error is generated.
 */

#include "piglit-util-gl.h"
#include "egl-util.h"

static enum piglit_result
draw(struct egl_state *state)
{
	EGLSurface surf;

	/* egl-util.c already has created state->surf from state->win,
	 * so this should fail. */

	surf = eglCreateWindowSurface(state->egl_dpy,
				      state->cfg, state->win, NULL);
	if (surf != EGL_NO_SURFACE) {
		fprintf(stderr, "eglCreateWindowSurface() didn't fail\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	if (eglGetError() != EGL_BAD_ALLOC) {
		fprintf(stderr, "eglCreateWindowSurface() "
			"error wasn't EGL_BAD_ALLOC\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	return PIGLIT_PASS;
}

int
main(int argc, char *argv[])
{
	struct egl_test test;

	egl_init_test(&test);
	test.draw = draw;

	if (egl_util_run(&test, argc, argv) != PIGLIT_PASS)
		return EXIT_FAILURE;
	return EXIT_SUCCESS;
}
