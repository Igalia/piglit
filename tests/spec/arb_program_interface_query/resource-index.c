/*
 * Copyright © 2015 Intel Corporation
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
 * \file resource-index.c
 *
 * Tests GetProgramResourceIndex interface.
 *
 * From the GL_ARB_program_interface_query spec:
 *     "The command returns the unsigned integer index assigned to a
 *     resource named <name> in the interface type <programInterface> of
 *     program object <program>.  The error INVALID_ENUM is generated if
 *     <programInterface> is ATOMIC_COUNTER_BUFFER, since active atomic
 *     counter buffer resources are not assigned name strings.
 *
 *     If <name> exactly matches the name string of one of the active
 *     resources for <programInterface>, the index of the matched
 *     resource is returned. Additionally, if <name> would exactly match
 *     the name string of an active resource if "[0]" were appended to <name>,
 *     the index of the matched resource is returned.  Otherwise, <name> is
 *     considered not to be the name of an active resource, and INVALID_INDEX
 *     is returned.  Note that if an interface enumerates a single active
 *     resource list entry for an array variable (e.g., "a[0]"), a <name>
 *     identifying any array element other than the first (e.g., "a[1]")
 *     is not considered to match.
 *
 *     For the interface TRANSFORM_FEEDBACK_VARYING, the value INVALID_INDEX
 *     should be returned when querying the index assigned to the special
 *     names "gl_NextBuffer", "gl_SkipComponents1", "gl_SkipComponents2",
 *     "gl_SkipComponents3", and "gl_SkipComponents4".
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_core_version = 32;

PIGLIT_GL_TEST_CONFIG_END

const char *xfb_markers[] = {
	"gl_NextBuffer",
	"gl_SkipComponents1",
	"gl_SkipComponents2",
	"gl_SkipComponents3",
	"gl_SkipComponents4",
	NULL
};

static const char vs_text[] =
	"#version 150\n"
	"in vec4 input0;\n"
	"in vec4 input1;\n"
	"uniform ubo { vec4 mod; };\n"
	"void main() {\n"
		"gl_Position = input0 * input1 * mod;\n"
	"}";

static const char vs_atomic_text[] =
	"#version 150\n"
	"#extension GL_ARB_explicit_attrib_location : require\n"
	"#extension GL_ARB_shader_atomic_counters : require\n"
	"in vec4 input0;\n"
	"layout(binding = 0, offset = 4) uniform atomic_uint atom[3];\n"
	"void main() {\n"
		"atomicCounterIncrement(atom[1]);\n"
		"gl_Position = input0;\n"
	"}";

static const char vs_subroutine_text[] =
	"#version 150\n"
	"#extension GL_ARB_explicit_attrib_location : require\n"
	"#extension GL_ARB_shader_subroutine : require\n"
	"in vec4 input0;\n"
	"subroutine vec4 vs_offset();\n"
	"subroutine uniform vs_offset VERTEX;\n"
	"subroutine (vs_offset) vec4 x() { return vec4(1.0, 0.0, 0.0, 0.0); }\n"
	"void main() {\n"
		"gl_Position = input0 + VERTEX();\n"
	"}";

static const char fs_text[] =
	"#version 150\n"
	"uniform vec4 color;\n"
	"uniform float array[8];\n"
	"out vec4 output0;\n"
	"out vec4 output1;\n"
	"void main() {\n"
		"output0 = color * array[0];\n"
		"output1 = color * array[7];\n"
	"}";

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

static void
validate_index(GLuint prog, GLenum iface, const char *name)
{
	GLuint idx = glGetProgramResourceIndex(prog, iface, name);
	const GLchar *names[] = { name };
	GLuint indices;

	/* Validate result against old API. */
	switch (iface) {
	case GL_UNIFORM:
		glGetUniformIndices(prog, 1, names, &indices);
		if (idx != indices)
			piglit_report_result(PIGLIT_FAIL);
		break;
	case GL_UNIFORM_BLOCK:
		if (glGetUniformBlockIndex(prog, name) != idx)
			piglit_report_result(PIGLIT_FAIL);
		break;
	}

	/* No errors should have happened. */
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);
}

