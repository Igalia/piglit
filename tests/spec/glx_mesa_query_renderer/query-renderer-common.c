/* Copyright © 2013 Intel Corporation
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
#include "query-renderer-common.h"

PFNGLXQUERYRENDERERSTRINGMESAPROC piglit_glXQueryRendererStringMESA = NULL;
PFNGLXQUERYCURRENTRENDERERSTRINGMESAPROC piglit_glXQueryCurrentRendererStringMESA = NULL;
PFNGLXQUERYRENDERERINTEGERMESAPROC piglit_glXQueryRendererIntegerMESA = NULL;
PFNGLXQUERYCURRENTRENDERERINTEGERMESAPROC piglit_glXQueryCurrentRendererIntegerMESA = NULL;
PFNGLXCREATECONTEXTATTRIBSARBPROC piglit_glXCreateContextAttribsARB = NULL;

static void *
get_and_verify_proc(const char *name)
{
	void *func;

	func = glXGetProcAddress((GLubyte *) name);
	if (func == NULL) {
		fprintf(stderr, "Could not get function pointer for %s\n",
			name);
		piglit_report_result(PIGLIT_FAIL);
	}

	return func;
}

void
initialize_function_pointers(Display *dpy)
{
	piglit_require_glx_extension(dpy, "GLX_MESA_query_renderer");

	piglit_glXQueryRendererStringMESA =
		(PFNGLXQUERYRENDERERSTRINGMESAPROC)
		get_and_verify_proc("glXQueryRendererStringMESA");
	piglit_glXQueryCurrentRendererStringMESA =
		(PFNGLXQUERYCURRENTRENDERERSTRINGMESAPROC)
		get_and_verify_proc("glXQueryCurrentRendererStringMESA");
	piglit_glXQueryRendererIntegerMESA =
		(PFNGLXQUERYRENDERERINTEGERMESAPROC)
		get_and_verify_proc("glXQueryRendererIntegerMESA");
	piglit_glXQueryCurrentRendererIntegerMESA =
		(PFNGLXQUERYCURRENTRENDERERINTEGERMESAPROC)
		get_and_verify_proc("glXQueryCurrentRendererIntegerMESA");

	piglit_require_glx_extension(dpy, "GLX_ARB_create_context");

	piglit_glXCreateContextAttribsARB =
		(PFNGLXCREATECONTEXTATTRIBSARBPROC)
		get_and_verify_proc("glXCreateContextAttribsARB");
}
