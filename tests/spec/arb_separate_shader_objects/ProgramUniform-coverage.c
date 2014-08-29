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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

/**
 * \name ProgramUniform-coverage.c
 * Nearly exhaustive test of all the glProgramUniform functions added by
 * GL_ARB_separate_shader_objects.
 *
 * For every entrypoint, a shader using a uniform of the correct type is
 * created using \c glCreateShaderProgramv.  The uniform is then set and
 * queried (using \c glGetUniform).  The test passes if the correct data is
 * returned and no GL errors are generated.
 *
 * \note
 * The following aspects of these interfaces are not tested by this test:
 *
 *     - Uniform arrays set by \c glProgramUniform*v or by
 *       \c glProgramUniformMatrix*v with \c count > 1.
 *
 *     - Transpose matrices set by \c glProgramUniformMatrix*v with \c transpose
 *       set to \c GL_TRUE.
 */
#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static const char common_body[] =
	"\n"
	"void main()\n"
	"{\n"
	"    gl_Position = vec4(v4) + vec4(v3, v1) + vec4(v2, 0, 0);\n"
	"}\n"
	;

static const char float_code[] =
	"uniform float v1;\n"
	"uniform vec2 v2;\n"
	"uniform vec3 v3;\n"
	"uniform vec4 v4;\n"
	;

static const char int_code[] =
	"uniform int v1;\n"
	"uniform ivec2 v2;\n"
	"uniform ivec3 v3;\n"
	"uniform ivec4 v4;\n"
	;

static const char uint_code[] =
	"uniform uint v1;\n"
	"uniform uvec2 v2;\n"
	"uniform uvec3 v3;\n"
	"uniform uvec4 v4;\n"
	;

static const char double_code[] =
	"uniform double v1;\n"
	"uniform dvec2 v2;\n"
	"uniform dvec3 v3;\n"
	"uniform dvec4 v4;\n"
	;

static const char square_mat_code[] =
	"uniform mat2 m2;\n"
	"uniform mat3 m3;\n"
	"uniform mat4 m4;\n"
	"\n"
	"void main()\n"
	"{\n"
	"    gl_Position = vec4(m4[0]) + vec4(m3[0], 0) + vec4(m2[0], m2[1]);\n"
	"}\n"
	;

static const char nonsquare_mat_code[] =
	"uniform mat2x3 m2x3;\n"
	"uniform mat2x4 m2x4;\n"
	"uniform mat3x2 m3x2;\n"
	"uniform mat3x4 m3x4;\n"
	"uniform mat4x2 m4x2;\n"
	"uniform mat4x3 m4x3;\n"
	"\n"
	"void main()\n"
	"{\n"
	"    gl_Position = vec4(m2x4[0]) + vec4(m3x4[0])\n"
	"        + vec4(m2x3[0], 0) + vec4(m4x3[0], 0)\n"
	"        + vec4(m3x2[0], m4x2[0]);\n"
	"}\n"
	;

static const char dmat_code[] =
	"uniform dmat2 m2;\n"
	"uniform dmat3 m3;\n"
	"uniform dmat4 m4;\n"
	"uniform dmat2x3 m2x3;\n"
	"uniform dmat2x4 m2x4;\n"
	"uniform dmat3x2 m3x2;\n"
	"uniform dmat3x4 m3x4;\n"
	"uniform dmat4x2 m4x2;\n"
	"uniform dmat4x3 m4x3;\n"
	"\n"
	"void main()\n"
	"{\n"
	"    gl_Position = vec4(m4[0]) + vec4(m3[0], 0) + vec4(m2[0], m2[1])\n"
	"        + vec4(m2x4[0]) + vec4(m3x4[0])\n"
	"        + vec4(m2x3[0], 0) + vec4(m4x3[0], 0)\n"
	"        + vec4(m3x2[0], m4x2[0]);\n"
	"}\n"
	;

#define BUILD_SHADER(check)						\
	do {								\
		if (check) {						\
			piglit_report_subtest_result(PIGLIT_SKIP,	\
						     subtest_name);	\
			return true;					\
		}							\
									\
		prog = glCreateShaderProgramv(GL_VERTEX_SHADER,		\
					      ARRAY_SIZE(shader_strings), \
					      shader_strings);		\
		if (!piglit_link_check_status(prog)) {			\
			piglit_report_subtest_result(PIGLIT_FAIL,	\
						     subtest_name);	\
			return false;					\
		}							\
	} while (false)

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

/**
 * \name Random number generation functions.
 *
 * Each of these functions generates series of random numbers for one of the
 * test cases.
 */
