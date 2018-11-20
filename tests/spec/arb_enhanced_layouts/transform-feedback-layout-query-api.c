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

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 32;
	config.supports_gl_core_version = 32;

PIGLIT_GL_TEST_CONFIG_END

static const char vs_two_buff_text[] =
	"#version 150\n"
	"#extension GL_ARB_enhanced_layouts: require\n"
	"\n"
	"layout(xfb_buffer = 1) out;\n"
	"layout(xfb_offset = 0) out float x1_out;\n"
	"layout(xfb_offset = 4) out float x2_out[2];\n"
	"layout(xfb_offset = 12) out vec3 x3_out;\n"
	"layout(xfb_buffer = 3) out;\n"
	"layout(xfb_offset = 0, xfb_buffer = 3) out float y1_out;\n"
	"layout(xfb_offset = 4) out vec4 y2_out;\n"
	"void main() {\n"
	"  gl_Position = vec4(0.0);\n"
	"  x1_out = 1.0;\n"
	"  x2_out[0] = 2.0;\n"
	"  x2_out[1] = 3.0;\n"
	"  x3_out = vec3(4.0, 5.0, 6.0);\n"
	"  y1_out = 7.0;\n"
	"  y2_out = vec4(8.0, 9.0, 10.0, 11.0);\n"
	"}";

static const char *varying_names[2][3] = {
	 {"x1_out", "x2_out", "x3_out"},
	 {"y1_out", "y2_out", ""} };


static GLuint
build_and_use_program(const char *vs_text)
{
	GLuint prog = piglit_build_simple_program_multiple_shaders(
				GL_VERTEX_SHADER, vs_text, 0);

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
		unsigned buffer)
{
	bool match = true;

	for (unsigned i = 0; i < num_values; i++) {
		char name[10];
		glGetProgramResourceName(prog, GL_TRANSFORM_FEEDBACK_VARYING,
					values[i], 10, NULL, name);

		bool match_found = false;
		for (unsigned i = 0; i < num_values; i++) {
			if (strcmp(name, varying_names[buffer][i]) == 0) {
				match_found = true;
				break;
			}
		}
		if (!match_found) {
			match = false;
			printf("ACTIVE_VARIABLES did no return an index for "
				"%s\n", varying_names[buffer][i]);
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

	piglit_require_GLSL_version(150);
	piglit_require_extension("GL_ARB_transform_feedback3");
	piglit_require_extension("GL_ARB_enhanced_layouts");

	prog = build_and_use_program(vs_two_buff_text);

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
				check_varyings_match(prog, &values[2], 3, 0);
		} else if (values[0] == 3) {
			if (values[1] == 2) {
				num_active[i] = true;
			} else {
				printf("Expected 2 NUM_ACTIVE_VARIABLES "
					"found %d\n", values[1]);
			}

			varying_idx[i] =
				 check_varyings_match(prog, &values[2], 2, 1);
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