static bool
subroutine_index_test()
{
	GLuint prog, idx;
	if (!piglit_is_extension_supported("GL_ARB_shader_subroutine")) {
		piglit_report_subtest_result(PIGLIT_SKIP, __func__);
		return true;
	}

	prog = piglit_build_simple_program(vs_subroutine_text, fs_text);

	if (!prog || !piglit_check_gl_error(GL_NO_ERROR)) {
		piglit_report_subtest_result(PIGLIT_FAIL, __func__);
		return false;
	}

	/* Test invalid subroutine interface type for this shader,
         * should result in GL_INVALID_INDEX.
         */
	idx = glGetProgramResourceIndex(prog,
		GL_TESS_EVALUATION_SUBROUTINE, "VERTEX");

	if (idx != GL_INVALID_INDEX) {
		piglit_report_subtest_result(PIGLIT_FAIL, __func__);
                return false;
	}

	idx = glGetProgramResourceIndex(prog, GL_VERTEX_SUBROUTINE, "VERTEX");

	/* Validate result against old API. */
	if (glGetSubroutineIndex(prog, GL_VERTEX_SHADER, "VERTEX") != idx)
		return false;

	piglit_report_subtest_result(PIGLIT_PASS, __func__);
	return true;
}

static bool
atomic_counter_index_test()
{
	GLuint prog;
	if (!piglit_is_extension_supported("GL_ARB_shader_atomic_counters")) {
		piglit_report_subtest_result(PIGLIT_SKIP, __func__);
		return true;
	}

	prog = piglit_build_simple_program(vs_atomic_text, fs_text);

	if (!prog || !piglit_check_gl_error(GL_NO_ERROR)) {
		piglit_report_subtest_result(PIGLIT_FAIL, __func__);
		return false;
	}

	/* Test GL_ATOMIC_COUNTER_BUFFER. */
	glGetProgramResourceIndex(prog, GL_ATOMIC_COUNTER_BUFFER, "atom");
	if (!piglit_check_gl_error(GL_INVALID_ENUM)) {
		piglit_report_subtest_result(PIGLIT_FAIL, __func__);
                return false;
	}

	piglit_report_subtest_result(PIGLIT_PASS, __func__);
	return true;
}

void
piglit_init(int argc, char **argv)
{
	GLuint prog, shader;
	bool pass = true;
	const char **marker = xfb_markers;

	piglit_require_extension("GL_ARB_program_interface_query");
	piglit_require_extension("GL_ARB_explicit_attrib_location");

	/* Test invalid program. */
	glGetProgramResourceIndex(42, GL_UNIFORM, "name");
	if (!piglit_check_gl_error(GL_INVALID_VALUE)) {
		piglit_report_subtest_result(PIGLIT_FAIL,
			"invalid program test 1");
		pass = false;
	}

	/* Test passing a shader, not program. */
	shader = piglit_compile_shader_text(GL_VERTEX_SHADER, vs_text);
	glGetProgramResourceIndex(shader, GL_UNIFORM, "name");
	if (!piglit_check_gl_error(GL_INVALID_OPERATION)) {
		piglit_report_subtest_result(PIGLIT_FAIL,
			"invalid program test 2");
		pass = false;
	}

	prog = piglit_build_simple_program(vs_text, fs_text);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	/* Valid enum but invalid name. */
	glGetProgramResourceIndex(prog, GL_PROGRAM_INPUT, "name");
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	/* Iterate transform feedback marker strings. */
	for (; *marker; marker++) {
		GLuint index = glGetProgramResourceIndex(prog,
			GL_TRANSFORM_FEEDBACK_VARYING, *marker);
		if (index == GL_INVALID_INDEX)
			continue;
		piglit_report_subtest_result(PIGLIT_FAIL,
			"xfb marker string test (%s)", *marker);
		pass = false;
        }

	/* Check valid but missing program resource. */
	if (glGetProgramResourceIndex(prog, GL_TRANSFORM_FEEDBACK_VARYING,
		"sandwich") != GL_INVALID_INDEX)
		piglit_report_result(PIGLIT_FAIL);

	/* Check invalid index with array resource (> 0) */
	if (glGetProgramResourceIndex(prog, GL_UNIFORM, "array[7]") !=
		GL_INVALID_INDEX)
		piglit_report_result(PIGLIT_FAIL);

	/* Valid inputs. */
	validate_index(prog, GL_PROGRAM_INPUT,  "input0");
	validate_index(prog, GL_PROGRAM_INPUT,  "input1");
	validate_index(prog, GL_PROGRAM_OUTPUT, "gl_Position");
	validate_index(prog, GL_PROGRAM_OUTPUT, "output0");
	validate_index(prog, GL_PROGRAM_OUTPUT, "output1");
	validate_index(prog, GL_UNIFORM, "color");
	validate_index(prog, GL_UNIFORM_BLOCK, "ubo");
	validate_index(prog, GL_UNIFORM, "array");
	validate_index(prog, GL_UNIFORM, "array[0]");

	pass = atomic_counter_index_test() && pass;
	pass = subroutine_index_test() && pass;

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