/*@{*/
static void
random_floats(float *v, unsigned count)
{
	unsigned i;

	for (i = 0; i < count; i++) {
		/* Values are from [0, RAND_MAX].  Remap to [-1,1].
		 */
		const float x = ((float) rand() / RAND_MAX) * 2.0f - 1.0f;

		/* Valid exponent range for single precision is [-127,127].
		 */
		const int exponent = (rand() % 255) - 127;

		v[i] = ldexp(x, exponent);
	}
}

static void
random_doubles(double *v, unsigned count)
{
	unsigned i;

	for (i = 0; i < count; i++) {
		/* Values are from [0, RAND_MAX].  Remap to [-1,1].
		 */
		const double x = ((double) rand() / RAND_MAX) * 2.0 - 1.0;

		/* Valid exponent range for double precision is [-1023, 1023].
		 */
		const int exponent = (rand() % 2047) - 1023;

		v[i] = ldexp(x, exponent);
	}
}

static void
random_ints(int *v, unsigned count)
{
	unsigned i;

	for (i = 0; i < count; i++)
		v[i] = rand();
}

static void
random_uints(unsigned int *v, unsigned count)
{
	random_ints((int *) v, count);
}
/*@}*/

/**
 * \name Data checking functions.
 *
 * Each of these functions verifies that one set of data matches another set
 * of data.  If a discrepency is found, the failing location is logged.
 */
/*@{*/
static bool
check_float_values(const float *expected, const float *actual, unsigned count)
{
	unsigned i;
	bool pass = true;

	for (i = 0; i < count; i++) {
		if (expected[i] != actual[i]) {
			printf("[%u]: expected %f, got %f\n",
			       i, expected[i], actual[i]);
			pass = false;
		}
	}

	return pass;
}

static bool
check_double_values(const double *expected, const double *actual,
		    unsigned count)
{
	unsigned i;
	bool pass = true;

	for (i = 0; i < count; i++) {
		if (expected[i] != actual[i]) {
			printf("[%u]: expected %f, got %f\n",
			       i, expected[i], actual[i]);
			pass = false;
		}
	}

	return pass;
}

static bool
check_int_values(const int *expected, const int *actual, unsigned count)
{
	unsigned i;
	bool pass = true;

	for (i = 0; i < count; i++) {
		if (expected[i] != actual[i]) {
			printf("[%u]: expected 0x%04x, got 0x%04x\n",
			       i, expected[i], actual[i]);
			pass = false;
		}
	}

	return pass;
}

static bool
check_uint_values(const unsigned *expected, const unsigned *actual,
		  unsigned count)
{
	return check_int_values((const int *) expected,
				(const int *) actual,
				count);
}
/*@}*/

static bool
test_float(const char *version_string)
{
	GLint loc;
	bool pass = true;
	float values[4];
	float got[ARRAY_SIZE(values)];
	GLuint prog;
	static const char subtest_name[] = "float scalar and vectors";
	const char *const shader_strings[] = {
		version_string,
		float_code,
		common_body
	};

	BUILD_SHADER(false);

	/* Try float
	 */
	loc = glGetUniformLocation(prog, "v1");

	random_floats(values, ARRAY_SIZE(values));
	glProgramUniform1f(prog, loc,
			   values[0]);
	pass = piglit_check_gl_error(0) && pass;

	glGetUniformfv(prog, loc, got);
	pass = piglit_check_gl_error(0) && pass;

	pass = check_float_values(values, got, 1) && pass;

	random_floats(values, ARRAY_SIZE(values));
	glProgramUniform1fv(prog, loc, 1, values);
	pass = piglit_check_gl_error(0) && pass;

	glGetUniformfv(prog, loc, got);
	pass = piglit_check_gl_error(0) && pass;

	pass = check_float_values(values, got, 1) && pass;

	/* Try vec2
	 */
	loc = glGetUniformLocation(prog, "v2");

	random_floats(values, ARRAY_SIZE(values));
	glProgramUniform2f(prog, loc,
			   values[0], values[1]);
	pass = piglit_check_gl_error(0) && pass;

	glGetUniformfv(prog, loc, got);
	pass = piglit_check_gl_error(0) && pass;

	pass = check_float_values(values, got, 2) && pass;

	random_floats(values, ARRAY_SIZE(values));
	glProgramUniform2fv(prog, loc, 1, values);
	pass = piglit_check_gl_error(0) && pass;

	glGetUniformfv(prog, loc, got);
	pass = piglit_check_gl_error(0) && pass;

	pass = check_float_values(values, got, 2) && pass;

	/* Try vec3
	 */
	loc = glGetUniformLocation(prog, "v3");

	random_floats(values, ARRAY_SIZE(values));
	glProgramUniform3f(prog, loc,
			   values[0], values[1], values[2]);
	pass = piglit_check_gl_error(0) && pass;

	glGetUniformfv(prog, loc, got);
	pass = piglit_check_gl_error(0) && pass;

	pass = check_float_values(values, got, 3) && pass;

	random_floats(values, ARRAY_SIZE(values));
	glProgramUniform3fv(prog, loc, 1, values);
	pass = piglit_check_gl_error(0) && pass;

	glGetUniformfv(prog, loc, got);
	pass = piglit_check_gl_error(0) && pass;

	pass = check_float_values(values, got, 3) && pass;

	/* Try vec4
	 */
	loc = glGetUniformLocation(prog, "v4");

	random_floats(values, ARRAY_SIZE(values));
	glProgramUniform4f(prog, loc,
			   values[0], values[1], values[2], values[3]);
	pass = piglit_check_gl_error(0) && pass;

	glGetUniformfv(prog, loc, got);
	pass = piglit_check_gl_error(0) && pass;

	pass = check_float_values(values, got, 4) && pass;

	random_floats(values, ARRAY_SIZE(values));
	glProgramUniform4fv(prog, loc, 1, values);
	pass = piglit_check_gl_error(0) && pass;

	glGetUniformfv(prog, loc, got);
	pass = piglit_check_gl_error(0) && pass;

	pass = check_float_values(values, got, 4) && pass;

	piglit_report_subtest_result(pass ? PIGLIT_PASS : PIGLIT_FAIL,
				     subtest_name);

	glDeleteProgram(prog);
	return pass;
}

