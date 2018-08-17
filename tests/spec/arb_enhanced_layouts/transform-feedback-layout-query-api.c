/*
 * Copyright © 2016 Intel Corporation
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
#include "piglit-shader-test.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 32;
	config.supports_gl_core_version = 32;

PIGLIT_GL_TEST_CONFIG_END

#define VS_TWO_BUFF_NAME "vs_two_buff.shader_source"

static const char *varying_names[2][3] = {
	 {"x1_out", "x2_out", "x3_out"},
	 {"y1_out", "y2_out", ""} };

static const int varying_types[2][3] = {
	{GL_FLOAT, GL_FLOAT, GL_FLOAT_VEC3},
	{GL_FLOAT, GL_FLOAT_VEC4} };

static const int varying_offsets[2][3] = {
	{0, 4, 12},
	{0, 4}};

static const int varying_buff_index[2][3] = {
	{0, 0, 0},
	{1, 1}};

static GLuint
compile_spirv_program(GLenum shader_type,
		      const unsigned spirv_asm_size,
		      const char *spirv_asm)
{
	GLuint shader, prog;

	shader = piglit_assemble_spirv(shader_type,
				       spirv_asm_size,
				       spirv_asm);

	glSpecializeShader(shader,
			   "main",
			   0, /* numSpecializationConstants */
			   NULL /* pConstantIndex */,
			   NULL /* pConstantValue */);

	prog = glCreateProgram();
	glAttachShader(prog, shader);
	glDeleteShader(shader);

	return prog;
}

static GLuint
build_and_use_program(const char *shader_test_filename, bool spirv)
{
	GLuint prog;
	char filepath[4096];
	char *source;
	unsigned source_size;

	piglit_join_paths(filepath, sizeof(filepath), 6, /* num parts */
			  piglit_source_dir(), "tests", "spec",
			  "arb_enhanced_layouts", "shader_source",
			  shader_test_filename);


	piglit_load_source_from_shader_test(filepath, GL_VERTEX_SHADER, spirv,
					    &source, &source_size);

	if (spirv)
		prog = compile_spirv_program(GL_VERTEX_SHADER, source_size,
					     source);
	else
		prog = piglit_build_simple_program_multiple_shaders(
			GL_VERTEX_SHADER, source, 0);

	free(source);

	glLinkProgram(prog);
	if (!piglit_link_check_status(prog))
		piglit_report_result(PIGLIT_FAIL);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	glUseProgram(prog);

	return prog;
}

static bool
check_varyings_match(GLuint prog, GLint *values, unsigned num_values,
		     unsigned buffer, bool spirv)
{
	bool match = true;
	char name[10];
	GLint got[3];
	GLenum props[] = {GL_TRANSFORM_FEEDBACK_BUFFER_INDEX,
			  GL_OFFSET, GL_TYPE};

	for (unsigned i = 0; i < num_values; i++) {
		glGetProgramResourceName(prog, GL_TRANSFORM_FEEDBACK_VARYING,
					values[i], 10, NULL, name);

		glGetProgramResourceiv(prog, GL_TRANSFORM_FEEDBACK_VARYING,
				       values[i], 3, props, 3, NULL, got);

		bool match_found = false;
		const char *varying_name = "";
		for (unsigned i = 0; i < num_values; i++) {
			if (!spirv)
				varying_name = varying_names[buffer][i];

			if ((strcmp(name, varying_name) == 0) &&
			    (got[0] == varying_buff_index[buffer][i]) &&
			    (got[1] == varying_offsets[buffer][i]) &&
				(got[2] == varying_types[buffer][i])) {
				match_found = true;
				break;
			}
		}

		if (!match_found) {
			match = false;
			printf("ACTIVE_VARIABLES did not return an index for "
			       "the resource with name: \"%s\", "
			       "buffer index: %d, offset: %d and type: %d\n",
			       varying_names[buffer][i], got[0], got[1],
			       got[2]);
			break;
		}
	}

	return match;
}

void
piglit_init(int argc, char **argv)
{
	bool pass = true;
	bool active_res = true;
	bool max_active = true;
	bool buff_bind[2] = { true, true };
	bool num_active[2] = { false, false };
	bool varying_idx[2] = { false, false };
	GLuint prog;
	bool spirv = false;

	piglit_require_GLSL_version(150);
	piglit_require_extension("GL_ARB_transform_feedback3");
	piglit_require_extension("GL_ARB_enhanced_layouts");

	if (argc > 1 && !strcmp(argv[1], "spirv")) {
		spirv = true;
		piglit_require_extension("GL_ARB_gl_spirv");
	}

	prog = build_and_use_program(VS_TWO_BUFF_NAME, spirv);

	GLint value;
        GLint values[5];
	glGetProgramInterfaceiv(prog, GL_TRANSFORM_FEEDBACK_BUFFER,
			GL_ACTIVE_RESOURCES, &value);
	if (value != 2) {
		printf("Expected 2 ACTIVE_RESOURCES found %d\n", value);
		active_res = false;
	}

	piglit_report_subtest_result(active_res ? PIGLIT_PASS : PIGLIT_FAIL,
			"Query ACTIVE_RESOURCES");

	glGetProgramInterfaceiv(prog, GL_TRANSFORM_FEEDBACK_BUFFER,
			GL_MAX_NUM_ACTIVE_VARIABLES, &value);
	if (value != 3) {
		printf("Expected MAX_NUM_ACTIVE_VARIABLES to be 3 found %d\n",
		value);
		max_active = false;
	}

	piglit_report_subtest_result(max_active ? PIGLIT_PASS : PIGLIT_FAIL,
			"Query MAX_NUM_ACTIVE_VARIABLES");

	GLenum props[] = {GL_BUFFER_BINDING, GL_NUM_ACTIVE_VARIABLES,
			GL_ACTIVE_VARIABLES};
	for (unsigned i = 0; i < 2; i++) {
		glGetProgramResourceiv(prog, GL_TRANSFORM_FEEDBACK_BUFFER, i,
				3, props, 5, NULL, values);

		if (values[0] != 1 && values[0] != 3)
			buff_bind[i] = false;
		if (values[0] == 1) {
			if(values[1] == 3) {
				num_active[i] = true;
			} else {
				printf("Expected 3 NUM_ACTIVE_VARIABLES "
					"found %d\n", values[1]);
			}

			varying_idx[i] =
				check_varyings_match(prog, &values[2], 3, 0,
						     spirv);
		} else if (values[0] == 3) {
			if (values[1] == 2) {
				num_active[i] = true;
			} else {
				printf("Expected 2 NUM_ACTIVE_VARIABLES "
					"found %d\n", values[1]);
			}

			varying_idx[i] =
				check_varyings_match(prog, &values[2], 2, 1,
						     spirv);
		}
	}

	piglit_report_subtest_result(buff_bind[0] && buff_bind[1] ?
			 PIGLIT_PASS : PIGLIT_FAIL,
			"Query BUFFER_BINDING");

	piglit_report_subtest_result(num_active[0] && num_active[1] ?
			 PIGLIT_PASS : PIGLIT_FAIL,
			"Query NUM_ACTIVE_VARIABLES");

	piglit_report_subtest_result(varying_idx[0] && varying_idx[1] ?
			 PIGLIT_PASS : PIGLIT_FAIL,
			"Query ACTIVE_VARIABLES");

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	pass = active_res && max_active && buff_bind[0] && buff_bind[1] &&
		num_active[0] && num_active[1] && varying_idx[0] &&
		varying_idx[1];

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result
piglit_display(void)
{
	/* Should never be reached */
	return PIGLIT_FAIL;
}
