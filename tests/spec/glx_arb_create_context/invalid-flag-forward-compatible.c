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

static bool check_version(int major, int minor)
{
	const int attribs[] = {
		GLX_CONTEXT_MAJOR_VERSION_ARB, major,
		GLX_CONTEXT_MINOR_VERSION_ARB, minor,
		None
	};
	GLXContext ctx;

	ctx = glXCreateContextAttribsARB(dpy, fbconfig, NULL, True, attribs);
	if (ctx != NULL) {
		glXDestroyContext(dpy, ctx);
		return true;
	}

	return false;
}

/**
 * Try to create a context with the GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB
 *
 * Versions previous to OpenGL 3.0 should always reject the
 * GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB as it is non-sense.
 *
 * \return
 * If the context is (correctly) not created, \c true is returned.  If the
 * context is (incorrectly) created, \c false is returned.
 */
static bool try_version(int major, int minor)
{
	const int attribs[] = {
		GLX_CONTEXT_MAJOR_VERSION_ARB, major,
		GLX_CONTEXT_MINOR_VERSION_ARB, minor,
		GLX_CONTEXT_FLAGS_ARB, GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
		None
	};
	GLXContext ctx;
	bool pass = true;

	ctx = glXCreateContextAttribsARB(dpy, fbconfig, NULL, True, attribs);
	XSync(dpy, 0);

	if (ctx != NULL) {
		fprintf(stderr,
			"Created OpenGL context %d.%d with forward-compatible "
			"flag, but this should have failed.\n",
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
	static const struct {
		int major;
		int minor;

		/** Linux OpenGL ABI only requires OpenGL 1.2. */
		bool version_must_be_supported;
	} all_gl_versions[] = {
		{ 1, 0, true },
		{ 1, 1, true },
		{ 1, 2, true },
		{ 1, 3, false },
		{ 1, 4, false },
		{ 1, 5, false },
		{ 2, 0, false },
		{ 2, 1, false }
	};
	bool pass = true;
	unsigned i;

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
	for (i = 0; i < ARRAY_SIZE(all_gl_versions); i++) {
		if (!all_gl_versions[i].version_must_be_supported
		    && !check_version(all_gl_versions[i].major,
				      all_gl_versions[i].minor)) {
			printf("OpenGL version %d.%d not supported by this "
			       "implementation.  Skipping forward-"
			       "compatibility flag check.\n",
			       all_gl_versions[i].major,
			       all_gl_versions[i].minor);
			continue;
		}

		pass = try_version(all_gl_versions[i].major,
				   all_gl_versions[i].minor)
			&& pass;
	}

	GLX_ARB_create_context_teardown();

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
	return 0;
}