static bool
test_square_mat(const char *version_string)
{
	GLint loc;
	bool pass = true;
	float values[16];
	float got[ARRAY_SIZE(values)];
	GLuint prog;
	static const char subtest_name[] = "square float matrices";
	const char *const shader_strings[] = {
		version_string,
		square_mat_code,
	};

	BUILD_SHADER(false);

	/* Try mat2
	 */
	loc = glGetUniformLocation(prog, "m2");

	random_floats(values, ARRAY_SIZE(values));
	glProgramUniformMatrix2fv(prog, loc, 1, GL_FALSE, values);
	pass = piglit_check_gl_error(0) && pass;

	glGetUniformfv(prog, loc, got);
	pass = piglit_check_gl_error(0) && pass;

	pass = check_float_values(values, got, 4) && pass;

	/* Try mat3
	 */
	loc = glGetUniformLocation(prog, "m3");

	random_floats(values, ARRAY_SIZE(values));
	glProgramUniformMatrix3fv(prog, loc, 1, GL_FALSE, values);
	pass = piglit_check_gl_error(0) && pass;

	glGetUniformfv(prog, loc, got);
	pass = piglit_check_gl_error(0) && pass;

	pass = check_float_values(values, got, 9) && pass;

	/* Try mat4
	 */
	loc = glGetUniformLocation(prog, "m4");

	random_floats(values, ARRAY_SIZE(values));
	glProgramUniformMatrix4fv(prog, loc, 1, GL_FALSE, values);
	pass = piglit_check_gl_error(0) && pass;

	glGetUniformfv(prog, loc, got);
	pass = piglit_check_gl_error(0) && pass;

	pass = check_float_values(values, got, 16) && pass;

	piglit_report_subtest_result(pass ? PIGLIT_PASS : PIGLIT_FAIL,
				     subtest_name);

	glDeleteProgram(prog);
	return pass;
}

