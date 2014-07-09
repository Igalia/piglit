/* © 2011 Intel Corporation
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
 * @file getuniform.c
 *
 * Tests that glGetUniform* work on various scalar/vector types.
 */

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	/* Not reached */
	return PIGLIT_FAIL;
}

static const char *source =
	"uniform vec4 u_vec4;\n"
	"uniform ivec4 u_ivec4;\n"
	"uniform bvec4 u_bvec4;\n"
	"uniform vec3 u_vec3;\n"
	"uniform ivec3 u_ivec3;\n"
	"uniform bvec3 u_bvec3;\n"
	"uniform vec2 u_vec2;\n"
	"uniform ivec2 u_ivec2;\n"
	"uniform bvec2 u_bvec2;\n"
	"uniform float u_float;\n"
	"uniform int u_int;\n"
	"uniform bool u_bool;\n"
	"\n"
	"void main()\n"
	"{"
	"	gl_Position = gl_Vertex;\n"
	"	gl_FrontColor =\n"
	"	    vec4(u_float + float(u_int) + float(u_bool),\n"
	"		 u_vec2.x + float(u_ivec2.x) + float(u_bvec2.x),\n"
	"		 u_vec3.x + float(u_ivec3.x) + float(u_bvec3.x),\n"
	"		 u_vec4.x + float(u_ivec4.x) + float(u_bvec4.x));\n"
	"}\n";

static GLuint prog;

static void
uniformiv_n(int loc, int count, int *values)
{
	switch (count) {
	case 1:
		glUniform1iv(loc, 1, values);
		break;
	case 2:
		glUniform2iv(loc, 1, values);
		break;
	case 3:
		glUniform3iv(loc, 1, values);
		break;
	case 4:
		glUniform4iv(loc, 1, values);
		break;
	default:
		abort();
	}
}

static void
uniformfv_n(int loc, int count, float *values)
{
	switch (count) {
	case 1:
		glUniform1fv(loc, 1, values);
		break;
	case 2:
		glUniform2fv(loc, 1, values);
		break;
	case 3:
		glUniform3fv(loc, 1, values);
		break;
	case 4:
		glUniform4fv(loc, 1, values);
		break;
	default:
		abort();
	}
}

static bool
check_f(const char *uniform_name, int chan, float expected, float result)
{
	if (expected == result)
		return true;

	fprintf(stderr, "%s.%c: expected %f, got %f\n",
		uniform_name + 2, "xyzw"[chan],
		expected, result);
	return false;
}

static bool
check_i(const char *uniform_name, int chan, int expected, int result)
{
	if (expected == result)
		return true;

	fprintf(stderr, "%s.%c: expected %d, got %d\n",
		uniform_name + 2, "xyzw"[chan],
		expected, result);

	return false;
}

static bool
test_bool_type(const char *name, int loc, int size)
{
	int tvals[4] = {1, 2, -3, -4};
	int fvals[4] = {0, 0, 0, 0};
	float retf[4];
	int reti[4];
	int i;

	/* Make the stack data have a consistent starting value, to
	 * distinguish unset out values.
	 */
	memset(retf, 0xd0, sizeof(retf));
	memset(reti, 0xd0, sizeof(reti));

	/* According to the ARB_shader_objects spec:
	 *
	 *     When loading values for a uniform declared as a Boolean, a
	 *     Boolean vector or an array of Booleans or an array of Boolean
	 *     vectors, both the Uniform*i{v} and Uniform*f{v} set of commands
	 *     can be used to load Boolean values. Type conversion is done by
	 *     the GL. The uniform is set to FALSE if the input value is 0 or
	 *     0.0f, and set to TRUE otherwise. The Uniform*ARB command used
	 *     must match the size of the uniform, as declared in the shader.
	 *
	 * We don't really care about loading of different types in this test,
	 * just getting types back out.
	 */

	uniformiv_n(loc, size, tvals);

	glGetUniformfvARB(prog, loc, retf);
	glGetUniformivARB(prog, loc, reti);
	for (i = 0; i < size; i++) {
		if (!check_f(name, i, 1.0, retf[i]))
			return false;

		if (!check_i(name, i, 1, reti[i]))
			return false;
	}

	uniformiv_n(loc, size, fvals);

	glGetUniformfvARB(prog, loc, retf);
	glGetUniformivARB(prog, loc, reti);
	for (i = 0; i < size; i++) {
		if (!check_f(name, i, 0.0, retf[i]))
			return false;

		if (!check_i(name, i, 0, reti[i]))
			return false;
	}

	return true;
}

