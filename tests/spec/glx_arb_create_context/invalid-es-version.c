/* Copyright © 2011 Intel Corporation
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

static bool try_version(int major, int minor)
{
	const int attribs[] = {
		GLX_CONTEXT_MAJOR_VERSION_ARB,
		major,
		GLX_CONTEXT_MINOR_VERSION_ARB,
		minor,
		GLX_CONTEXT_PROFILE_MASK_ARB,
		GLX_CONTEXT_ES2_PROFILE_BIT_EXT,
		None
	};
	GLXContext ctx;
	bool pass = true;

	ctx = glXCreateContextAttribsARB(dpy, fbconfig, NULL, True, attribs);
	XSync(dpy, 0);

	if (ctx != NULL) {
		fprintf(stderr,
			"Created OpenGL ES context with invalid version "
			"%d.%d\n",
			major, minor);
		glXDestroyContext(dpy, ctx);
		pass = false;
	}

	/* The GLX_ARB_create_context_profile spec says:
	 *
	 *     "* If attribute GLX_CONTEXT_PROFILE_MASK_ARB has no bits set;
	 *        has any bits set other than GLX_CONTEXT_CORE_PROFILE_BIT_ARB
	 *        and GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB; has more than
	 *        one of these bits set; or if the implementation does not
	 *        support the requested profile, then GLXBadProfileARB is
	 *        generated."
	 *
	 * Implementations that support GLX_EXT_create_context_es2_profile can
	 * only support GLX_CONTEXT_ES2_PROFILE_BIT_EXT with version 2.0.
	 * Therefore, they cannot support that profile with any other version,
	 * and GLXBadProfileARB should be generated.
	 */
	if (!validate_glx_error_code(Success, GLXBadProfileARB)) {
		if (ctx == NULL)
			fprintf(stderr, "Version = %d.%d\n", major, minor);

		pass = false;
	}

	return pass;
}

int main(int argc, char **argv)
{
	bool pass = true;

	GLX_ARB_create_context_setup();
	piglit_require_glx_extension(dpy, "GLX_ARB_create_context_profile");
	piglit_require_glx_extension(dpy, "GLX_EXT_create_context_es2_profile");

	/* The latest version of GLX_EXT_create_context_es2_profile spec says:
	 *
	 *     "If the version requested is a valid and supported OpenGL-ES
	 *     version, and the GLX_CONTEXT_ES_PROFILE_BIT_EXT bit is set in
	 *     the GLX_CONTEXT_PROFILE_MASK_ARB attribute (see below), then the
	 *     context returned will implement the OpenGL ES version
	 *     requested."
	 *
	 * Try a bunch of OpenGL ES versions that don't exist.
	 */
	pass = try_version(1, 2) && pass;
	pass = try_version(2, 1) && pass;
	pass = try_version(3, 2) && pass;

	GLX_ARB_create_context_teardown();

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
	return 0;
}
