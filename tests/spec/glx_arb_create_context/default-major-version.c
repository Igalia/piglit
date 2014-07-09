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
#include "piglit-util-gl.h"
#include "piglit-glx-util.h"
#include "common.h"

int main(int argc, char **argv)
{
	static const int attribs[] = {
		GLX_CONTEXT_MINOR_VERSION_ARB, 2,
		None
	};
	GLXContext ctx;
	const char *version_string;
	int major;
	int minor;

	GLX_ARB_create_context_setup();

	/* The GLX_ARB_create_context spec says:
	 *
	 *     "The default values for GLX_CONTEXT_MAJOR_VERSION_ARB and
	 *     GLX_CONTEXT_MINOR_VERSION_ARB are 1 and 0 respectively. In this
	 *     case, implementations will typically return the most recent
	 *     version of OpenGL they support which is backwards compatible
	 *     with OpenGL 1.0 (e.g. 3.0, 3.1 + GL_ARB_compatibility, or 3.2
	 *     compatibility profile)."
	 *
	 * Request an OpenGL 1.2 context by explicitly setting the minor
	 * version to 2 and leaving the major version at the default value of
	 * 1.  The Linux OpenGL ABI requires at least OpenGL 1.2, so this must
	 * create a context.
	 */
	ctx = glXCreateContextAttribsARB(dpy, fbconfig, NULL, True, attribs);
	glXMakeContextCurrent(dpy, glxWin, glxWin, ctx);
	piglit_dispatch_default_init(PIGLIT_DISPATCH_GL);

	version_string = (char *) glGetString(GL_VERSION);

	if (!parse_version_string(version_string, &major, &minor)) {
		fprintf(stderr,
			"Unable to parse GL version string: %s\n",
			version_string);
		piglit_report_result(PIGLIT_FAIL);
	}

	if (major < 1 || (major == 1 && minor < 2)) {
		fprintf(stderr,
			"GL version too low: %s\n"
			"Expected 1.2 or greater.\n",
			version_string);
		piglit_report_result(PIGLIT_FAIL);
	}

	glXMakeContextCurrent(dpy, None, None, None);
	glXDestroyContext(dpy, ctx);

	GLX_ARB_create_context_teardown();

	piglit_report_result(PIGLIT_PASS);
	return 0;
}
