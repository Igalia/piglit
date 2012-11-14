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

enum context_flavor {
	CONTEXT_GL_CORE,
	CONTEXT_GL_COMPAT,
	CONTEXT_GL_ES1,
	CONTEXT_GL_ES2,
};

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

	else if (strcmp(env, "gbm") == 0) {
#ifdef PIGLIT_HAS_GBM
		return WAFFLE_PLATFORM_GBM;
#else
		fprintf(stderr, "environment var PIGLIT_PLATFORM=gbm, but "
		        "piglit was built without GBM support\n");
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
	memcpy(r + 2 * a_length, b, b_copy_size);
	r[2 * r_length] = 0;
	return r;
}

static struct waffle_config*
choose_config(struct piglit_wfl_framework *wfl_fw,
              enum context_flavor flavor,
              const int32_t partial_attrib_list[])
{
	const struct piglit_gl_test_config *test_config = wfl_fw->gl_fw.test_config;

	struct waffle_config *config;
	int32_t head_attrib_list[64];
	int32_t *full_attrib_list;
	int32_t junk;
	int i;

	/* Derived class must not provide any context attributes. */
	assert(waffle_attrib_list_get(partial_attrib_list, WAFFLE_CONTEXT_API, &junk) == false);
	assert(waffle_attrib_list_get(partial_attrib_list, WAFFLE_CONTEXT_PROFILE, &junk) == false);
	assert(waffle_attrib_list_get(partial_attrib_list, WAFFLE_CONTEXT_MAJOR_VERSION, &junk) == false);
	assert(waffle_attrib_list_get(partial_attrib_list, WAFFLE_CONTEXT_MINOR_VERSION, &junk) == false);

	switch (flavor) {
		case CONTEXT_GL_CORE:
			assert(test_config->supports_gl_core_version);

			i = 0;
			head_attrib_list[i++] = WAFFLE_CONTEXT_API;
			head_attrib_list[i++] = WAFFLE_CONTEXT_OPENGL;

			if (test_config->supports_gl_core_version >= 32) {
				head_attrib_list[i++] = WAFFLE_CONTEXT_PROFILE;
				head_attrib_list[i++] = WAFFLE_CONTEXT_CORE_PROFILE;
			}

			head_attrib_list[i++] = WAFFLE_CONTEXT_MAJOR_VERSION;
			head_attrib_list[i++] = test_config->supports_gl_core_version / 10;

			head_attrib_list[i++] = WAFFLE_CONTEXT_MINOR_VERSION;
			head_attrib_list[i++] = test_config->supports_gl_core_version % 10;

			head_attrib_list[i++] = 0;
			break;

		case CONTEXT_GL_COMPAT:
			assert(test_config->supports_gl_compat_version);

			i = 0;
			head_attrib_list[i++] = WAFFLE_CONTEXT_API;
			head_attrib_list[i++] = WAFFLE_CONTEXT_OPENGL;

			if (test_config->supports_gl_compat_version >= 32) {
				head_attrib_list[i++] = WAFFLE_CONTEXT_PROFILE;
				head_attrib_list[i++] = WAFFLE_CONTEXT_COMPATIBILITY_PROFILE;
			}

			head_attrib_list[i++] = WAFFLE_CONTEXT_MAJOR_VERSION;
			head_attrib_list[i++] = test_config->supports_gl_compat_version / 10;

			head_attrib_list[i++] = WAFFLE_CONTEXT_MINOR_VERSION;
			head_attrib_list[i++] = test_config->supports_gl_compat_version % 10;

			head_attrib_list[i++] = 0;
			break;

		case CONTEXT_GL_ES1:
			assert(test_config->supports_gl_es1);

			i = 0;
			head_attrib_list[i++] = WAFFLE_CONTEXT_API;
			head_attrib_list[i++] = WAFFLE_CONTEXT_OPENGL_ES1;
			head_attrib_list[i++] = 0;
			break;

		case CONTEXT_GL_ES2:
			assert(test_config->supports_gl_es2);

			i = 0;
			head_attrib_list[i++] = WAFFLE_CONTEXT_API;
			head_attrib_list[i++] = WAFFLE_CONTEXT_OPENGL_ES2;
			head_attrib_list[i++] = 0;
			break;

		default:
			assert(0);
			break;
	}

	full_attrib_list = concat_attrib_lists(head_attrib_list,
	                                       partial_attrib_list);

	config = waffle_config_choose(wfl_fw->display,
	                              full_attrib_list);
	if (!config)
		wfl_log_debug("waffle_config_choose");

	free(full_attrib_list);
	return config;
}