static bool
test_nonsquare_mat(const char *version_string)
{
	GLint loc;
	bool pass = true;
	float values[12];
	float got[ARRAY_SIZE(values)];
	GLuint prog;
	static const char subtest_name[] = "non-square float matrices";
	const char *const shader_strings[] = {
		version_string,
		nonsquare_mat_code,
	};

	/* Non-square matrices are only available in GLSL 1.20 or later.
	 */
	BUILD_SHADER(strstr(version_string, "110") != NULL);

	/* Try mat2x3
	 */
	loc = glGetUniformLocation(prog, "m2x3");

	random_floats(values, ARRAY_SIZE(values));
	glProgramUniformMatrix2x3fv(prog, loc, 1, GL_FALSE, values);
	pass = piglit_check_gl_error(0) && pass;

	glGetUniformfv(prog, loc, got);
	pass = piglit_check_gl_error(0) && pass;

	pass = check_float_values(values, got, 6) && pass;

	/* Try mat2x4
	 */
	loc = glGetUniformLocation(prog, "m2x4");

	random_floats(values, ARRAY_SIZE(values));
	glProgramUniformMatrix2x4fv(prog, loc, 1, GL_FALSE, values);
	pass = piglit_check_gl_error(0) && pass;

	glGetUniformfv(prog, loc, got);
	pass = piglit_check_gl_error(0) && pass;

	pass = check_float_values(values, got, 8) && pass;

	/* Try mat3x2
	 */
	loc = glGetUniformLocation(prog, "m3x2");

	random_floats(values, ARRAY_SIZE(values));
	glProgramUniformMatrix3x2fv(prog, loc, 1, GL_FALSE, values);
	pass = piglit_check_gl_error(0) && pass;

	glGetUniformfv(prog, loc, got);
	pass = piglit_check_gl_error(0) && pass;

	pass = check_float_values(values, got, 6) && pass;

	/* Try mat3x4
	 */
	loc = glGetUniformLocation(prog, "m3x4");

	random_floats(values, ARRAY_SIZE(values));
	glProgramUniformMatrix3x4fv(prog, loc, 1, GL_FALSE, values);
	pass = piglit_check_gl_error(0) && pass;

	glGetUniformfv(prog, loc, got);
	pass = piglit_check_gl_error(0) && pass;

	pass = check_float_values(values, got, 12) && pass;

	/* Try mat4x2
	 */
	loc = glGetUniformLocation(prog, "m4x2");

	random_floats(values, ARRAY_SIZE(values));
	glProgramUniformMatrix4x2fv(prog, loc, 1, GL_FALSE, values);
	pass = piglit_check_gl_error(0) && pass;

	glGetUniformfv(prog, loc, got);
	pass = piglit_check_gl_error(0) && pass;

	pass = check_float_values(values, got, 8) && pass;

	/* Try mat4x3
	 */
	loc = glGetUniformLocation(prog, "m4x3");

	random_floats(values, ARRAY_SIZE(values));
	glProgramUniformMatrix4x3fv(prog, loc, 1, GL_FALSE, values);
	pass = piglit_check_gl_error(0) && pass;

	glGetUniformfv(prog, loc, got);
	pass = piglit_check_gl_error(0) && pass;

	pass = check_float_values(values, got, 12) && pass;

	piglit_report_subtest_result(pass ? PIGLIT_PASS : PIGLIT_FAIL,
				     subtest_name);

	glDeleteProgram(prog);
	return pass;
}

static bool
test_double(const char *version_string)
{
	GLint loc;
	bool pass = true;
	double values[4];
	double got[ARRAY_SIZE(values)];
	GLuint prog;
	static const char subtest_name[] = "double scalar and vectors";
	const char *const shader_strings[] = {
		version_string,
		double_code,
		common_body
	};

	BUILD_SHADER(version_string == NULL);

	/* Try double
	 */
	random_doubles(values, ARRAY_SIZE(values));
	loc = glGetUniformLocation(prog, "v1");
	glProgramUniform1d(prog, loc,
			   values[0]);
	pass = piglit_check_gl_error(0) && pass;

	glGetUniformdv(prog, loc, got);
	pass = piglit_check_gl_error(0) && pass;

	pass = check_double_values(values, got, 1) && pass;

	random_doubles(values, ARRAY_SIZE(values));
	glProgramUniform1dv(prog, loc, 1, values);
	pass = piglit_check_gl_error(0) && pass;

	glGetUniformdv(prog, loc, got);
	pass = piglit_check_gl_error(0) && pass;

	pass = check_double_values(values, got, 1) && pass;

	/* Try dvec2
	 */
	random_doubles(values, ARRAY_SIZE(values));
	loc = glGetUniformLocation(prog, "v2");
	glProgramUniform2d(prog, loc,
			   values[0], values[1]);
	pass = piglit_check_gl_error(0) && pass;

	glGetUniformdv(prog, loc, got);
	pass = piglit_check_gl_error(0) && pass;

	pass = check_double_values(values, got, 2) && pass;

	random_doubles(values, ARRAY_SIZE(values));
	glProgramUniform2dv(prog, loc, 1, values);
	pass = piglit_check_gl_error(0) && pass;

	glGetUniformdv(prog, loc, got);
	pass = piglit_check_gl_error(0) && pass;

	pass = check_double_values(values, got, 2) && pass;

	/* Try dvec3
	 */
	random_doubles(values, ARRAY_SIZE(values));
	loc = glGetUniformLocation(prog, "v3");
	glProgramUniform3d(prog, loc,
			   values[0], values[1], values[2]);
	pass = piglit_check_gl_error(0) && pass;

	glGetUniformdv(prog, loc, got);
	pass = piglit_check_gl_error(0) && pass;

	pass = check_double_values(values, got, 3) && pass;

	random_doubles(values, ARRAY_SIZE(values));
	glProgramUniform3dv(prog, loc, 1, values);
	pass = piglit_check_gl_error(0) && pass;

	glGetUniformdv(prog, loc, got);
	pass = piglit_check_gl_error(0) && pass;

	pass = check_double_values(values, got, 3) && pass;

	/* Try dvec4
	 */
	random_doubles(values, ARRAY_SIZE(values));
	loc = glGetUniformLocation(prog, "v4");
	glProgramUniform4d(prog, loc,
			   values[0], values[1], values[2], values[3]);
	pass = piglit_check_gl_error(0) && pass;

	glGetUniformdv(prog, loc, got);
	pass = piglit_check_gl_error(0) && pass;

	pass = check_double_values(values, got, 4) && pass;

	random_doubles(values, ARRAY_SIZE(values));
	glProgramUniform4dv(prog, loc, 1, values);
	pass = piglit_check_gl_error(0) && pass;

	glGetUniformdv(prog, loc, got);
	pass = piglit_check_gl_error(0) && pass;

	pass = check_double_values(values, got, 4) && pass;

	piglit_report_subtest_result(pass ? PIGLIT_PASS : PIGLIT_FAIL,
				     subtest_name);

	glDeleteProgram(prog);
	return pass;
}

