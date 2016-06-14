/*
 * Copyright © 2016 Intel Corporation
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

/** @file egl-create-msaa-pbuffer-surface.c
 *
 * Test EGLCreatePBufferSurface behaviour with EGL_SAMPLES set.
 */

#include "piglit-util-gl.h"
#include "egl-util.h"

static bool draw_called = false;

static enum piglit_result
draw(struct egl_state *state)
{
	EGLSurface surf;
	const EGLint srfPbufferAttr[] =
	{
		EGL_WIDTH, 256,
		EGL_HEIGHT, 256,
		EGL_NONE
	};

	draw_called = true;

	surf = eglCreatePbufferSurface(state->egl_dpy,
				       state->cfg, srfPbufferAttr);

	if (eglGetError() != EGL_SUCCESS || surf == EGL_NO_SURFACE) {
		fprintf(stderr, "eglCreatePbufferSurface failed\n");
		return PIGLIT_FAIL;
	}

	eglDestroySurface(state->egl_dpy, surf);
	return PIGLIT_PASS;
}

int
main(int argc, char *argv[])
{
	struct egl_test test;
	enum piglit_result test_result;
	const EGLint test_attribs[] =
	{
		EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
		EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
		EGL_SAMPLES, 4,
		EGL_NONE
	};

	egl_init_test(&test);
	test.draw = draw;
	test.stop_on_failure = false;
	test.config_attribs = test_attribs;

	test_result = egl_util_run(&test, argc, argv);

	/* Such EGLConfig may not exist, in this case test fails to init
         * and event_loop never calls draw.
         */
	if (!draw_called) {
		fprintf(stderr, "could not init such EGLconfig, skip ...\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	piglit_report_result(test_result);
}
