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

/** @file minmax.c
 *
 * Test for the minimum maximum values listed in section 23 "State Tables"
 * (23.54) of the GL Core profile 4.3 spec relating to ARB_viewport_array.
 * Tested GLenums are GL_MAX_VIEWPORT_DIMS, GL_MAX_VIEWPORTS,
 * GL_VIEWPORT_SUBPIXEL_BITS, GL_VIEWPORT_BOUNDS_RANGE,
 * GL_LAYER_PROVOKING_VERTEX, GL_VIEWPORT_INDEX_PROVOKING_VERTEX.
 */

#include "piglit-util-gl.h"
#include "minmax-test.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_core_version = 32;
	config.supports_gl_compat_version = 32;

	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	/* UNREACHED */
	return PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	GLint layer, index;

	piglit_require_extension("GL_ARB_viewport_array");
	piglit_print_minmax_header();

	piglit_test_min_viewport_dimensions(); /* GL_MAX_VIEWPORT_DIMS */
	piglit_test_min_int(GL_MAX_VIEWPORTS, 16);
	piglit_test_min_int(GL_VIEWPORT_SUBPIXEL_BITS, 0);

	/* ARB_viewport_array extension spec says:
	 *    "NOTE 2: range for viewport bounds:
	 *    On GL3-capable hardware the VIEWPORT_BOUNDS_RANGE should be at
	 *    least [-16384, 16383].
	 *    On GL4-capable hardware the VIEWPORT_BOUNDS_RANGE should be at
	 *    least [-32768, 32767]."
	 */
	/* Since no known way to determine GL3 versus GL4 capable hardware use
	   GL version instead */
	if (piglit_get_gl_version() < 40)
		piglit_test_range_float(GL_VIEWPORT_BOUNDS_RANGE, -16384, 16383);
	else
		piglit_test_range_float(GL_VIEWPORT_BOUNDS_RANGE, -32768, 32767);

	/**
	 * ARB_viewport_array extension spec says regarding PROVOKING_VERTEX:
	 *    "NOTE 3: Valid values are: FIRST_VERTEX_CONVENTION,
	 *    LAST_VERTEX_CONVENTION, PROVOKING_VERTEX, UNDEFINED_VERTEX."
	 */
	glGetIntegerv(GL_LAYER_PROVOKING_VERTEX, &layer);
	piglit_minmax_pass = piglit_check_gl_error(GL_NO_ERROR)
		&& piglit_minmax_pass;

	switch (layer) {
	case GL_FIRST_VERTEX_CONVENTION:
	case GL_LAST_VERTEX_CONVENTION:
	case GL_PROVOKING_VERTEX:
	case GL_UNDEFINED_VERTEX:
		printf("%s				      %s\n",
		       piglit_get_gl_enum_name(GL_LAYER_PROVOKING_VERTEX),
		       piglit_get_gl_enum_name(layer));
		break;
	default:
		 piglit_minmax_pass = false;
		 printf("Invalid value for GL_LAYER_PROVOKING_VERTEX\n");
		 break;
	}

	glGetIntegerv(GL_VIEWPORT_INDEX_PROVOKING_VERTEX, &index);
	piglit_minmax_pass = piglit_check_gl_error(GL_NO_ERROR)
		&& piglit_minmax_pass;

	switch (index) {
	case GL_FIRST_VERTEX_CONVENTION:
	case GL_LAST_VERTEX_CONVENTION:
	case GL_PROVOKING_VERTEX:
	case GL_UNDEFINED_VERTEX:
		printf("%s			      %s\n",
		       piglit_get_gl_enum_name(GL_VIEWPORT_INDEX_PROVOKING_VERTEX),
		       piglit_get_gl_enum_name(index));
		break;
	default:
		piglit_minmax_pass = false;
		printf("Invalid value for GL_VIEWPORT_INDEX_PROVOKING_VERTEX\n");
		break;
	}

	piglit_report_result(piglit_minmax_pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
