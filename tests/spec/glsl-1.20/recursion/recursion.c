/*
 * Copyright © 2011 Intel Corporation
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

/**
 * \file glsl-recursion.c
 * Verify that shaders containing static recursion are rejected.
 *
 * From page 44 (page 50 of the PDF) of the GLSL 1.20 spec:
 *
 *     "Recursion is not allowed, not even statically. Static recursion is
 *     present if the static function call graph of the program contains
 *     cycles."
 *
 * This langauge leaves a lot of questions unanswered.
 *
 *     - Is the error generated at compile-time or link-time?
 *
 *     - Is it an error to have a recursive function that is never statically
 *       called by main or any function called directly or indirectly by main?
 *       Technically speaking, such a function is not in the "static function
 *       call graph of the program" at all.
 *
 * This set of tests checks for a variety of forms of recursion in shaders.
 * Logs are dumped at both compile-time and link-time.  Errors are only
 * checked at link time.  However, a compile error will also generate a link
 * error (linking an uncompiled shader).
 *
 * \author Ian Romanick <ian.d.romanick@intel.com>
 */
#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static const char simple_text[] =
	"#version 120\n"
	"int A(void) { return A(); }\n"
	"\n"
	"void main() {\n"
	"  A();\n"
	"  gl_Position = gl_Vertex;\n"
	"}"
	;

static const char unreachable_text[] =
	"#version 120\n"
	"int A(void) { return A(); }\n"
	"\n"
	"void main() {\n"
	"  gl_Position = gl_Vertex;\n"
	"}"
	;

static const char unreachable_opt_text[] =
	"#version 120\n"
	"int A(void) { return A(); }\n"
	"\n"
	"void main() {\n"
	"  if (false) A();\n"
	"  gl_Position = gl_Vertex;\n"
	"}"
	;

static const char indirect_text[] =
	"#version 120\n"
	"int A(void);\n"
	"int B(void) { return A(); }\n"
	"int A(void) { return B(); }\n"
	"\n"
	"void main() {\n"
	"  A();\n"
	"  gl_Position = gl_Vertex;\n"
	"}"
	;

static const char indirect_sep1_text[] =
	"#version 120\n"
	"int B(void);\n"
	"int A(void) { return B(); }\n"
	"\n"
	"void main() {\n"
	"  A();\n"
	"  gl_Position = gl_Vertex;\n"
	"}"
	;

static const char indirect_sep2_text[] =
	"#version 120\n"
	"int A(void);\n"
	"int B(void) { return A(); }\n"
	;

static const char indirect_complex_text[] =
	"#version 120\n"
	"int A(bool);\n"
	"int B(bool from_a) { if (!from_a) return A(true); return 0; }\n"
	"int A(bool from_b) { if (!from_b) return B(true); return 0; }\n"
	"\n"
	"void main() {\n"
	"  A(false);\n"
	"  B(false);\n"
	"  gl_Position = gl_Vertex;\n"
	"}"
	;

static const char indirect_complex1_text[] =
	"#version 120\n"
	"int B(bool);\n"
	"int A(bool from_b) { if (!from_b) return B(true); return 0; }\n"
	"\n"
	"void main() {\n"
	"  A(false);\n"
	"  B(false);\n"
	"  gl_Position = gl_Vertex;\n"
	"}"
	;

static const char indirect_complex2_text[] =
	"#version 120\n"
	"int A(bool);\n"
	"int B(bool from_a) { if (!from_a) return A(true); return 0; }\n"
	;

struct test_vector {
	const char *name;
	const char *description;
	const char *shader_source[4];
};

