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

/** @file layout-std140-base-size-and-alignment.c
 *
 * Tests that glGetActiveUniformsiv() returns the correct offset for
 * any basic type valid in std140, and for a float just following
 * that, thus testing the size and base alignment for them.
 */

#include "piglit-util-gl.h"
#include "uniform-types.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.supports_gl_core_version = 31;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

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
		"	float pad;\n"
		"	%s %s u;\n"
		"	float size_test;\n"
		"};\n"
		"\n"
		"void main() {\n"
		"	gl_FragColor = vec4(pad) + vec4(%s) + vec4(size_test);\n"
		"}\n";
	char *fs_source;
	GLuint prog;
	const char *uniform_names[] = { "u", "size_test" };
	GLuint uniform_indices[2];
	GLint offsets[2];
	int offset, size, expected_offset;
	const struct uniform_type *transposed_type;
	bool pass;
	const char *deref;

	if (row_major)
		transposed_type = get_transposed_type(type);
	else
		transposed_type = type;

	if (type->size == 4)
		deref = "u";
	else if (type->size <= 16)
		deref = "u.x";
	else
		deref = "u[0].x";

	asprintf(&fs_source, fs_template,
		 row_major && type->size > 16 ? "layout(row_major) " : "",
		 type->type,
		 deref);
	prog = piglit_build_simple_program(NULL, fs_source);
	free(fs_source);

	glGetUniformIndices(prog, 2, uniform_names, uniform_indices);
	glGetActiveUniformsiv(prog, 2, uniform_indices,
			      GL_UNIFORM_OFFSET, offsets);

	glDeleteProgram(prog);

	offset = offsets[0];
	size = offsets[1] - offsets[0];

	/* "pad" at the start of the UBO is a float, so our test
	 * uniform would start at byte 4 if not for alignment.
	 */
	expected_offset = 4;
	expected_offset = align(expected_offset, transposed_type->alignment);

	pass = (offset == expected_offset &&
		size == transposed_type->size);

	printf("%-10s %10s %8d %-16d %8d %-16d%s\n",
	       type->type,
	       row_major ? "y" : "n",
	       offset,
	       expected_offset,
	       size,
	       transposed_type->size,
	       pass ? "" : " FAIL");

	return pass;
}

void
piglit_init(int argc, char **argv)
{
	bool pass = true;
	unsigned int i;

	piglit_require_extension("GL_ARB_uniform_buffer_object");
	piglit_require_GLSL_version(140);

	printf("%-10s %10s %8s %16s %8s %-16s\n",
	       "type", "row_major",
	       "offset", "expected offset", "size", "expected size");

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
