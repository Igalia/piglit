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

static bool try_profile(int profile)
{
	const int attribs[] = {
		GLX_CONTEXT_PROFILE_MASK_ARB, profile,
		None
	};
	GLXContext ctx;
	bool pass = true;

	ctx = glXCreateContextAttribsARB(dpy, fbconfig, NULL, True, attribs);
	XSync(dpy, 0);

	if (ctx != NULL) {
		fprintf(stderr,
			"Created OpenGL context with invalid profile "
			"0x%08x, but this should have failed.\n",
			profile);
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
	 */
	if (!validate_glx_error_code(Success, GLXBadProfileARB)) {
		fprintf(stderr, "Profile = 0x%08x\n", profile);
		pass = false;
	}

	return pass;
}

int main(int argc, char **argv)
{
	bool pass = true;
	uint32_t i;

	GLX_ARB_create_context_setup();
	piglit_require_glx_extension(dpy, "GLX_ARB_create_context_profile");

	/* The GLX_ARB_create_context_profile spec says:
	 *
	 *     "* If attribute GLX_CONTEXT_PROFILE_MASK_ARB has no bits set;
	 *        has any bits set other than GLX_CONTEXT_CORE_PROFILE_BIT_ARB
	 *        and GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB; has more than
	 *        one of these bits set...then GLXBadProfileARB is generated."
	 */
	pass = try_profile(0)
		&& pass;

	pass = try_profile(GLX_CONTEXT_CORE_PROFILE_BIT_ARB
			   | GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB)
		&& pass;

	/* This loop will need to be modified as new profiles are defined by
	 * the GLX spec.  The conditional code below for
	 * GLX_EXT_create_context_es2_profile is an example of how this should
	 * be handled.
	 */
	for (i = 0x00000008; i != 0; i <<= 1) {
		pass = try_profile(i)
			&& pass;
	}

	if (!piglit_is_glx_extension_supported(dpy, "GLX_EXT_create_context_es2_profile")) {
		pass = try_profile(GLX_CONTEXT_ES2_PROFILE_BIT_EXT)
			&& pass;
	}

	GLX_ARB_create_context_teardown();

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
	return 0;
}
