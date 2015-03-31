/*
 * Copyright © 2014 Intel Corporation
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
 * \file resource-location.c
 *
 * Tests GetProgramResourceLocation interface. Iterates over valid enums and
 * checks for an invalid one. Then the test compiles shader programs to query
 * locations of all valid enums and validates the result. Tests verify location
 * values against old matching API functions. GetProgramResourceLocationIndex
 * is not included in this test.
 *
 * From the GL_ARB_program_interface_query spec:
 *     "The commands
 *
 *     int GetProgramResourceLocation(uint program, enum programInterface,
 *                                    const char *name);
 *     int GetProgramResourceLocationIndex(uint program, enum programInterface,
 *                                         const char *name);
 *
 *     returns the location or the fragment color index, respectively, assigned
 *     to the variable named <name> in interface <programInterface> of program
 *     object <program>.  For both commands, the error INVALID_OPERATION is
 *     generated if <program> has not been linked or was last linked
 *     unsuccessfully.  For GetProgramResourceLocation, <programInterface> must
 *     be one of UNIFORM, PROGRAM_INPUT, PROGRAM_OUTPUT,
 *     VERTEX_SUBROUTINE_UNIFORM, TESS_CONTROL_SUBROUTINE_UNIFORM,
 *     TESS_EVALUATION_SUBROUTINE_UNIFORM, GEOMETRY_SUBROUTINE_UNIFORM,
 *     FRAGMENT_SUBROUTINE_UNIFORM, or COMPUTE_SUBROUTINE_UNIFORM.  For
 *     GetProgramResourceLocationIndex, <programInterface> must be
 *     PROGRAM_OUTPUT.  The value -1 will be returned by either command if an
 *     error occurs, if <name> does not identify an active variable on
 *     <programInterface>, or if <name> identifies an active variable that does
 *     not have a valid location assigned, as described above.  The locations
 *     returned by these commands are the same locations returned when querying
 *     the LOCATION and LOCATION_INDEX resource properties.
 */

#include "piglit-util-gl.h"
#include "common.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_core_version = 32;

PIGLIT_GL_TEST_CONFIG_END

static const GLenum valid_enums[] = {
	GL_UNIFORM,
	GL_PROGRAM_INPUT,
	GL_PROGRAM_OUTPUT,
};

static const GLenum valid_enums_sub[] = {
	GL_VERTEX_SUBROUTINE_UNIFORM,
	GL_GEOMETRY_SUBROUTINE_UNIFORM,
	GL_FRAGMENT_SUBROUTINE_UNIFORM,
};

static const GLenum valid_enums_sub_tes[] = {
	GL_TESS_CONTROL_SUBROUTINE_UNIFORM,
	GL_TESS_EVALUATION_SUBROUTINE_UNIFORM,
	GL_COMPUTE_SUBROUTINE_UNIFORM
};

static const GLenum valid_enums_sub_com[] = {
	GL_COMPUTE_SUBROUTINE_UNIFORM
};

static const char vs_subroutine_text[] =
	"#version 150\n"
	"#extension GL_ARB_explicit_attrib_location : require\n"
	"#extension GL_ARB_explicit_uniform_location : require\n"
	"#extension GL_ARB_shader_subroutine : require\n"
	"in vec4 vertex;\n"
	"subroutine vec4 vs_offset();\n"
	"layout (location = 3) subroutine uniform vs_offset VERTEX;\n"
	"subroutine (vs_offset) vec4 x() { return vec4(1.0, 0.0, 0.0, 0.0); }\n"
	"void main() {\n"
		"gl_Position = vertex + VERTEX();\n"
	"}";

static const char fs_subroutine_text[] =
	"#version 150\n"
	"#extension GL_ARB_explicit_attrib_location : require\n"
	"#extension GL_ARB_explicit_uniform_location : require\n"
	"#extension GL_ARB_shader_subroutine : require\n"
	"subroutine vec4 fs_offset();\n"
	"layout (location = 3) subroutine uniform fs_offset FRAGMENT;\n"
	"subroutine (fs_offset) vec4 red() { return vec4(1.0, 0.0, 0.0, 1.0); }\n"
	"out vec4 result;\n"
	"void main() {\n"
		"result = FRAGMENT();\n"
	"}";

