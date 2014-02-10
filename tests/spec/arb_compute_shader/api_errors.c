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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/** \file
 *
 * Test cases in which the ARB_compute_shader API is expected to
 * generate an error.
 */

#include "piglit-util-gl-common.h"
#include "piglit-shader.h"


static struct piglit_gl_test_config *piglit_config;


PIGLIT_GL_TEST_CONFIG_BEGIN
	piglit_config = &config;
	config.supports_gl_compat_version = 33;
	config.supports_gl_core_version = 33;
PIGLIT_GL_TEST_CONFIG_END


static const char *trivial_correct_shader =
	"#version 330\n"
	"#extension GL_ARB_compute_shader: enable\n"
	"\n"
	"layout(local_size_x = 1) in;\n"
	"\n"
	"void main()\n"
	"{\n"
	"}\n";


static const char *trivial_link_fail_shader =
	"#version 330\n"
	"#extension GL_ARB_compute_shader: enable\n"
	"\n"
	"void main()\n"
	"{\n"
	"}\n";


static const char *trivial_vertex_shader =
	"#version 330\n"
	"\n"
	"void main()\n"
	"{\n"
	"  gl_Position = vec4(0.0);\n"
	"}\n";


static enum piglit_result
query_work_group_size_expect_error(GLint prog)
{
	const GLint orig_query_result[3] = { 1234, 2345, 3456 };
	GLint query_result[3];
	int i;

	for (i = 0; i < 3; i++)
		query_result[i] = orig_query_result[i];

	glGetProgramiv(prog, GL_COMPUTE_WORK_GROUP_SIZE, query_result);

	if (!piglit_check_gl_error(GL_INVALID_OPERATION))
		return PIGLIT_FAIL;
	for (i = 0; i < 3; i++) {
		if (query_result[i] != orig_query_result[i]) {
			printf("Error was generated, but query returned a "
			       "result anyway.");
			return PIGLIT_FAIL;
		}
	}
	return PIGLIT_PASS;
}


static enum piglit_result
query_work_group_size_unlinked(void *data)
{
	/* From the ARB_compute_shader spec, in the description of the
	 * COMPUTE_WORK_GROUP_SIZE query:
	 *
	 *     If <program> is the name of a program that has not been
	 *     successfully linked, or is the name of a linked program
	 *     object that contains no compute shaders, then an
	 *     INVALID_OPERATION error is generated.
	 *
	 * In this test, we use an unlinked program.
	 */
	GLint prog = piglit_build_simple_program_unlinked_multiple_shaders(
		GL_COMPUTE_SHADER, trivial_correct_shader, 0);
	return query_work_group_size_expect_error(prog);
}


static enum piglit_result
query_work_group_size_link_fail(void *data)
{
	/* From the ARB_compute_shader spec, in the description of the
	 * COMPUTE_WORK_GROUP_SIZE query:
	 *
	 *     If <program> is the name of a program that has not been
	 *     successfully linked, or is the name of a linked program
	 *     object that contains no compute shaders, then an
	 *     INVALID_OPERATION error is generated.
	 *
	 * In this test, we use a program that fails to link.
	 */
	GLint ok;
	GLint prog = piglit_build_simple_program_unlinked_multiple_shaders(
		GL_COMPUTE_SHADER, trivial_link_fail_shader, 0);
	glLinkProgram(prog);
	glGetProgramiv(prog, GL_LINK_STATUS, &ok);
	if (ok) {
		printf("Expected link failure, got link success\n");
		return PIGLIT_FAIL;
	}
	if (!piglit_check_gl_error(GL_NO_ERROR))
		return PIGLIT_FAIL;

	return query_work_group_size_expect_error(prog);
}


static enum piglit_result
query_work_group_size_no_compute(void *data)
{
	/* From the ARB_compute_shader spec, in the description of the
	 * COMPUTE_WORK_GROUP_SIZE query:
	 *
	 *     If <program> is the name of a program that has not been
	 *     successfully linked, or is the name of a linked program
	 *     object that contains no compute shaders, then an
	 *     INVALID_OPERATION error is generated.
	 *
	 * In this test, we use a program that has no compute shaders.
	 */
	GLint prog = piglit_build_simple_program_multiple_shaders(
		GL_VERTEX_SHADER, trivial_vertex_shader, 0);
	return query_work_group_size_expect_error(prog);
}


static const struct piglit_subtest subtests[] = {
	{
		"Query COMPUTE_WORK_GROUP_SIZE on unlinked program",
		"query-work-group-size-unlinked",
		query_work_group_size_unlinked,
		NULL
	},
	{
		"Query COMPUTE_WORK_GROUP_SIZE on program that failed to link",
		"query-work-group-size-link-fail",
		query_work_group_size_link_fail,
		NULL
	},
	{
		"Query COMPUTE_WORK_GROUP_SIZE on program without compute shaders",
		"query-work-group-size-no-compute",
		query_work_group_size_no_compute,
		NULL
	},
	{
		NULL,
		NULL,
		NULL,
		NULL
	}
};


enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}


void
piglit_init(int argc, char **argv)
{
	enum piglit_result result;

	piglit_require_extension("GL_ARB_compute_shader");
	result = piglit_run_selected_subtests(subtests,
					      piglit_config->selected_subtests,
					      piglit_config->num_selected_subtests,
					      PIGLIT_SKIP);
	piglit_report_result(result);
}
