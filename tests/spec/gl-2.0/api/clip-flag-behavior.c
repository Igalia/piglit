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

/**
 * \file clip-flag-behavior.c
 *
 * This test verifies the following basic behaviors of the GL_CLIP_PLANEi
 * flags:
 *
 * - There are exactly MAX_CLIP_PLANES of them, and trying to access a
 *   nonexistent flag produces a GL_INVALID_ENUM error.
 *
 * - They default to false.
 *
 * - Their behavior under glGetBooleanv, glIsEnabled, glEnable, and
 *   glDisable is consistent.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

static char *bool_to_string(bool b)
{
	if (b)
		return "true";
	else
		return "false";
}

static bool
check_bool(GLboolean b, bool expected)
{
	/* Negate the bools before comparing so that distinct nonzero
	 * values compare equal.
	 */
	if ((!b) == (!expected)) {
		return true;
	} else {
		printf("Expected %s, got %s\n", bool_to_string(expected),
		       bool_to_string(b));
		return false;
	}
}

/**
 * Print "OK" and return true.  This is helpful in chaining with the
 * functions above, e.g.:
 *
 * bool pass = piglit_check_gl_error(...) && check_bool(...) && print_ok();
 */
static bool
print_ok()
{
	printf("OK\n");
	return true;
}

static bool
check_enable_state(char *enum_name, GLenum enum_value, bool expected)
{
	GLboolean b;

	bool pass = true;
	printf("Trying glIsEnabled(%s): ", enum_name);
	b = glIsEnabled(enum_value);
	pass = piglit_check_gl_error(GL_NO_ERROR) && check_bool(b, expected)
		&& print_ok() && pass;

	printf("Trying glGetBooleanv(%s): ", enum_name);
	glGetBooleanv(enum_value, &b);
	pass = piglit_check_gl_error(GL_NO_ERROR) && check_bool(b, expected)
		&& print_ok() && pass;

	return pass;
}

void
piglit_init(int argc, char **argv)
{
	GLint max_clip_planes;
	bool pass = true;
	GLint plane;
	GLboolean b;
	char enum_name[38];
	GLenum enum_value;

	printf("Querying GL_MAX_CLIP_PLANES: ");
	glGetIntegerv(GL_MAX_CLIP_PLANES, &max_clip_planes);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);
	printf("%d\n", max_clip_planes);
	if (max_clip_planes < 0) {
		printf("Error: GL_MAX_CLIP_PLANES must be >= 0\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	/* Check behavior of GL_CLIP_PLANE0 + i where i < max_clip_planes */
	for (plane = 0; plane < max_clip_planes; ++plane) {
		enum_value = GL_CLIP_PLANE0 + plane;
		sprintf(enum_name, "GL_CLIP_PLANE0 + %d", plane);

		pass = check_enable_state(enum_name, enum_value, false) && pass;

		printf("Trying glEnable(%s): ", enum_name);
		glEnable(enum_value);
		pass = piglit_check_gl_error(GL_NO_ERROR) && print_ok() && pass;

		pass = check_enable_state(enum_name, enum_value, true) && pass;

		printf("Trying glDisable(%s): ", enum_name);
		glDisable(enum_value);
		pass = piglit_check_gl_error(GL_NO_ERROR) && print_ok() && pass;

		pass = check_enable_state(enum_name, enum_value, false) && pass;
	}

	/* Check behavior of GL_CLIP_PLANE0 + n where n == max_clip_planes */
	enum_value = GL_CLIP_PLANE0 + max_clip_planes;
	sprintf(enum_name, "GL_CLIP_PLANE0 + %d", max_clip_planes);

	printf("Trying glIsEnabled(%s): ", enum_name);
	b = glIsEnabled(enum_value);
	pass = piglit_check_gl_error(GL_INVALID_ENUM) && print_ok() && pass;

	printf("Trying glGetBooleanv(%s): ", enum_name);
	glGetBooleanv(enum_value, &b);
	pass = piglit_check_gl_error(GL_INVALID_ENUM) && print_ok() && pass;

	printf("Trying glEnable(%s): ", enum_name);
	glEnable(enum_value);
	pass = piglit_check_gl_error(GL_INVALID_ENUM) && print_ok() && pass;

	printf("Trying glDisable(%s): ", enum_name);
	glDisable(enum_value);
	pass = piglit_check_gl_error(GL_INVALID_ENUM) && print_ok() && pass;

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
