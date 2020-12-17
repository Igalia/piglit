/*
 * Copyright 2020 Intel Corporation
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

#include "piglit-util.h"
#include "piglit-util-gl.h"
#include "piglit-util-egl.h"
#include "../../egl-util.h"

#ifndef EGL_PROTECTED_CONTENT_EXT
#define EGL_PROTECTED_CONTENT_EXT         0x32C0
#endif

static enum piglit_result
test_draw(struct egl_state *state)
{
	piglit_draw_rect(-0.8f, -0.8f, 1.6f, 1.6f);

	glFlush();

	// SwapBuffers is undefined operation. If your implementation
	// supports it, you should be able to visualize encrypted data.
	//
	// eglSwapBuffers(state->egl_dpy, state->surf);

	return eglGetError() != EGL_SUCCESS ? PIGLIT_FAIL : PIGLIT_PASS;
}

int
main(int argc, char **argv)
{
	const EGLint conf_attribs[] = {
 		EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
		EGL_NONE
	};
	const EGLint ctx_attribs[] = {
		EGL_PROTECTED_CONTENT_EXT, true,
		EGL_NONE,
	};
	const EGLint surf_attribs[] = {
		EGL_PROTECTED_CONTENT_EXT, true,
		EGL_NONE,
	};
	const char *extensions[] = { "EGL_EXT_protected_content", NULL };


	struct egl_test test;
	egl_init_test(&test);
	test.extensions = extensions;
	test.config_attribs = conf_attribs;
	test.context_attribs = ctx_attribs;
	test.surface_attribs = surf_attribs;
	test.draw = test_draw;
	test.window_width = 320;
	test.window_height = 240;

	if (egl_util_run(&test, argc, argv) != PIGLIT_PASS)
		return EXIT_FAILURE;
	return EXIT_SUCCESS;
}
