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

#pragma once

#include <waffle.h>

#include "piglit_gl_framework.h"

struct piglit_wfl_framework {
	struct piglit_gl_framework gl_fw;

	/**
	 * One of WAFFLE_PLATFORM_*.
	 */
	int32_t platform;

	struct waffle_display *display;
	struct waffle_config *config;
	struct waffle_context *context;
	struct waffle_window *window;
};

/**
 * Typesafe cast.
 */
struct piglit_wfl_framework*
piglit_wfl_framework(struct piglit_gl_framework *gl_fw);

/**
 * @param platform must be one of WAFFLE_PLATFORM_*.
 */
bool
piglit_wfl_framework_init(struct piglit_wfl_framework *wfl_fw,
                          const struct piglit_gl_test_config *test_config,
                          int32_t platform,
                          const int32_t partial_config_attrib_list[]);

void
piglit_wfl_framework_teardown(struct piglit_wfl_framework *wfl_fw);

/**
 * Used by subclasses to choose the waffle platform. Returns one of
 * WAFFLE_PLATFORM_*.
 */
int32_t
piglit_wfl_framework_choose_platform(void);
