/*
 * Copyright © 2015 Aditya Atluri <adityaavinash1@gmail.com>
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
 * Test for the minimum and maximum values specified in the
 * ARB_shader_storage_buffer_object spec.
 */
#include "piglit-util-gl.h"
#include "minmax-test.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 40;
	config.supports_gl_core_version = 40;

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
	piglit_require_extension("GL_ARB_shader_storage_buffer_object");
	piglit_print_minmax_header();

	piglit_test_min_int(GL_MAX_VERTEX_SHADER_STORAGE_BLOCKS, 0);
	piglit_test_min_int(GL_MAX_GEOMETRY_SHADER_STORAGE_BLOCKS, 0);
	piglit_test_min_int(GL_MAX_TESS_CONTROL_SHADER_STORAGE_BLOCKS, 0);
	piglit_test_min_int(GL_MAX_TESS_EVALUATION_SHADER_STORAGE_BLOCKS, 0);
	piglit_test_min_int(GL_MAX_FRAGMENT_SHADER_STORAGE_BLOCKS, 8);
	piglit_test_min_int(GL_MAX_COMPUTE_SHADER_STORAGE_BLOCKS, 8);
	piglit_test_min_int(GL_MAX_COMBINED_SHADER_STORAGE_BLOCKS, 8);
	piglit_test_min_int(GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS, 8);
	piglit_test_min_int64(GL_MAX_SHADER_STORAGE_BLOCK_SIZE, 16777216); // 2^24
	piglit_test_max_int(GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT, 256);
	piglit_test_min_int(GL_MAX_COMBINED_SHADER_OUTPUT_RESOURCES, 8);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);
	piglit_report_result(piglit_minmax_pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
