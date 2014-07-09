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

static bool try_profile(int profile)
{
	const EGLint attribs[] = {
		EGL_CONTEXT_OPENGL_PROFILE_MASK_KHR, profile,
		/* We have to ask for GL >=3.2 here, otherwise profile
		 * attributes are ignored, even if they are invalid.
		 */
		EGL_CONTEXT_MAJOR_VERSION_KHR, 3,
		EGL_CONTEXT_MINOR_VERSION_KHR, 2,
		EGL_NONE
	};
	EGLContext ctx;
	bool pass = true;

	ctx = eglCreateContext(egl_dpy, cfg, EGL_NO_CONTEXT, attribs);

	if (ctx != EGL_NO_CONTEXT) {
		fprintf(stderr,
			"Created OpenGL context with invalid profile "
			"0x%08x, but this should have failed.\n",
			profile);
		eglDestroyContext(egl_dpy, ctx);
		pass = false;
	}

	/* The EGL_KHR_create_context spec says:
	 *
	 *     "* If an OpenGL context is requested, the requested version is
	 *        greater than [sic] 3.2, and the value for attribute
	 *        EGL_CONTEXT_OPENGL_PROFILE_MASK_KHR has no bits set; has any
	 *        bits set other than EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT_KHR
	 *        and EGL_CONTEXT_OPENGL_COMPATIBILITY_PROFILE_BIT_KHR; has more
	 *        than one of these bits set; or if the implementation does not
	 *        support the requested profile, then an EGL_BAD_MATCH error is
	 *        generated."
	 */
	if (!piglit_check_egl_error(EGL_BAD_MATCH))
		piglit_report_result(PIGLIT_FAIL);

	return pass;
}

int main(int argc, char **argv)
{
	bool pass = true;
	uint32_t i;

	if (!EGL_KHR_create_context_setup(EGL_OPENGL_BIT)) {
		fprintf(stderr, "Desktop GL not available.\n");
		piglit_report_result(PIGLIT_SKIP);
	}
	eglBindAPI(EGL_OPENGL_API);

	/* The EGL_KHR_create_context spec says:
	 *
	 *     "* If an OpenGL context is requested, the requested version is
	 *        greater than [sic] 3.2, and the value for attribute
	 *        EGL_CONTEXT_OPENGL_PROFILE_MASK_KHR has no bits set; has any
	 *        bits set other than EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT_KHR
	 *        and EGL_CONTEXT_OPENGL_COMPATIBILITY_PROFILE_BIT_KHR; has more
	 *        than one of these bits set; or if the implementation does not
	 *        support the requested profile, then an EGL_BAD_MATCH error is
	 *        generated."
	 */
	pass = try_profile(0)
		&& pass;

	pass = try_profile(EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT_KHR
			   | EGL_CONTEXT_OPENGL_COMPATIBILITY_PROFILE_BIT_KHR)
		&& pass;

	for (i = EGL_CONTEXT_OPENGL_COMPATIBILITY_PROFILE_BIT_KHR << 1; i != 0; i <<= 1) {
		pass = try_profile(i)
			&& pass;
	}

	EGL_KHR_create_context_teardown();

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
	return 0;
}
