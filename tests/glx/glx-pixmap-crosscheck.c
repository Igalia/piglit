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

/** @file glx-pixmap-crosscheck.c
 *
 * There are three ways to create a GLXPixmap, depending on the GLX version
 * and extensions, and two ways to destroy them.  The spec says you should
 * use the matching destructor for a given constructor, but does not say
 * what to expect if you aren't that fastidious.
 */

#include "piglit-util-gl.h"
#include "piglit-glx-util.h"

int piglit_width = 50, piglit_height = 50;
static Display *dpy;
static XVisualInfo *visinfo;

int result = PIGLIT_PASS;

static int
expect_no_error(Display *dpy, XErrorEvent *err)
{
	/*
	 * Just warn if errors happen, since we're testing something that's
	 * not guaranteed to work.  All we're really looking for here is
	 * a failure to crash.
	 */
	result = PIGLIT_WARN;
	return 0;
}

typedef GLXPixmap (*pfn_create_pixmap)(Display *, GLXFBConfigSGIX, Pixmap);

int
main(int argc, char **argv)
{
	const char *extensions;
	GLXFBConfig fbc;
	Pixmap p;
	GLXPixmap g;
	pfn_create_pixmap create_pixmap_with_config = NULL;

	dpy = XOpenDisplay(NULL);
	if (dpy == NULL) {
		fprintf(stderr, "couldn't open display\n");
		piglit_report_result(PIGLIT_FAIL);
	}

        piglit_glx_get_error(dpy, NULL);
	piglit_require_glx_version(dpy, 1, 3);

	visinfo = piglit_get_glx_visual(dpy);
	fbc = piglit_glx_get_fbconfig_for_visinfo(dpy, visinfo);
	p = XCreatePixmap(dpy, DefaultRootWindow(dpy), piglit_width,
			  piglit_height, visinfo->depth);

	extensions = glXQueryExtensionsString(dpy, DefaultScreen(dpy));
	if (strstr(extensions, "GLX_SGIX_fbconfig")) {
		const GLubyte entrypoint[] = "glXCreateGLXPixmapWithConfigSGIX";
		create_pixmap_with_config =
		    (pfn_create_pixmap)glXGetProcAddressARB(entrypoint);
	}

	XSetErrorHandler(expect_no_error);

	/* pre-1.3 ctor, 1.3 dtor */
	g = glXCreateGLXPixmap(dpy, visinfo, p);
	glXDestroyPixmap(dpy, g);
	XSync(dpy, 0);

	/* extension ctor, 1.3 dtor */
	if (create_pixmap_with_config) {
		g = create_pixmap_with_config(dpy, fbc, p);
		glXDestroyPixmap(dpy, g);
		XSync(dpy, 0);
	}

	/* 1.3 ctor, 1.2 dtor */
	g = glXCreatePixmap(dpy, fbc, p, NULL);
	glXDestroyGLXPixmap(dpy, g);
	XSync(dpy, 0);

	piglit_report_result(result);

	return 0;
}
