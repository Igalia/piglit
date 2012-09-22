/*
 * Copyright © 2012 Intel Corporation
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

#include "piglit-util-gl-common.h"
#include "piglit-util-waffle.h"

#include "piglit_wfl_framework.h"

struct piglit_wfl_framework*
piglit_wfl_framework(struct piglit_gl_framework *gl_fw)
{
	return (struct piglit_wfl_framework*) gl_fw;
}

int32_t
piglit_wfl_framework_choose_platform(void)
{
	const char *env = getenv("PIGLIT_PLATFORM");

	if (env == NULL) {
#ifdef PIGLIT_HAS_GLX
		return WAFFLE_PLATFORM_GLX;
#else
		fprintf(stderr, "environment var PIGLIT_PLATFORM must be set "
		        "when piglit is built without GLX support\n");
		piglit_report_result(PIGLIT_FAIL);
#endif
	}

	else if (strcmp(env, "glx") == 0) {
#ifdef PIGLIT_HAS_GLX
		return WAFFLE_PLATFORM_GLX;
#else
		fprintf(stderr, "environment var PIGLIT_PLATFORM=glx, but "
		        "piglit was built without GLX support\n");
		piglit_report_result(PIGLIT_FAIL);
#endif
	}

	else if (strcmp(env, "x11_egl") == 0) {
#if defined(PIGLIT_HAS_X11) && defined(PIGLIT_HAS_EGL)
		return WAFFLE_PLATFORM_X11_EGL;
#else
		fprintf(stderr, "environment var PIGLIT_PLATFORM=x11_egl, "
		        "but piglit was built without X11/EGL support\n");
		piglit_report_result(PIGLIT_FAIL);
#endif
	}

	else if (strcmp(env, "wayland") == 0) {
#ifdef PIGLIT_HAS_WAYLAND
		return WAFFLE_PLATFORM_WAYLAND;
#else
		fprintf(stderr, "environment var PIGLIT_PLATFORM=wayland, "
		        "but piglit was built without Wayland support\n");
		piglit_report_result(PIGLIT_FAIL);
#endif
	}

	else {
		fprintf(stderr, "environment var PIGLIT_PLATFORM has bad "
			"value \"%s\"\n", env);
		piglit_report_result(PIGLIT_FAIL);
	}

	assert(false);
	return 0;
}

static int32_t
get_context_api(void)
{
	/* FINISHME: Choose the OpenGL API at runtime. */
#if defined(PIGLIT_USE_OPENGL)
	return  WAFFLE_CONTEXT_OPENGL;
#elif defined(PIGLIT_USE_OPENGL_ES1)
	return WAFFLE_CONTEXT_OPENGL_ES1;
#elif defined(PIGLIT_USE_OPENGL_ES2)
	return WAFFLE_CONTEXT_OPENGL_ES2;
#else
#	error
#endif
}

/**
 * \brief Concatenate two zero-terminated attribute lists.
 *
 * This function interprets null pointers as empty lists, just as Waffle does.
 */
static int32_t*
concat_attrib_lists(const int32_t a[], const int32_t b[])
{
	int a_length = waffle_attrib_list_length(a);
	int b_length = waffle_attrib_list_length(b);
	int r_length = a_length + b_length;

	/* +1 for the terminal 0. */
	int r_size = (2 * r_length + 1) * sizeof(int32_t);

	/* Don't copy the terminal 0.
	 *
	 * If a list is null, then the copy size is conveniently zero.
	 */
	int a_copy_size = 2 * a_length * sizeof(int32_t);
	int b_copy_size = 2 * b_length * sizeof(int32_t);

	int32_t *r = calloc(1, r_size);
	memcpy(r, a, a_copy_size);
	memcpy(r + a_copy_size, b, b_copy_size);
	r[r_size - 1] = 0;
	return r;
}

static void
choose_config(struct piglit_wfl_framework *wfl_fw,
              const int32_t partial_attrib_list[])
{
	int32_t head_attrib_list[3];
	int32_t *full_attrib_list;
	int32_t junk;

	/* Derived class must not provide the context api. */
	assert(waffle_attrib_list_get(partial_attrib_list,
	                              WAFFLE_CONTEXT_API,
	                              &junk) == false);

	head_attrib_list[0] = WAFFLE_CONTEXT_API;
	head_attrib_list[1] = get_context_api();
	head_attrib_list[2] = 0;

	full_attrib_list = concat_attrib_lists(head_attrib_list,
	                                       partial_attrib_list);
	wfl_fw->config = wfl_checked_config_choose(wfl_fw->display,
	                                           full_attrib_list);
	free(full_attrib_list);
}

bool
piglit_wfl_framework_init(struct piglit_wfl_framework *wfl_fw,
                          const struct piglit_gl_test_config *test_config,
                          int32_t platform,
                          const int32_t partial_config_attrib_list[])
{
	static bool is_waffle_initialized = false;
	static int32_t initialized_platform = 0;

	bool ok = true;

	if (is_waffle_initialized) {
		assert(platform == initialized_platform);
	} else {
		const int32_t attrib_list[] = {
			WAFFLE_PLATFORM, platform,
			0,
		};

		wfl_checked_init(attrib_list);
		is_waffle_initialized = true;
		initialized_platform = platform;
	}

	ok = piglit_gl_framework_init(&wfl_fw->gl_fw, test_config);
	if (!ok)
		goto fail;

	wfl_fw->platform = platform;
	wfl_fw->display = wfl_checked_display_connect(NULL);
	choose_config(wfl_fw, partial_config_attrib_list);
	wfl_fw->context = wfl_checked_context_create(wfl_fw->config, NULL);
	wfl_fw->window = wfl_checked_window_create(wfl_fw->config,
	                                           test_config->window_width,
	                                           test_config->window_height);
	wfl_checked_make_current(wfl_fw->display,
	                         wfl_fw->window,
	                         wfl_fw->context);

#ifdef PIGLIT_USE_OPENGL
	piglit_dispatch_default_init();
#endif

	return true;

fail:
	piglit_wfl_framework_teardown(wfl_fw);
	return false;
}

void
piglit_wfl_framework_teardown(struct piglit_wfl_framework *wfl_fw)
{
	waffle_window_destroy(wfl_fw->window);
	waffle_context_destroy(wfl_fw->context);
	waffle_config_destroy(wfl_fw->config);
	waffle_display_disconnect(wfl_fw->display);

	piglit_gl_framework_teardown(&wfl_fw->gl_fw);
}
