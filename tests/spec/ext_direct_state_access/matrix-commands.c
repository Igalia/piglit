/*
 * Copyright © 2019 Advanced Micro Devices, Inc.
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

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 20;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

static void
n_floats(float* m, int n) {
	int i;
	for (i = 0; i < n; ++i) {
		m[i] = (float) (rand() % 1000);
	}
}

static void
n_doubles(double* m, int n) {
	int i;
	for (i = 0; i < n; ++i) {
		m[i] = (double) (rand() % 1000);
	}
}

/* The GL_EXT_direct_state_access spec says:
 *
 *    The [new matrix commands] commands are equivalent (assuming no errors)
 *    to the following:
 *
 *        int savedMatrixMode;
 *
 *        GetIntegerv(MATRIX_MODE, &savedMatrixMode);
 *        if (matrixMode >= TEXTURE0 && matrixMode <= TEXTURE31) {
 *            int savedActiveTexture;
 *            MatrixMode(TEXTURE);
 *            GetIntegerv(ACTIVE_TEXTURE, &savedActiveTexture);
 *            ActiveTexture(matrixMode);
 *            XXX(...);
 *            ActiveTexture(savedActiveTexture);
 *        } else {
 *            MatrixMode(matrixMode);
 *            XXX(...);
 *        }
 *
 *
 * So each test implements the 'XXX()' function in 2 versions: one using
 * the core functions (e.g: glLoadMatrixf) + the above pattern, and one
 * using EXT_dsa functions (e.g: glMatrixLoadfEXT).
 *
 * Then we loop over each tests function using all possible combinations
 * of matrix mode (MODELVIEW, PROJECTION, TEXTURE, TEXTUREn) and verify
 * that the matrix values from both versions are identical.
 */

enum mc_test_pass {
	INIT_PASS = 1,
	CORE_PASS = 2,
	EXT_DSA_PASS = 3
};

static void
test_MatrixLoadf(enum mc_test_pass p, GLenum matrix_mode)
{
	static float m[16];
	if (p == INIT_PASS) {
		n_floats(m, ARRAY_SIZE(m));
	} else if (p == CORE_PASS) {
		glLoadMatrixf(m);
	} else if (p == EXT_DSA_PASS) {
		glMatrixLoadfEXT(matrix_mode, m);
	}
}

static void
test_MatrixLoadd(enum mc_test_pass p, GLenum matrix_mode)
{
	static double m[16];
	if (p == INIT_PASS) {
		n_doubles(m, ARRAY_SIZE(m));
	} else if (p == CORE_PASS) {
		glLoadMatrixd(m);
	} else if (p == EXT_DSA_PASS) {
		glMatrixLoaddEXT(matrix_mode, m);
	}
}

static void
test_MatrixMultf(enum mc_test_pass p, GLenum matrix_mode)
{
	static float m1[16];
	static float m2[16];
	if (p == INIT_PASS) {
		n_floats(m1, ARRAY_SIZE(m1));
		n_floats(m2, ARRAY_SIZE(m2));
	} else if (p == CORE_PASS) {
		glLoadMatrixf(m1);
		glMultMatrixf(m2);
	} else if (p == EXT_DSA_PASS) {
		glMatrixLoadfEXT(matrix_mode, m1);
		glMatrixMultfEXT(matrix_mode, m2);
	}
}

static void
test_MatrixMultd(enum mc_test_pass p, GLenum matrix_mode)
{
	static double m1[16];
	static double m2[16];
	if (p == INIT_PASS) {
		n_doubles(m1, ARRAY_SIZE(m1));
		n_doubles(m2, ARRAY_SIZE(m2));
	} else if (p == CORE_PASS) {
		glLoadMatrixd(m1);
		glMultMatrixd(m2);
	} else if (p == EXT_DSA_PASS) {
		glMatrixLoaddEXT(matrix_mode, m1);
		glMatrixMultdEXT(matrix_mode, m2);
	}
}

