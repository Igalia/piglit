/*
 * Copyright © 2012 Intel Corporation
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

/** @file common.c
 *
 * Support code for running tests of GLX_OML_sync_control.
 */

#include "piglit-util-gl.h"
#include "piglit-glx-util.h"
#include "common.h"

PFNGLXGETSYNCVALUESOMLPROC __piglit_glXGetSyncValuesOML;
PFNGLXGETMSCRATEOMLPROC __piglit_glXGetMscRateOML;
PFNGLXSWAPBUFFERSMSCOMLPROC __piglit_glXSwapBuffersMscOML;
PFNGLXWAITFORMSCOMLPROC __piglit_glXWaitForMscOML;
PFNGLXWAITFORSBCOMLPROC __piglit_glXWaitForSbcOML;
Window win;
XVisualInfo *visinfo;

void
piglit_oml_sync_control_test_run(enum piglit_result (*draw)(Display *dpy))
{
	Display *dpy;
	GLXContext ctx;
	const int proc_count = 5;
	__GLXextFuncPtr *procs[proc_count];
	const char *names[proc_count];
	int i;

#define ADD_FUNC(name)							\
	do {								\
		procs[i] = (__GLXextFuncPtr *)&(__piglit_##name);	\
		names[i] = #name;					\
		i++;							\
	} while (0)

	i = 0;
	ADD_FUNC(glXGetSyncValuesOML);
	ADD_FUNC(glXGetMscRateOML);
	ADD_FUNC(glXSwapBuffersMscOML);
	ADD_FUNC(glXWaitForMscOML);
	ADD_FUNC(glXWaitForSbcOML);

	dpy = XOpenDisplay(NULL);
	if (dpy == NULL) {
		fprintf(stderr, "couldn't open display\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	piglit_require_glx_extension(dpy, "GLX_OML_sync_control");
	piglit_glx_get_all_proc_addresses(procs, names, ARRAY_SIZE(procs));

	visinfo = piglit_get_glx_visual(dpy);
	win = piglit_get_glx_window(dpy, visinfo);
	ctx = piglit_get_glx_context(dpy, visinfo);
	glXMakeCurrent(dpy, win, ctx);

	piglit_dispatch_default_init(PIGLIT_DISPATCH_GL);

	XMapWindow(dpy, win);

	piglit_glx_event_loop(dpy, draw);
}
