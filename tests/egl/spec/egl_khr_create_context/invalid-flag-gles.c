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
	if (!piglit_check_egl_error(EGL_BAD_ATTRIBUTE))
		piglit_report_result(PIGLIT_FAIL);

	return pass;
}


int main(int argc, char **argv)
{
	bool pass = true;
	uint32_t flag;
	bool ran_test = false;

	/* According to the EGL_KHR_create_context spec, version 15, there
	 * exists exactly one valid flag for OpenGL ES contexts: the debug
	 * flag.
	 *
	 *     If the EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR flag bit is set in
	 *     EGL_CONTEXT_FLAGS_KHR, then a <debug context> will be created.
	 *     [...] This bit is supported for OpenGL and OpenGL ES contexts.
	 */
	const EGLint valid_flags = EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR;
	const EGLint invalid_flags = ~valid_flags;

	if (EGL_KHR_create_context_setup(EGL_OPENGL_ES_BIT)) {
		ran_test = true;
		flag = 0x80000000;

		while (flag) {
			if (flag & invalid_flags) {
                           pass = pass && try_flag(flag);
			}

                        flag >>= 1;
		}

		EGL_KHR_create_context_teardown();
	}

	if (EGL_KHR_create_context_setup(EGL_OPENGL_ES2_BIT)) {
		ran_test = true;
		flag = 0x80000000;

		while (flag) {
			if (flag & invalid_flags) {
				pass = pass && try_flag(flag);
			}

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
