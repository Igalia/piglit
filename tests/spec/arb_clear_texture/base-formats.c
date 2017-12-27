/*
 * Copyright (c) 2014 Intel Corporation
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

/** @file base-formats.c
 *
 * Test using glClearTexSubImage with a range of formats.
 */

#include "common.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 13;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

static const struct format
formats[] = {
	{ GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, 4 },
	{ GL_RGB, GL_RGB, GL_UNSIGNED_BYTE, 3 },
	{ GL_RGBA, GL_BGRA, GL_UNSIGNED_BYTE, 4 },
	{ GL_RGB, GL_BGR, GL_UNSIGNED_BYTE, 3 },
	{ GL_RGBA, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, 4 },
	{ GL_RGBA, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, 4 },
	{ GL_ALPHA, GL_ALPHA, GL_UNSIGNED_BYTE, 1 },
};

void
piglit_init(int argc, char **argv)
{
	bool pass;

	pass = test_formats(formats, ARRAY_SIZE(formats));

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result
piglit_display(void)
{
	/* unused */
	return PIGLIT_FAIL;
}