static const char gs_subroutine_text[] =
	"#version 150\n"
	"#extension GL_ARB_explicit_attrib_location : require\n"
	"#extension GL_ARB_explicit_uniform_location : require\n"
	"#extension GL_ARB_shader_subroutine : require\n"
	"layout(triangles) in;\n"
	"layout(triangle_strip, max_vertices = 3) out;\n"
	"subroutine vec4 gs_offset();\n"
	"layout (location = 3) subroutine uniform gs_offset GEOMETRY;\n"
	"subroutine (gs_offset) vec4 x() { return vec4(1.0, 0.0, 0.0, 0.0); }\n"
	"void main() {\n"
		"for(int i = 0; i < 3; i++) {\n"
			"gl_Position = gl_in[i].gl_Position + GEOMETRY();\n"
			"EmitVertex();\n"
		"}\n"
		"EndPrimitive();\n"
	"}";

static const char tcs_subroutine_text[] =
	"#version 150\n"
	"#extension GL_ARB_explicit_attrib_location : require\n"
	"#extension GL_ARB_explicit_uniform_location : require\n"
	"#extension GL_ARB_shader_subroutine : require\n"
	"#extension GL_ARB_tessellation_shader : require\n"
	"layout(vertices = 3) out;\n"
	"subroutine vec4 tcs_offset();\n"
	"layout (location = 3) subroutine uniform tcs_offset TESS_CONTROL;\n"
	"subroutine (tcs_offset) vec4 x() { return vec4(1.0, 0.0, 0.0, 0.0); }\n"
	"void main() {\n"
		"gl_out[gl_InvocationID].gl_Position = vec4(0.0); + TESS_CONTROL();\n"
	"}";

static const char tes_subroutine_text[] =
	"#version 150\n"
	"#extension GL_ARB_explicit_attrib_location : require\n"
	"#extension GL_ARB_explicit_uniform_location : require\n"
	"#extension GL_ARB_shader_subroutine : require\n"
	"#extension GL_ARB_tessellation_shader : require\n"
	"layout(triangles) in;\n"
	"subroutine vec4 tes_offset();\n"
	"layout (location = 3) subroutine uniform tes_offset TESS_EVALUATION;\n"
	"subroutine (tes_offset) vec4 x() { return vec4(1.0, 0.0, 0.0, 0.0); }\n"
	"void main() {\n"
		"gl_Position = vec4(0.0) + TESS_EVALUATION();\n"
	"}";

static const char compute_subroutine_text[] =
	"#version 150\n"
	"#extension GL_ARB_explicit_attrib_location : require\n"
	"#extension GL_ARB_explicit_uniform_location : require\n"
	"#extension GL_ARB_shader_subroutine : require\n"
	"#extension GL_ARB_shader_image_load_store : require\n"
	"#extension GL_ARB_compute_shader : require\n"
	"layout(local_size_x = 4) in;\n"
	"layout(size4x32) uniform image2D tex;\n"
	"subroutine vec4 com_offset();\n"
	"layout (location = 3) subroutine uniform com_offset COMPUTE;\n"
	"subroutine (com_offset) vec4 x() { return vec4(1.0, 0.0, 0.0, 0.0); }\n"
	"void main() {\n"
		"imageStore(tex, ivec2(0.0), COMPUTE());\n"
	"}";

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

