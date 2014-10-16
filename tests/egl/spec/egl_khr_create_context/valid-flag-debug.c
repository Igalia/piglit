/* Copyright © 2013 Intel Corporation
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

/**
 * @file
 * @brief Test EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR.
 *
 * Call eglCreateContext with EGL_CONTEXT_FLAGS_KHR=EGL_CONTEXT_OPENGL_BIT_KHR.
 * If context creation succeeds, then verify the context is really a debug
 * context by verifying GL_CONTEXT_FLAGS contains GL_CONTEXT_FLAG_DEBUG_BIT.
 * If context creation fails, then verify that EGL_BAD_MATCH is emitted.
 *
 * A commandline argument specifies which OpenGL API to test.
 *
 * From version 15 of the EGL_KHR_create_context spec:
 *
 *    If the EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR flag bit is set in
 *    EGL_CONTEXT_FLAGS_KHR, then a <debug context> will be created.
 *    [...] This bit is supported for
 *    OpenGL and OpenGL ES contexts.
 *
 * and
 *
 *    If <config> does not support a client API context compatible
 *    with the requested API major and minor version, context flags,
 *    and context reset notification behavior (for client API types
 *    where these attributes are supported), then an EGL_BAD_MATCH
 *    error is generated.
 */

#include "piglit-util-egl.h"
#include "piglit-util-gl.h"
#include "common.h"

const char *progname;

static void
usage_error(void)
{
	fprintf(stderr, "%s: usage error\n", progname);
	fprintf(stderr, "%s gl|gles1|gles2|gles3\n", progname);
	piglit_report_result(PIGLIT_FAIL);
}

static void
try_debug_flag(EGLenum context_api, EGLenum context_bit)
{
	GLint actual_flags = 0;
	piglit_dispatch_api dispatch_api;

	EGLint attribs[64];
	int i = 0;

	if (!EGL_KHR_create_context_setup(context_bit))
		piglit_report_result(PIGLIT_SKIP);

	if (!piglit_egl_bind_api(context_api))
		piglit_report_result(PIGLIT_SKIP);

        attribs[i++] = EGL_CONTEXT_FLAGS_KHR;
        attribs[i++] = EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR;

        switch (context_bit) {
        case EGL_OPENGL_BIT:
           break;
        case EGL_OPENGL_ES_BIT:
              attribs[i++] = EGL_CONTEXT_MAJOR_VERSION_KHR;
              attribs[i++] = 1;
              break;
        case EGL_OPENGL_ES2_BIT:
              attribs[i++] = EGL_CONTEXT_MAJOR_VERSION_KHR;
              attribs[i++] = 2;
              break;
        case EGL_OPENGL_ES3_BIT_KHR:
           attribs[i++] = EGL_CONTEXT_MAJOR_VERSION_KHR;
           attribs[i++] = 3;
           break;
        default:
           assert(0);
           break;
        }

        attribs[i++] = EGL_NONE;

	ctx = eglCreateContext(egl_dpy, cfg, EGL_NO_CONTEXT, attribs);
	if (!ctx) {
		if (piglit_check_egl_error(EGL_BAD_MATCH)) {
			piglit_report_result(PIGLIT_SKIP);
		} else {
			piglit_report_result(PIGLIT_FAIL);
		}
	}

	if (!eglMakeCurrent(egl_dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, ctx)) {
		fprintf(stderr, "eglMakeCurrent() failed\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	switch (context_bit) {
	case EGL_OPENGL_BIT:
		dispatch_api = PIGLIT_DISPATCH_GL;
		break;
	case EGL_OPENGL_ES_BIT:
		dispatch_api = PIGLIT_DISPATCH_ES1;
		break;
	case EGL_OPENGL_ES2_BIT:
	case EGL_OPENGL_ES3_BIT_KHR:
		dispatch_api = PIGLIT_DISPATCH_ES2;
		break;
	default:
		dispatch_api = 0;
		assert(0);
		break;
	}

	piglit_dispatch_default_init(dispatch_api);

	switch (context_bit) {
	case EGL_OPENGL_BIT:
		if (piglit_get_gl_version() < 31 &&
		    !piglit_is_extension_supported("GL_KHR_debug")) {
			fprintf(stderr, "In OpenGL, either OpenGL 3.1 or "
				"GL_KHR_debug is required to query "
				"GL_CONTEXT_FLAGS\n");
			piglit_report_result(PIGLIT_SKIP);
		}
		break;
	case EGL_OPENGL_ES_BIT:
	case EGL_OPENGL_ES2_BIT:
	case EGL_OPENGL_ES3_BIT_KHR:
		if (!piglit_is_extension_supported("GL_KHR_debug")) {
			fprintf(stderr, "In OpenGL ES, GL_KHR_debug is "
				"required to query GL_CONTEXT_FLAGS\n");
			piglit_report_result(PIGLIT_SKIP);
		}
		break;
	default:
		assert(0);
		break;
	}

	glGetIntegerv(GL_CONTEXT_FLAGS, &actual_flags);

	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		fprintf(stderr, "glGetIntegerv(GL_CONTEXT_FLAGS) failed\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	/* Verify that this is actually a debug context. */
	if (!(actual_flags & GL_CONTEXT_FLAG_DEBUG_BIT)) {
		fprintf(stderr,
			"GL_CONTEXT_FLAGS=0x%x does not contain "
			"GL_CONTEXT_FLAG_DEBUG_BIT=0x%x\n",
			actual_flags, GL_CONTEXT_FLAG_DEBUG_BIT);
		piglit_report_result(PIGLIT_FAIL);
	}

	eglDestroyContext(egl_dpy, ctx);
	EGL_KHR_create_context_teardown();

	piglit_report_result(PIGLIT_PASS);
}

int
main(int argc, char **argv)
{
	progname = argv[0];
	piglit_strip_arg(&argc, argv, "-auto");

	if (argc != 2)
		usage_error();

	if (strcmp(argv[1], "gl") == 0) {
		try_debug_flag(EGL_OPENGL_API, EGL_OPENGL_BIT);
	} else if (strcmp(argv[1], "gles1") == 0) {
		try_debug_flag(EGL_OPENGL_ES_API, EGL_OPENGL_ES_BIT);
	} else if (strcmp(argv[1], "gles2") == 0) {
		try_debug_flag(EGL_OPENGL_ES_API, EGL_OPENGL_ES2_BIT);
	} else if (strcmp(argv[1], "gles3") == 0) {
		try_debug_flag(EGL_OPENGL_ES_API, EGL_OPENGL_ES3_BIT_KHR);
	} else {
		usage_error();
	}

	abort();
	return EXIT_FAILURE;
}