/**
 * Handle the special case when a test requests GL 3.1.
 */
static bool
special_case_gl_31(const struct piglit_gl_test_config *test_config,
                   enum context_flavor flavor)
{
	int gl_version;

	if (flavor == CONTEXT_GL_CORE
	    && test_config->supports_gl_core_version == 31) {

		gl_version = piglit_get_gl_version();
		assert(gl_version >= 31);

		if (gl_version == 31
		    && piglit_is_extension_supported("GL_ARB_compatibility"))
			return false;
		else
			return true;

	} else if (flavor == CONTEXT_GL_COMPAT
	           && test_config->supports_gl_compat_version == 31) {

		gl_version = piglit_get_gl_version();
		assert(gl_version >= 31);

		if (gl_version == 31
		    && !piglit_is_extension_supported("GL_ARB_compatibility"))
			return false;
		else
			return true;
	} else {
		/* No need to check the special case. */
		return true;
	}
}

static bool
make_context_current_singlepass(struct piglit_wfl_framework *wfl_fw,
                                const struct piglit_gl_test_config *test_config,
                                enum context_flavor flavor,
                                const int32_t partial_config_attrib_list[])
{
	bool ok;

	assert(wfl_fw->config == NULL);
	assert(wfl_fw->context == NULL);
	assert(wfl_fw->window == NULL);

	wfl_fw->config = choose_config(wfl_fw, flavor,
	                               partial_config_attrib_list);
	if (!wfl_fw->config)
		goto fail;

	wfl_fw->context = waffle_context_create(wfl_fw->config, NULL);
	if (!wfl_fw->context) {
		wfl_log_debug("waffle_context_create");
		goto fail;
	}

	wfl_fw->window = wfl_checked_window_create(wfl_fw->config,
	                                           test_config->window_width,
	                                           test_config->window_height);

	wfl_checked_make_current(wfl_fw->display,
	                         wfl_fw->window,
	                         wfl_fw->context);

#ifdef PIGLIT_USE_OPENGL
	piglit_dispatch_default_init();
#endif

	ok = special_case_gl_31(test_config, flavor);
	if (!ok)
		goto fail;

	return true;

fail:
	waffle_window_destroy(wfl_fw->window);
	waffle_context_destroy(wfl_fw->context);
	waffle_config_destroy(wfl_fw->config);

	wfl_fw->window = NULL;
	wfl_fw->context = NULL;
	wfl_fw->config = NULL;

	return false;
}

static void
make_context_current(struct piglit_wfl_framework *wfl_fw,
                     const struct piglit_gl_test_config *test_config,
                     const int32_t partial_config_attrib_list[])
{
	bool ok = false;

#if defined(PIGLIT_USE_OPENGL)

	if (test_config->supports_gl_core_version) {
		ok = make_context_current_singlepass(wfl_fw, test_config,
		                                     CONTEXT_GL_CORE,
		                                     partial_config_attrib_list);
		if (ok) {
			return;
		} else {
			printf("piglit: info: Failed to create GL %d.%d "
			       "core context\n",
			       test_config->supports_gl_core_version / 10,
			       test_config->supports_gl_core_version % 10);
		}
	}

	if (test_config->supports_gl_compat_version) {
		ok = make_context_current_singlepass(wfl_fw, test_config,
		                                     CONTEXT_GL_COMPAT,
		                                     partial_config_attrib_list);
		if (ok) {
			return;
		} else {
			printf("piglit: info: Failed to create GL %d.%d "
			       "compatibility context\n",
			       test_config->supports_gl_compat_version / 10,
			       test_config->supports_gl_compat_version % 10);
		}
	}

#elif defined(PIGLIT_USE_OPENGL_ES1)
	ok = make_context_current_singlepass(wfl_fw, test_config,
	                                     CONTEXT_GL_ES1,
	                                     partial_config_attrib_list);

	if (ok)
		return;
	else
		printf("piglit: info: Failed to create GL ES1 context\n");

#elif defined(PIGLIT_USE_OPENGL_ES2)
	ok = make_context_current_singlepass(wfl_fw, test_config,
	                                     CONTEXT_GL_ES2,
	                                     partial_config_attrib_list);

	if (ok)
		return;
	else
		printf("piglit: info: Failed to create GL ES2 context\n");
#else
#	error
#endif

	if (!ok) {
		printf("piglit: info: Failed to create any GL context\n");
		piglit_report_result(PIGLIT_SKIP);
	}
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
	make_context_current(wfl_fw, test_config, partial_config_attrib_list);

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
