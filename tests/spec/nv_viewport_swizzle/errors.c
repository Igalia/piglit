/*
 * Copyright 2020 Ilia Mirkin
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
#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 11;
	config.supports_gl_es_version = 31;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;
	config.khr_no_error_support = PIGLIT_HAS_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	GLint max_viewports;
	GLint swizzle_x, swizzle_y, swizzle_z, swizzle_w;

	piglit_require_extension("GL_NV_viewport_swizzle");

	glGetIntegerv(GL_MAX_VIEWPORTS, &max_viewports);
	glGetIntegeri_v(GL_VIEWPORT_SWIZZLE_X_NV, 0, &swizzle_x);
	glGetIntegeri_v(GL_VIEWPORT_SWIZZLE_Y_NV, 0, &swizzle_y);
	glGetIntegeri_v(GL_VIEWPORT_SWIZZLE_Z_NV, 0, &swizzle_z);
	glGetIntegeri_v(GL_VIEWPORT_SWIZZLE_W_NV, 0, &swizzle_w);

	if (swizzle_x != GL_VIEWPORT_SWIZZLE_POSITIVE_X_NV ||
	    swizzle_y != GL_VIEWPORT_SWIZZLE_POSITIVE_Y_NV ||
	    swizzle_z != GL_VIEWPORT_SWIZZLE_POSITIVE_Z_NV ||
	    swizzle_w != GL_VIEWPORT_SWIZZLE_POSITIVE_W_NV) {
		printf("Invalid initial state of viewport swizzles.\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	glViewportSwizzleNV(max_viewports,
			    swizzle_x, swizzle_y, swizzle_z, swizzle_w);
	if (!piglit_check_gl_error(GL_INVALID_VALUE)) {
		printf("Out-of-bounds viewport index generates wrong error.\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	glViewportSwizzleNV(0, GL_RGBA8, swizzle_y, swizzle_z, swizzle_w);
	if (!piglit_check_gl_error(GL_INVALID_ENUM)) {
		printf("Wrong error for invalid swizzle_x enum.\n");
		piglit_report_result(PIGLIT_FAIL);
	}
	glViewportSwizzleNV(0, swizzle_x, GL_RGBA8, swizzle_z, swizzle_w);
	if (!piglit_check_gl_error(GL_INVALID_ENUM)) {
		printf("Wrong error for invalid swizzle_y enum.\n");
		piglit_report_result(PIGLIT_FAIL);
	}
	glViewportSwizzleNV(0, swizzle_x, swizzle_y, GL_RGBA8, swizzle_w);
	if (!piglit_check_gl_error(GL_INVALID_ENUM)) {
		printf("Wrong error for invalid swizzle_z enum.\n");
		piglit_report_result(PIGLIT_FAIL);
	}
	glViewportSwizzleNV(0, swizzle_x, swizzle_y, swizzle_z, GL_RGBA8);
	if (!piglit_check_gl_error(GL_INVALID_ENUM)) {
		printf("Wrong error for invalid swizzle_w enum.\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	piglit_report_result(PIGLIT_PASS);
}
