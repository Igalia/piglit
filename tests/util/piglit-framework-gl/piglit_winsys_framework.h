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

#include "piglit_wfl_framework.h"

/**
 * Abstract class. Use winsys_framework_factory() to create a concrete
 * instance.
 */
struct piglit_winsys_framework {
	struct piglit_wfl_framework wfl_fw;

	/**
	 * Has the user requested a redisplay with
	 * piglit_gl_framework::post_redisplay?
	 */
	bool need_redisplay;

	/**
	 * Must be implemented by subclasses.
	 *
	 * TODO(chad): This could be removed.
	 */
	void
	(*show_window)(struct piglit_winsys_framework *winsys_fw);

	void
	(*enter_event_loop)(struct piglit_winsys_framework *winsys_fw);

	/**
	 * Set with piglit_gl_framework::set_keyboard_func. May be null.
	 */
	void
	(*user_keyboard_func)(unsigned char key, int x, int y);

	/**
	 * Set with piglit_gl_framework::user_reshape_func. May be null.
	 */
	void
	(*user_reshape_func)(int width, int height);
};

/**
 * Typesafe cast.
 */
struct piglit_winsys_framework*
piglit_winsys_framework(struct piglit_gl_framework *gl_fw);

struct piglit_gl_framework*
piglit_winsys_framework_factory(const struct piglit_gl_test_config *test_config);

/**
 * @param platform must be one of WAFFLE_PLATFORM_*.
 */
bool
piglit_winsys_framework_init(struct piglit_winsys_framework *winsys_fw,
                             const struct piglit_gl_test_config *test_config,
                             int32_t platform);

void
piglit_winsys_framework_teardown(struct piglit_winsys_framework *winsys_fw);
