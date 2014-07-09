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
	bool pass = true;
	GLXContext ctx;

	GLX_ARB_create_context_setup();

	/* The GLX_ARB_create_context spec says:
	 *
	 *     "<attrib_list> may be NULL or empty (first attribute is None),
	 *     in which case all attributes assume their default values as
	 *     described below.
	 *
	 *     ...
	 *
	 *     The default values for GLX_CONTEXT_MAJOR_VERSION_ARB and
	 *     GLX_CONTEXT_MINOR_VERSION_ARB are 1 and 0 respectively. In this
	 *     case, implementations will typically return the most recent
	 *     version of OpenGL they support which is backwards compatible
	 *     with OpenGL 1.0 (e.g. 3.0, 3.1 + GL_ARB_compatibility, or 3.2
	 *     compatibility profile)."
	 *
	 * The Linux OpenGL ABI requires at least OpenGL 1.2, so this must
	 * create a context.
	 */
	ctx = glXCreateContextAttribsARB(dpy, fbconfig, NULL, True, NULL);
	if (ctx == NULL) {
		fprintf(stderr,	"Unable to create OpenGL context!\n");
		pass = false;
	} else {
		glXDestroyContext(dpy, ctx);
	}

	GLX_ARB_create_context_teardown();

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
	return 0;
}
