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
	static const int attribs[] = {
		GLX_CONTEXT_MAJOR_VERSION_ARB,
		2,
		GLX_CONTEXT_MINOR_VERSION_ARB,
		0,
		GLX_CONTEXT_PROFILE_MASK_ARB,
		GLX_CONTEXT_ES2_PROFILE_BIT_EXT,
		None
	};

	bool pass = true;
	GLXContext ctx;

	GLX_ARB_create_context_setup();
	piglit_require_glx_extension(dpy, "GLX_ARB_create_context_profile");
	piglit_require_glx_extension(dpy, "GLX_EXT_create_context_es2_profile");

	/* The GLX_EXT_create_context_es2_profile doesn't say anything about
	 * indirect-rendering contexts for ES2.  However, there is no protocol
	 * defined, so it seems impossible that this could ever work.
	 */
	ctx = glXCreateContextAttribsARB(dpy, fbconfig, NULL, False, attribs);
	XSync(dpy, 0);

	if (ctx != NULL) {
		PFNGLGETSHADERPRECISIONFORMATPROC func;
		GLint r[] = { ~0, ~0 };
		GLint p = ~0;

		/* Try to call an ES2 function that does not exist in desktop
		 * OpenGL or have GLX protocol defined.  If this works, then
		 * we'll assume the implementation is using some magic
		 * protocol for ES2.  If it doesn't work, then the test fails.
		 */
		func = (PFNGLGETSHADERPRECISIONFORMATPROC)
			glXGetProcAddress((const GLubyte *)
					  "glGetShaderPrecisionFormat");
		if (func == NULL) {
			fprintf(stderr,
				"Indirect rendering OpenGL ES 2.0 context was "
				"created, but could not get\n"
				"function address for "
				"glGetShaderPrecisionFormat.\n");
			pass = false;
			goto done;
		}

		if (!glXMakeCurrent(dpy, glxWin, ctx)) {
			fprintf(stderr,
				"Indirect rendering OpenGL ES 2.0 "
				"context was created, but\n"
				"it could not be made current.\n");
			pass = false;
			goto done;
		}

		(*func)(GL_VERTEX_SHADER, GL_MEDIUM_FLOAT, r, &p);
		if (r[0] < 14 || r[1] < 14 || p < 10) {
			fprintf(stderr,
				"Indirect rendering OpenGL ES 2.0 "
				"context was created, but\n"
				"glGetShaderPrecisionFormat produced "
				"incorrect results.\n");
			pass = false;
		}
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

done:
	if (ctx != NULL) {
		glXMakeCurrent(dpy, None, NULL);
		glXDestroyContext(dpy, ctx);
	}

	GLX_ARB_create_context_teardown();

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
	return 0;
}
