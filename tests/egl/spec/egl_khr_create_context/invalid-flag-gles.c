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

static bool try_flag(uint32_t flag)
{
	const EGLint attribs[] = {
		EGL_CONTEXT_FLAGS_KHR, flag,
		EGL_NONE
	};
	bool pass = true;

	ctx = eglCreateContext(egl_dpy, cfg, EGL_NO_CONTEXT, attribs);
	if (ctx != NULL) {
		fprintf(stderr,
			"Created GLES context with invalid flag 0x%08x, "
			"but this should have failed.\n",
			flag);
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
	bool pass = true;
	uint32_t flag = 0x80000000;
	bool ran_test = false;

	/* The EGL_KHR_create_context spec says:
	 *
	 *     "The value for attribute EGL_CONTEXT_FLAGS_KHR specifies a set of
	 *     flag bits affecting the context. Flags are only defined for OpenGL
	 *     context creation, and specifying a flags value other than zero for
	 *     other types of contexts, including OpenGL ES contexts, will generate
	 *     an error."
	 */
	uint32_t first_valid_flag = 0;

	if (EGL_KHR_create_context_setup(EGL_OPENGL_ES_BIT)) {
		ran_test = true;
		while (flag != first_valid_flag) {
			pass = pass && try_flag(flag);
			flag >>= 1;
		}

		EGL_KHR_create_context_teardown();
	}

	if (EGL_KHR_create_context_setup(EGL_OPENGL_ES2_BIT)) {
		ran_test = true;
		flag = 0x80000000;
		while (flag != first_valid_flag) {
			pass = pass && try_flag(flag);
			flag >>= 1;
		}

		EGL_KHR_create_context_teardown();
	}

	if (!ran_test) {
		piglit_report_result(PIGLIT_SKIP);
	}

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);

	return EXIT_SUCCESS;
}
