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
 *
 * Authors:
 *    Chad Versace <chad.versace@intel.com>
 */

/**
 * \file hiz-depth-test-fbo-d24s8.c
 *
 * Checks that depth tests work correctly when rendering to a window FBO with
 * the following attachments:
 *     - GL_COLOR_ATTACHMENT0: GL_RGBA
 *     - GL_DEPTH_STENCIL_ATTACHMENT: GL_DEPTH24_STENCIL8
 *
 * \author Chad Versace <chad.versace@intel.com>
 */

#include "piglit-util-gl.h"
#include "hiz/hiz-util.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 400;
	config.window_height = 400;
	config.window_visual = PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

struct hiz_fbo_options fbo_options = {
	GL_RGBA,
	0,
	0,
	GL_DEPTH24_STENCIL8,
};

void
piglit_init(int argc, char **argv)
{
	/* empty */
}

enum piglit_result
piglit_display()
{
	if (hiz_run_test_depth_test_fbo(&fbo_options))
		return PIGLIT_PASS;
	else
		return PIGLIT_FAIL;
}
