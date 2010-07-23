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

#include "piglit-util.h"
#include "piglit-glx-util.h"

int piglit_automatic;

XVisualInfo *
piglit_get_glx_visual(Display *dpy)
{
	XVisualInfo *visinfo;
	int attrib[] = {
		GLX_RGBA,
		GLX_RED_SIZE, 1,
		GLX_GREEN_SIZE, 1,
		GLX_BLUE_SIZE, 1,
		GLX_DOUBLEBUFFER,
		None
	};
	int screen = DefaultScreen(dpy);

	visinfo = glXChooseVisual(dpy, screen, attrib);
	if (visinfo == NULL) {
		fprintf(stderr,
			"Couldn't get an RGBA, double-buffered visual\n");
		piglit_report_result(PIGLIT_FAILURE);
		exit(1);
	}

	return visinfo;
}

GLXContext
piglit_get_glx_context(Display *dpy, XVisualInfo *visinfo)
{
	GLXContext ctx;

	ctx = glXCreateContext(dpy, visinfo, NULL, True);
	if (ctx == None) {
		fprintf(stderr, "glXCreateContext failed\n");
		piglit_report_result(PIGLIT_FAILURE);
	}

	return ctx;
}

Window
piglit_get_glx_window(Display *dpy, XVisualInfo *visinfo)
{
	XSetWindowAttributes window_attr;
	unsigned long mask;
	int screen = DefaultScreen(dpy);
	Window root_win = RootWindow(dpy, screen);
	Window win;

	window_attr.background_pixel = 0;
	window_attr.border_pixel = 0;
	window_attr.colormap = XCreateColormap(dpy, root_win,
					       visinfo->visual, AllocNone);
	window_attr.event_mask = StructureNotifyMask | ExposureMask |
		KeyPressMask;
	mask = CWBackPixel | CWBorderPixel | CWColormap | CWEventMask;
	win = XCreateWindow(dpy, root_win, 0, 0,
			    piglit_width, piglit_height,
			    0, visinfo->depth, InputOutput,
			    visinfo->visual, mask, &window_attr);

	XMapWindow(dpy, win);

	return win;
}

void
piglit_require_glx_extension(Display *dpy, const char *name)
{
	const char *glx_extension_list;
	int screen = DefaultScreen(dpy);

	/* This is a bogus way of checking for the extension.
	 * Needs more GLEW.
	 */
	glx_extension_list = glXQueryExtensionsString(dpy, screen);
	if (strstr(glx_extension_list, name) == NULL) {
		fprintf(stderr, "Test requires %s\n", name);
		piglit_report_result(PIGLIT_SKIP);
	}
}


void
piglit_glx_event_loop(Display *dpy, enum piglit_result (*draw)(Display *dpy))
{
	for (;;) {
		XEvent event;
		XNextEvent (dpy, &event);

		if (event.type == KeyPress) {
			KeySym sym = XKeycodeToKeysym (dpy,
						       event.xkey.keycode, 0);

			if (sym == XK_Escape || sym == XK_q || sym == XK_Q)
				break;
			else
				draw(dpy);
		} else if (event.type == Expose) {
			enum piglit_result result = draw(dpy);

			if (piglit_automatic) {
				XCloseDisplay(dpy);
				piglit_report_result(result);
				break;
			}
		}
        }
}


void
piglit_glx_set_no_input(void)
{
	Display *d;
	GLXDrawable win;
	XWMHints *hints;

	d = glXGetCurrentDisplay();
	win = glXGetCurrentDrawable();

	hints = XAllocWMHints();
	hints->flags |= InputHint;
	hints->input = False;

	XSetWMHints(d, win, hints);

	XFree(hints);
}
