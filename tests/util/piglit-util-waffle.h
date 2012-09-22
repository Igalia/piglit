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

/**
 * \file
 * \brief Waffle utilities
 *
 * Each function wfl_checked_*, if the backing Waffle function fails, prints
 * the error message emitted by Waffle and ends the test.
 */

#pragma once

#include <waffle.h>

/**
 * \brief Print the current Waffle error.
 *
 * The \a func_name is the name of most recently called Waffle function.
 */
void
wfl_log_error(const char *func_name);

/**
 * \brief Print the current Waffle error if PIGLIT_DEBUG=1.
 *
 * The \a func_name is the name of most recently called Waffle function.
 */
void
wfl_log_debug(const char *func_name);

/**
 * \brief Print the current Waffle error and end the test.
 *
 * The \a func_name is the name of most recently called Waffle function.
 *
 * If the error is WAFFLE_ERROR_UNSUPPORTED_ON_PLATFORM, skip the test.
 * Otherwise, fail the test.
 */
void
wfl_fatal_error(const char *func_name);

static inline void
wfl_checked_init(const int32_t *attrib_list)
{
	bool ok = waffle_init(attrib_list);
	if (!ok)
		wfl_fatal_error("waffle_init");
}

static inline struct waffle_display*
wfl_checked_display_connect(const char *name)
{
	struct waffle_display *dpy = waffle_display_connect(name);
	if (!dpy)
		wfl_fatal_error("waffle_display_connect");
	return dpy;
}

static inline struct waffle_config*
wfl_checked_config_choose(struct waffle_display *dpy,
                           const int32_t *attrib_list)
{
	struct waffle_config *config = waffle_config_choose(dpy, attrib_list);
	if (!config)
		wfl_fatal_error("waffle_attrib_list");
	return config;
}

static inline struct waffle_context*
wfl_checked_context_create(struct waffle_config *config,
                           struct waffle_context *shared_ctx)
{
	struct waffle_context *ctx = waffle_context_create(config, NULL);
	if (!ctx)
		wfl_fatal_error("waffle_context_create");
	return ctx;
}

static inline struct waffle_window*
wfl_checked_window_create(struct waffle_config *config,
                             int32_t width, int32_t height)
{
	struct waffle_window *window = waffle_window_create(config, width, height);
	if (!window)
		wfl_fatal_error("waffle_window_create");
	return window;
}

static inline bool
wfl_checked_make_current(struct waffle_display *dpy,
                         struct waffle_window *window,
                         struct waffle_context *ctx)
{
	bool ok = waffle_make_current(dpy, window, ctx);
	if (!ok)
		wfl_fatal_error("waffle_make_current");
	return ok;
}
