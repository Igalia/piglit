/*
 * Copyright 2012 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#pragma once

#include <waffle.h>

#ifdef PIGLIT_HAS_X11
#	include <X11/Xlib.h>
#endif

#include "glut_waffle.h"

struct glut_window {
        struct waffle_window *waffle;

#ifdef PIGLIT_HAS_X11
	struct {
		Display *display;
		Window window;
	} x11;
#endif

        int id;

        GLUT_EGLreshapeCB reshape_cb;
        GLUT_EGLdisplayCB display_cb;
        GLUT_EGLkeyboardCB keyboard_cb;
};

struct glut_waffle_state {
	/** \brief One of `WAFFLE_PLATFORM_*`. */
	int waffle_platform;

	/** \brief One of `WAFFLE_CONTEXT_OPENGL*`.
	 *
	 * The default value is `WAFFLE_CONTEXT_OPENGL`. To change the value,
	 * call glutInitAPIMask().
	 */
	int waffle_context_api;

	/** \brief A bitmask of enum glut_display_mode`. */
	int display_mode;

	int window_width;
	int window_height;

	struct waffle_display *display;
	struct waffle_context *context;
	struct glut_window *window;

	int redisplay;
	int window_id_pool;
};

extern struct glut_waffle_state *const _glut;

void
glutFatal(char *format, ...);

void
glutFatalWaffleError(const char *waffle_func);
