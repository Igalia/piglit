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

static bool try_attribute(int attribute)
{
	const EGLint attribs[] = {
		attribute,
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
	if (!piglit_check_egl_error(EGL_BAD_ATTRIBUTE))
		piglit_report_result(PIGLIT_FAIL);

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

	if (!EGL_KHR_create_context_setup(EGL_OPENGL_BIT)) {
		fprintf(stderr, "Desktop GL not available.\n");
		piglit_report_result(PIGLIT_SKIP);
	}
	eglBindAPI(EGL_OPENGL_API);

	for (i = 0; i < ARRAY_SIZE(bad_attributes); i++) {
		pass = try_attribute(bad_attributes[i]) && pass;
	}

	EGL_KHR_create_context_teardown();

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);

	return EXIT_SUCCESS;
}
