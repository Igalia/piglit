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

int main(int argc, char **argv)
{
	static const EGLint compatibility_attribs[] = {
		EGL_CONTEXT_OPENGL_PROFILE_MASK_KHR,
		EGL_CONTEXT_OPENGL_COMPATIBILITY_PROFILE_BIT_KHR,
		EGL_CONTEXT_MAJOR_VERSION_KHR,
		3,
		EGL_CONTEXT_MINOR_VERSION_KHR,
		2,
		EGL_NONE
	};

	static const EGLint core_attribs[] = {
		EGL_CONTEXT_OPENGL_PROFILE_MASK_KHR,
		EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT_KHR,
		EGL_CONTEXT_MAJOR_VERSION_KHR,
		3,
		EGL_CONTEXT_MINOR_VERSION_KHR,
		2,
		EGL_NONE
	};

	bool pass = true;
	bool got_core_with_profile = false;
	bool got_core_without_profile = false;
	bool got_compatibility = false;
	EGLContext ctx;

	if (!EGL_KHR_create_context_setup(EGL_OPENGL_BIT)) {
		fprintf(stderr, "Desktop GL not available.\n");
		piglit_report_result(PIGLIT_SKIP);
	}
	eglBindAPI(EGL_OPENGL_API);

	/* The EGL_KHR_create_context spec says:
	 *
	 *     "All OpenGL 3.2 and later implementations are required to
	 *     implement the core profile, but implementation of the
	 *     compatibility profile is optional."
	 *
	 * If it is possible to create a context with the compatibility
	 * profile, then it must also be possible to create a context with the
	 * core profile.  Conversely, if it is not possible to create a
	 * context with the core profile, it must also not be possible to create
	 * a context with the compatibility profile.
	 */
	ctx = eglCreateContext(egl_dpy, cfg, EGL_NO_CONTEXT, core_attribs);

	if (ctx != EGL_NO_CONTEXT) {
		eglDestroyContext(egl_dpy, ctx);
		got_core_with_profile = true;
	} else if (!piglit_check_egl_error(EGL_BAD_MATCH)) {
		/* The EGL_KHR_create_context spec says:
		 *
		 *     "* If <config> does not support a client API context
		 *        compatible with the requested API major and minor
		 *        version, context flags, and context reset notification
		 *        behavior (for client API types where these attributes
		 *        are supported), then an EGL_BAD_MATCH error is
		 *        generated."
		 */
		piglit_report_result(PIGLIT_FAIL);
	}

	/* The EGL_KHR_create_context spec says:
	 *
	 *     "The default value for EGL_CONTEXT_PROFILE_MASK_KHR is
	 *     EGL_CONTEXT_CORE_PROFILE_BIT_KHR."
	 */
	ctx = eglCreateContext(egl_dpy, cfg, EGL_NO_CONTEXT, core_attribs + 2);

	if (ctx != EGL_NO_CONTEXT) {
		eglDestroyContext(egl_dpy, ctx);
		got_core_without_profile = true;
	} else if (!piglit_check_egl_error(EGL_BAD_MATCH)) {
		/* The EGL_KHR_create_context spec says:
		 *
		 *     "* If <config> does not support a client API context
		 *        compatible with the requested API major and minor
		 *        version, context flags, and context reset notification
		 *        behavior (for client API types where these attributes
		 *        are supported), then an EGL_BAD_MATCH error is
		 *        generated."
		 */
		piglit_report_result(PIGLIT_FAIL);
	}

	ctx = eglCreateContext(egl_dpy, cfg, EGL_NO_CONTEXT, compatibility_attribs);

	if (ctx != EGL_NO_CONTEXT) {
		eglDestroyContext(egl_dpy, ctx);
		got_compatibility = true;
	} else if (!piglit_check_egl_error(EGL_BAD_MATCH)) {
		/* The EGL_KHR_create_context spec says:
		 *
		 *     "* If <config> does not support a client API context
		 *        compatible with the requested API major and minor
		 *        version, context flags, and context reset notification
		 *        behavior (for client API types where these attributes
		 *        are supported), then an EGL_BAD_MATCH error is
		 *        generated."
		 */
		piglit_report_result(PIGLIT_FAIL);
	}

	EGL_KHR_create_context_teardown();

	if (!(got_core_with_profile || got_core_without_profile)
	    && got_compatibility) {
		fprintf(stderr,
			"Compatibility profile context was created, but core "
			"context was not.\n");
		pass = false;
	}

	/* Creation of a core context with or without the core profile mask
	 * should have the same result.
	 */
	if (got_core_with_profile != got_core_without_profile) {
		fprintf(stderr,
			"Core profile context was created %s profile mask "
			"but not %s profile mask.\n",
			got_core_with_profile ? "with" : "without",
			got_core_with_profile ? "without" : "with");
		pass = false;
	}

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
	return 0;
}
