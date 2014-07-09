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

/** @file getactiveuniformsiv-uniform-type.c
 *
 * Tests that glGetActiveUniformsiv() returns the correct enum for
 * GL_UNIFORM_TYPE for variables in a UBO.
 */

#include "piglit-util-gl.h"
#include "uniform-types.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.supports_gl_core_version = 31;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

static bool
test_format(const struct uniform_type *type)
{
	/* Using 140 to get unsigned ints. */
	const char *fs_template =
		"#version 140\n"
		"layout(std140) uniform ubo {\n"
		"	float align_test;\n"
		"	%s u;\n"
		"};\n"
		"\n"
		"void main() {\n"
		"	gl_FragColor = vec4(align_test + float(%s));\n"
		"}\n";
	char *fs_source;
	GLuint prog;
	const char *uniform_name = "u";
	GLuint uniform_index;
	GLint uniform_type;
	const char *deref;

	if (type->size == 4) {
		deref = "u";
	} else if (type->size <= 16) {
		deref = "u.x";
	} else {
		deref = "u[0].x";
	}

	asprintf(&fs_source, fs_template, type->type, deref);
	prog = piglit_build_simple_program(NULL, fs_source);
	free(fs_source);

	glGetUniformIndices(prog, 1, &uniform_name, &uniform_index);
	glGetActiveUniformsiv(prog, 1, &uniform_index,
			      GL_UNIFORM_TYPE, &uniform_type);

	glDeleteProgram(prog);

	printf("%-20s %20s %20s%s\n",
	       type->type,
	       piglit_get_gl_enum_name(uniform_type),
	       piglit_get_gl_enum_name(type->gl_type),
	       uniform_type == type->gl_type ? "" : " FAIL");

	return uniform_type == type->gl_type;
}

void
piglit_init(int argc, char **argv)
{
	bool pass = true;
	unsigned int i;

	piglit_require_extension("GL_ARB_uniform_buffer_object");
	piglit_require_GLSL_version(140);

	printf("%-20s %20s %20s\n", "type", "GL_UNIFORM_TYPE", "expected");
	printf("--------------------------------------------------------------\n");
	for (i = 0; uniform_types[i].type; i++) {
		pass = test_format(&uniform_types[i]) && pass;
	}

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result piglit_display(void)
{
	/* UNREACHED */
	return PIGLIT_FAIL;
}