static void
test_MatrixRotatef(enum mc_test_pass p, GLenum matrix_mode)
{
	static float m[4];
	if (p == INIT_PASS) {
		n_floats(m, ARRAY_SIZE(m));
	} else if (p == CORE_PASS) {
		glLoadIdentity();
		glRotatef(m[0], m[1], m[2], m[3]);
	} else if (p == EXT_DSA_PASS) {
		glMatrixLoadIdentityEXT(matrix_mode);
		glMatrixRotatefEXT(matrix_mode, m[0], m[1], m[2], m[3]);
	}
}

static void
test_MatrixRotated(enum mc_test_pass p, GLenum matrix_mode)
{
	static double m[4];
	if (p == INIT_PASS) {
		n_doubles(m, ARRAY_SIZE(m));
	} else if (p == CORE_PASS) {
		glLoadIdentity();
		glRotated(m[0], m[1], m[2], m[3]);
	} else if (p == EXT_DSA_PASS) {
		glMatrixLoadIdentityEXT(matrix_mode);
		glMatrixRotatedEXT(matrix_mode, m[0], m[1], m[2], m[3]);
	}
}

static void
test_MatrixScalef(enum mc_test_pass p, GLenum matrix_mode)
{
	static float m[3];
	if (p == INIT_PASS) {
		n_floats(m, ARRAY_SIZE(m));
	} else if (p == CORE_PASS) {
		glLoadIdentity();
		glScalef(m[0], m[1], m[2]);
	} else if (p == EXT_DSA_PASS) {
		glMatrixLoadIdentityEXT(matrix_mode);
		glMatrixScalefEXT(matrix_mode, m[0], m[1], m[2]);
	}
}

static void
test_MatrixScaled(enum mc_test_pass p, GLenum matrix_mode)
{
	static double m[3];
	if (p == INIT_PASS) {
		n_doubles(m, ARRAY_SIZE(m));
	} else if (p == CORE_PASS) {
		glLoadIdentity();
		glScaled(m[0], m[1], m[2]);
	} else if (p == EXT_DSA_PASS) {
		glMatrixLoadIdentityEXT(matrix_mode);
		glMatrixScaledEXT(matrix_mode, m[0], m[1], m[2]);
	}
}

static void
test_MatrixTranslatef(enum mc_test_pass p, GLenum matrix_mode)
{
	static float m[3];
	if (p == INIT_PASS) {
		n_floats(m, ARRAY_SIZE(m));
	} else if (p == CORE_PASS) {
		glLoadIdentity();
		glTranslatef(m[0], m[1], m[2]);
	} else if (p == EXT_DSA_PASS) {
		glMatrixLoadIdentityEXT(matrix_mode);
		glMatrixTranslatefEXT(matrix_mode, m[0], m[1], m[2]);
	}
}

static void
test_MatrixTranslated(enum mc_test_pass p, GLenum matrix_mode)
{
	static double m[3];
	if (p == INIT_PASS) {
		n_doubles(m, ARRAY_SIZE(m));
	} else if (p == CORE_PASS) {
		glLoadIdentity();
		glTranslated(m[0], m[1], m[2]);
	} else if (p == EXT_DSA_PASS) {
		glMatrixLoadIdentityEXT(matrix_mode);
		glMatrixTranslatedEXT(matrix_mode, m[0], m[1], m[2]);
	}
}

static void
test_MatrixLoadIdentity(enum mc_test_pass p, GLenum matrix_mode)
{
	if (p == CORE_PASS) {
		glLoadIdentity();
	} else if (p == EXT_DSA_PASS) {
		glMatrixLoadIdentityEXT(matrix_mode);
	}
}

static void
test_MatrixOrtho(enum mc_test_pass p, GLenum matrix_mode)
{
	static float m[6];
	if (p == INIT_PASS) {
		n_floats(m, 3);
		/* make sure left != right, etc */
		m[3] = 1 + m[0];
		m[4] = 1 + m[1];
		m[5] = 1 + m[2];
	} else if (p == CORE_PASS) {
		glLoadIdentity();
		glOrtho(m[0], m[3], m[1], m[4], m[2], m[5]);
	} else if (p == EXT_DSA_PASS) {
		glMatrixLoadIdentityEXT(matrix_mode);
		glMatrixOrthoEXT(matrix_mode, m[0], m[3], m[1], m[4], m[2], m[5]);
	}
}

