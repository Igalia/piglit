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
#include "piglit-util-gl-common.h"
#include "piglit-util-egl.h"
#include "common.h"

static bool try_attribute(int attribute)
{
	/* If the attribute is EGL_CONTEXT_OPENGL_PROFILE_MASK_KHR, use a valid
	 * value for that attribute.  This ensures that the attribute is
	 * rejected for the correct reason.
	 */
	const EGLint attribs[] = {
		attribute,
		(attribute == EGL_CONTEXT_OPENGL_PROFILE_MASK_KHR)
		? EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT_KHR : 0,
		EGL_NONE
	};
	bool pass = true;

	ctx = eglCreateContext(egl_dpy, cfg, EGL_NO_CONTEXT, attribs);
	if (ctx != NULL) {
		fprintf(stderr,
			"Created OpenGL context with invalid attribute "
			"0x%08x, but this should have failed.\n",
			attribute);
		eglDestroyContext(egl_dpy, ctx);
		pass = false;
	}

	/* The EGL_KHR_create_context spec says:
	 *
	 *     "* If an attribute name or attribute value in <attrib_list> is not
	 *        recognized (including unrecognized bits in bitmask attributes),
	 *        then an EGL_BAD_ATTRIBUTE error is generated."
	 */
	piglit_expect_egl_error(EGL_BAD_ATTRIBUTE, PIGLIT_FAIL);

	return pass;
}

int main(int argc, char **argv)
{
	EGLint bad_attributes[] = {
		0xffff0000,
		EGL_SAMPLE_BUFFERS
	};
	bool pass = true;
	unsigned i;
	int gl_version;

	if (!EGL_KHR_create_context_setup(EGL_OPENGL_BIT)) {
		fprintf(stderr, "Desktop GL not available.\n");
		piglit_report_result(PIGLIT_SKIP);
	}
	eglBindAPI(EGL_OPENGL_API);

	for (i = 0; i < ARRAY_SIZE(bad_attributes); i++) {
		pass = try_attribute(bad_attributes[i]) && pass;
	}

	/* Create a context so that we can determine the GL version. */
	ctx = eglCreateContext(egl_dpy, cfg, EGL_NO_CONTEXT, NULL);
	if (ctx == EGL_NO_CONTEXT) {
		fprintf(stderr, "eglCreateContext() failed\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	if (!eglMakeCurrent(egl_dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, ctx)) {
		fprintf(stderr, "eglMakeCurrent() failed\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	piglit_dispatch_default_init();

	gl_version = piglit_get_gl_version();

	eglMakeCurrent(egl_dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
	eglDestroyContext(egl_dpy, ctx);

	/* The EGL_KHR_create_context spec says:
	 *
	 * "* If an OpenGL context is requested, the requested version is
	 *    greater than [sic] 3.2, and the value for attribute
	 *    EGL_CONTEXT_OPENGL_PROFILE_MASK_KHR has no bits set; has any bits set
	 *    other than EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT_KHR and
	 *    EGL_CONTEXT_OPENGL_COMPATIBILITY_PROFILE_BIT_KHR; has more than one of
	 *    these bits set; or if the implementation does not support the
	 *    requested profile, then an EGL_BAD_PROFILE_KHR error is
	 *    generated."
	 */

	if (gl_version >= 32) {
		pass = try_attribute(EGL_CONTEXT_OPENGL_PROFILE_MASK_KHR)
		       && pass;
	}

	EGL_KHR_create_context_teardown();

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);

	return EXIT_SUCCESS;
}
