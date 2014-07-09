/*
 * Copyright © 2014 Intel Corporation
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
 * ARB_compute_shader extension.
 */

#include "piglit-util-gl.h"
#include "minmax-test.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.supports_gl_core_version = 31;

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
	piglit_require_extension("GL_ARB_compute_shader");
	piglit_print_minmax_header();

	piglit_test_min_int_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, 65535);
	piglit_test_min_int_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, 65535);
	piglit_test_min_int_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, 65535);
	piglit_test_min_int_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, 1024);
	piglit_test_min_int_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, 1024);
	piglit_test_min_int_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, 64);
	piglit_test_min_int(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, 1024);
	piglit_test_min_int(GL_MAX_COMPUTE_UNIFORM_BLOCKS, 12);
	piglit_test_min_int(GL_MAX_COMPUTE_TEXTURE_IMAGE_UNITS, 16);
	piglit_test_min_int(GL_MAX_COMPUTE_ATOMIC_COUNTER_BUFFERS, 8);
	piglit_test_min_int(GL_MAX_COMPUTE_ATOMIC_COUNTERS, 8);
	piglit_test_min_int(GL_MAX_COMPUTE_SHARED_MEMORY_SIZE, 32768);
	piglit_test_min_int(GL_MAX_COMPUTE_UNIFORM_COMPONENTS, 512);
	piglit_test_min_int(GL_MAX_COMPUTE_IMAGE_UNIFORMS, 8);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);
	piglit_report_result(piglit_minmax_pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
