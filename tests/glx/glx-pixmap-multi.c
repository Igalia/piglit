/*
 * Copyright 2011 Red Hat, Inc.
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
 *      Adam Jackson <ajax@redhat.com>
 *
 */

/** @file glx-pixmap-multi.c
 *
 * Nothing in the protocol prohibits you from creating multiple GLXPixmaps
 * attached to the same X pixmap, so one must assume it's allowed.
 */

#include "piglit-util-gl.h"
#include "piglit-glx-util.h"

int piglit_width = 50, piglit_height = 50;
static Display *dpy;
static XVisualInfo *visinfo;

int pass = 1;

static int
handler(Display *dpy, XErrorEvent *err)
{
	pass = 0;

#if 0
	printf("Error serial %x error %x request %x minor %x xid %x\n",
	       err->serial, err->error_code, err->request_code, err->minor_code,
	       err->resourceid);
#endif

	return 0;
}

int
main(int argc, char **argv)
{
	Pixmap p;
	GLXPixmap g1, g2;
	GLXFBConfig fbc;

	dpy = XOpenDisplay(NULL);
	if (dpy == NULL) {
		fprintf(stderr, "couldn't open display\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	visinfo = piglit_get_glx_visual(dpy);

	XSetErrorHandler(handler);

	p = XCreatePixmap(dpy, DefaultRootWindow(dpy), piglit_width,
			  piglit_height, visinfo->depth);
	g1 = glXCreateGLXPixmap(dpy, visinfo, p);
	g2 = glXCreateGLXPixmap(dpy, visinfo, p);

	glXDestroyGLXPixmap(dpy, g1);
	glXDestroyGLXPixmap(dpy, g2);

	fbc = piglit_glx_get_fbconfig_for_visinfo(dpy, visinfo);
	g1 = glXCreatePixmap(dpy, fbc, p, NULL);
	g2 = glXCreatePixmap(dpy, fbc, p, NULL);
	glXDestroyPixmap(dpy, g1);
	glXDestroyPixmap(dpy, g2);

	XFreePixmap(dpy, p);

	XSync(dpy, 0);

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);

	return 0;
}
