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

/** @file egl-nok-texture-from-pixmap.c
 *
 * Test EGL_NOK_texture_from_pixmap
 */

#include "piglit-util-gl.h"
#include "egl-util.h"

#ifdef EGL_NOK_texture_from_pixmap

const char *extensions[] = { "EGL_NOK_texture_from_pixmap", NULL };

static const EGLint pixmap_attribs[] = {
	EGL_TEXTURE_FORMAT,	EGL_TEXTURE_RGB,
	EGL_TEXTURE_TARGET,	EGL_TEXTURE_2D,
	EGL_NONE
};

static enum piglit_result
draw(struct egl_state *state)
{
	EGLSurface *pixmap;
	EGLint inv;
	float red[] = { 0.4, 0.0, 0.0, 1.0 };
	float purple[] = { 0.5, 0.0, 0.5, 1.0 };

	if (!eglGetConfigAttrib(state->egl_dpy, state->cfg,
				EGL_Y_INVERTED_NOK, &inv)) {
		fprintf(stderr,
			"eglGetConfigAttrib(EGL_Y_INVERTED_NOK) failed\n");
		return PIGLIT_FAIL;
	}
	
	printf("EGL_Y_INVERTED_NOK: %s\n", inv ? "TRUE" : "FALSE");

	pixmap = egl_util_create_pixmap(state, 100, 100, pixmap_attribs);
	if (!eglMakeCurrent(state->egl_dpy, pixmap, pixmap, state->ctx)) {
		fprintf(stderr, "eglMakeCurrent() failed\n");
		piglit_report_result(PIGLIT_FAIL);
	}
	
	/* Clear pixmap to purple */
	glClearColor(0.5, 0.0, 0.5, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	if (!eglMakeCurrent(state->egl_dpy,
			    state->surf, state->surf, state->ctx)) {
		fprintf(stderr, "eglMakeCurrent() failed\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	glViewport(0, 0, state->width, state->height);
	piglit_ortho_projection(state->width, state->height, GL_FALSE);

	glClearColor(0.4, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glEnable(GL_TEXTURE_2D);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	eglBindTexImage(state->egl_dpy, pixmap, EGL_BACK_BUFFER);
	piglit_draw_rect_tex(20, 20, 100, 100,  0, 0, 1, 1);
	eglSwapBuffers(state->egl_dpy, state->surf);	

	if (!piglit_probe_pixel_rgba(10, 10, red) ||
	    !piglit_probe_pixel_rgba(50, 10, red) ||
	    !piglit_probe_pixel_rgba(10, 50, red) ||
	    !piglit_probe_pixel_rgba(50, 50, purple) ||
	    !piglit_probe_pixel_rgba(110, 110, purple) ||
	    !piglit_probe_pixel_rgba(130, 130, red))
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
