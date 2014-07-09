/*
 * Copyright © 2012 Intel Corporation
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

/** @file row-major.c
 *
 * From the GL_ARB_uniform_buffer_object spec:
 *
 *     "The row_major qualifier overrides only the column_major
 *      qualifier; other qualifiers are inherited. It only affects the
 *      layout of matrices. Elements within a matrix row will be
 *      contiguous in memory.
 *
 *      The column_major qualifier overrides only the row_major
 *      qualifier; other qualifiers are inherited. It only affects the
 *      layout of matrices. Elements within a matrix column will be
 *      contiguous in memory.
 *
 *      When multiple arguments are listed in a layout declaration,
 *      the affect will be the same as if they were declared one at a
 *      time, in order from left to right, each in turn inheriting
 *      from and overriding the result from the previous
 *      qualification.
 *
 *      For example
 *
 *          layout(row_major, column_major)
 *
 *      results in the qualification being column_major."
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static const char *source =
	"#extension GL_ARB_uniform_buffer_object : enable\n"
	"\n"
	"/* Use std140 to avoid needing to ref every single uniform */\n"
	"layout(std140) uniform;\n"
	"\n"
	"layout(column_major) uniform a {\n"
	"	mat4 a_cm1;\n"
	"	layout(column_major) mat4 a_cm2;\n"
	"	layout(row_major) mat4 a_rm1;\n"
	"	layout(row_major, column_major) mat4 a_cm3;\n"
	"	vec4 a_non_matrix;\n"
	"};\n"
	"\n"
	"layout(row_major) uniform b {\n"
	"	mat4 b_rm1;\n"
	"	layout(column_major) mat4 b_cm1;\n"
	"	layout(row_major) mat4 b_rm2;\n"
	"	vec4 b_non_matrix;\n"
	"};\n"
	"\n"
	"uniform c {\n"
	"	mat4 c_cm1;\n"
	"	layout(column_major) mat4 c_cm2;\n"
	"	layout(row_major) mat4 c_rm1;\n"
	"	vec4 c_non_matrix;\n"
	"};\n"
	"\n"
	"/* Set the default layout to row_major.  Spam in some block layout\n"
	" * qualifiers to make sure they don't accidentally clear row_major.\n"
	" */\n"
	"layout(row_major, std140) uniform;\n"
	"layout(std140) uniform;\n"
	"\n"
	"layout(column_major) uniform d {\n"
	"	mat4 d_cm1;\n"
	"	layout(column_major) mat4 d_cm2;\n"
	"	layout(row_major) mat4 d_rm1;\n"
	"	vec4 d_non_matrix;\n"
	"};\n"
	"\n"
	"layout(row_major) uniform e {\n"
	"	mat4 e_rm1;\n"
	"	layout(column_major) mat4 e_cm1;\n"
	"	layout(row_major) mat4 e_rm2;\n"
	"	vec4 e_non_matrix;\n"
	"};\n"
	"\n"
	"layout(std140) uniform f {\n"
	"	mat4 f_rm1;\n"
	"	layout(column_major) mat4 f_cm1;\n"
	"	layout(row_major) mat4 f_rm2;\n"
	"	vec4 f_non_matrix;\n"
	"};\n"
	"\n"
	"uniform mat4 non_ubo_mat;\n"
	"uniform vec4 non_mat;\n"
	"\n"
	"void main() {\n"
	"	gl_FragColor = (\n"
	"		non_ubo_mat[0] + \n"
	"		non_mat + \n"
	"		a_cm1[0] + \n"
	"		a_cm2[0] + \n"
	"		a_rm1[0] + \n"
	"		a_cm3[0] + \n"
	"		b_cm1[0] + \n"
	"		b_rm1[0] + \n"
	"		b_rm2[0] + \n"
	"		c_cm1[0] + \n"
	"		c_cm2[0] + \n"
	"		c_rm1[0] + \n"
	"		d_cm1[0] + \n"
	"		d_cm2[0] + \n"
	"		d_rm1[0] + \n"
	"		e_cm1[0] + \n"
	"		e_rm1[0] + \n"
	"		e_rm2[0] + \n"
	"		f_cm1[0] + \n"
	"		f_rm1[0] + \n"
	"		f_rm2[0] + \n"
	"		a_non_matrix + \n"
	"		b_non_matrix + \n"
	"		c_non_matrix + \n"
	"		d_non_matrix + \n"
	"		e_non_matrix + \n"
	"		f_non_matrix);\n"
	"}\n";

static struct {
	const char *name;
	bool row_major;
} uniforms[] = {
	{ "a_non_matrix", false },
	{ "a_cm1", false },
	{ "a_cm2", false },
	{ "a_cm3", false },
	{ "a_rm1", true },

	{ "b_non_matrix", false },
	{ "b_cm1", false },
	{ "b_rm1", true },
	{ "b_rm2", true },

	{ "c_non_matrix", false },
	{ "c_cm1", false },
	{ "c_cm2", false },
	{ "c_rm1", true },

	{ "d_non_matrix", false },
	{ "d_cm1", false },
	{ "d_cm2", false },
	{ "d_rm1", true },

	{ "e_non_matrix", false },
	{ "e_cm1", false },
	{ "e_rm1", true },
	{ "e_rm2", true },

	{ "f_non_matrix", false },
	{ "f_cm1", false },
	{ "f_rm1", true },
	{ "f_rm2", true },

	{ "non_ubo_mat", false },
	{ "non_mat", false },
};

void
piglit_init(int argc, char **argv)
{
	bool pass = true;
	unsigned int i;
	GLuint prog;

	piglit_require_extension("GL_ARB_uniform_buffer_object");
	prog = piglit_build_simple_program(NULL, source);

	for (i = 0; i < ARRAY_SIZE(uniforms); i++) {
		GLuint index;
		GLint row_major;

		glGetUniformIndices(prog, 1, &uniforms[i].name, &index);
		if (index == GL_INVALID_INDEX) {
			printf("Failed to get index for %s\n",
			       uniforms[i].name);
			pass = false;
			continue;
		}

		glGetActiveUniformsiv(prog, 1, &index, GL_UNIFORM_IS_ROW_MAJOR,
				      &row_major);

		if (row_major != uniforms[i].row_major) {
			fprintf(stderr, "Uniform %s should %sbe row major\n",
				uniforms[i].name,
				uniforms[i].row_major ? "" : "not ");
			pass = false;
		}
	}

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result piglit_display(void)
{
	/* UNREACHED */
	return PIGLIT_FAIL;
}

