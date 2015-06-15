/*
 * Copyright © 2010, 2014 Intel Corporation
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
 * Authors: Neil Roberts <neil.s.roberts@intel.com>
 *          Kristian Høgsberg <krh@bitplanet.net>
 */

/** @file egl-configless-context.c
 *
 * Test the EGL_MESA_configless_context extension
 */

/* Chunks of code in this file are taken from egl-util.c */

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "piglit-util-gl.h"
#include "piglit-util-egl.h"

#ifndef EGL_MESA_configless_context
#define EGL_MESA_configless_context 1
#define EGL_NO_CONFIG_MESA			((EGLConfig)0)
#endif

struct state {
	Display *dpy;
	EGLDisplay egl_dpy;
	EGLint egl_major, egl_minor;
	EGLContext ctx;
};

struct window {
	EGLConfig config;
	Window win;
	EGLSurface surface;
};

static EGLint
get_config_attrib(EGLDisplay egl_dpy,
		  EGLConfig config,
		  EGLenum attrib)
{
	EGLBoolean status;
	EGLint value;

	status = eglGetConfigAttrib(egl_dpy,
				    config,
				    attrib,
				    &value);
	if (!status) {
		fprintf(stderr, "eglGetConfigAttrib failed\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	return value;
}

static EGLConfig
choose_config(EGLDisplay egl_dpy,
	      int depth,
	      EGLBoolean has_depth_buffer)
{
	EGLint attribs[32], *a = attribs;
	EGLConfig configs[128];
	EGLBoolean status;
	EGLint config_count, i;
	EGLint buffer_size;
	EGLint depth_size;
	EGLConfig best_config = 0;

	switch (depth) {
	case 16:
		*(a++) = EGL_RED_SIZE;
		*(a++) = 5;
		*(a++) = EGL_GREEN_SIZE;
		*(a++) = 6;
		*(a++) = EGL_BLUE_SIZE;
		*(a++) = 5;
		*(a++) = EGL_ALPHA_SIZE;
		*(a++) = 0;
		break;
	default:
	case 24:
		*(a++) = EGL_RED_SIZE;
		*(a++) = 8;
		*(a++) = EGL_GREEN_SIZE;
		*(a++) = 8;
		*(a++) = EGL_BLUE_SIZE;
		*(a++) = 8;
		*(a++) = EGL_ALPHA_SIZE;
		*(a++) = 0;
		break;
	case 32:
		*(a++) = EGL_RED_SIZE;
		*(a++) = 8;
		*(a++) = EGL_GREEN_SIZE;
		*(a++) = 8;
		*(a++) = EGL_BLUE_SIZE;
		*(a++) = 8;
		*(a++) = EGL_ALPHA_SIZE;
		*(a++) = 8;
		break;
	}

	if (has_depth_buffer) {
		*(a++) = EGL_DEPTH_SIZE;
		*(a++) = 1;
	}

	*(a++) = EGL_RENDERABLE_TYPE;
	*(a++) = EGL_OPENGL_BIT;

	*(a++) = EGL_SURFACE_TYPE;
	*(a++) = EGL_WINDOW_BIT;

	*(a++) = EGL_NONE;

	assert(a - attribs < sizeof attribs / sizeof attribs[0]);

	status = eglChooseConfig(egl_dpy,
				 attribs,
				 configs, sizeof configs / sizeof configs[0],
				 &config_count);
	if (status != EGL_TRUE || config_count == 0) {
		fprintf(stderr, "eglChooseConfig failed\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	for (i = 0; i < config_count; i++) {
		buffer_size = get_config_attrib(egl_dpy,
						configs[i],
						EGL_BUFFER_SIZE);
		depth_size = get_config_attrib(egl_dpy,
					       configs[i],
					       EGL_DEPTH_SIZE);
		if (buffer_size == depth) {
			if (!!depth_size == !!has_depth_buffer)
				return configs[i];
			if (best_config == 0)
				best_config = configs[i];
		}
	}

	fprintf(stderr,
		"warning: couldn't find a %i-bit config "
		"with %s depth buffer\n",
		depth,
		has_depth_buffer ? "a" : "no");

	return best_config ? best_config : configs[0];
}

static struct window *
create_window(struct state *state,
	      int depth,
	      EGLBoolean has_depth_buffer,
	      EGLBoolean double_buffer)
{
	XSetWindowAttributes window_attr;
	XVisualInfo template, *vinfo;
	struct window *window = malloc(sizeof *window);
	int screen = DefaultScreen(state->dpy);
	Window root_win = RootWindow(state->dpy, screen);
	EGLint visual_id;
	EGLint surface_attrs[] = {
		EGL_RENDER_BUFFER,
		double_buffer ? EGL_BACK_BUFFER : EGL_SINGLE_BUFFER,
		EGL_NONE
	};
	int count;

	window->config = choose_config(state->egl_dpy, depth, has_depth_buffer);

	if (!eglGetConfigAttrib(state->egl_dpy,
				window->config,
				EGL_NATIVE_VISUAL_ID,
				&visual_id)) {
		fprintf(stderr, "eglGetConfigAttrib() failed\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	template.visualid = visual_id;
	vinfo = XGetVisualInfo(state->dpy, VisualIDMask, &template, &count);
	if (count != 1) {
		fprintf(stderr, "XGetVisualInfo() failed\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	window_attr.background_pixel = 0;
	window_attr.border_pixel = 0;
	window_attr.colormap = XCreateColormap(state->dpy,
					       root_win,
					       vinfo->visual,
					       AllocNone);
	window->win = XCreateWindow(state->dpy,
				    root_win,
				    0, 0, /* x/y */
				    128, 128, /* width/height */
				    0, /* border_width */
				    vinfo->depth,
				    InputOutput,
				    vinfo->visual,
				    CWBackPixel | CWBorderPixel | CWColormap,
				    &window_attr);

	XMapWindow(state->dpy, window->win);

	XFree(vinfo);

	window->surface = eglCreateWindowSurface(state->egl_dpy,
						 window->config,
						 (NativeWindowType) window->win,
						 surface_attrs);

	return window;
}

static void
bind_window(struct state *state,
	    struct window *window)
{
	EGLBoolean status;

	status = eglMakeCurrent(state->egl_dpy,
				window->surface,
				window->surface,
				state->ctx);
	if (!status) {
		fprintf(stderr, "eglMakeCurrent failed");
		piglit_report_result(PIGLIT_FAIL);
	}
}

static void
run_tests(struct state *state)
{
	struct window *shallow =
		create_window(state, 16, EGL_FALSE, EGL_TRUE);
	struct window *deep =
		create_window(state, 32, EGL_FALSE, EGL_TRUE);
	struct window *with_depth =
		create_window(state, 32, EGL_TRUE, EGL_TRUE);
	struct window *other_window;
	static const GLfloat red[] = { 1.0f, 0.0f, 0.0f, 1.0f };
	static const GLfloat green[] = { 0.0f, 1.0f, 0.0f, 1.0f };
	static const GLfloat blue[] = { 0.0f, 0.0f, 1.0f, 1.0f };
	GLboolean success = GL_TRUE;
	GLboolean status;
	GLint value;

	bind_window(state, shallow);

	piglit_dispatch_default_init(PIGLIT_DISPATCH_GL);

	/* The initial value for glDrawBuffer should have been decided
	 * when the config was first bound to a surface so it should
	 * default to GL_BACK */
	glGetIntegerv(GL_DRAW_BUFFER, &value);
	assert(value == GL_BACK);

	/* Try rendering to the 16-bit buffer */
	glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
	piglit_draw_rect(-1.0f, -1.0f, 2.0f, 2.0f);
	success &= piglit_probe_pixel_rgb(0, 0, red);

	/* And the 32-bit buffer */
	bind_window(state, deep);
	glColor4f(0.0f, 1.0f, 0.0f, 1.0f);
	piglit_draw_rect(-1.0f, -1.0f, 2.0f, 2.0f);
	success &= piglit_probe_pixel_rgb(0, 0, green);

	/* And the one with a depth buffer */
	bind_window(state, with_depth);
	glColor4f(0.0f, 0.0f, 1.0f, 1.0f);
	piglit_draw_rect(-1.0f, -1.0f, 2.0f, 2.0f);
	success &= piglit_probe_pixel_rgb(0, 0, blue);

	/* Make sure the 16-bit buffer is still intact */
	bind_window(state, shallow);
	success &= piglit_probe_pixel_rgb(0, 0, red);

	if (!success)
		piglit_report_result(PIGLIT_FAIL);

	/* Try to find a pair of windows that have different configs
	 * so we can try to bind them together */
	if (shallow->config == deep->config)
		other_window = with_depth;
	else
		other_window = deep;

	/* We shouldn't be allowed to bind incompatible surfaces
	 * together */
	if (shallow->config == other_window->config) {
		fprintf(stderr,
			"warning: not testing binding draw and read surfaces "
			"with different configs\n");
	} else {
		status = eglMakeCurrent(state->egl_dpy,
					shallow->surface,
					other_window->surface,
					state->ctx);
		if (status) {
			fprintf(stderr,
				"Binding incompatible surfaces together "
				"unexpectedly succeeded\n");
			piglit_report_result(PIGLIT_FAIL);
		}
	}
}

int
main(int argc, char **argv)
{
	static const EGLint config_attribs[] = {
		EGL_NONE
	};
	struct state state;

	state.dpy = XOpenDisplay(NULL);
	if (state.dpy == NULL) {
		fprintf(stderr, "couldn't open display\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	eglBindAPI(EGL_OPENGL_API);

	state.egl_dpy = eglGetDisplay(state.dpy);
	if (state.egl_dpy == EGL_NO_DISPLAY) {
		fprintf(stderr, "eglGetDisplay() failed\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	if (!eglInitialize(state.egl_dpy, &state.egl_major, &state.egl_minor)) {
		fprintf(stderr, "eglInitialize() failed\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	if (!piglit_is_egl_extension_supported(state.egl_dpy,
					       "EGL_MESA_configless_context")) {
		fprintf(stderr,
			"The EGL_MESA_configless_context extension "
			"is not supported\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	state.ctx = eglCreateContext(state.egl_dpy,
				     EGL_NO_CONFIG_MESA,
				     EGL_NO_CONTEXT,
				     config_attribs);
	if (state.ctx == EGL_NO_CONTEXT) {
		fprintf(stderr, "eglCreateContext() failed\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	run_tests(&state);

	piglit_report_result(PIGLIT_PASS);

	return 0;
}
