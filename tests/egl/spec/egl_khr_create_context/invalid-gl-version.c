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

static bool try_version(int major, int minor)
{
	const EGLint attribs[] = {
		EGL_CONTEXT_MAJOR_VERSION_KHR, major,
		EGL_CONTEXT_MINOR_VERSION_KHR, minor,
		EGL_NONE
	};
	bool pass = true;

	ctx = eglCreateContext(egl_dpy, cfg, EGL_NO_CONTEXT, attribs);
	if (ctx != NULL) {
		fprintf(stderr,
			"Created OpenGL context with invalid version %d.%d\n",
			major, minor);
		eglDestroyContext(egl_dpy, ctx);
		pass = false;
	}

	/* The EGL_KHR_create_context spec says:
	 *
	 *     "If an OpenGL context is requested and the values for attributes
	 *     EGL_CONTEXT_MAJOR_VERSION_KHR and EGL_CONTEXT_MINOR_VERSION_KHR,
	 *     when considered together with the value for attribute
	 *     EGL_CONTEXT_FORWARD_COMPATIBLE_BIT_KHR, specify an OpenGL
	 *     version and feature set that are not defined, than an
	 *     EGL_BAD_MATCH error is generated."
	 */
	if (!piglit_check_egl_error(EGL_BAD_MATCH))
		piglit_report_result(PIGLIT_FAIL);

	return pass;
}

int main(int argc, char **argv)
{
	bool pass = true;
	bool ran_test = false;

	/* The EGL_KHR_create_context spec says:
	 *
	 *     "The defined versions of OpenGL at the time of writing are OpenGL
	 *     1.0, 1.1, 1.2, 1.3, 1.4, 1.5, 2.0, 2.1, 3.0, 3.1, 3.2, 4.0, 4.1,
	 *     and 4.2. Feature deprecation was introduced with OpenGL 3.0, so
	 *     forward-compatible contexts may only be requested for OpenGL 3.0
	 *     and above. Thus, examples of invalid combinations of attributes
	 *     include:
	 *
	 *       - Major version < 1 or > 4
	 *       - Major version == 1 and minor version < 0 or > 5
	 *       - Major version == 2 and minor version < 0 or > 1
	 *       - Major version == 3 and minor version < 0 or > 2
	 *       - Major version == 4 and minor version < 0 or > 2
	 *       - Forward-compatible flag set and major version < 3
	 */
	if (EGL_KHR_create_context_setup(EGL_OPENGL_ES_BIT)) {
		ran_test = true;
		pass = pass && try_version(-1, 0);
		pass = pass && try_version(0, 0);
		pass = pass && try_version(0, -1);
		pass = pass && try_version(1, 2);

		EGL_KHR_create_context_teardown();
	}
	if (EGL_KHR_create_context_setup(EGL_OPENGL_ES2_BIT)) {
		ran_test = true;
		pass = pass && try_version(2, -1);
		pass = pass && try_version(2, 1);

		EGL_KHR_create_context_teardown();
	}
	if (EGL_KHR_create_context_setup(EGL_OPENGL_BIT)) {
		ran_test = true;
		eglBindAPI(EGL_OPENGL_API);
		pass = pass && try_version(-1, 0);
		pass = pass && try_version(0, 0);
		pass = pass && try_version(1, -1);
		pass = pass && try_version(1, 6);
		pass = pass && try_version(2, -1);
		pass = pass && try_version(2, 2);
		pass = pass && try_version(3, -1);

		/* There is no expectation that 3.4 will ever exist because it would
		 * have to include functionality not in 4.0, and that would be weird.
		 */
		pass = pass && try_version(3, 4);

		EGL_KHR_create_context_teardown();
	}

	if (!ran_test) {
		piglit_report_result(PIGLIT_SKIP);
	}

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);

	return EXIT_SUCCESS;
}
