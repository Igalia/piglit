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

/** @file getactiveuniformblockiv-uniform-block-data-size.c
 *
 * From the GL_ARB_uniform_buffer_object spec:
 *
 *     "For uniform blocks laid out according to [std140] rules, the
 *      minimum buffer object size returned by the
 *      UNIFORM_BLOCK_DATA_SIZE query is derived by taking the offset
 *      of the last basic machine unit consumed by the last uniform of
 *      the uniform block (including any end-of-array or
 *      end-of-structure padding), adding one, and rounding up to the
 *      next multiple of the base alignment required for a vec4."
 */

#include "piglit-util-gl.h"
#include "uniform-types.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.supports_gl_core_version = 31;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static int
align(int v, int a)
{
	return (v + a - 1) & ~(a - 1);
}

static bool
test_format(const struct uniform_type *type, bool row_major)
{
	/* Using 140 to get unsigned ints. */
	const char *fs_template =
		"#version 140\n"
		"layout(std140) uniform ubo {\n"
		"	float align_test;\n"
		"	%s%s u;\n"
		"};\n"
		"\n"
		"void main() {\n"
		"	gl_FragColor = vec4(align_test);\n"
		"}\n";
	char *fs_source;
	GLuint prog;
	GLint data_size;
	int expected;
	const struct uniform_type *transposed_type;

	if (row_major)
		transposed_type = get_transposed_type(type);
	else
		transposed_type = type;

	asprintf(&fs_source, fs_template,
		 row_major ? "layout(row_major) " : "",
		 type->type);
	prog = piglit_build_simple_program(NULL, fs_source);
	free(fs_source);

	/* There's only one block, so it's uniform block 0. */
	glGetActiveUniformBlockiv(prog, 0,
				  GL_UNIFORM_BLOCK_DATA_SIZE,
				  &data_size);

	glDeleteProgram(prog);

	/* "align_test" at the start of the UBO is a float, so our
	 * test uniform would start at byte 4 if not for alignment.
	 */
	expected = 4;

	/* The actual space for our object. */
	expected = align(expected, transposed_type->alignment);
	expected += transposed_type->size;

	/* Finally, align to a vec4 like std140 says. */
	expected = align(expected, 16);

	printf("%-20s %10s %10d %10d%s\n",
	       type->type,
	       row_major ? "y" : "n",
	       data_size,
	       expected,
	       data_size == expected ? "" : " FAIL");

	return data_size == expected;
}

void
piglit_init(int argc, char **argv)
{
	bool pass = true;
	unsigned int i;

	piglit_require_extension("GL_ARB_uniform_buffer_object");
	piglit_require_GLSL_version(140);

	printf("%-20s %10s %10s %10s\n",
	       "type", "row_major", "DATA_SIZE", "expected");

	for (i = 0; uniform_types[i].type; i++) {
		pass = test_format(&uniform_types[i], false) && pass;
		pass = test_format(&uniform_types[i], true) && pass;
	}

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result piglit_display(void)
{
	/* UNREACHED */
	return PIGLIT_FAIL;
}

