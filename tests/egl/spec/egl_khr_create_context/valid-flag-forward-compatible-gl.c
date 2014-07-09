/* Copyright © 2012 Intel Corporation
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
#include "piglit-util-egl.h"
#include "common.h"

int gl_version;

static bool try_flag(int flag)
{
	const EGLint attribs[] = {
		EGL_CONTEXT_FLAGS_KHR, flag,
		EGL_NONE
	};

	ctx = eglCreateContext(egl_dpy, cfg, EGL_NO_CONTEXT, attribs);
	if (ctx != NULL) {
		/* Get GL version in order to know whether we can test
		 * EGL_CONTEXT_OPENGL_FORWARD_COMPATIBLE_BIT_KHR.
		 */
		if (flag == 0) {
			if (!eglMakeCurrent(egl_dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, ctx)) {
				fprintf(stderr, "eglMakeCurrent() failed\n");
				piglit_report_result(PIGLIT_FAIL);
			}

			piglit_dispatch_default_init(PIGLIT_DISPATCH_GL);

			gl_version = piglit_get_gl_version();
		}
		eglDestroyContext(egl_dpy, ctx);
	} else if (!piglit_check_egl_error(EGL_BAD_MATCH)) {
		/* The EGL_KHR_create_context spec says:
		 *
		 *     "* If <config> does not support a client API context compatible
		 *        with the requested API major and minor version, context flags,
		 *        and context reset notification behavior (for client API types
		 *        where these attributes are supported), then an EGL_BAD_MATCH
		 *        error is generated.
		 */
		piglit_report_result(PIGLIT_FAIL);
	}

	return true;
}

int main(int argc, char **argv)
{
	bool pass = true;

	if (!EGL_KHR_create_context_setup(EGL_OPENGL_BIT)) {
		fprintf(stderr, "Desktop GL not available.\n");
		piglit_report_result(PIGLIT_SKIP);
	}
	eglBindAPI(EGL_OPENGL_API);

	/* The EGL_KHR_create_context spec says:
	 *
	 *    "The default value of EGL_CONTEXT_FLAGS_KHR is zero."
	 */
	pass = pass && try_flag(0);
	if (gl_version >= 30) {
		pass = pass && try_flag(EGL_CONTEXT_OPENGL_FORWARD_COMPATIBLE_BIT_KHR);
	} else {
		piglit_report_result(PIGLIT_SKIP);
	}

	EGL_KHR_create_context_teardown();

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);

	return EXIT_SUCCESS;
}
