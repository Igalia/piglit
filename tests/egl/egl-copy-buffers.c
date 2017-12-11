/*
 * Copyright © 2017 Intel Corporation
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
#include "piglit-util-egl.h"
#include "piglit-util-gl.h"
#include "egl-util.h"

/*
 * For legacy reasons, eglGetDisplay uses a detection heuristics to establish
 * the underlying platform.
 *
 * Yet there is another API eglCopyBuffers that invokes the detection, a seeming
 * remainder from the days perior to the EGL platform extensions.
 *
 * With the EGL extensions, the platform specified by the user may differ from
 * the detected one. which will result in eglCopyBuffers failure.
 *
 * The check should be dropped.
 */

static void
test_setup(void)
{
	/* Set the env. variable to force the platform 'detection' to use
	 * a platform different than X11.
	 *
	 * NOTE: This is not perfect, since driver may ignore the variable, yet
	 * we aim to provide a consistent experience across test runs, build
	 * permutation and/or driver used.
	 */
	setenv("EGL_PLATFORM", "drm", true);

	/* XXX: test should flag regardless of the following call - testing
	 * has confirmed it.
	 *
	 * NOTE: We cannot do the test twice - with and W/o the call; the
	 * detection result is stored in static variable :-\
	 */
	piglit_egl_get_default_display(EGL_NONE);

	/* Use X11 since it's the only platform that has EGL pixmap surfaces */
	piglit_require_egl_extension(EGL_NO_DISPLAY, "EGL_EXT_platform_x11");
}

static enum piglit_result
draw(struct egl_state *state)
{
	EGLNativePixmapType pixmap;
	enum piglit_result result = PIGLIT_PASS;

	/* Green for a pass */
	glClearColor(0.0, 1.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	pixmap = egl_util_create_native_pixmap(state, egl_default_window_width,
					       egl_default_window_height);
	eglCopyBuffers(state->egl_dpy, state->surf, pixmap);
	if (!piglit_check_egl_error(EGL_SUCCESS)) {
		fprintf(stderr, "eglCopyBuffers() failed\n");

		/* Red for a fail */
		glClearColor(1.0, 0.0, 0.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);

		result = PIGLIT_FAIL;
	}
	eglSwapBuffers(state->egl_dpy, state->surf);
	return result;
}

int
main(int argc, char *argv[])
{
	static const EGLint test_attribs[] = {
		EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
		EGL_NONE
	};
	struct egl_test test;

	egl_init_test(&test);
	test.draw = draw;
	test.config_attribs = test_attribs;

	test_setup();

	if (egl_util_run(&test, argc, argv) != PIGLIT_PASS)
		return EXIT_FAILURE;
	return EXIT_SUCCESS;
}