static void
test_MatrixFrustum(enum mc_test_pass p, GLenum matrix_mode)
{
	static float m[6];
	if (p == INIT_PASS) {
		n_floats(m, 3);
		/* make sure left != right, etc */
		m[3] = 1 + m[0];
		m[4] = 1 + m[1];
		m[5] = 1 + m[2];
	} else if (p == CORE_PASS) {
		glLoadIdentity();
		glFrustum(m[0], m[3], m[1], m[4], m[2], m[5]);
	} else if (p == EXT_DSA_PASS) {
		glMatrixLoadIdentityEXT(matrix_mode);
		glMatrixFrustumEXT(matrix_mode, m[0], m[3], m[1], m[4], m[2], m[5]);
	}
}

static void
test_MatrixPushPop(enum mc_test_pass p, GLenum matrix_mode)
{
	static float m[16];
	if (p == INIT_PASS) {
		n_floats(m, ARRAY_SIZE(m));
	} else if (p == CORE_PASS) {
		glLoadMatrixf(m);
		glPushMatrix();
		glPopMatrix();
	} else if (p == EXT_DSA_PASS) {
		glMatrixLoadfEXT(matrix_mode, m);
		glMatrixPushEXT(matrix_mode);
		glMatrixPopEXT(matrix_mode);
	}
}

static void
test_MatrixLoadTransposef(enum mc_test_pass p, GLenum matrix_mode)
{
	static float m[16];
	if (p == INIT_PASS) {
		n_floats(m, ARRAY_SIZE(m));
	} else if (p == CORE_PASS) {
		glLoadTransposeMatrixf(m);
	} else if (p == EXT_DSA_PASS) {
		glMatrixLoadTransposefEXT(matrix_mode, m);
	}
}

static void
test_MatrixLoadTransposed(enum mc_test_pass p, GLenum matrix_mode)
{
	static double m[16];
	if (p == INIT_PASS) {
		n_doubles(m, ARRAY_SIZE(m));
	} else if (p == CORE_PASS) {
		glLoadTransposeMatrixd(m);
	} else if (p == EXT_DSA_PASS) {
		glMatrixLoadTransposedEXT(matrix_mode, m);
	}
}

static void
test_MatrixMultTransposef(enum mc_test_pass p, GLenum matrix_mode)
{
	static float m1[16];
	static float m2[16];
	if (p == INIT_PASS) {
		n_floats(m1, ARRAY_SIZE(m1));
		n_floats(m2, ARRAY_SIZE(m2));
	} else if (p == CORE_PASS) {
		glLoadMatrixf(m1);
		glMultTransposeMatrixf(m2);
	} else if (p == EXT_DSA_PASS) {
		glMatrixLoadfEXT(matrix_mode, m1);
		glMatrixMultTransposefEXT(matrix_mode, m2);
	}
}

static void
test_MatrixMultTransposed(enum mc_test_pass p, GLenum matrix_mode)
{
	static double m1[16];
	static double m2[16];
	if (p == INIT_PASS) {
		n_doubles(m1, ARRAY_SIZE(m1));
		n_doubles(m2, ARRAY_SIZE(m2));
	} else if (p == CORE_PASS) {
		glLoadMatrixd(m1);
		glMultTransposeMatrixd(m2);
	} else if (p == EXT_DSA_PASS) {
		glMatrixLoaddEXT(matrix_mode, m1);
		glMatrixMultTransposedEXT(matrix_mode, m2);
	}
}

typedef void (*test_matrix_command_fn)(enum mc_test_pass, GLenum);

static GLenum* modes;
static GLenum* get_modes;
static int matrix_mode_count;
static int max_texture_coords;
static GLenum use_display_list = GL_NONE;

