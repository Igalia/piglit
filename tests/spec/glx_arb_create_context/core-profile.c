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
#include "piglit-util.h"
#include "piglit-glx-util.h"
#include "common.h"

int main(int argc, char **argv)
{
	static const int compatibility_attribs[] = {
		GLX_CONTEXT_PROFILE_MASK_ARB,
		GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
		GLX_CONTEXT_MAJOR_VERSION_ARB,
		3,
		GLX_CONTEXT_MINOR_VERSION_ARB,
		2,
		None
	};

	static const int core_attribs[] = {
		GLX_CONTEXT_PROFILE_MASK_ARB,
		GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
		GLX_CONTEXT_MAJOR_VERSION_ARB,
		3,
		GLX_CONTEXT_MINOR_VERSION_ARB,
		2,
		None
	};

	bool pass = true;
	bool got_core_with_profile = false;
	bool got_core_without_profile = false;
	bool got_compatibility = false;
	GLXContext ctx;


	GLX_ARB_create_context_setup();
	piglit_require_glx_extension(dpy, "GLX_ARB_create_context_profile");

	/* The GLX_ARB_create_context_profile spec says:
	 *
	 *     "All OpenGL 3.2 implementations are required to implement the
	 *     core profile, but implementation of the compatibility profile
	 *     is optional."
	 *
	 * If it is possible to create a context with the compatibility
	 * profile, then it must also be possible to create a context with the
	 * core profile.  Conversely, if it is not possible to create a
	 * context with the core profile, it must also not possible to create
	 * a context with the compatibility profile.
	 */
	ctx = glXCreateContextAttribsARB(dpy, fbconfig, NULL, True,
					 core_attribs);
	XSync(dpy, 0);

	if (ctx != NULL) {
		glXDestroyContext(dpy, ctx);
		got_core_with_profile = true;
	} else {
		/* The GLX_ARB_create_context_profile spec says:
		 *
		 *     "* If <config> does not support compatible OpenGL
		 *        contexts providing the requested API major and minor
		 *        version, forward-compatible flag, and debug context
		 *        flag, GLXBadFBConfig is generated."
		 */
		if (!validate_glx_error_code(Success, GLXBadFBConfig))
			pass = false;
	}

	/* The GLX_ARB_create_context_profile extension spec says:
	 *
	 *     "The default value for GLX_CONTEXT_PROFILE_MASK_ARB is
	 *     GLX_CONTEXT_CORE_PROFILE_BIT_ARB."
	 */
	ctx = glXCreateContextAttribsARB(dpy, fbconfig, NULL, True,
					 core_attribs + 2);
	XSync(dpy, 0);

	if (ctx != NULL) {
		glXDestroyContext(dpy, ctx);
		got_core_without_profile = true;
	} else {
		/* The GLX_ARB_create_context_profile spec says:
		 *
		 *     "* If <config> does not support compatible OpenGL
		 *        contexts providing the requested API major and minor
		 *        version, forward-compatible flag, and debug context
		 *        flag, GLXBadFBConfig is generated."
		 */
		if (!validate_glx_error_code(Success, GLXBadFBConfig))
			pass = false;
	}

	ctx = glXCreateContextAttribsARB(dpy, fbconfig, NULL, True,
					 compatibility_attribs);
	XSync(dpy, 0);

	if (ctx != NULL) {
		glXDestroyContext(dpy, ctx);
		got_compatibility = true;
	} else {
		/* The GLX_ARB_create_context_profile spec says:
		 *
		 *     "* If <config> does not support compatible OpenGL
		 *        contexts providing the requested API major and minor
		 *        version, forward-compatible flag, and debug context
		 *        flag, GLXBadFBConfig is generated."
		 */
		if (!validate_glx_error_code(Success, GLXBadFBConfig))
			pass = false;
	}

	GLX_ARB_create_context_teardown();

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