static bool
test_dmat(const char *version_string)
{
	GLint loc;
	bool pass = true;
	double values[16];
	double got[ARRAY_SIZE(values)];
	GLuint prog;
	static const char subtest_name[] = "double matrices";
	const char *const shader_strings[] = {
		version_string,
		dmat_code,
	};

	BUILD_SHADER(version_string == NULL);

	/* Try dmat2
	 */
	loc = glGetUniformLocation(prog, "m2");

	random_doubles(values, ARRAY_SIZE(values));
	glProgramUniformMatrix2dv(prog, loc, 1, GL_FALSE, values);
	pass = piglit_check_gl_error(0) && pass;

	glGetUniformdv(prog, loc, got);
	pass = piglit_check_gl_error(0) && pass;

	pass = check_double_values(values, got, 4) && pass;

	/* Try dmat3
	 */
	loc = glGetUniformLocation(prog, "m3");

	random_doubles(values, ARRAY_SIZE(values));
	glProgramUniformMatrix3dv(prog, loc, 1, GL_FALSE, values);
	pass = piglit_check_gl_error(0) && pass;

	glGetUniformdv(prog, loc, got);
	pass = piglit_check_gl_error(0) && pass;

	pass = check_double_values(values, got, 9) && pass;

	/* Try dmat4
	 */
	loc = glGetUniformLocation(prog, "m4");

	random_doubles(values, ARRAY_SIZE(values));
	glProgramUniformMatrix4dv(prog, loc, 1, GL_FALSE, values);
	pass = piglit_check_gl_error(0) && pass;

	glGetUniformdv(prog, loc, got);
	pass = piglit_check_gl_error(0) && pass;

	pass = check_double_values(values, got, 16) && pass;

	/* Try dmat2x3
	 */
	loc = glGetUniformLocation(prog, "m2x3");

	random_doubles(values, ARRAY_SIZE(values));
	glProgramUniformMatrix2x3dv(prog, loc, 1, GL_FALSE, values);
	pass = piglit_check_gl_error(0) && pass;

	glGetUniformdv(prog, loc, got);
	pass = piglit_check_gl_error(0) && pass;

	pass = check_double_values(values, got, 6) && pass;

	/* Try dmat2x4
	 */
	loc = glGetUniformLocation(prog, "m2x4");

	random_doubles(values, ARRAY_SIZE(values));
	glProgramUniformMatrix2x4dv(prog, loc, 1, GL_FALSE, values);
	pass = piglit_check_gl_error(0) && pass;

	glGetUniformdv(prog, loc, got);
	pass = piglit_check_gl_error(0) && pass;

	pass = check_double_values(values, got, 8) && pass;

	/* Try dmat3x2
	 */
	loc = glGetUniformLocation(prog, "m3x2");

	random_doubles(values, ARRAY_SIZE(values));
	glProgramUniformMatrix3x2dv(prog, loc, 1, GL_FALSE, values);
	pass = piglit_check_gl_error(0) && pass;

	glGetUniformdv(prog, loc, got);
	pass = piglit_check_gl_error(0) && pass;

	pass = check_double_values(values, got, 6) && pass;

	/* Try dmat3x4
	 */
	loc = glGetUniformLocation(prog, "m3x4");

	random_doubles(values, ARRAY_SIZE(values));
	glProgramUniformMatrix3x4dv(prog, loc, 1, GL_FALSE, values);
	pass = piglit_check_gl_error(0) && pass;

	glGetUniformdv(prog, loc, got);
	pass = piglit_check_gl_error(0) && pass;

	pass = check_double_values(values, got, 12) && pass;

	/* Try dmat4x2
	 */
	loc = glGetUniformLocation(prog, "m4x2");

	random_doubles(values, ARRAY_SIZE(values));
	glProgramUniformMatrix4x2dv(prog, loc, 1, GL_FALSE, values);
	pass = piglit_check_gl_error(0) && pass;

	glGetUniformdv(prog, loc, got);
	pass = piglit_check_gl_error(0) && pass;

	pass = check_double_values(values, got, 8) && pass;

	/* Try dmat4x3
	 */
	loc = glGetUniformLocation(prog, "m4x3");

	random_doubles(values, ARRAY_SIZE(values));
	glProgramUniformMatrix4x3dv(prog, loc, 1, GL_FALSE, values);
	pass = piglit_check_gl_error(0) && pass;

	glGetUniformdv(prog, loc, got);
	pass = piglit_check_gl_error(0) && pass;

	pass = check_double_values(values, got, 12) && pass;

	piglit_report_subtest_result(pass ? PIGLIT_PASS : PIGLIT_FAIL,
				     subtest_name);

	glDeleteProgram(prog);
	return pass;
}