static bool
test_float_type(const char *name, int loc, int size)
{
	float vals[4] = {1.2, -3.9, 4.9, 0.0};
	float retf[4];
	int reti[4];
	int i;

	/* Make the stack data have a consistent starting value, to
	 * distinguish unset out values.
	 */
	memset(retf, 0xd0, sizeof(retf));
	memset(reti, 0xd0, sizeof(reti));

	/* According to the ARB_shader_objects spec:
	 *
	 *     For all other uniform types the Uniform*ARB command used must
	 *     match the size and type of the uniform, as declared in the
	 *     shader. No type conversions are done.
	 */

	uniformfv_n(loc, size, vals);

	glGetUniformfvARB(prog, loc, retf);
	glGetUniformivARB(prog, loc, reti);
	for (i = 0; i < size; i++) {
		if (!check_f(name, i, vals[i], retf[i]))
			return false;

		/* While the GL 3.2 core spec doesn't explicitly
		 * state how conversion of float uniforms to integer
		 * values works, in section 6.2 "State Tables" on
		 * page 267 it says:
		 *
		 *     "Unless otherwise specified, when floating
		 *      point state is returned as integer values or
		 *      integer state is returned as floating-point
		 *      values it is converted in the fashion
		 *      described in section 6.1.2"
		 *
		 * That section, on page 248, says:
		 *
		 *     "If GetIntegerv or GetInteger64v are called,
		 *      a floating-point value is rounded to the
		 *      nearest integer..."
		 *
		 * So we assume rounding.
		 */
		if (!check_i(name, i, round(vals[i]), reti[i]))
			return false;
	}

	return true;
}

static bool
test_int_type(const char *name, int loc, int size)
{
	int vals[4] = {0, 1, 20, -40};
	float retf[4];
	int reti[4];
	int i;

	/* Make the stack data have a consistent starting value, to
	 * distinguish unset out values.
	 */
	memset(retf, 0xd0, sizeof(retf));
	memset(reti, 0xd0, sizeof(reti));

	/* According to the ARB_shader_objects spec:
	 *
	 *     For all other uniform types the Uniform*ARB command used must
	 *     match the size and type of the uniform, as declared in the
	 *     shader. No type conversions are done.
	 */

	uniformiv_n(loc, size, vals);

	glGetUniformfvARB(prog, loc, retf);
	glGetUniformivARB(prog, loc, reti);
	for (i = 0; i < size; i++) {
		if (!check_f(name, i, vals[i], retf[i]))
			return false;
		if (!check_i(name, i, vals[i], reti[i]))
			return false;
	}

	return true;
}

static struct {
	const char *name;
	bool (*test_func)(const char *name, int loc, int size);
	int size;
} uniforms[] = {
	{ "u_vec4", test_float_type, 4 },
	{ "u_vec3", test_float_type, 3 },
	{ "u_vec2", test_float_type, 2 },
	{ "u_float", test_float_type, 1 },
	{ "u_ivec4", test_int_type, 4 },
	{ "u_ivec3", test_int_type, 3 },
	{ "u_ivec2", test_int_type, 2 },
	{ "u_int", test_int_type, 1 },
	{ "u_bvec4", test_bool_type, 4 },
	{ "u_bvec3", test_bool_type, 3 },
	{ "u_bvec2", test_bool_type, 2 },
	{ "u_bool", test_bool_type, 1 },
};

void
piglit_init(int argc, char **argv)
{
	int i;
	bool pass = true;

	piglit_require_vertex_shader();

	prog = piglit_build_simple_program(source, NULL);
	glUseProgram(prog);

	for (i = 0; i < ARRAY_SIZE(uniforms); i++) {
		const char *name = uniforms[i].name;
		int loc = glGetUniformLocation(prog, name);
		assert(loc != -1);

		pass = pass && uniforms[i].test_func(name, loc,
						     uniforms[i].size);
	}

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
