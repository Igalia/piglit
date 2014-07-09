/*
 * Copyright © 2013 LunarG, Inc.
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
 * Author: Jon Ashburn <jon@lunarg.com>
 */

/**
 * Tests GL_ARB_viewport_array  validity for indices.
 * Use both valid and invalid parameters (index, first, count)
 * for all these new API entry points:
 * glDepthRangeArrayv, glDepthRangeIndexed, glGetDoublei_v
 *
 */

#include "piglit-util-gl.h"
#include <stdarg.h>

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 32;
	config.supports_gl_core_version = 32;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

/**
 * Test that DepthRangeArrayv, DepthRangeIndexed, GetDoublei_v give the
 * "expected_error" gl error.  Given the values for "first" and "count"
 * or "index" in range [first, first+count).
 */
static bool
check_dr_index(GLuint first, GLsizei count, GLenum expected_error)
{
	static const GLclampd dv[] = {0.213, 1.0};
	GLclampd *mv, dvGet[2];
	unsigned int i;
	bool pass = true;
	const unsigned int numIterate = (expected_error == GL_NO_ERROR) ? count : 1;

	mv = malloc(sizeof(GLclampd) * 2 * count);
	if (mv == NULL)
		return false;
	for (i = 0; i < count; i++) {
		mv[i * 2] = dv[0];
		mv[i * 2 + 1] = dv[1];
	}
	glDepthRangeArrayv(first, count, mv);
	free(mv);
	pass = piglit_check_gl_error(expected_error) && pass;

	/* only iterate multiple indices for no error case */
	for (i = count; i > count - numIterate; i--) {
		glDepthRangeIndexed(first + i - 1, dv[0], dv[1]);
		pass = piglit_check_gl_error(expected_error) && pass;
		glGetDoublei_v(GL_DEPTH_RANGE, first + i - 1, dvGet);
		pass = piglit_check_gl_error(expected_error) && pass;
	}

	return pass;
}

/**
 * Test first + count or index valid and invalid values.
 * Valid range is 0 thru (MAX_VIEWPORTS-1).
 */
static bool
test_dr_indices(GLint maxVP)
{
	bool pass = true;


	/**
	 * valid largest range depth index
	 * OpenGL Core 4.3 Spec, section 13.6.1 ref:
	 *    "Multiple viewports are available and are numbered zero
	 *    through the value of MAX_VIEWPORTS minus one."
	 */
	if (!check_dr_index(0, maxVP, GL_NO_ERROR)) {
		printf("Got error for valid depth range, max range=%u\n",
		       maxVP);
		pass = false;
	}

	/**
	 * invalid count + first index for DepthRange
	 * OpenGL Spec Core 4.3 Spec, section 13.6.1 ref:
	 *     "An INVALID_VALUE error is generated if first + count
	 *     is greater than the valuue of MAX_VIEWPORTS."
	 */
	if (!check_dr_index(maxVP - 2, 3, GL_INVALID_VALUE)) {
		printf("Wrong error for invalid DepthRange index range\n");
		pass = false;
	}

	/**
	 * invalid count for DepthRange
	 * OpenGL Spec Core 4.3 Spec, section 13.6.1 ref:
	 *    "An INVALID_VALUE error is generated if count is negative."
	 */
	glDepthRangeArrayv(0, -1, NULL);
	if (!piglit_check_gl_error(GL_INVALID_VALUE)) {
		printf("Wrong error for invalid DepthRange count\n");
		pass = false;
	}

	return pass;
}


enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	bool pass = true;
	GLint maxVP;

	piglit_require_extension("GL_ARB_viewport_array");

	glGetIntegerv(GL_MAX_VIEWPORTS, &maxVP);
	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		piglit_report_result(PIGLIT_FAIL);
	} else {
		pass = test_dr_indices(maxVP) && pass;
		pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
		piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
	}
}