static bool
test_int(const char *version_string)
{
	GLint loc;
	bool pass = true;
	int values[4];
	int got[ARRAY_SIZE(values)];
	GLuint prog;
	static const char subtest_name[] = "integer scalar and vectors";
	const char *const shader_strings[] = {
		version_string,
		int_code,
		common_body
	};

	BUILD_SHADER(version_string == NULL);

	/* Try int
	 */
	random_ints(values, ARRAY_SIZE(values));
	loc = glGetUniformLocation(prog, "v1");
	glProgramUniform1i(prog, loc,
			   values[0]);
	pass = piglit_check_gl_error(0) && pass;

	glGetUniformiv(prog, loc, got);
	pass = piglit_check_gl_error(0) && pass;

	pass = check_int_values(values, got, 1) && pass;

	random_ints(values, ARRAY_SIZE(values));
	glProgramUniform1iv(prog, loc, 1, values);
	pass = piglit_check_gl_error(0) && pass;

	glGetUniformiv(prog, loc, got);
	pass = piglit_check_gl_error(0) && pass;

	pass = check_int_values(values, got, 1) && pass;

	/* Try ivec2
	 */
	random_ints(values, ARRAY_SIZE(values));
	loc = glGetUniformLocation(prog, "v2");
	glProgramUniform2i(prog, loc,
			   values[0], values[1]);
	pass = piglit_check_gl_error(0) && pass;

	glGetUniformiv(prog, loc, got);
	pass = piglit_check_gl_error(0) && pass;

	pass = check_int_values(values, got, 2) && pass;

	random_ints(values, ARRAY_SIZE(values));
	glProgramUniform2iv(prog, loc, 1, values);
	pass = piglit_check_gl_error(0) && pass;

	glGetUniformiv(prog, loc, got);
	pass = piglit_check_gl_error(0) && pass;

	pass = check_int_values(values, got, 2) && pass;

	/* Try ivec3
	 */
	random_ints(values, ARRAY_SIZE(values));
	loc = glGetUniformLocation(prog, "v3");
	glProgramUniform3i(prog, loc,
			   values[0], values[1], values[2]);
	pass = piglit_check_gl_error(0) && pass;

	glGetUniformiv(prog, loc, got);
	pass = piglit_check_gl_error(0) && pass;

	pass = check_int_values(values, got, 3) && pass;

	random_ints(values, ARRAY_SIZE(values));
	glProgramUniform3iv(prog, loc, 1, values);
	pass = piglit_check_gl_error(0) && pass;

	glGetUniformiv(prog, loc, got);
	pass = piglit_check_gl_error(0) && pass;

	pass = check_int_values(values, got, 3) && pass;

	/* Try ivec4
	 */
	random_ints(values, ARRAY_SIZE(values));
	loc = glGetUniformLocation(prog, "v4");
	glProgramUniform4i(prog, loc,
			   values[0], values[1], values[2], values[3]);
	pass = piglit_check_gl_error(0) && pass;

	glGetUniformiv(prog, loc, got);
	pass = piglit_check_gl_error(0) && pass;

	pass = check_int_values(values, got, 4) && pass;

	random_ints(values, ARRAY_SIZE(values));
	glProgramUniform4iv(prog, loc, 1, values);
	pass = piglit_check_gl_error(0) && pass;

	glGetUniformiv(prog, loc, got);
	pass = piglit_check_gl_error(0) && pass;

	pass = check_int_values(values, got, 4) && pass;

	piglit_report_subtest_result(pass ? PIGLIT_PASS : PIGLIT_FAIL,
				     subtest_name);

	glDeleteProgram(prog);
	return pass;
}

