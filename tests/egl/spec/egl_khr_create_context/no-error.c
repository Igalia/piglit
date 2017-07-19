/* Copyright 2017 Grigori Goronzy <greg@chown.ath.cx>
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

#include <stdio.h>
#include <stdbool.h>

#include "piglit-util-gl.h"
#include "piglit-util-egl.h"
#include "common.h"

#define BOOLSTR(x) ((x) ? "yes" : "no")

static void
check_extension(EGLint mask)
{
	if (!EGL_KHR_create_context_setup(mask))
		piglit_report_result(PIGLIT_SKIP);

	piglit_require_egl_extension(egl_dpy, "EGL_KHR_create_context_no_error");
	piglit_require_egl_extension(egl_dpy, "EGL_KHR_surfaceless_context");

	EGL_KHR_create_context_teardown();
}


/* GL_CONTEXT_FLAGS doesn't exist before OpenGL 3.0 or OpenGL ES 3.2. */
static bool
api_has_context_flags(EGLenum api)
{
	return ((api == EGL_OPENGL_API && piglit_get_gl_version() >= 30) ||
		(api == EGL_OPENGL_ES_API && piglit_get_gl_version() >= 32));
}

static enum piglit_result
check_no_error(EGLenum api, bool debug, bool robust)
{
	static bool is_dispatch_init = false;
	enum piglit_result result = PIGLIT_SKIP;
	EGLContext ctx;
	EGLint attribs[13];
	size_t ai = 0;
	GLint context_flags = 0;
	EGLint mask = (api == EGL_OPENGL_API) ? EGL_OPENGL_BIT : EGL_OPENGL_ES2_BIT;

	printf("info: %s debug=%s robustness=%s\n",
	       (api == EGL_OPENGL_API) ? "OpenGL" : "OpenGL ES",
	       BOOLSTR(debug), BOOLSTR(robust));

	if (!EGL_KHR_create_context_setup(mask))
		goto out;

	if (eglBindAPI(api) != EGL_TRUE)
		goto out;

	if (robust &&
	    !piglit_is_egl_extension_supported(egl_dpy,
					       "EGL_EXT_create_context_robustness")) {
		printf("info: EGL_EXT_create_context_robustness not supported\n");
		goto out;
	}

	if (api == EGL_OPENGL_ES_API) {
		attribs[ai++] = EGL_CONTEXT_CLIENT_VERSION;
		attribs[ai++] = 2;
	}
	if (debug) {
		attribs[ai++] = EGL_CONTEXT_FLAGS_KHR;
		attribs[ai++] = EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR;
	}
	if (robust) {
		attribs[ai++] = EGL_CONTEXT_OPENGL_ROBUST_ACCESS_EXT;
		attribs[ai++] = EGL_TRUE;
	}
	/* Always use OpenGL 2.0 or OpenGL ES 2.0 to keep this test reasonably
	 * simple; there are enough variants as-is.
	 */
	attribs[ai++] = EGL_CONTEXT_MAJOR_VERSION_KHR;
	attribs[ai++] = 2;
	attribs[ai++] = EGL_CONTEXT_MINOR_VERSION_KHR;
	attribs[ai++] = 0;
	attribs[ai++] = EGL_CONTEXT_OPENGL_NO_ERROR_KHR;
	attribs[ai++] = EGL_TRUE;
	attribs[ai++] = EGL_NONE;

	ctx = eglCreateContext(egl_dpy, cfg, EGL_NO_CONTEXT, attribs);
	if (debug || robust) {
		if (piglit_check_egl_error(EGL_BAD_MATCH) && ctx == NULL) {
			/* KHR_no_error doesn't allow the no error mode to be
			 * enabled with KHR_debug or ARB_robustness, so
			 * context creation is expected to fail.
			 */
			printf("info: context creation failed (expected)\n");
			result = PIGLIT_PASS;
		} else {
			result = PIGLIT_FAIL;
		}
		goto out;
	}
	if (ctx == NULL) {
		result = PIGLIT_FAIL;
		goto out;
	}

	if (!piglit_check_egl_error(EGL_SUCCESS)) {
		printf("error: unexpected EGL error\n");
		result = PIGLIT_FAIL;
		goto out;
	}

	if (eglMakeCurrent(egl_dpy, EGL_NO_SURFACE, EGL_NO_SURFACE,
			   ctx) != EGL_TRUE) {
		printf("error: failed to make context current\n");
		result = PIGLIT_FAIL;
		goto out;
	}

	if (!is_dispatch_init) {
		/* We must postpone initialization of piglit-dispatch until
		 * a context is current.
		 */
		piglit_dispatch_default_init(PIGLIT_DISPATCH_GL);
		is_dispatch_init = true;
	}

	/* The EGL_KHR_create_context_no_error extension unfortunately allows
	 * "no-op" implementations. That is, the EGL extension can be supported
	 * without any support on the GL side of things. This means we can't
	 * fail if KHR_no_error turns out to be not enabled at this point.
	 */
	if (!piglit_is_extension_supported("GL_KHR_no_error")) {
		printf("warning: context does not report GL_KHR_no_error "
		       "availability\n");
		result = PIGLIT_WARN;
		goto out;
	}

	if (api_has_context_flags(api)) {
		glGetIntegerv(GL_CONTEXT_FLAGS, &context_flags);
		if (!(context_flags & GL_CONTEXT_FLAG_NO_ERROR_BIT_KHR)) {
			printf("error: context does not have "
			       "GL_CONTEXT_FLAG_NO_ERROR_BIT_KHR set\n");
			result = PIGLIT_FAIL;
			goto out;
		}
	}

	/* Everything turned out to be fine */
	result = PIGLIT_PASS;
out:
	printf("info: %s\n", piglit_result_to_string(result));
	EGL_KHR_create_context_teardown();
	return result;
}

int main(int argc, char **argv)
{
	enum piglit_result result = PIGLIT_SKIP;

	check_extension(EGL_OPENGL_BIT);
	check_extension(EGL_OPENGL_ES2_BIT);

	/* Check that KHR_no_error gets enabled and its interaction with debug and
	 * robustness context flags. */
	piglit_merge_result(&result, check_no_error(EGL_OPENGL_API, false, false));
	piglit_merge_result(&result, check_no_error(EGL_OPENGL_ES_API, false, false));
	piglit_merge_result(&result, check_no_error(EGL_OPENGL_API, true, false));
	piglit_merge_result(&result, check_no_error(EGL_OPENGL_ES_API, true, false));
	piglit_merge_result(&result, check_no_error(EGL_OPENGL_API, false, true));
	piglit_merge_result(&result, check_no_error(EGL_OPENGL_ES_API, false, true));
	piglit_merge_result(&result, check_no_error(EGL_OPENGL_API, true, true));
	piglit_merge_result(&result, check_no_error(EGL_OPENGL_ES_API, true, true));

	piglit_report_result(result);
}
