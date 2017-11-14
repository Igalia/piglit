/*
 * Copyright (C) 2008  VMware, Inc.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
 * KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL ALLEN AKIN BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 */

/**
 * @file texture-unit.c:  Test texture unit things
 * Author: Brian Paul  31 Dec 2008
 *
 * We're generally just testing API-related things, not rendering.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 20;
	config.window_visual = PIGLIT_GL_VISUAL_RGB;
	config.khr_no_error_support = PIGLIT_HAS_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

static int max_combined_units;
static int max_image_units;
static int max_coord_units;
static int max_units;

static void
setup(void)
{
	glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS,
		      &max_combined_units);
	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &max_image_units);
	glGetIntegerv(GL_MAX_TEXTURE_COORDS, &max_coord_units);
	glGetIntegerv(GL_MAX_TEXTURE_UNITS, &max_units);
}

static bool
test_limits(void)
{
	if (max_image_units < max_units) {
		printf("GL_MAX_TEXTURE_IMAGE_UNITS < GL_MAX_TEXTURE_UNITS");
		return false;
	}
	if (max_coord_units < max_units) {
		printf("GL_MAX_TEXTURE_COORD_UNITS < GL_MAX_TEXTURE_UNITS");
		return false;
	}
	return true;
}

static bool
test_active_texture(void)
{
	/* test glActiveTexture() */
	for (int i = 0; i < max_combined_units; i++) {
		glActiveTexture(GL_TEXTURE0 + i);
		if (!piglit_check_gl_error(GL_NO_ERROR)) {
			printf("glActiveTexture(GL_TEXTURE%d) failed", i);
			return false;
		}

		int unit;
		glGetIntegerv(GL_ACTIVE_TEXTURE, &unit);
		if (!piglit_check_gl_error(GL_NO_ERROR) ||
		    unit != GL_TEXTURE0 + i) {
			printf("glGetIntegerv(GL_ACTIVE_TEXTURE) failed");
			return false;
		}
	}

	/* this should fail: */
	glActiveTexture(GL_TEXTURE0 + max_combined_units);
	if (!piglit_check_gl_error(GL_INVALID_ENUM)) {
		printf("glActiveTexture(GL_TEXTURE%d) failed to generate an "
		       "error", max_combined_units);
		return false;
	}


	/* test glClientActiveTexture() */
	for (int i = 0; i < max_coord_units; i++) {
		glClientActiveTexture(GL_TEXTURE0 + i);
		if (!piglit_check_gl_error(GL_NO_ERROR)) {
			printf("glClientActiveTexture(GL_TEXTURE%d) failed",
			       i);
			return false;
		}

		int unit;
		glGetIntegerv(GL_CLIENT_ACTIVE_TEXTURE, &unit);
		if (!piglit_check_gl_error(GL_NO_ERROR) ||
		    unit != GL_TEXTURE0 + i) {
			printf("glGetIntegerv(GL_CLIENT_ACTIVE_TEXTURE) "
			       "failed");
			return false;
		}
	}

	/* this should fail: */
	glClientActiveTexture(GL_TEXTURE0 + max_coord_units);
	if (!piglit_check_gl_error(GL_INVALID_ENUM)) {
		printf("glClientActiveTexture(GL_TEXTURE%d) failed to "
		       "generate an error", max_coord_units);
		return false;
	}

	return true;
}

static bool
test_texture_matrices(void)
{
	glActiveTexture(GL_TEXTURE0);
	glMatrixMode(GL_TEXTURE);

	/* set texture matrices */
	for (int i = 0; i < max_coord_units; i++) {
		glActiveTexture(GL_TEXTURE0 + i);

		/* generate matrix */
		float m[16];
		for (int j = 0; j < 16; j++) {
			m[j] = (float)(i * 100 + j);
		}

		glLoadMatrixf(m);
	}

	/* query texture matrices */
	for (int i = 0; i < max_coord_units; i++) {
		glActiveTexture(GL_TEXTURE0 + i);

		/* get matrix and check it */
		float m[16] = {0};
		glGetFloatv(GL_TEXTURE_MATRIX, m);

		if (!piglit_check_gl_error(GL_NO_ERROR)) {
			printf("Query of texture matrix %d raised an error",
			       i);
			return false;
		}

		for (int j = 0; j < 16; j++) {
			if (m[j] != (float)(i * 100 + j)) {
				printf("Query of texture matrix %d failed",
				       i);
				return false;
			}
		}
	}

	return true;
}

static bool
test_texture_coord_gen(void)
{
	glActiveTexture(GL_TEXTURE0);
	glMatrixMode(GL_TEXTURE);

	/* test texgen enable/disable */
	for (int i = 0; i < max_combined_units; i++) {
		glActiveTexture(GL_TEXTURE0 + i);

		glEnable(GL_TEXTURE_GEN_S);
		glEnable(GL_TEXTURE_GEN_T);
		glEnable(GL_TEXTURE_GEN_R);
		glEnable(GL_TEXTURE_GEN_Q);
		if (i < max_coord_units) {
			/* should be no error */
			if (!piglit_check_gl_error(GL_NO_ERROR)) {
				printf("GL error was generated by enabling "
				       "GL_TEXTURE_GEN_x, unit %d", i);
				return false;
			}
			glDisable(GL_TEXTURE_GEN_S);
			glDisable(GL_TEXTURE_GEN_T);
			glDisable(GL_TEXTURE_GEN_R);
			glDisable(GL_TEXTURE_GEN_Q);
		} else {
			/* should be an error */
			if (!piglit_check_gl_error(GL_INVALID_OPERATION)) {
				printf("GL error not generated by invalid "
				       "enable of GL_TEXTURE_GEN_x, unit %d",
				       i);
				return false;
			}
		}
	}

	return true;
}

static bool
test_texcoord_arrays(void)
{
	for (int i = 0; i < max_coord_units; i++) {
		glClientActiveTexture(GL_TEXTURE0 + i);

		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		if (!piglit_check_gl_error(GL_NO_ERROR)) {
			printf("GL error was generated by glEnableClientState"
			       " for unit %d", i);
			return false;
		}
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	}

	return true;
}

void
piglit_init(int argc, char **argv)
{
	bool pass = true;

	setup();

	pass = pass && test_limits();

	pass = pass && test_active_texture();

	pass = pass && test_texture_matrices();

	pass = pass && test_texture_coord_gen();

	pass = pass && test_texcoord_arrays();

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result
piglit_display(void)
{
	/* UNREACHED */
	return PIGLIT_FAIL;
}
