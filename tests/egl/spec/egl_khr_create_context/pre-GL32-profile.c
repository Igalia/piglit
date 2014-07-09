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
#include "common.h"

static bool try_version(int major, int minor)
{
	EGLContext ctx;

	const EGLint attribs_with_profile[] = {
		EGL_CONTEXT_MAJOR_VERSION_KHR,
		major,

		EGL_CONTEXT_MINOR_VERSION_KHR,
		minor,

		EGL_CONTEXT_OPENGL_PROFILE_MASK_KHR,
		EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT_KHR,

		EGL_NONE
	};

	const EGLint attribs_without_profile[] = {
		EGL_CONTEXT_MAJOR_VERSION_KHR,
		major,

		EGL_CONTEXT_MINOR_VERSION_KHR,
		minor,

		EGL_NONE
	};

	/* The EGL_KHR_create_context spec says:
	 *
	 *     "When the current rendering API is EGL_OPENGL_API, the value of
	 *     EGL_CONTEXT_OPENGL_PROFILE_MASK_KHR requests an OpenGL context
	 *     supporting the corresponding profile... If the requested OpenGL
	 *     version is less than 3.2, EGL_CONTEXT_OPENGL_PROFILE_MASK_KHR is
	 *     ignored and the functionality of the context is determined solely
	 *     by the requested version."
	 *
	 * Try to create a context without any profile specified.  If that
	 * works, try to create a context with the core profile specified.
	 * That should also work.
	 */
	ctx = eglCreateContext(egl_dpy, cfg, EGL_NO_CONTEXT,
			       attribs_without_profile);

	if (ctx == EGL_NO_CONTEXT)
		return true;

	eglDestroyContext(egl_dpy, ctx);

	ctx = eglCreateContext(egl_dpy, cfg, EGL_NO_CONTEXT,
			       attribs_with_profile);
	if (ctx != EGL_NO_CONTEXT) {
		eglDestroyContext(egl_dpy, ctx);
		return true;
	} else {
		fprintf(stderr,
			"Failed to create %d.%d context with core profile "
			"(profile value should be\nignored)\n",
			major, minor);
		return false;
	}
}

int main(int argc, char **argv)
{
	bool pass = true;

	if (!EGL_KHR_create_context_setup(EGL_OPENGL_BIT)) {
		fprintf(stderr, "Desktop GL not available.\n");
		piglit_report_result(PIGLIT_SKIP);
	}
	eglBindAPI(EGL_OPENGL_API);

	pass = try_version(1, 0) && pass;
	pass = try_version(1, 1) && pass;
	pass = try_version(1, 2) && pass;
	pass = try_version(1, 3) && pass;
	pass = try_version(1, 4) && pass;
	pass = try_version(1, 5) && pass;
	pass = try_version(2, 0) && pass;
	pass = try_version(2, 1) && pass;
	pass = try_version(3, 0) && pass;
	pass = try_version(3, 1) && pass;

	EGL_KHR_create_context_teardown();

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
	return 0;
}
