/*
 * Copyright © 2016 Samuel Pitoiset
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

/** \file
 *
 * Test for the minimum maximum values specified in the
 * ARB_compute_variable_group_size extension.
 */

#include "piglit-util-gl.h"
#include "minmax-test.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 33;
	config.supports_gl_core_version = 33;

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
	piglit_require_extension("GL_ARB_compute_variable_group_size");
	piglit_print_minmax_header();

	piglit_test_min_int_v(GL_MAX_COMPUTE_VARIABLE_GROUP_SIZE_ARB, 0, 512);
	piglit_test_min_int_v(GL_MAX_COMPUTE_VARIABLE_GROUP_SIZE_ARB, 1, 512);
	piglit_test_min_int_v(GL_MAX_COMPUTE_VARIABLE_GROUP_SIZE_ARB, 2, 64);
	piglit_test_min_int(GL_MAX_COMPUTE_VARIABLE_GROUP_INVOCATIONS_ARB, 512);
	piglit_test_min_int_v(GL_MAX_COMPUTE_FIXED_GROUP_SIZE_ARB, 0, 1024);
	piglit_test_min_int_v(GL_MAX_COMPUTE_FIXED_GROUP_SIZE_ARB, 1, 1024);
	piglit_test_min_int_v(GL_MAX_COMPUTE_FIXED_GROUP_SIZE_ARB, 2, 64);
	piglit_test_min_int(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, 1024);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);
	piglit_report_result(piglit_minmax_pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
