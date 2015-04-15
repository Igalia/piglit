/*
 * Copyright (c) 2015 Intel Corporation
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/** \file minmax.c
 *
 * Test for the minimum and maximum values specified in the
 * ARB_framebuffer_no_attachments extension. This test is written against
 * OpenGL 2.0, for OpenGL < 4.2 spec states:
 *
 *     "For implementations supporting this extension on older versions, the
 *     minimums can be determined from the table below.
 *
 *     the minimum for           is the minimum defined for
 *     -----------------------   --------------------------
 *     MAX_FRAMEBUFFER_WIDTH     MAX_TEXTURE_SIZE
 *     MAX_FRAMEBUFFER_HEIGHT    MAX_TEXTURE_SIZE
 *     MAX_FRAMEBUFFER_LAYERS    MAX_ARRAY_TEXTURE_LAYERS
 *     MAX_FRAMEBUFFER_SAMPLES   MAX_SAMPLES
 *     "
 */

#include "piglit-util-gl.h"
#include "minmax-test.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 20;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	/* UNREACHED */
	return PIGLIT_FAIL;
}

static void
test_min_int(GLenum value, GLenum min_value)
{
	GLint min;
	glGetIntegerv(min_value, &min);
	piglit_test_min_int(value, min);
}

static void
texture_array_subtest()
{
	bool result;
	if (!piglit_is_extension_supported("GL_EXT_texture_array"))
		piglit_report_subtest_result(PIGLIT_SKIP, "layers");

	test_min_int(GL_MAX_FRAMEBUFFER_LAYERS, GL_MAX_ARRAY_TEXTURE_LAYERS);

	result = piglit_minmax_pass ? PIGLIT_PASS : PIGLIT_FAIL;

	piglit_report_subtest_result(result, "layers");
}

void
piglit_init(int argc, char **argv)
{
        if (piglit_get_gl_version() < 30 ||
		!piglit_is_extension_supported("GL_ARB_framebuffer_object"))
		piglit_report_result(PIGLIT_SKIP);

	piglit_require_extension("GL_ARB_framebuffer_no_attachments");
	piglit_print_minmax_header();

	test_min_int(GL_MAX_FRAMEBUFFER_WIDTH, GL_MAX_TEXTURE_SIZE);
	test_min_int(GL_MAX_FRAMEBUFFER_HEIGHT, GL_MAX_TEXTURE_SIZE);
	test_min_int(GL_MAX_FRAMEBUFFER_SAMPLES, GL_MAX_SAMPLES);

	texture_array_subtest();

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	piglit_report_result(piglit_minmax_pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