#define str(s) #s
#define CHECK_SUB(s) \
	if (glGetProgramResourceLocation(prog, \
		GL_##s ## _SUBROUTINE_UNIFORM, str(s)) != 3) { \
		piglit_report_subtest_result(PIGLIT_FAIL, __func__); \
		return PIGLIT_FAIL; \
	}

/* Test subroutine uniform location query with compute. */
static bool
test_subroutine_stages_compute()
{
	GLuint prog, i;

	if (!piglit_is_extension_supported("GL_ARB_shader_subroutine")) {
		piglit_report_subtest_result(PIGLIT_SKIP, __func__);
		return true;
	}

	if (!piglit_is_extension_supported("GL_ARB_compute_shader")) {
		piglit_report_subtest_result(PIGLIT_SKIP, __func__);
		return true;
	}

        prog = piglit_build_simple_program_multiple_shaders(
                     GL_COMPUTE_SHADER, compute_subroutine_text,
                     0);

	glUseProgram(prog);

	/* Iterate through all valid subroutine enums passing invalid name. */
	for (i = 0; i < (sizeof(valid_enums_sub_com)/sizeof(GLenum)); i++) {
		glGetProgramResourceLocation(prog, valid_enums_sub_com[i],
                                             "name");
			if (!piglit_check_gl_error(GL_NO_ERROR))
				piglit_report_result(PIGLIT_FAIL);
	}

	CHECK_SUB(COMPUTE);

	piglit_report_subtest_result(PIGLIT_PASS, __func__);
	return true;
}

/* Test subroutine uniform location query with tessellation. */
static bool
test_subroutine_stages_tcs_tes()
{
	GLuint prog, i;

	if (!piglit_is_extension_supported("GL_ARB_shader_subroutine")) {
		piglit_report_subtest_result(PIGLIT_SKIP, __func__);
		return true;
	}

	if (!piglit_is_extension_supported("GL_ARB_tessellation_shader")) {
		piglit_report_subtest_result(PIGLIT_SKIP, __func__);
		return true;
	}

        prog = piglit_build_simple_program_multiple_shaders(
                     GL_VERTEX_SHADER, vs_loc,
                     GL_TESS_CONTROL_SHADER, tcs_subroutine_text,
                     GL_TESS_EVALUATION_SHADER, tes_subroutine_text,
                     GL_FRAGMENT_SHADER, fs_loc,
                     0);

	glUseProgram(prog);

	/* Iterate through all valid subroutine enums passing invalid name. */
	for (i = 0; i < (sizeof(valid_enums_sub_tes)/sizeof(GLenum)); i++) {
		glGetProgramResourceLocation(prog, valid_enums_sub_tes[i],
                                             "name");
			if (!piglit_check_gl_error(GL_NO_ERROR))
				piglit_report_result(PIGLIT_FAIL);
	}

	CHECK_SUB(TESS_CONTROL);
	CHECK_SUB(TESS_EVALUATION);

	piglit_report_subtest_result(PIGLIT_PASS, __func__);
	return true;
}

/* Test subroutine uniform location query with vs, fs and gs. */
static bool
test_subroutine_stages_vs_fs_gs()
{
	GLuint prog, i;

	if (!piglit_is_extension_supported("GL_ARB_shader_subroutine")) {
		piglit_report_subtest_result(PIGLIT_SKIP, __func__);
		return true;
	}

        prog = piglit_build_simple_program_multiple_shaders(
                     GL_VERTEX_SHADER, vs_subroutine_text,
                     GL_GEOMETRY_SHADER, gs_subroutine_text,
                     GL_FRAGMENT_SHADER, fs_subroutine_text,
                     0);

	glUseProgram(prog);

	/* Iterate through all valid subroutine enums passing invalid name. */
	for (i = 0; i < (sizeof(valid_enums_sub)/sizeof(GLenum)); i++) {
		glGetProgramResourceLocation(prog, valid_enums_sub[i], "name");
			if (!piglit_check_gl_error(GL_NO_ERROR))
				piglit_report_result(PIGLIT_FAIL);
	}

	CHECK_SUB(VERTEX);
	CHECK_SUB(FRAGMENT);
	CHECK_SUB(GEOMETRY);

	piglit_report_subtest_result(PIGLIT_PASS, __func__);
	return true;
}


static void
validate_location(GLuint prog, GLenum iface, const char *name, GLuint value)
{
	GLuint loc = glGetProgramResourceLocation(prog, iface, name);

	/* Validate result against old API. */
	switch (iface) {
	case GL_UNIFORM:
		if (glGetUniformLocation(prog, name) != loc)
			piglit_report_result(PIGLIT_FAIL);
		break;
	case GL_PROGRAM_INPUT:
		if (glGetAttribLocation(prog, name) != loc)
			piglit_report_result(PIGLIT_FAIL);
		break;
	case GL_PROGRAM_OUTPUT:
		if (glGetFragDataLocation(prog, name) != loc)
			piglit_report_result(PIGLIT_FAIL);
		break;
	}

	/* Expected value. */
	if (loc != value) {
		fprintf(stderr, "got value %d for %s, expected %d\n",
			loc, name, value);
		piglit_report_result(PIGLIT_FAIL);
	}

	/* No errors should have happened. */
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);
}

