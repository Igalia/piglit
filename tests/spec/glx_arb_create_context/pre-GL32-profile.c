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

static bool try_version(int major, int minor)
{
	GLXContext ctx;

	const int attribs_with_profile[] = {
		GLX_CONTEXT_MAJOR_VERSION_ARB,
		major,

		GLX_CONTEXT_MINOR_VERSION_ARB,
		minor,

		GLX_CONTEXT_PROFILE_MASK_ARB,
		GLX_CONTEXT_CORE_PROFILE_BIT_ARB,

		None
	};

	const int attribs_without_profile[] = {
		GLX_CONTEXT_MAJOR_VERSION_ARB,
		major,

		GLX_CONTEXT_MINOR_VERSION_ARB,
		minor,

		None
	};

	/* The GLX_ARB_create_context_profile spec says:
	 *
	 *     "The attribute name GLX_CONTEXT_PROFILE_MASK_ARB requests an
	 *     OpenGL context supporting a specific <profile> of the API....If
	 *     the requested OpenGL version is less than 3.2,
	 *     GLX_CONTEXT_PROFILE_MASK_ARB is ignored and the functionality
	 *     of the context is determined solely by the requested version."
	 *
	 * Try to create a context without any profile specified.  If that
	 * works, try to create a context with the core profile specified.
	 * That should also work.
	 */
	ctx = glXCreateContextAttribsARB(dpy, fbconfig, NULL, True,
					 attribs_without_profile);
	XSync(dpy, 0);

	if (ctx == NULL)
		return true;

	glXDestroyContext(dpy, ctx);

	ctx = glXCreateContextAttribsARB(dpy, fbconfig, NULL, True,
					 attribs_with_profile);
	XSync(dpy, 0);
	if (ctx != NULL) {
		glXDestroyContext(dpy, ctx);
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

	GLX_ARB_create_context_setup();
	piglit_require_glx_extension(dpy, "GLX_ARB_create_context_profile");

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

	GLX_ARB_create_context_teardown();

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
	return 0;
}
