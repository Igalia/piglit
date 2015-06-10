/*
 * Copyright © 2015 Advanced Micro Devices, Inc.
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
#include "egl-util.h"

static bool test_srgb;

static enum piglit_result
draw(struct egl_state *state)
{
	enum piglit_result result = PIGLIT_PASS;
	float green[] = {0, 0.3, 0, 0};
	float expected_green[4];
	float expected_blend[4];

	eglMakeCurrent(state->egl_dpy, state->surf, state->surf, state->ctx);

	glViewport(0, 0, state->width, state->height);
	piglit_ortho_projection(state->width, state->height, GL_FALSE);

	glClearColor(0.5, 0.5, 0.5, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glColor4f(green[0], green[1], green[2], green[3]);

	/* Draw first without blending. */
	piglit_draw_rect(0, 0, 20, 20);

	/* Draw second with blending. */
	piglit_draw_rect(20, 0, 20, 20);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	piglit_draw_rect(20, 0, 20, 20);
	glDisable(GL_BLEND);

	/* Check first. */
	memcpy(expected_green, green, sizeof(float) * 4);
	if (test_srgb)
		expected_green[1] = piglit_linear_to_srgb(green[1]);
	if (!piglit_probe_rect_rgb(0, 0, 20, 20, expected_green))
		result = PIGLIT_FAIL;

	/* Check second. */
	memcpy(expected_blend, green, sizeof(float) * 4);
	if (test_srgb)
		expected_blend[1] = piglit_linear_to_srgb(green[1] * 2.0);
	else
		expected_blend[1] *= 2;
	if (!piglit_probe_rect_rgb(20, 0, 20, 20, expected_blend))
		result = PIGLIT_FAIL;

	eglSwapBuffers(state->egl_dpy, state->surf);
	return result;
}

int
main(int argc, char *argv[])
{
	struct egl_test test;
	const char *extensions[] = {"EGL_KHR_gl_colorspace", NULL};
	const EGLint surface_linear[] = {
		EGL_GL_COLORSPACE, EGL_GL_COLORSPACE_LINEAR,
		EGL_NONE
	};
	const EGLint surface_srgb[] = {
		EGL_GL_COLORSPACE, EGL_GL_COLORSPACE_SRGB,
		EGL_NONE
	};
	const EGLint test_attribs[] = {
		EGL_RENDERABLE_TYPE, EGL_OPENGL_ES_BIT,
		EGL_NONE
	};

	piglit_strip_arg(&argc, argv, "-fbo");
	test_srgb = piglit_strip_arg(&argc, argv, "srgb");

	egl_init_test(&test);
	test.draw = draw;
	test.stop_on_failure = true;
	test.config_attribs = test_attribs;
	test.surface_attribs = test_srgb ? surface_srgb : surface_linear;
	test.extensions = extensions;

	return egl_util_run(&test, argc, argv) == PIGLIT_PASS ? EXIT_SUCCESS
							      : EXIT_FAILURE;
}