static bool
test_uint(const char *version_string)
{
	GLint loc;
	bool pass = true;
	unsigned int values[4];
	unsigned int got[ARRAY_SIZE(values)];
	GLuint prog;
	static const char subtest_name[] =
		"unsigned integer scalar and vectors";
	const char *const shader_strings[] = {
		version_string,
		uint_code,
		common_body
	};

	BUILD_SHADER(version_string == NULL);

	/* Try uint
	 */
	random_uints(values, ARRAY_SIZE(values));
	loc = glGetUniformLocation(prog, "v1");
	glProgramUniform1ui(prog, loc,
			   values[0]);
	pass = piglit_check_gl_error(0) && pass;

	glGetUniformuiv(prog, loc, got);
	pass = piglit_check_gl_error(0) && pass;

	pass = check_uint_values(values, got, 1) && pass;

	random_uints(values, ARRAY_SIZE(values));
	glProgramUniform1uiv(prog, loc, 1, values);
	pass = piglit_check_gl_error(0) && pass;

	glGetUniformuiv(prog, loc, got);
	pass = piglit_check_gl_error(0) && pass;

	pass = check_uint_values(values, got, 1) && pass;

	/* Try uvec2
	 */
	random_uints(values, ARRAY_SIZE(values));
	loc = glGetUniformLocation(prog, "v2");
	glProgramUniform2ui(prog, loc,
			   values[0], values[1]);
	pass = piglit_check_gl_error(0) && pass;

	glGetUniformuiv(prog, loc, got);
	pass = piglit_check_gl_error(0) && pass;

	pass = check_uint_values(values, got, 2) && pass;

	random_uints(values, ARRAY_SIZE(values));
	glProgramUniform2uiv(prog, loc, 1, values);
	pass = piglit_check_gl_error(0) && pass;

	glGetUniformuiv(prog, loc, got);
	pass = piglit_check_gl_error(0) && pass;

	pass = check_uint_values(values, got, 2) && pass;

	/* Try uvec3
	 */
	random_uints(values, ARRAY_SIZE(values));
	loc = glGetUniformLocation(prog, "v3");
	glProgramUniform3ui(prog, loc,
			   values[0], values[1], values[2]);
	pass = piglit_check_gl_error(0) && pass;

	glGetUniformuiv(prog, loc, got);
	pass = piglit_check_gl_error(0) && pass;

	pass = check_uint_values(values, got, 3) && pass;

	random_uints(values, ARRAY_SIZE(values));
	glProgramUniform3uiv(prog, loc, 1, values);
	pass = piglit_check_gl_error(0) && pass;

	glGetUniformuiv(prog, loc, got);
	pass = piglit_check_gl_error(0) && pass;

	pass = check_uint_values(values, got, 3) && pass;

	/* Try uvec4
	 */
	random_uints(values, ARRAY_SIZE(values));
	loc = glGetUniformLocation(prog, "v4");
	glProgramUniform4ui(prog, loc,
			   values[0], values[1], values[2], values[3]);
	pass = piglit_check_gl_error(0) && pass;

	glGetUniformuiv(prog, loc, got);
	pass = piglit_check_gl_error(0) && pass;

	pass = check_uint_values(values, got, 4) && pass;

	random_uints(values, ARRAY_SIZE(values));
	glProgramUniform4uiv(prog, loc, 1, values);
	pass = piglit_check_gl_error(0) && pass;

	glGetUniformuiv(prog, loc, got);
	pass = piglit_check_gl_error(0) && pass;

	pass = check_uint_values(values, got, 4) && pass;

	piglit_report_subtest_result(pass ? PIGLIT_PASS : PIGLIT_FAIL,
				     subtest_name);

	glDeleteProgram(prog);
	return pass;
}

