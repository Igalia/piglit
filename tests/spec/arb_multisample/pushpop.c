/*
 * Copyright © 2011 Intel Corporation
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

#include "piglit-util-gl.h"

/**
 * @file pushpop.c
 *
 * From the GL_ARB_multisample spec:
 *
 *     "An additional group of state variables, MULTISAMPLE_BIT_ARB,
 *      is defined by this extension.  When PushAttrib is called with
 *      bit MULTISAMPLE_BIT_ARB set, the multisample group of state
 *      variables is pushed onto the attribute stack.  When PopAttrib
 *      is called, these state variables are restored to their
 *      previous values if they were pushed.  Some multisample state
 *      is included in the ENABLE_BIT group as well. In order to avoid
 *      incompatibility with GL implementations that do not support
 *      SGIS_multisample, ALL_ATTRIB_BITS does not include
 *      MULTISAMPLE_BIT_ARB."
 *
 *      Get Value                       Get Command    Type    Initial Value    Attribute
 *      ---------                       -----------    ----    -------------    ---------
 *      MULTISAMPLE_ARB                 IsEnabled      B       TRUE             multisample/enable
 *      SAMPLE_ALPHA_TO_COVERAGE_ARB    IsEnabled      B       FALSE            multisample/enable
 *      SAMPLE_ALPHA_TO_ONE_ARB         IsEnabled      B       FALSE            multisample/enable
 *      SAMPLE_COVERAGE_ARB             IsEnabled      B       FALSE            multisample/enable
 *
 *      SAMPLE_COVERAGE_VALUE_ARB       GetFloatv      R+      1                multisample
 *      SAMPLE_COVERAGE_INVERT_ARB      GetBooleanv    B       FALSE            multisample
 *
 */

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	/* UNREACHED */
	return PIGLIT_FAIL;
}

static bool
test_bool(GLenum e, const char *name, bool val)
{
	GLboolean ret = glIsEnabled(e);

	if (ret != val) {
		fprintf(stderr, "  %s %d doesn't match expected %d\n",
			name, ret, val);
		return false;
	} else {
		return true;
	}
}

static bool
test_enable_bits(bool val)
{
	bool pass = true;

	pass = test_bool(GL_MULTISAMPLE, "multisample", val) && pass;
	pass = test_bool(GL_SAMPLE_ALPHA_TO_COVERAGE, "alpha to coverage", val) && pass;
	pass = test_bool(GL_SAMPLE_ALPHA_TO_ONE, "alpha to one", val) && pass;
	pass = test_bool(GL_SAMPLE_COVERAGE, "sample coverage", val) && pass;

	return pass;
}

static void
set_enable_bits(bool val)
{
	if (val) {
		glEnable(GL_MULTISAMPLE);
		glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
		glEnable(GL_SAMPLE_ALPHA_TO_ONE);
		glEnable(GL_SAMPLE_COVERAGE);
	} else {
		glDisable(GL_MULTISAMPLE);
		glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
		glDisable(GL_SAMPLE_ALPHA_TO_ONE);
		glDisable(GL_SAMPLE_COVERAGE);
	}
}

static bool
test_coverage(bool mode)
{
	bool pass = true;
	GLboolean invert;

	float expected = 0.25 + (mode ? 0.5 : 0.0), coverage;

	glGetFloatv(GL_SAMPLE_COVERAGE_VALUE, &coverage);
	if (coverage != expected) {
		fprintf(stderr,
			"  coverage value %f doesn't match expected %f\n",
			expected, coverage);
		pass = false;
	}

	glGetBooleanv(GL_SAMPLE_COVERAGE_INVERT, &invert);
	if (invert != mode) {
		fprintf(stderr,
			"  coverage invert value %d doesn't match expected %d\n",
			invert, mode);
		pass = false;
	}

	return pass;
}

static bool
test_state(bool enable_on, bool coverage_mode)
{
	return test_enable_bits(enable_on) & test_coverage(coverage_mode);
}

static void
set_coverage(bool mode)
{
	float coverage_val = 0.25 + (mode ? 0.5 : 0.0);

	glSampleCoverageARB(coverage_val, mode);
}

static bool
pushpop(GLuint bit, const char *test,
	bool affects_enabled, bool affects_other)
{
	printf("%s test:\n", test);

	set_enable_bits(true);
	set_coverage(true);

	if (bit != 0) {
		glPushAttrib(bit);
		set_enable_bits(false);
		set_coverage(false);
		glPopAttrib();
	}

	if (!test_state(affects_enabled, affects_other))
		return false;

	/* Now, test the bits the other direction.  Caught a bug in my
	 * first pass of fixing Mesa.
	 */
	set_enable_bits(false);
	set_coverage(false);

	if (bit != 0) {
		glPushAttrib(bit);
		set_enable_bits(true);
		set_coverage(true);
		glPopAttrib();
	}

	return test_state(!affects_enabled, !affects_other);
}

void
piglit_init(int argc, char **argv)
{
	bool pass = true;

	piglit_require_extension("GL_ARB_multisample");

	pass = pushpop(0, "sanity test", true, true);
	if (!pass)
		piglit_report_result(PIGLIT_FAIL);
	pass = pushpop(GL_MULTISAMPLE_BIT, "GL_MULTISAMPLE_BIT",
		       true, true) && pass;
	pass = pushpop(GL_ENABLE_BIT, "GL_ENABLE_BIT",
		       true, false) && pass;
	pass = pushpop(GL_ALL_ATTRIB_BITS, "GL_ALL_ATTRIB_BITS",
		       true, false) && pass;

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
