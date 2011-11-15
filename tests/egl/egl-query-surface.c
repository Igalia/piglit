/*
 * Copyright © 2011 Intel Corporation
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

/** @file egl-query-drawable.c
 *
 * Test behavior of eglQuerySurface(). See EGL 1.4 spec, Section 3.5.6
 * Surface Attributes.
 *
 * For usage information, see usage_error().
 */

#include <EGL/egl.h>
#include "piglit-util-egl.h"
#include "egl-util.h"

static EGLint window_width = 119;
static EGLint window_height= 137;

static void
usage_error()
{
	const char *message =
		"usage:\n"
		"    egl-query-surface [-auto] --bad-surface\n"
		"        Call eglQuerySurface(surf=0) and expect that error\n"
		"        EGL_BAD_SURFACE is generated.\n"
		"\n"
		"    egl-query-surface [-auto] --bad-attr\n"
		"        Call eglQuerySurface(attr=EGL_DONT_CARE) and expect that\n"
		"        error EGL_BAD_ATTRIBUTE is generated.\n"
		"\n"
		"    egl-query-surface [-auto] --attr=EGL_WIDTH\n"
		"    egl-query-surface [-auto] --attr=EGL_HEIGHT\n"
		"        Call eglQueryDrawable() with the given attribute.\n";
	printf("%s", message);
	piglit_report_result(PIGLIT_FAIL);
}

static enum piglit_result
query_width(struct egl_state *state)
{
	EGLint width;
	EGLBoolean ok;

	assert(state->width == window_width);
	ok = eglQuerySurface(state->egl_dpy, state->surf, EGL_WIDTH, &width);
	piglit_expect_egl_error(EGL_SUCCESS, PIGLIT_FAIL);
	if (!ok) {
		fprintf(stderr, "error: eglQuerySurface() failed\n");
		return PIGLIT_FAIL;
	}
	if (width != state->width) {
		fprintf(stderr,
			"error: width=%d but eglQuerySurface(EGL_WIDTH) "
			"returned %d\n", state->height, width);
		return PIGLIT_FAIL;
	}
	return PIGLIT_PASS;
}

static enum piglit_result
query_height(struct egl_state *state)
{
	EGLint height;
	EGLBoolean ok;

	assert(state->height == window_height);
	ok = eglQuerySurface(state->egl_dpy, state->surf, EGL_HEIGHT, &height);
	piglit_expect_egl_error(EGL_SUCCESS, PIGLIT_FAIL);
	if (!ok) {
		fprintf(stderr, "error: eglQuerySurface() failed\n");
		return PIGLIT_FAIL;
	}
	if (height != state->height) {
		fprintf(stderr,
			"error: height=%d but eglQuerySurface(EGL_HEIGHT) "
			"returned %d\n", state->height, height);
		return PIGLIT_FAIL;
	}
	return PIGLIT_PASS;
}

static enum piglit_result
query_bad_surface(struct egl_state *state)
{
	EGLint width;
	EGLBoolean ok;

	ok = eglQuerySurface(state->egl_dpy, 0, EGL_WIDTH, &width);
	if (ok) {
		fprintf(stderr,
		        "error: eglQuerySurface(surface=0) succeeded\n");
		return PIGLIT_FAIL;
	}
	piglit_expect_egl_error(EGL_BAD_SURFACE, PIGLIT_FAIL);
	return PIGLIT_PASS;
}

static enum piglit_result
query_bad_parameter(struct egl_state *state)
{
	EGLint junk;
	EGLBoolean ok;

	ok = eglQuerySurface(state->egl_dpy, state->surf, EGL_DONT_CARE,
	                     &junk);
	if (ok) {
		fprintf(stderr,
		        "error: eglQuerySurface(attribute=EGL_DONT_CARE) "
		        "succeeded\n");
		return PIGLIT_FAIL;
	}
	piglit_expect_egl_error(EGL_BAD_ATTRIBUTE, PIGLIT_FAIL);
	return PIGLIT_PASS;
}

static void
remove_arg(char **argv, int i)
{
	int j;
	for (j = i; argv[j] != NULL; ++j) {
		argv[j] = argv[j + 1];
	}
}

static void
parse_args(char **argv,
           int *out_argc,
           enum piglit_result (**out_test)(struct egl_state*))
{
	int i;

	/* Count of parsed args, excluding -auto. */
	int num_parsed_args = 0;

	for (i = 1; argv[i] != NULL;) {
		const char *arg = argv[i];
		if (!strncmp(arg, "--bad-surface", 13)) {
			++num_parsed_args;
			remove_arg(argv, i);
			*out_test = query_bad_surface;
		} else if (!strncmp(arg, "--bad-attr", 10)) {
			++num_parsed_args;
			remove_arg(argv, i);
			*out_test = query_bad_parameter;
		} else if (!strncmp(arg, "--attr=EGL_WIDTH", 16)) {
			++num_parsed_args;
			remove_arg(argv, i);
			*out_test = query_width;
		} else if (!strncmp(arg, "--attr=EGL_HEIGHT", 17)) {
			++num_parsed_args;
			remove_arg(argv, i);
			*out_test = query_height;
		} else {
		   /* Unrecognized argument. */
		   ++i;
		}
	}

	if (num_parsed_args != 1) {
	   usage_error();
	}

	*out_argc -= num_parsed_args;
}

int
main(int argc, char *argv[])
{
	struct egl_test test;
	enum piglit_result (*test_func)(struct egl_state *state) = NULL;

	parse_args(argv, &argc, &test_func);

	egl_init_test(&test);
	test.draw = test_func;
	test.window_width = window_width;
	test.window_height = window_height;

	return egl_util_run(&test, argc, argv);
}
