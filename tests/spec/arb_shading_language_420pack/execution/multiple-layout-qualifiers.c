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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/** @file multiple-layout-qualifiers.c
 *
 * From the GL_ARB_shading_language_420pack spec:
 *
 *     "More than one layout qualifier may appear in a single declaration. If
 *      the same layout-qualifier-name occurs in multiple layout qualifiers for
 *      the same declaration, the last one overrides the former ones."
 *
 *      For example
 *
 *          layout(column_major) layout(row_major)
 *
 *      results in the qualification being row_major."
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 10;
	config.window_height = 10;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static const char *source =
	"#extension GL_ARB_shading_language_420pack: enable\n"
	"#extension GL_ARB_uniform_buffer_object : enable\n"
	"\n"
	"/* Use std140 to avoid needing to ref every single uniform */\n"
	"layout(std140) uniform;\n"
	"\n"
	"layout(column_major) uniform a {\n"
	"	layout(column_major) layout(row_major) mat4 a_rm1;\n"
	"	layout(row_major) layout(column_major) mat4 a_cm1;\n"
	"};\n"
	"\n"
	"layout(row_major) uniform b {\n"
	"	layout(column_major) layout(row_major) mat4 a_rm2;\n"
	"	layout(row_major) layout(column_major) mat4 a_cm2;\n"
	"};\n"
	"\n"
	"uniform c {\n"
	"	layout(column_major) layout(row_major) mat4 a_rm3;\n"
	"	layout(row_major) layout(column_major) mat4 a_cm3;\n"
	"};\n"
	"\n"
	"void main() {\n"
	"	gl_FragColor = (\n"
	"		a_cm1[0] + \n"
	"		a_cm2[0] + \n"
	"		a_cm3[0] + \n"
	"		a_rm1[0] + \n"
	"		a_rm2[0] + \n"
	"		a_rm3[0]);\n"
	"}\n";

static struct {
	const char *name;
	bool row_major;
} uniforms[] = {
	{ "a_cm1", false },
	{ "a_cm2", false },
	{ "a_rm1", true },
	{ "a_rm2", true },
};

void
piglit_init(int argc, char **argv)
{
	bool pass = true;
	unsigned int i;
	GLuint prog;

	piglit_require_extension("GL_ARB_shading_language_420pack");
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