static const struct test_vector all_tests[] = {
	{
		"simple",
		"Trivial test of recursion.  main calls A, and A calls A.\n",
		{ simple_text, NULL }
	},
	{
		"unreachable",
		"Shader contains a function A that calls itself, but A is\n"
		"trivially unreachable from main.\n",
		{ unreachable_text, NULL }
	},
	{
		"unreachable-constant-folding",
		"Shader contains a function A that calls itself, but A is\n"
		"unreachable from main if a constant folding is performed\n"
		"before the check for recursion.\n",
		{ unreachable_opt_text, NULL }
	},
	{
		"indirect",
		"Trivial test of indirect recursion.  main calls A, A calls\n"
		"B, and B calls A.\n",
		{ indirect_text, NULL }
	},
	{
		"indirect-separate",
		"Trivial test of indirect recursion.  main calls A, A calls\n"
		"B, and B calls A.  A and B are in separate compilation\n"
		"units.\n",
		{ indirect_sep1_text, indirect_sep2_text, NULL }
	},
	{
		"indirect-complex",
		"Two functions A and B are statically mutually recursive,\n"
		"but the parameters passed to the functions ensure that no\n"
		"recursion actually occurs.  This is still an error.\n",
		{ indirect_complex_text, NULL }
	},
	{
		"indirect-complex-separate",
		"Two functions A and B are statically mutually recursive,\n"
		"but the parameters passed to the functions ensure that no\n"
		"recursion actually occurs.  This is still an error.  A and\n"
		"B are in separate compilation units.\n",
		{ indirect_complex1_text, indirect_complex2_text, NULL }
	},
};

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

bool
do_named_test(const char *name)
{
	bool pass = true;
	unsigned i;
	unsigned j;

	for (i = 0; i < ARRAY_SIZE(all_tests); i++) {
		GLint ok;
		GLuint prog;
		GLint size;

		if (name != NULL && strcmp(name, all_tests[i].name) != 0)
			continue;

		printf("Starting test \"%s\":\n", all_tests[i].name);

		prog = glCreateProgram();

		for (j = 0; all_tests[i].shader_source[j] != NULL; j++) {
			GLuint vs;

			vs = glCreateShader(GL_VERTEX_SHADER);
			glShaderSource(vs, 1,
					    (const GLchar **)
					    & all_tests[i].shader_source[j],
					    NULL);
			glCompileShader(vs);

			/* Some drivers return a size of 1 for an empty log.
			 * This is the size of a log that contains only a
			 * terminating NUL character.
			 */
			printf("Compilation info log for shader %u:\n", j);
			glGetShaderiv(vs, GL_INFO_LOG_LENGTH, &size);
			if (size > 1) {
				GLchar *info = malloc(size);

				glGetShaderInfoLog(vs, size, NULL, info);
				printf("%s\n", info);
				free(info);
			} else {
				printf("<empty log>\n\n");
			}

			glAttachShader(prog, vs);
			glDeleteShader(vs);
		}

		glLinkProgram(prog);

		/* Some drivers return a size of 1 for an empty log.  This is
		 * the size of a log that contains only a terminating NUL
		 * character.
		 */
		printf("Link info log:\n");
		glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &size);
		if (size > 1) {
			GLchar *info = malloc(size);
			glGetProgramInfoLog(prog, size, NULL, info);
			printf("%s\n", info);
			free(info);
		} else {
			printf("<empty log>\n\n");
		}

		glGetProgramiv(prog, GL_LINK_STATUS, &ok);
		if (ok) {
			fprintf(stderr,
				"Shader with recursion compiled and linked, "
				"but it should have failed.\n");
			pass = false;
		}
		printf("Done with test \"%s\".\n\n", all_tests[i].name);

		glDeleteProgram(prog);

		if (name != NULL)
			break;
	}

	return pass;
}

void
piglit_init(int argc, char **argv)
{
	bool pass = true;
	const char *glsl_version_string;

	piglit_require_vertex_shader();

	glsl_version_string = (const char *)
		glGetString(GL_SHADING_LANGUAGE_VERSION);
	if (strtod(glsl_version_string, NULL) < 1.2) {
		printf("Requires GLSL 1.20 (have version `%s')\n",
		       glsl_version_string);
		piglit_report_result(PIGLIT_SKIP);
	}

	if (argc == 1) {
		pass = do_named_test(NULL);
	} else {
		int i;
		for (i = 1; i < argc; i++) {
			pass = do_named_test(argv[i]) && pass;
		}
	}

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
