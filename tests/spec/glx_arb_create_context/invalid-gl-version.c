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

static bool try_version(int major, int minor)
{
	const int attribs[] = {
		GLX_CONTEXT_MAJOR_VERSION_ARB, major,
		GLX_CONTEXT_MINOR_VERSION_ARB, minor,
		None
	};
	GLXContext ctx;
	bool pass = true;

	ctx = glXCreateContextAttribsARB(dpy, fbconfig, NULL, True, attribs);
	XSync(dpy, 0);

	if (ctx != NULL) {
		fprintf(stderr,
			"Created OpenGL context with invalid version %d.%d\n",
			major, minor);
		glXDestroyContext(dpy, ctx);
		pass = false;
	}

	/* The GLX_ARB_create_context spec says:
	 *
	 *     "If attributes GLX_CONTEXT_MAJOR_VERSION_ARB and
	 *     GLX_CONTEXT_MINOR_VERSION_ARB, when considered together with
	 *     attributes GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB and
	 *     GLX_RENDER_TYPE, specify an OpenGL version and feature set that
	 *     are not defined, BadMatch is generated."
	 */
	if (!validate_glx_error_code(BadMatch, -1)) {
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

	/* The GLX_ARB_create_context spec says:
	 *
	 *     "The defined versions of OpenGL at the time of writing are
	 *     OpenGL 1.0, 1.1, 1.2, 1.3, 1.4, 1.5, 2.0, 2.1, 3.0, 3.1, and
	 *     3.2.  Feature deprecation was introduced with OpenGL 3.0, so
	 *     forward-compatible contexts may only be requested for OpenGL
	 *     3.0 and above. Thus, examples of invalid combinations of
	 *     attributes include:
	 *
	 *       - Major version < 1 or > 3
	 *       - Major version == 1 and minor version < 0 or > 5
	 *       - Major version == 2 and minor version < 0 or > 1
	 *       - Major version == 3 and minor version > 2
	 *       - Forward-compatible flag set and major version < 3
	 *       - Color index rendering and major version >= 3"
	 */
	pass = try_version(-1, 0) && pass;
	pass = try_version(0, 0) && pass;
	pass = try_version(1, -1) && pass;
	pass = try_version(1, 6) && pass;
	pass = try_version(2, -1) && pass;
	pass = try_version(2, 2) && pass;
	pass = try_version(3, -1) && pass;

	/* Since the writing of the GLX_ARB_create_context_spec, versions 3.3,
	 * 4.0, 4.1, and 4.2 have been released.  There is no expectation that
	 * 3.4 will ever exist becuase it would have to include functionality
	 * not in 4.0, and that would be weird.
	 */
	pass = try_version(3, 4) && pass;

	GLX_ARB_create_context_teardown();

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
	return 0;
}
