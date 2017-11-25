/*
 * Copyright © 2017 Fabian Bieler
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
 * @file matrix-property-bindings.c:
 * Access GL transformation state in ARB_vertex_program.
 *
 * Set matrix property bindings with the OpenGL glLoadMatrix API and access it
 * in ARB vertex programs.
 */

#include "piglit-util-gl.h"
#include "piglit-matrix.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 13;
	config.window_visual = PIGLIT_GL_VISUAL_RGB;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

#ifdef _WIN32
#define SRAND(x) srand(x)
#define DRAND() ((float)rand() / RAND_MAX)
#else
#define SRAND(x) srand48(x)
#define DRAND() drand48()
#endif

/*
 * This vertex program compares test_param[i] against expectedi using epsilon
 * as tolerance for all i from 0 to 3 inclusive.
 * On match result.color is set to green, red otherwise.
 */
static const char *vp_template =
	"!!ARBvp1.0\n"
	"PARAM epsilon = 0.00390625;\n"
	"PARAM expected0 = {%f, %f, %f, %f};\n"
	"PARAM expected1 = {%f, %f, %f, %f};\n"
	"PARAM expected2 = {%f, %f, %f, %f};\n"
	"PARAM expected3 = {%f, %f, %f, %f};\n"
	"PARAM test_param[4] = { %s };\n"
	"TEMP tmp1;\n"
	"TEMP tmp2;\n"

	"SUB tmp1, expected0, test_param[0];\n"
	"ABS tmp1, tmp1;\n"
	"SLT tmp1, tmp1, epsilon;\n"
	"DP4 tmp2.x, tmp1, tmp1;\n" /* tmp2.x = 4 */

	"SUB tmp1, expected1, test_param[1];\n"
	"ABS tmp1, tmp1;\n"
	"SLT tmp1, tmp1, epsilon;\n"
	"DP4 tmp2.y, tmp1, tmp1;\n" /* tmp2.y = 4 */

	"SUB tmp1, expected2, test_param[2];\n"
	"ABS tmp1, tmp1;\n"
	"SLT tmp1, tmp1, epsilon;\n"
	"DP4 tmp2.z, tmp1, tmp1;\n" /* tmp2.z = 4 */

	"SUB tmp1, expected3, test_param[3];\n"
	"ABS tmp1, tmp1;\n"
	"SLT tmp1, tmp1, epsilon;\n"
	"DP4 tmp2.w, tmp1, tmp1;\n" /* tmp2.w = 4 */

	"DP4 tmp2.x, tmp2, tmp2;\n" /* tmp2.x = 64 */

	"SLT tmp1.x, tmp2.x, 64;\n" /* tmp1.x = 0 */
	"SGE tmp1.y, tmp2.x, 64;\n" /* tmp1.y = 1 */
	"SWZ result.color, tmp1, x, y, 0, 1;\n"

	"MOV result.position, vertex.position;\n"
	"END";

/**
 * Check that the constant parameter \name is equal to \m.
 *
 * Since we also test for derived state involving floating point computation
 * don't test for strict equality but rather only check if the parameter's
 * components are within and epsilon of their expected values.
 */
static bool
check_prg_param_(const float *m, const char *name)
{
	char *vp_text;
	const float green[3] = {0.0, 1.0, 0.0};

	asprintf(&vp_text, vp_template, m[0], m[4], m[8], m[12], m[1], m[5],
		 m[9], m[13], m[2], m[6], m[10], m[14], m[3], m[7], m[11],
		 m[15], name);
	GLuint prog = piglit_compile_program(GL_VERTEX_PROGRAM_ARB, vp_text);
	free(vp_text);
	glBindProgramARB(GL_VERTEX_PROGRAM_ARB, prog);

	glClear(GL_COLOR_BUFFER_BIT);
	piglit_draw_rect(-1, -1, 2, 2);

	glDeleteProgramsARB(1, &prog);

	if (piglit_probe_pixel_rgb_silent(piglit_width / 2, piglit_height / 2,
					  green, NULL))
		return true;
	printf("Failed parameter: '%s'.\n", name);
	return false;
}