static enum piglit_result
test_matrix_command(void* data)
{
	int i;
	bool pass = true;
	int expected_matrix_mode, matrix_mode;
	int expected_active_texture, active_texture;

	float ref_value[16];
	float got[16];
	GLuint list;

	static const float identity[] = {
		1, 0, 0, 0,    0, 1, 0, 0,    0, 0, 1, 0,    0, 0, 0, 1
	};

	test_matrix_command_fn test_fn = (test_matrix_command_fn) data;

	for (i = 0; i < matrix_mode_count; i++) {
		memset(ref_value, 0, 16 * sizeof(float));
		memset(got, 0, 16 * sizeof(float));
		expected_matrix_mode = modes[(i + 1) % 3];
		expected_active_texture = GL_TEXTURE0 + rand() % max_texture_coords;

		test_fn(INIT_PASS, modes[i]);

		/* Execute the core version */
		if (modes[i] >= GL_TEXTURE0) {
			glMatrixMode(GL_TEXTURE);
			glActiveTexture(modes[i]);
		} else {
			glMatrixMode(modes[i]);

			/* The GL_EXT_direct_state_access spec says:
			 *
			 * Is glMatrixLoadfEXT(GL_TEXTURE, matrixData), etc. legal?
			 *
			 * This will update the texture matrix based on the current active
			 * texture.
			 */
			if (modes[i] == GL_TEXTURE) {
				/* So in the GL_TEXTURE case we set the current active
				 * texture to a known value
				 */
				glActiveTexture(expected_active_texture);
			}
		}

		test_fn(CORE_PASS, modes[i]);

		/* Read matrix value */
		glGetFloatv(get_modes[i], ref_value);

		/* Reset state */
		glLoadIdentity();
		glGetFloatv(get_modes[i], got);

		glMatrixMode(expected_matrix_mode);
		glActiveTexture(expected_active_texture);

		if (!piglit_check_gl_error(GL_NO_ERROR)) {
			piglit_loge("Error with mode %s.",
				piglit_get_gl_enum_name(modes[i]));
			return PIGLIT_FAIL;
		}

		if (use_display_list != GL_NONE) {
			list = glGenLists(1);
			glNewList(list, use_display_list);
		}

		/* Execute the EXT_dsa version */
		test_fn(EXT_DSA_PASS, modes[i]);

		if (use_display_list != GL_NONE) {
			glEndList(list);
		}

		/* Verify state wasn't changed */
		glGetIntegerv(GL_MATRIX_MODE, &matrix_mode);
		glGetIntegerv(GL_ACTIVE_TEXTURE, &active_texture);
		pass = (expected_matrix_mode == matrix_mode) && pass;
		pass = (expected_active_texture == active_texture) && pass;
		if (!pass) {
			piglit_loge("State incorrectly modified with mode %s",
				piglit_get_gl_enum_name(modes[i]));
			return PIGLIT_FAIL;
		}

		/* Read matrix value */
		if (modes[i] >= GL_TEXTURE0) {
			glMatrixMode(GL_TEXTURE);
			glActiveTexture(modes[i]);
		} else {
			glMatrixMode(modes[i]);
			glActiveTexture(expected_active_texture);
		}
		glGetFloatv(get_modes[i], got);

		if (use_display_list == GL_COMPILE) {
			/* At this point the matrix should still
			 * be identity because the display list hasn't
			 * be executed
			 */
			if (memcmp(identity, got, sizeof(got)) != 0) {
				piglit_loge("Matrix %s has been modified before glCallList()",
					piglit_get_gl_enum_name(modes[i]));
				return PIGLIT_FAIL;
			}

			glCallList(list);

			/* Re-read matrix */
			glGetFloatv(get_modes[i], got);
		}
		if (use_display_list != GL_NONE) {
			glDeleteLists(list, 1);
		}

		/* Compare values */
		pass = piglit_check_gl_error(GL_NO_ERROR) &&
		       (memcmp(ref_value, got, sizeof(got)) == 0) &&
		       pass;
		if (!pass) {
			piglit_loge("Incorrect matrix with mode %s",
				piglit_get_gl_enum_name(modes[i]));
			return PIGLIT_FAIL;
		}
	}

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	int i;
	enum piglit_result result;

	/* 1 subtest per added function */
	static struct piglit_subtest tests[] = {
		{
			"MatrixLoadfEXT",
			NULL,
			test_matrix_command,
			test_MatrixLoadf
		},
		{
			"MatrixLoadd",
			NULL,
			test_matrix_command,
			test_MatrixLoadd
		},
		{
			"MatrixMultfEXT",
			NULL,
			test_matrix_command,
			test_MatrixMultf
		},
		{
			"MatrixMultdEXT",
			NULL,
			test_matrix_command,
			test_MatrixMultd
		},
		{
			"MatrixRotatefEXT",
			NULL,
			test_matrix_command,
			test_MatrixRotatef
		},
		{
			"MatrixRotatedEXT",
			NULL,
			test_matrix_command,
			test_MatrixRotated
		},
		{
			"MatrixScalefEXT",
			NULL,
			test_matrix_command,
			test_MatrixScalef
		},
		{
			"MatrixScaledEXT",
			NULL,
			test_matrix_command,
			test_MatrixScaled
		},
		{
			"MatrixTranslatefEXT",
			NULL,
			test_matrix_command,
			test_MatrixTranslatef
		},
		{
			"MatrixTranslatedEXT",
			NULL,
			test_matrix_command,
			test_MatrixTranslated
		},
		{
			"MatrixLoadIdentityEXT",
			NULL,
			test_matrix_command,
			test_MatrixLoadIdentity
		},
		{
			"MatrixOrthoEXT",
			NULL,
			test_matrix_command,
			test_MatrixOrtho
		},
		{
			"MatrixFrustumEXT",
			NULL,
			test_matrix_command,
			test_MatrixFrustum
		},
		{
			"MatrixPushPopEXT",
			NULL,
			test_matrix_command,
			test_MatrixPushPop
		},
		{
			"MatrixLoadTransposefEXT",
			NULL,
			test_matrix_command,
			test_MatrixLoadTransposef
		},
		{
			"MatrixLoadTransposedEXT",
			NULL,
			test_matrix_command,
			test_MatrixLoadTransposed
		},
		{
			"MatrixMultTransposefEXT",
			NULL,
			test_matrix_command,
			test_MatrixMultTransposef
		},
		{
			"MatrixMultTransposedEXT",
			NULL,
			test_matrix_command,
			test_MatrixMultTransposed
		},
		{
			NULL
		}
	};


	piglit_require_extension("GL_EXT_direct_state_access");

	matrix_mode_count = 3; /*  MODELVIEW + PROJECTION + TEXTURE */

	/* Also test with [GL_TEXTURE0, GL_TEXTURE31] (if supported) */
	glGetIntegerv(GL_MAX_TEXTURE_COORDS, &max_texture_coords);
	matrix_mode_count += MIN2(32, max_texture_coords);

	/* Declare all matrix modes we want to test */
	modes = (GLenum*) malloc(matrix_mode_count * sizeof(GLenum));
	get_modes = (GLenum*) malloc(matrix_mode_count * sizeof(GLenum));

	modes[0] = GL_MODELVIEW;
	get_modes[0] = GL_MODELVIEW_MATRIX;

	modes[1] = GL_PROJECTION;
	get_modes[1] = GL_PROJECTION_MATRIX;

	modes[2] = GL_TEXTURE;
	get_modes[2] = GL_TEXTURE_MATRIX;
	for (i = 0; i < MIN2(32, max_texture_coords); i++) {
		modes[3 + i] = GL_TEXTURE0 + i;
		get_modes[3 + i] = GL_TEXTURE_MATRIX;
	}


	result = piglit_run_selected_subtests(tests, NULL, 0, PIGLIT_PASS);

	/* Re-run the same test but using display list GL_COMPILE */
	for (i = 0; tests[i].name; i++) {
		char* test_name_display_list;
		asprintf(&test_name_display_list, "%s + display list GL_COMPILE", tests[i].name);
		tests[i].name = test_name_display_list;
	}
	use_display_list = GL_COMPILE;
	result = piglit_run_selected_subtests(tests, NULL, 0, result);

	/* Re-run the same test but using display list GL_COMPILE_AND_EXECUTE */
	for (i = 0; tests[i].name; i++) {
		char* test_name_display_list;
		asprintf(&test_name_display_list, "%s_AND_EXECUTE", tests[i].name);
		tests[i].name = test_name_display_list;
	}
	use_display_list = GL_COMPILE_AND_EXECUTE;
	result = piglit_run_selected_subtests(tests, NULL, 0, result);


	free(modes);
	free(get_modes);

	piglit_report_result(result);
}

enum piglit_result
piglit_display(void)
{
	/* UNREACHABLE */
	return PIGLIT_FAIL;
}
