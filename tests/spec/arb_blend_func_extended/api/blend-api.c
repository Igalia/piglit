/* Copyright © 2012 Red Hat.
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
 * \file blend-api.c
 *
 * \author Dave Airlie
 * Test additions to blending API from ARB_blend_func_extended
 */
#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

void piglit_init(int argc, char **argv)
{
	GLint max_dual_source;

	piglit_require_gl_version(30);
	piglit_require_extension("GL_ARB_blend_func_extended");

	/* This test needs some number of draw buffers, so make sure the
	 * implementation isn't broken.  This enables the test to generate a
	 * useful failure message.
	 */
	glGetIntegerv(GL_MAX_DUAL_SOURCE_DRAW_BUFFERS, &max_dual_source);
	if (max_dual_source < 1) {
		fprintf(stderr,
			"ARB_blend_func_extended requires GL_MAX_DUAL_SOURCE_DRAW_BUFFERS >= 1.  "
			"Only got %d!\n",
			max_dual_source);
		piglit_report_result(PIGLIT_FAIL);
	}

	printf("Querying blend mode (SRC1_COLOR, 0).\n");
	/* try all new blending modes */
	glBlendFunc(GL_SRC1_COLOR, GL_ZERO);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	printf("Querying blend mode (SRC1_ALPHA, 0)\n");
	glBlendFunc(GL_SRC1_ALPHA, GL_ZERO);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	printf("Querying blend mode (1-SRC1_COLOR, 0)\n");
	glBlendFunc(GL_ONE_MINUS_SRC1_COLOR, GL_ZERO);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	printf("Querying blend mode (1-SRC1_ALPHA, 0)\n");
	glBlendFunc(GL_ONE_MINUS_SRC1_ALPHA, GL_ZERO);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	printf("Querying blend mode (0, SRC1_COLOR)\n");
	glBlendFunc(GL_ZERO, GL_SRC1_COLOR);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	printf("Querying blend mode (0, SRC1_ALPHA)\n");
	glBlendFunc(GL_ZERO, GL_SRC1_ALPHA);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	printf("Querying blend mode (0, 1-SRC1_COLOR)\n");
	glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC1_COLOR);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	printf("Querying blend mode (0, 1-SRC1_ALPHA)\n");
	glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC1_ALPHA);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	/* GL_SRC_ALPHA_SATURATE is now handled as a DST attrib */
	printf("Querying blend mode (0, SRC_ALPHA_SATURATE)\n");
	glBlendFunc(GL_ZERO, GL_SRC_ALPHA_SATURATE);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	piglit_report_result(PIGLIT_PASS);
}