/**
 * printf-like version of function above.
 */
static bool
check_prg_param(const float *m, const char *format, ...) PRINTFLIKE(2, 3);
static bool
check_prg_param(const float *m, const char *format, ...)
{
	char *name;
	va_list ap;

	va_start(ap, format);
	vasprintf(&name, format, ap);
	va_end(ap);

	const bool r = check_prg_param_(m, name);
	free(name);
	return r;
}

/**
 * Load random 16 floats between 0 and 1 into matrix \pname
 * and return them in \m.
 */
static void
load_matrix(float m[16], const GLenum pname)
{
	glMatrixMode(pname);
	for (int i = 0; i < 16; ++i)
		m[i] = DRAND();
	glLoadMatrixf(m);
}

/**
 * Check that matrix \pname contains the values \m.
 * if \idx is positive add it as and index to the matrix array.
 * if \idx is zero check both indexed and non-indexed variants.
 * if \idx is negative check non-indexed matrix.
 * Also check the matrix' transpose, inverse and transpose inverse.
 */
static bool
check_matrix_variants(const char *prefix, const float m[16], const int idx)
{
	bool pass = true;
	float m_T[16], m_inv[16], m_inv_T[16];

	piglit_matrix_transpose(m_T, m);
	piglit_matrix_inverse(m_inv, m);
	piglit_matrix_transpose(m_inv_T, m_inv);

	if (idx >= 0) {
		pass = check_prg_param(m, "state.matrix.%s[%d]", prefix,
				       idx) && pass;
		pass = check_prg_param(m_T, "state.matrix.%s[%d].transpose",
				       prefix, idx) && pass;
		pass = check_prg_param(m_inv, "state.matrix.%s[%d].inverse",
				       prefix, idx) && pass;
		pass = check_prg_param(m_inv_T,
				       "state.matrix.%s[%d].invtrans", prefix,
				       idx) && pass;
	}
	if (idx <= 0) {
		pass = check_prg_param(m, "state.matrix.%s", prefix) && pass;
		pass = check_prg_param(m_T, "state.matrix.%s.transpose",
				       prefix) && pass;
		pass = check_prg_param(m_inv, "state.matrix.%s.inverse",
				       prefix) && pass;
		pass = check_prg_param(m_inv_T, "state.matrix.%s.invtrans",
				       prefix) && pass;
	}

	return pass;
}

/**
 * Load random data in matrix \pname and check it by it's shader name \name
 * with (optional) index \idx.
 */
static bool
load_and_test_matrix(const char *name, const GLenum pname, const int idx)
{
	float mat[16];

	load_matrix(mat, pname);
	return check_matrix_variants(name, mat, idx);
}

enum piglit_result
piglit_display(void)
{
	bool pass = true;

	/* Test modelview and projection matrices. */
	pass = load_and_test_matrix("modelview", GL_MODELVIEW, -1) && pass;

	pass = load_and_test_matrix("projection", GL_PROJECTION, -1) && pass;

	/* Test modelview-projection matrix. */
	float mvp[16], proj[16], mview[16];
	load_matrix(mview, GL_MODELVIEW);
	load_matrix(proj, GL_PROJECTION);
	piglit_matrix_mul_matrix(mvp, proj, mview);
	pass = check_matrix_variants("mvp", mvp, -1) && pass;

	/* Test texture matrices. */
	int max_texture_coords;
	glGetIntegerv(GL_MAX_TEXTURE_COORDS, &max_texture_coords);
	for (int t = 0; t < max_texture_coords; ++t) {
		glActiveTexture(GL_TEXTURE0 + t);
		pass = load_and_test_matrix("texture", GL_TEXTURE, t) && pass;
	}

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_ARB_vertex_program");

	glEnable(GL_VERTEX_PROGRAM_ARB);

	SRAND(17);
}
