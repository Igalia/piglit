/*
 * Copyright © 2010 Intel Corporation
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

/** @file link-mismatch-blocks.c
 *
 * From the GL_ARB_uniform_buffer_object spec:
 *
 *     "Uniform block names and variable names declared within uniform
 *      blocks are scoped at the program level. Matching block names
 *      from multiple compilation units in the same program must match
 *      in terms of having the same number of declarations with the
 *      same sequence of types and the same sequence of member names,
 *      as well as having the same member-wise layout qualification
 *      (see next section). Any mismatch will generate a link error. "
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

struct test {
	const char *a_header;
	const char *a_body;
	const char *b_header;
	const char *b_body;
};

struct test tests[] = {
	/* Mismatched names */
	{
		"layout(std140) uniform;\n"
		"uniform ubo1 {\n"
		"	vec4 a;\n"
		"};\n",

		"a",

		"layout(std140) uniform;\n"
		"uniform ubo1 {\n"
		"	vec4 b;\n"
		"};\n",

		"b",
	},

	/* Mismatched type: vector elements */
	{
		"layout(std140) uniform;\n"
		"uniform ubo1 {\n"
		"	vec4 a;\n"
		"};\n",

		"a",

		"layout(std140) uniform;\n"
		"uniform ubo1 {\n"
		"	vec3 a;\n"
		"};\n",

		"vec4(a, 0)",
	},

	/* Mismatched type: base type */
	{
		"layout(std140) uniform;\n"
		"uniform ubo1 {\n"
		"	vec4 a;\n"
		"};\n",

		"a",

		"layout(std140) uniform;\n"
		"uniform ubo1 {\n"
		"	ivec4 a;\n"
		"};\n",

		"vec4(a)",
	},

	/* Mismatched number of members. */
	{
		"layout(std140) uniform;\n"
		"uniform ubo1 {\n"
		"	vec4 a;\n"
		"	vec4 b;\n"
		"};\n",

		"a",

		"layout(std140) uniform;\n"
		"uniform ubo1 {\n"
		"	vec4 a;\n"
		"};\n",

		"a",
	},

	/* Mismatched number of members */
	{
		"layout(std140) uniform;\n"
		"uniform ubo1 {\n"
		"	vec4 a;\n"
		"};\n",

		"a",

		"layout(std140) uniform;\n"
		"uniform ubo1 {\n"
		"	vec4 a;\n"
		"	vec4 b;\n"
		"};\n",

		"a",
	},

	/* row_major mismatch */
	{
		"layout(std140) uniform;\n"
		"uniform ubo1 {\n"
		"	layout(row_major) mat4 a;\n"
		"};\n",

		"a[0]",

		"layout(std140) uniform;\n"
		"uniform ubo1 {\n"
		"	mat4 a;\n"
		"};\n",

		"a[0]",
	},
};


enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

static char *
get_shader(GLenum target, const char *header, const char *body)
{
	const char *vs_template =
		"#extension GL_ARB_uniform_buffer_object : require\n"
		"%s"
		"varying vec4 v;\n"
		"void main()\n"
		"{\n"
		"	gl_Position = gl_Vertex;\n"
		"	v = %s\n;"
		"}\n";
	const char *fs_template =
		"#extension GL_ARB_uniform_buffer_object : require\n"
		"%s"
		"varying vec4 v;\n"
		"void main()\n"
		"{\n"
		"	gl_FragColor = v + %s;\n"
		"}\n";
	const char *template = NULL;
	char *source;

	switch (target) {
	case GL_VERTEX_SHADER:
		template = vs_template;
		break;
	case GL_FRAGMENT_SHADER:
		template = fs_template;
		break;
	default:
		fprintf(stderr, "bad target %d\n", target);
		piglit_report_result(PIGLIT_FAIL);
		break;
	}

	asprintf(&source, template, header, body);

	return source;
}

static bool
test_link_fail(struct test *test, GLenum a_target, GLenum b_target)
{
	GLuint a, b, prog;
	GLint ok;
	char *a_source, *b_source;

	a_source = get_shader(a_target, test->a_header, test->a_body);
	b_source = get_shader(b_target, test->b_header, test->b_body);

	a = piglit_compile_shader_text(a_target, a_source);
	b = piglit_compile_shader_text(b_target, b_source);

	prog = glCreateProgram();
	glAttachShader(prog, a);
	glAttachShader(prog, b);
	glLinkProgram(prog);
	glDeleteShader(a);
	glDeleteShader(b);

	ok = piglit_link_check_status_quiet(prog);
	glDeleteProgram(prog);
	if (ok) {
		fprintf(stderr,
			"Linking shaders succeeded when it should have "
			"failed:\n%s\n%s\n",
			a_source, b_source);
		return false;
	}
	return true;
}

static bool
do_test(struct test *test)
{
	if (!test_link_fail(test, GL_VERTEX_SHADER, GL_FRAGMENT_SHADER))
		return false;
	if (!test_link_fail(test, GL_VERTEX_SHADER, GL_VERTEX_SHADER))
		return false;
	if (!test_link_fail(test, GL_FRAGMENT_SHADER, GL_FRAGMENT_SHADER))
		return false;

	return true;
}

void
piglit_init(int argc, char **argv)
{
	int i;
	bool pass = true;

	piglit_require_extension("GL_ARB_uniform_buffer_object");

	for (i = 0; i < ARRAY_SIZE(tests); i++) {
		pass = do_test(&tests[i]) && pass;
	}

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
