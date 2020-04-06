/*
 * Copyright © 2020 Advanced Micro Devices, Inc.
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

/** @file api_errors.c
 *
 * This test checks if: -
 * 1. the dither control parameter is set to the correct value by default
 * 2. correct value of dither control parameter can be retrieved after the
 *    same is set
 * 3. correct error value is generated when an invalid value is passed to the
 *    glAlphaToCoverageDitherControlNV API.
 *      Allowed values are:-
 *         GL_ALPHA_TO_COVERAGE_DITHER_DEFAULT_NV
 *         GL_ALPHA_TO_COVERAGE_DITHER_ENABLE_NV
 *         GL_ALPHA_TO_COVERAGE_DITHER_DISABLE_NV
 */
#include "piglit-util-gl.h"
#include "piglit-util.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 44;
	config.khr_no_error_support = PIGLIT_HAS_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

static bool
test_errors(void)
{
	bool pass = true;
	int  ditherVal = 0;

	/* Check if the default value is correct  */
	glGetIntegerv(GL_ALPHA_TO_COVERAGE_DITHER_MODE_NV, &ditherVal);
	pass &= piglit_check_gl_error(GL_NO_ERROR);
	pass &= (ditherVal == GL_ALPHA_TO_COVERAGE_DITHER_DEFAULT_NV);

	/* Check if proper values are returned when set  */
	glAlphaToCoverageDitherControlNV(GL_ALPHA_TO_COVERAGE_DITHER_ENABLE_NV);
	pass &= piglit_check_gl_error(GL_NO_ERROR);
	glGetIntegerv(GL_ALPHA_TO_COVERAGE_DITHER_MODE_NV, &ditherVal);
	pass &= piglit_check_gl_error(GL_NO_ERROR);
	pass &= (ditherVal == GL_ALPHA_TO_COVERAGE_DITHER_ENABLE_NV);

	glAlphaToCoverageDitherControlNV(GL_ALPHA_TO_COVERAGE_DITHER_DISABLE_NV);
	pass &= piglit_check_gl_error(GL_NO_ERROR);
	glGetIntegerv(GL_ALPHA_TO_COVERAGE_DITHER_MODE_NV, &ditherVal);
	pass &= piglit_check_gl_error(GL_NO_ERROR);
	pass &= (ditherVal == GL_ALPHA_TO_COVERAGE_DITHER_DISABLE_NV);

	glAlphaToCoverageDitherControlNV(GL_ALPHA_TO_COVERAGE_DITHER_DEFAULT_NV);
	pass &= piglit_check_gl_error(GL_NO_ERROR);
	glGetIntegerv(GL_ALPHA_TO_COVERAGE_DITHER_MODE_NV, &ditherVal);
	pass &= piglit_check_gl_error(GL_NO_ERROR);
	pass &= (ditherVal == GL_ALPHA_TO_COVERAGE_DITHER_DEFAULT_NV);

	/* Check if proper error value is generated on passing invalid value  */
	glAlphaToCoverageDitherControlNV(1);
	pass &= piglit_check_gl_error(GL_INVALID_ENUM);

	return pass;
}

void
piglit_init(int argc, char **argv)
{
	bool pass = true;

        piglit_require_gl_version(44);
        piglit_require_extension("GL_NV_alpha_to_coverage_dither_control");

	pass &= test_errors();

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result
piglit_display(void)
{
	/* UNREACHABLE */
	return PIGLIT_FAIL;
}
