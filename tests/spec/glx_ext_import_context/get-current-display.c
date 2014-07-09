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

#include "piglit-util-gl.h"
#include "piglit-glx-util.h"
#include "common.h"

int main(int argc, char **argv)
{
	bool pass = true;
	Display *got_dpy;
	Window win;

	GLX_EXT_import_context_setup();

	/* No context is current, so glXGetCurrentDisplayEXT should return
	 * NULL.
	 */
	got_dpy = glXGetCurrentDisplayEXT();
	if (got_dpy != NULL) {
		fprintf(stderr,
			"Got %p display with no context active, should be "
			"NULL.\n",
			got_dpy);
		pass = false;
	}

	/* Make a context current.  glXGetCurrentDisplayEXT should return the
	 * dpy that was passed to glXMakeCurrent.
	 */
	win = piglit_get_glx_window(dpy, visinfo);
	glXMakeCurrent(dpy, win, indirectCtx);

	got_dpy = glXGetCurrentDisplayEXT();
	if (got_dpy != dpy) {
		fprintf(stderr,
			"Got %p display, expected %p.\n",
			got_dpy, dpy);
		pass = false;
	}

	GLX_EXT_import_context_teardown();

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
	return 0;
}
