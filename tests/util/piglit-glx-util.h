/*
 * Copyright © 2009 Intel Corporation
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
 * Authors:
 *    Eric Anholt <eric@anholt.net>
 *
 */

#pragma once

#include "X11/Xmd.h"
#include "GL/glx.h"
#include "GL/glxproto.h"

#ifndef GLXBadProfileARB
#define GLXBadProfileARB 13
#endif

Display *piglit_get_glx_display();
XVisualInfo * piglit_get_glx_visual(Display *dpy);
GLXContext piglit_get_glx_context(Display *dpy, XVisualInfo *visinfo);
GLXContext piglit_get_glx_context_share(Display *dpy, XVisualInfo *visinfo, GLXContext share);
Window piglit_get_glx_window(Display *dpy, XVisualInfo *visinfo);
Window piglit_get_glx_window_fullscreen(Display *dpy, XVisualInfo *visinfo);
Window piglit_get_glx_window_unmapped(Display *dpy, XVisualInfo *visinfo);
bool piglit_is_glx_extension_supported(Display *dpy, const char *name);
void piglit_require_glx_extension(Display *dpy, const char *name);
void piglit_require_glx_version(Display *dpy, int major, int minor);
void piglit_glx_event_loop(Display *dpy,
			   enum piglit_result (*draw)(Display *dpy));
void piglit_glx_set_no_input(void);
void piglit_glx_window_set_no_input(Display *dpy, GLXDrawable win);

enum piglit_result
piglit_glx_iterate_visuals(enum piglit_result (*draw)(Display *dpy,
						      GLXFBConfig config));

enum piglit_result
piglit_glx_iterate_pixmap_fbconfigs(enum piglit_result (*draw)(Display *dpy,
							       GLXFBConfig config));

GLXFBConfig
piglit_glx_get_fbconfig_for_visinfo(Display *dpy, XVisualInfo *visinfo);
int
piglit_glx_get_error(Display *dpy, XErrorEvent *err);

const char *
piglit_glx_error_string(int err);

struct piglit_glx_proc_reference {
	__GLXextFuncPtr *procedure;
	const char *name;
};

#define PIGLIT_GLX_PROC(var, name) { (__GLXextFuncPtr *)&(var), #name }

void
piglit_glx_get_all_proc_addresses(const struct piglit_glx_proc_reference *procedures,
				  unsigned num);