void piglit_init(int argc, char **argv)
{
	bool pass = true;
	int gl_version;
	int glsl_major;
	int glsl_minor;
	int glsl_version;
	const char *version_for_float_shader = NULL;
	const char *version_for_double_shader = NULL;
	const char *version_for_int_shader = NULL;
	GLint context_flags = 0;

	piglit_require_vertex_shader();
	piglit_require_extension("GL_ARB_separate_shader_objects");

	gl_version = piglit_get_gl_version();
	if (gl_version >= 30)
		glGetIntegerv(GL_CONTEXT_FLAGS, &context_flags);

	piglit_get_glsl_version(NULL, &glsl_major, &glsl_minor);
	glsl_version = (glsl_major * 100) + glsl_minor;

	/* Select a shading language version string based on the GL version
	 * and whether or not we're running in a core profile.
	 */
	switch (gl_version / 10) {
	case 1:
	case 2:
		/* Selecting 1.20 will enable the non-square matrix tests.
		 */
		version_for_float_shader = (glsl_version >= 120)
			? "#version 120\n" : "#version 110\n";

		if (glsl_version >= 130)
			version_for_int_shader = "#version 130\n";
		break;
	case 3:
		/* OpenGL 3.0 deprecated GLSL 1.10 and 1.20.  OpenGL 3.1
		 * removed almost all deprecated features.
		 * Forworad-compatible contexts remove all deprecated
		 * features.
		 */
		if (gl_version == 30) {
			version_for_float_shader =
				(context_flags
				 & GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT)
				? "#version 130\n" : "#version 120\n";

			version_for_int_shader = "#version 130\n";
		} else {
			/* Section 1.6.1 (OpenGL Shading Language) of the
			 * OpenGL 3.1 spec says:
			 *
			 *     "OpenGL 3.1 implementations are guaranteed to
			 *     support at least version 1.30 of the shading
			 *     language."
			 *
			 * This is likely a copy-and-paste error from version
			 * 3.0.  This should be 1.40.
			 *
			 * Section 1.6.1 (OpenGL Shading Language) of the
			 * OpenGL 3.2 spec says:
			 *
			 *     "OpenGL 3.2 implementations are guaranteed to
			 *     support versions 1.40 and 1.50 of the OpenGL
			 *     Shading Language."
			 *
			 * Section 1.7.1 (OpenGL Shading Language) of the
			 * OpenGL 3.3 spec says:
			 *
			 *     "OpenGL 3.3 implementations are guaranteed to
			 *     support version 3.30 of the OpenGL Shading
			 *     Language."
			 *
			 * Based on all of this, pick version 1.40 for OpenGL
			 * versions before 3.3, and version 3.30 for version
			 * 3.3.
			 */
			if (gl_version < 33) {
				version_for_float_shader = "#version 140\n";
				version_for_int_shader = "#version 140\n";
			} else {
				version_for_float_shader =
					"#version 330 core\n";
				version_for_int_shader = "#version 330 core\n";
			}

			if (piglit_is_extension_supported("GL_ARB_gpu_shader_fp64")) {
				/* The GL_ARB_gpu_shader_fp64 extensions spec
				 * says:
				 *
				 *     "OpenGL 3.2 and GLSL 1.50 are required."
				 */
				version_for_double_shader =
					"#version 150 core\n"
					"#extension GL_ARB_gpu_shader_fp64: require\n"
					;
			}
		}
		break;
	case 4:
		/* Section 1.7.1 (OpenGL Shading Language) of the
		 * OpenGL 4.0 spec says:
		 *
		 *     "OpenGL 4.0 implementations are guaranteed to support
		 *     version 4.00 of the OpenGL Shading Language."
		 *
		 * Section 1.7.1 (OpenGL Shading Language) of the
		 * OpenGL 4.1 spec says:
		 *
		 *     "OpenGL 4.1 implementations are guaranteed to support
		 *     version 4.10 of the OpenGL Shading Language."
		 *
		 * Section 1.7.1 (OpenGL Shading Language) of the
		 * OpenGL 4.2 spec says:
		 *
		 *     "OpenGL 4.2 implementations are guaranteed to support
		 *     version 4.20 of the OpenGL Shading Language....The core
		 *     profile of OpenGL 4.2 is also guaranteed to support all
		 *     previous versions of the OpenGL Shading Language back
		 *     to version 1.40."
		 *
		 * Section 1.3.1 (OpenGL Shading Language) of the
		 * OpenGL 4.3 spec says:
		 *
		 *     "OpenGL 4.3 implementations are guaranteed to support
		 *     version 4.30 of the OpenGL Shading Language....The core
		 *     profile of OpenGL 4.3 is also guaranteed to support all
		 *     previous versions of the OpenGL Shading Language back
		 *     to version 1.40."
		 *
		 * Section 1.3.1 (OpenGL Shading Language) of the
		 * OpenGL 4.4 spec says:
		 *
		 *     "OpenGL 4.4 implementations are guaranteed to support
		 *     version 4.40 of the OpenGL Shading Language....The core
		 *     profile of OpenGL 4.4 is also guaranteed to support all
		 *     previous versions of the OpenGL Shading Language back
		 *     to version 1.40."
		 *
		 * Even though 4.1 doesn't say anything about GLSL 4.00, the
		 * inference is that the addition starting in 4.2 was a
		 * clarification.
		 */
		version_for_float_shader = "#version 400 core\n";
		version_for_double_shader = "#version 400 core\n";
		version_for_int_shader = "#version 400 core\n";
		break;
	
	default:
		fprintf(stderr, "Unknown GL version!\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	pass = test_float(version_for_float_shader) && pass;
	pass = test_square_mat(version_for_float_shader) && pass;

	pass = test_nonsquare_mat(version_for_float_shader) && pass;

	pass = test_int(version_for_int_shader) && pass;
	pass = test_uint(version_for_int_shader) && pass;

	pass = test_double(version_for_double_shader) && pass;
	pass = test_dmat(version_for_double_shader) && pass;

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
