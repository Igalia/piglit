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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/** \file
 *
 * Test for the minimum maximum values specified in the
 * ARB_tessellation_shader extension.
 */

#include "piglit-util-gl-common.h"
#include "minmax-test.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 32;
	config.supports_gl_core_version = 32;

PIGLIT_GL_TEST_CONFIG_END

#define GETINTV(x) glGetIntegerv(GL_ ## x, &x)

enum piglit_result
piglit_display(void)
{
	/* UNREACHED */
	return PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	GLint MAX_TESS_CONTROL_UNIFORM_COMPONENTS;
	GLint MAX_TESS_EVALUATION_UNIFORM_COMPONENTS;
	GLint MAX_TESS_CONTROL_UNIFORM_BLOCKS;
	GLint MAX_TESS_EVALUATION_UNIFORM_BLOCKS;
	GLint MAX_UNIFORM_BLOCK_SIZE;

	piglit_require_extension("GL_ARB_tessellation_shader");
	piglit_print_minmax_header();

	piglit_test_min_int(GL_MAX_TESS_GEN_LEVEL, 64);
	piglit_test_min_int(GL_MAX_PATCH_VERTICES, 32);
	piglit_test_min_int(GL_MAX_TESS_CONTROL_UNIFORM_COMPONENTS, 1024);
	piglit_test_min_int(GL_MAX_TESS_EVALUATION_UNIFORM_COMPONENTS, 1024);
	piglit_test_min_int(GL_MAX_TESS_CONTROL_TEXTURE_IMAGE_UNITS, 16);
	piglit_test_min_int(GL_MAX_TESS_EVALUATION_TEXTURE_IMAGE_UNITS, 16);
	piglit_test_min_int(GL_MAX_TESS_CONTROL_OUTPUT_COMPONENTS, 128);
	piglit_test_min_int(GL_MAX_TESS_PATCH_COMPONENTS, 120);
	piglit_test_min_int(GL_MAX_TESS_CONTROL_TOTAL_OUTPUT_COMPONENTS, 4096);
	piglit_test_min_int(GL_MAX_TESS_EVALUATION_OUTPUT_COMPONENTS, 128);
	piglit_test_min_int(GL_MAX_TESS_CONTROL_INPUT_COMPONENTS, 128);
	piglit_test_min_int(GL_MAX_TESS_EVALUATION_INPUT_COMPONENTS, 128);
	piglit_test_min_int(GL_MAX_TESS_CONTROL_UNIFORM_BLOCKS, 12);
	piglit_test_min_int(GL_MAX_TESS_EVALUATION_UNIFORM_BLOCKS, 12);

	GETINTV(MAX_TESS_CONTROL_UNIFORM_COMPONENTS);
	GETINTV(MAX_TESS_EVALUATION_UNIFORM_COMPONENTS);
	GETINTV(MAX_TESS_CONTROL_UNIFORM_BLOCKS);
	GETINTV(MAX_TESS_EVALUATION_UNIFORM_BLOCKS);
	GETINTV(MAX_UNIFORM_BLOCK_SIZE);
	piglit_test_min_int(GL_MAX_COMBINED_TESS_CONTROL_UNIFORM_COMPONENTS,
			    MAX_TESS_CONTROL_UNIFORM_COMPONENTS +
			    MAX_TESS_CONTROL_UNIFORM_BLOCKS * (MAX_UNIFORM_BLOCK_SIZE/4));
	piglit_test_min_int(GL_MAX_COMBINED_TESS_EVALUATION_UNIFORM_COMPONENTS,
			    MAX_TESS_EVALUATION_UNIFORM_COMPONENTS +
			    MAX_TESS_EVALUATION_UNIFORM_BLOCKS * (MAX_UNIFORM_BLOCK_SIZE/4));

	piglit_test_min_int(GL_MAX_COMBINED_UNIFORM_BLOCKS, 60);
	piglit_test_min_int(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, 80);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);
	piglit_report_result(piglit_minmax_pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
