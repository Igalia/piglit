/* Copyright © 2012, 2015 Intel Corporation
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
 * @file viewport.c
 * @brief Test calling glViewport with no surface bound via
 *  EGL_KHR_surfaceless_context.
 *
 * Creates an EGL context and binds it without a surface via
 * EGL_KHR_surfaceless_context and then calls glViewport. This exposes
 * a crash in the i965 driver which tries to perform some actions on
 * the non-existent surface whenever the viewport changes.
 */

#include "piglit-util-gl.h"
#include "piglit-util-egl.h"
#include <EGL/egl.h>
#include <EGL/eglext.h>

static EGLDisplay egl_dpy;
static EGLConfig cfg;
static EGLContext ctx;

static void
choose_config(void)
{
	EGLint config_attribs[] = {
		EGL_NONE
	};
	EGLint count;
	EGLint major, minor;

	egl_dpy = eglGetDisplay(EGL_DEFAULT_DISPLAY);

	if (!eglInitialize(egl_dpy, &major, &minor)) {
		fprintf(stderr, "eglInitialize() failed\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	if (!eglChooseConfig(egl_dpy, config_attribs, &cfg, 1, &count) ||
	    count == 0) {
		fprintf(stderr, "eglChooseConfig() failed\n");
		piglit_report_result(PIGLIT_FAIL);
	}
}

static void
create_context(void)
{
	if (!piglit_egl_bind_api(EGL_OPENGL_API))
		piglit_report_result(PIGLIT_SKIP);

	ctx = eglCreateContext(egl_dpy, cfg, EGL_NO_CONTEXT,
			       NULL /* attrib_list */);
	if (!ctx) {
		piglit_report_result(piglit_check_egl_error(EGL_BAD_MATCH) ?
				     PIGLIT_SKIP :
				     PIGLIT_FAIL);
	}
}

int
main(int argc, char **argv)
{
	static const GLint expected_viewport[] = { 0, 0, 42, 42 };
	GLint actual_viewport[4];
	int i;

	choose_config();
	piglit_require_egl_extension(egl_dpy, "EGL_KHR_surfaceless_context");
	create_context();

	/* Bind the context with no surface */
	if (!eglMakeCurrent(egl_dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, ctx)) {
		fprintf(stderr, "eglMakeCurrent() failed\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	piglit_dispatch_default_init(PIGLIT_DISPATCH_GL);

	/* Try changing the viewport */
	glViewport(expected_viewport[0],
		   expected_viewport[1],
		   expected_viewport[2],
		   expected_viewport[3]);

	/* Check that it worked */
	glGetIntegerv(GL_VIEWPORT, actual_viewport);
	for (i = 0; i < 4; i++) {
		if (expected_viewport[i] != actual_viewport[i]) {
			fprintf(stderr,
				"Viewport does not match\n"
				" expected: %i %i %i %i\n"
				" actual:   %i %i %i %i\n",
				expected_viewport[0],
				expected_viewport[1],
				expected_viewport[2],
				expected_viewport[3],
				actual_viewport[0],
				actual_viewport[1],
				actual_viewport[2],
				actual_viewport[3]);
			piglit_report_result(PIGLIT_FAIL);
		}
	}

	piglit_report_result(PIGLIT_PASS);
}
