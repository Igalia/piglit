/* Copyright © 2011 Intel Corporation
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
#include <ctype.h>
#include <errno.h>
#include "piglit-util-gl.h"
#include "piglit-glx-util.h"
#include "common.h"

PFNGLXCREATECONTEXTATTRIBSARBPROC __piglit_glXCreateContextAttribsARB = NULL;

Display *dpy = NULL;
GLXFBConfig fbconfig = None;
XVisualInfo *visinfo = NULL;
Window win = None;
GLXWindow glxWin = None;

int glx_error_code = -1;
int x_error_code = Success;

int piglit_height = 50;
int piglit_width = 50;

static int (*old_handler)(Display *, XErrorEvent *);

static int x_error_handler(Display *dpy, XErrorEvent *e)
{

	x_error_code = e->error_code;
	glx_error_code = piglit_glx_get_error(dpy, e);

	return 0;
}

bool
parse_version_string(const char *string, int *major, int *minor)
{
	char *next;
	int ma;
	int mi;

	if (string == NULL)
		return false;

	errno = 0;
	ma = strtol(string, &next, 10);
	if (next == NULL || errno != 0)
		return false;

	while (!isdigit(*next) && *next != '\0')
		next++;

	if (next[0] == '\0')
		return false;

	mi = strtol(next, &next, 10);
	if (errno != 0)
		return false;

	*major = ma;
	*minor = mi;
	return true;
}

void
GLX_ARB_create_context_setup(void)
{
	dpy = piglit_get_glx_display();

	piglit_require_glx_version(dpy, 1, 4);
	piglit_require_glx_extension(dpy, "GLX_ARB_create_context");

	__piglit_glXCreateContextAttribsARB =
		(PFNGLXCREATECONTEXTATTRIBSARBPROC)
		glXGetProcAddress((const GLubyte *)
				  "glXCreateContextAttribsARB");
	assert(__piglit_glXCreateContextAttribsARB != NULL);

	visinfo = piglit_get_glx_visual(dpy);
	fbconfig = piglit_glx_get_fbconfig_for_visinfo(dpy, visinfo);

	win = piglit_get_glx_window_unmapped(dpy, visinfo);
	glxWin = glXCreateWindow(dpy, fbconfig, win, NULL);

	piglit_glx_get_error(dpy, NULL);
	old_handler = XSetErrorHandler(x_error_handler);
}

void
GLX_ARB_create_context_teardown(void)
{
	if (glxWin != None) {
		glXDestroyWindow(dpy, glxWin);
		glxWin = None;
	}

	XFree(visinfo);
	visinfo = NULL;

	XSetErrorHandler(old_handler);
}

bool validate_glx_error_code(int expected_x_error, int expected_glx_error)
{
	bool pass = true;

	if (expected_glx_error == -1
	    && expected_x_error == Success
	    && (glx_error_code != -1 || x_error_code != Success)) {
		fprintf(stderr,
			"X error %d (%s (%d)) was generated, but "
			"no error was expected.\n",
			x_error_code,
			piglit_glx_error_string(glx_error_code),
			glx_error_code);
		pass = false;
	}

	if (expected_glx_error != -1
	    && glx_error_code != expected_glx_error) {
		fprintf(stderr,
			"X error %d (%s (%d)) was generated, but "
			"%s (%d) was expected.\n",
			x_error_code,
			piglit_glx_error_string(glx_error_code),
			glx_error_code,
			piglit_glx_error_string(expected_glx_error),
			expected_glx_error);
		pass = false;
	} else if (expected_x_error != Success
		   && x_error_code != expected_x_error) {
		fprintf(stderr,
			"X error %d (%s (%d)) was generated, but "
			"X error %d was expected.\n",
			x_error_code,
			piglit_glx_error_string(glx_error_code),
			glx_error_code,
			expected_x_error);
		pass = false;
	}

	x_error_code = Success;
	glx_error_code = -1;
	return pass;
}