void
piglit_init(int argc, char **argv)
{
	GLuint prog, shader;
	unsigned i;
	bool pass = true;

	piglit_require_extension("GL_ARB_program_interface_query");
	piglit_require_extension("GL_ARB_explicit_attrib_location");
	piglit_require_extension("GL_ARB_explicit_uniform_location");

	/* Test invalid program. */
	glGetProgramResourceLocation(42, GL_UNIFORM, "name");
	if (!piglit_check_gl_error(GL_INVALID_VALUE)) {
		piglit_report_subtest_result(PIGLIT_FAIL, "invalid program test 1");
		pass = false;
	}

	/* Test passing a shader, not program. */
	shader = piglit_compile_shader_text(GL_VERTEX_SHADER, vs_loc);
	glGetProgramResourceLocation(shader, GL_UNIFORM, "name");
	if (!piglit_check_gl_error(GL_INVALID_OPERATION)) {
		piglit_report_subtest_result(PIGLIT_FAIL, "invalid program test 2");
		pass = false;
	}

	prog = piglit_build_simple_program_unlinked(vs_loc, fs_loc);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	/* Test unlinked program. */
	glGetProgramResourceLocation(prog, GL_UNIFORM, "name");
	if (!piglit_check_gl_error(GL_INVALID_OPERATION)) {
		piglit_report_subtest_result(PIGLIT_FAIL, "invalid program test 3");
		pass = false;
	}

	if (pass)
		piglit_report_subtest_result(PIGLIT_PASS, "invalid program tests");

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	/* Test a linked program. */
	glLinkProgram(prog);
	glUseProgram(prog);

	/* Iterate through all valid enums passing invalid name. */
	for (i = 0; i < (sizeof(valid_enums)/sizeof(GLenum)); i++) {
		glGetProgramResourceLocation(prog, valid_enums[i], "name");
			if (!piglit_check_gl_error(GL_NO_ERROR))
				piglit_report_result(PIGLIT_FAIL);
	}

	/* Test invalid enum, there is no defined error by the spec. */
	glGetProgramResourceLocation(prog, GL_ATOMIC_COUNTER_BUFFER, "name");
	if (glGetError() == GL_NO_ERROR) {
		piglit_report_subtest_result(PIGLIT_FAIL, "invalid enum test");
		pass = false;
	} else {
		piglit_report_subtest_result(PIGLIT_PASS, "invalid enum test");
	}
	/* Test 3 illegal array cases referenced in the spec as 'bug 9254'. */
	if (glGetProgramResourceLocation(prog, GL_UNIFORM, "array[+1]") != -1) {
		piglit_report_subtest_result(PIGLIT_FAIL, "array case 1");
		pass = false;
	}

	if (glGetProgramResourceLocation(prog, GL_UNIFORM, "array[01]") != -1) {
		piglit_report_subtest_result(PIGLIT_FAIL, "array case 2");
		pass = false;
	}

	if (glGetProgramResourceLocation(prog, GL_UNIFORM, "array[ 0]") != -1) {
		piglit_report_subtest_result(PIGLIT_FAIL, "array case 3");
		pass = false;
	}

	if (pass)
		piglit_report_subtest_result(PIGLIT_PASS, "invalid array input");

	/* Valid inputs. */
	validate_location(prog, GL_UNIFORM,        "color",    9);
	validate_location(prog, GL_PROGRAM_INPUT,  "input0",   3);
	validate_location(prog, GL_PROGRAM_INPUT,  "input1",   6);
	validate_location(prog, GL_PROGRAM_OUTPUT, "output0",  1);
	validate_location(prog, GL_PROGRAM_OUTPUT, "output1",  0);

	/* Array indexing cases. */
	validate_location(prog, GL_UNIFORM,        "array",    1);
	validate_location(prog, GL_UNIFORM,        "array[0]", 1);
	validate_location(prog, GL_UNIFORM,        "array[1]", 2);

	/* All valid inputs succeeded if we got this far. */
	piglit_report_subtest_result(PIGLIT_PASS, "valid inputs");

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	/* Tests that require GL_ARB_shader_subroutine. */
	pass = test_subroutine_stages_vs_fs_gs() && pass;
	pass = test_subroutine_stages_tcs_tes() && pass;
	pass = test_subroutine_stages_compute() && pass;

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
