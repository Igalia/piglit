/**
 * Copyright © 2018 Intel Corporation
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

/**
 * Test that GL_DEPTH_CLAMP_FAR_AMD and GL_DEPTH_CLAMP_NEAR_AMD is a
 * valid state
 *
 * Table 6.9 (Transformation state) of OpenGL 4.1 Core added
 * DEPTH_CLAMP_FAR_AMD and DEPTH_CLAMP_NEAR_AMD
 *
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

PIGLIT_GL_TEST_CONFIG_END

static bool
check_all_enum_queries(GLenum e, const char *name, GLboolean expected)
{
	GLint i;
	GLfloat f;
	GLboolean b;
	GLdouble d;

	if (glIsEnabled(e) != expected) {
		fprintf(stderr, "%s was not enabled properly\n", name);
		return false;
	}

	glGetIntegerv(e, &i);
	if (i != (GLint) expected) {
		fprintf(stderr, "%s: i expected to be 0, but returned %d\n",
			name, i);
		return false;
	}

	glGetFloatv(e, &f);
	if (f != (GLfloat) expected) {
		fprintf(stderr, "%s: f expected to be 0.0, but returned %f\n",
			name, f);
		return false;
	}

	glGetBooleanv(e, &b);
	if (b != expected) {
		fprintf(stderr, "%s: b expected to be 0, but returned %d\n",
			name, (int)b);
		return false;
	}

	glGetDoublev(e, &d);
	if (d != (GLdouble) expected) {
		fprintf(stderr, "%s: d expected to be 0.0, but returned %f\n",
			name, d);
		return false;
	}

	return true;
}

void
piglit_init(int argc, char **argv)
{
	bool pass = true;

	piglit_require_extension("GL_AMD_depth_clamp_separate");
	piglit_require_extension("GL_ARB_depth_clamp");

	/* Check the initial state */
	pass = check_all_enum_queries(GL_DEPTH_CLAMP, "GL_DEPTH_CLAMP",
				      GL_FALSE) && pass;
	pass = check_all_enum_queries(GL_DEPTH_CLAMP_FAR_AMD,
				      "GL_DEPTH_CLAMP_FAR_AMD",
				      GL_FALSE) && pass;
	pass = check_all_enum_queries(GL_DEPTH_CLAMP_NEAR_AMD,
				      "GL_DEPTH_CLAMP_NEAR_AMD",
				      GL_FALSE) && pass;

	glEnable(GL_DEPTH_CLAMP_NEAR_AMD);
	pass = check_all_enum_queries(GL_DEPTH_CLAMP, "GL_DEPTH_CLAMP",
				      GL_TRUE) && pass;
	pass = check_all_enum_queries(GL_DEPTH_CLAMP_FAR_AMD,
				      "GL_DEPTH_CLAMP_FAR_AMD",
				      GL_FALSE) && pass;
	pass = check_all_enum_queries(GL_DEPTH_CLAMP_NEAR_AMD,
				      "GL_DEPTH_CLAMP_NEAR_AMD",
				      GL_TRUE) && pass;

	glEnable(GL_DEPTH_CLAMP_FAR_AMD);
	pass = check_all_enum_queries(GL_DEPTH_CLAMP, "GL_DEPTH_CLAMP",
				      GL_TRUE) && pass;
	pass = check_all_enum_queries(GL_DEPTH_CLAMP_FAR_AMD,
				      "GL_DEPTH_CLAMP_FAR_AMD",
				      GL_TRUE) && pass;
	pass = check_all_enum_queries(GL_DEPTH_CLAMP_NEAR_AMD,
				      "GL_DEPTH_CLAMP_NEAR_AMD",
				      GL_TRUE) && pass;

	glDisable(GL_DEPTH_CLAMP_NEAR_AMD);
	pass = check_all_enum_queries(GL_DEPTH_CLAMP, "GL_DEPTH_CLAMP",
				      GL_TRUE) && pass;
	pass = check_all_enum_queries(GL_DEPTH_CLAMP_FAR_AMD,
				      "GL_DEPTH_CLAMP_FAR_AMD",
				      GL_TRUE) && pass;
	pass = check_all_enum_queries(GL_DEPTH_CLAMP_NEAR_AMD,
				      "GL_DEPTH_CLAMP_NEAR_AMD",
				      GL_FALSE) && pass;

	glDisable(GL_DEPTH_CLAMP_FAR_AMD);
	pass = check_all_enum_queries(GL_DEPTH_CLAMP, "GL_DEPTH_CLAMP",
				      GL_FALSE) && pass;
	pass = check_all_enum_queries(GL_DEPTH_CLAMP_FAR_AMD,
				      "GL_DEPTH_CLAMP_FAR_AMD",
				      GL_FALSE) && pass;
	pass = check_all_enum_queries(GL_DEPTH_CLAMP_NEAR_AMD,
				      "GL_DEPTH_CLAMP_NEAR_AMD",
				      GL_FALSE) && pass;

	/* The GL_AMD_depth_clamp_separate spec says:
	 *
	 *    In addition to DEPTH_CLAMP_NEAR_AMD and DEPTH_CLAMP_FAR_AMD,
	 *    the token DEPTH_CLAMP may be used to simultaneously enable or
	 *    disable depth clamping at both the near and far planes.
	 */
	glEnable(GL_DEPTH_CLAMP);
	pass = check_all_enum_queries(GL_DEPTH_CLAMP, "GL_DEPTH_CLAMP",
				      GL_TRUE) && pass;
	pass = check_all_enum_queries(GL_DEPTH_CLAMP_FAR_AMD,
				      "GL_DEPTH_CLAMP_FAR_AMD",
				      GL_TRUE) && pass;
	pass = check_all_enum_queries(GL_DEPTH_CLAMP_NEAR_AMD,
				      "GL_DEPTH_CLAMP_NEAR_AMD",
				      GL_TRUE) && pass;

	glDisable(GL_DEPTH_CLAMP);
	pass = check_all_enum_queries(GL_DEPTH_CLAMP, "GL_DEPTH_CLAMP",
				      GL_FALSE) && pass;
	pass = check_all_enum_queries(GL_DEPTH_CLAMP_FAR_AMD,
				      "GL_DEPTH_CLAMP_FAR_AMD",
				      GL_FALSE) && pass;
	pass = check_all_enum_queries(GL_DEPTH_CLAMP_NEAR_AMD,
				      "GL_DEPTH_CLAMP_NEAR_AMD",
				      GL_FALSE) && pass;

	glEnable(GL_DEPTH_CLAMP_FAR_AMD);
	glEnable(GL_DEPTH_CLAMP_NEAR_AMD);
	glDisable(GL_DEPTH_CLAMP);
	pass = check_all_enum_queries(GL_DEPTH_CLAMP, "GL_DEPTH_CLAMP",
				      GL_FALSE) && pass;
	pass = check_all_enum_queries(GL_DEPTH_CLAMP_FAR_AMD,
				      "GL_DEPTH_CLAMP_FAR_AMD",
				      GL_FALSE) && pass;
	pass = check_all_enum_queries(GL_DEPTH_CLAMP_NEAR_AMD,
				      "GL_DEPTH_CLAMP_NEAR_AMD",
				      GL_FALSE) && pass;

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}
