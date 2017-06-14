/* Copyright © 2017 Intel Corporation
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

/** @file coverage.c
 * Positive and negative enum coverage test.
 */

#include "piglit-util-gl.h"
#include "minmax-test.h"

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
check_initial_state(void)
{
	union {
		GLfloat f;
		GLboolean b;
		GLint i;
	} values;
	bool pass = true;
	const GLenum expected_error =
		piglit_is_extension_supported("GL_NV_fog_distance")
		? GL_NO_ERROR : GL_INVALID_ENUM;

	printf("Check getting initial state...\n");
	values.i = 0xDEADBEEF;
	glGetBooleanv(GL_FOG_DISTANCE_MODE_NV, &values.b);
	pass = piglit_check_gl_error(expected_error) && pass;
	if (expected_error != GL_NO_ERROR) {
		if (values.i != 0xDEADBEEF) {
			fprintf(stderr,
				"glGetBooleanv should not have written output, "
				"but it did.\n");
			pass = false;
		}
	} else {
		/* None of the possible values for GL_FOG_DISTANCE_MODE_NV are
		 * zero, so the Boolean getter can only return GL_TRUE.
		 */
		if (values.b != GL_TRUE) {
			fprintf(stderr,
				"glGetBooleanv did not write a valid value "
				"(values.b = 0x%02x)\n", values.b);
			pass = false;
		}
	}

	values.i = 0xDEADBEEF;
	glGetFloatv(GL_FOG_DISTANCE_MODE_NV, &values.f);
	pass = piglit_check_gl_error(expected_error) && pass;
	if (expected_error != GL_NO_ERROR) {
		if (values.i != 0xDEADBEEF) {
			fprintf(stderr,
				"glGetFloatv should not have written output, "
				"but it did.\n");
			pass = false;
		}
	} else {
		if (values.f != (float)GL_EYE_RADIAL_NV &&
		    values.f != (float)GL_EYE_PLANE &&
		    values.f != (float)GL_EYE_PLANE_ABSOLUTE_NV) {
			fprintf(stderr,
				"glGetFloatv did not write a valid value "
				"(f = %f, unsigned(f) = 0x%04x)\n",
				values.f, (unsigned) values.f);
			pass = false;
		}
	}

	values.i = 0xDEADBEEF;
	glGetIntegerv(GL_FOG_DISTANCE_MODE_NV, &values.i);
	pass = piglit_check_gl_error(expected_error) && pass;
	if (expected_error != GL_NO_ERROR) {
		if (values.i != 0xDEADBEEF) {
			fprintf(stderr,
				"glGetIntegerv should not have written output, "
				"but it did.\n");
			pass = false;
		}
	} else {
		if (values.i != GL_EYE_RADIAL_NV &&
		    values.i != GL_EYE_PLANE &&
		    values.i != GL_EYE_PLANE_ABSOLUTE_NV) {
			fprintf(stderr,
				"glGetIntegerv did not write a valid value "
				"(i = 0x%04x)\n", values.i);
			pass = false;
		}
	}

	return pass;
}

static bool
check_readback_value(GLenum expected_error, GLint expected_value,
		     const char *function)
{
	GLint got_value;

	got_value = 0xDEADBEEF;
	glGetIntegerv(GL_FOG_DISTANCE_MODE_NV, &got_value);
	if (expected_error != GL_NO_ERROR) {
		if (got_value != 0xDEADBEEF) {
			fprintf(stderr,
				"glGetIntegerv should not have written output, "
				"but it did.\n");
			return false;
		}
	} else {
		if (got_value != expected_value) {
			fprintf(stderr,
				"Did not read back the value that was just "
				"set by glFog%s (got 0x%04x, expected "
				"0x%04x)\n",
				function, got_value, expected_value);
			return false;
		}
	}

	return true;
}

static bool
check_setting_state(void)
{
	bool pass = true;
	const GLenum expected_error =
		piglit_is_extension_supported("GL_NV_fog_distance")
		? GL_NO_ERROR : GL_INVALID_ENUM;

	static const GLint modes[] = {
		GL_EYE_RADIAL_NV,
		GL_EYE_PLANE,
		GL_EYE_PLANE_ABSOLUTE_NV
	};


	printf("Check setting state...\n");
	for (unsigned i = 0; i < ARRAY_SIZE(modes); i++) {
		const GLfloat float_mode = modes[i];

		glFogi(GL_FOG_DISTANCE_MODE_NV, modes[i]);
		pass = piglit_check_gl_error(expected_error) && pass;
		pass = check_readback_value(expected_error, modes[i], "i") &&
			pass;

		glFogiv(GL_FOG_DISTANCE_MODE_NV, &modes[i]);
		pass = piglit_check_gl_error(expected_error) && pass;
		pass = check_readback_value(expected_error, modes[i], "iv") &&
			pass;

		glFogf(GL_FOG_DISTANCE_MODE_NV, float_mode);
		pass = piglit_check_gl_error(expected_error) && pass;
		pass = check_readback_value(expected_error, modes[i], "f") &&
			pass;

		glFogfv(GL_FOG_DISTANCE_MODE_NV, &float_mode);
		pass = piglit_check_gl_error(expected_error) && pass;
		pass = check_readback_value(expected_error, modes[i], "fv") &&
			pass;
	}

	/* Seriously... nobody supports GL_SGIS_fog_function.  Use its value
	 * as a negative test for GL_NV_fog_distance.
	 */
	if (piglit_is_extension_supported("GL_NV_fog_distance") &&
	    !piglit_is_extension_supported("GL_SGIS_fog_function")) {
		const GLint int_mode = GL_FOG_FUNC_SGIS;
		const GLfloat float_mode = (GLfloat) int_mode;

		glFogi(GL_FOG_DISTANCE_MODE_NV, modes[0]);
		pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

		printf("Check setting invalid state...\n");
		glFogi(GL_FOG_DISTANCE_MODE_NV, int_mode);
		pass = piglit_check_gl_error(GL_INVALID_ENUM) && pass;
		pass = check_readback_value(GL_NO_ERROR, modes[0], "i") &&
			pass;

		glFogiv(GL_FOG_DISTANCE_MODE_NV, &int_mode);
		pass = piglit_check_gl_error(GL_INVALID_ENUM) && pass;
		pass = check_readback_value(GL_NO_ERROR, modes[0], "iv") &&
			pass;

		glFogf(GL_FOG_DISTANCE_MODE_NV, float_mode);
		pass = piglit_check_gl_error(GL_INVALID_ENUM) && pass;
		pass = check_readback_value(GL_NO_ERROR, modes[0], "f") &&
			pass;

		glFogfv(GL_FOG_DISTANCE_MODE_NV, &float_mode);
		pass = piglit_check_gl_error(GL_INVALID_ENUM) && pass;
		pass = check_readback_value(GL_NO_ERROR, modes[0], "fv") &&
			pass;
	}

	return pass;
}

void
piglit_init(int argc, char **argv)
{
	bool pass = true;

	if (!piglit_is_extension_supported("GL_NV_fog_distance")) {
		printf("Expecting all setters and getters to generate "
		       "errors.\n\n");
	}

	pass = check_initial_state() && pass;
	pass = check_setting_state() && pass;

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
