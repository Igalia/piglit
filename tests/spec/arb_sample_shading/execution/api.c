/*
 * Copyright © 2013 Intel Corporation
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

/**
 * @file api.c
 *
 * Tests new APIs and enums added by ARB_sample_shading spec:
 */


#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	/* Unreached */
	return PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	float value;
	bool pass = true;
	piglit_require_extension("GL_ARB_sample_shading");

	pass = !glIsEnabled(GL_SAMPLE_SHADING_ARB) && pass;
	glEnable(GL_SAMPLE_SHADING_ARB);
	pass = glIsEnabled(GL_SAMPLE_SHADING_ARB) && pass;
	glDisable(GL_SAMPLE_SHADING_ARB);
	pass = !glIsEnabled(GL_SAMPLE_SHADING_ARB) && pass;
	piglit_check_gl_error(GL_NO_ERROR);

	glGetFloatv(GL_MIN_SAMPLE_SHADING_VALUE_ARB, &value);
	pass = (value == 0.0) && pass;
	glMinSampleShadingARB(0.5);
	glGetFloatv(GL_MIN_SAMPLE_SHADING_VALUE_ARB, &value);
	pass = (value == 0.5) && pass;
        /* Verify GL_MIN_SAMPLE_SHADING_VALUE_ARB is clamped to range [0, 1] */
	glMinSampleShadingARB(1.5);
	glGetFloatv(GL_MIN_SAMPLE_SHADING_VALUE_ARB, &value);
	pass = (value == 1.0) && pass;
	glMinSampleShadingARB(-0.5);
	glGetFloatv(GL_MIN_SAMPLE_SHADING_VALUE_ARB, &value);
	pass = (value == 0.0) && pass;
	piglit_check_gl_error(GL_NO_ERROR);

	piglit_report_result( pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
