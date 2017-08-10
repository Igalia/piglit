/*
 * Copyright © 2015 Red Hat
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
 * \file double-gettransformfeedbackvarying.c
 *
 * Verify that transform feedback on double varyings returns
 * the correct types.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

        config.supports_gl_core_version = 32;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGB;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

#define MAX_EXPECTED_OUTPUT_COMPONENTS 8

static const char *vstext = {
	"#version 150\n"
	"#extension GL_ARB_gpu_shader_fp64 : require\n"
	"in vec4 vertex;"
	"out %s tfout;"
	"void main() {"
	"  gl_Position = vertex;"
	"  tfout = %s(0.2lf);"
	"}"
};

static struct get_tests {
	GLenum type;
	const char *glsltype;
	int size;
} tests[] = {
	{ GL_DOUBLE, "double", 1 },
	{ GL_DOUBLE_VEC2 , "dvec2", 1 },
	{ GL_DOUBLE_VEC3 , "dvec3", 1 },
	{ GL_DOUBLE_VEC4 , "dvec4", 1 },
	{ GL_DOUBLE_MAT2 , "dmat2", 1 },
	{ GL_DOUBLE_MAT2x3 , "dmat2x3", 1 },
	{ GL_DOUBLE_MAT2x4 , "dmat2x4", 1 },
	{ GL_DOUBLE_MAT3 , "dmat3", 1 },
	{ GL_DOUBLE_MAT3x2 , "dmat3x2", 1 },
	{ GL_DOUBLE_MAT3x4 , "dmat3x4", 1 },
	{ GL_DOUBLE_MAT4 , "dmat4", 1 },
	{ GL_DOUBLE_MAT4x2 , "dmat4x2", 1 },
	{ GL_DOUBLE_MAT4x3 , "dmat4x3", 1 },
};
	
static bool size_and_type_ok = true;
static const char *varyings[] = { "tfout" };

void run_test(struct get_tests *test)
{
	GLsizei size;
	GLenum type;
	GLuint vs;
	GLuint prog;
	char vstest[1024];

	snprintf(vstest, 1024, vstext, test->glsltype, test->glsltype);

	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vstest);
	prog = glCreateProgram();
	glAttachShader(prog, vs);
	glBindAttribLocation(prog, 0, "vertex");
	glTransformFeedbackVaryings(prog, ARRAY_SIZE(varyings),
				    varyings,
				    GL_INTERLEAVED_ATTRIBS_EXT);
	glLinkProgram(prog);
	if (!piglit_link_check_status(prog)) {
		glDeleteProgram(prog);
		piglit_report_result(PIGLIT_FAIL);
	}

	glGetTransformFeedbackVarying(prog, 0, 0, NULL, &size,
				      &type, NULL);
	if (size != test->size) {
		printf("For %s, size %d vs %d\n", test->glsltype, size, test->size);
		size_and_type_ok = false;
	}
	if (type != test->type) {
		printf("For %s, size %d vs %d\n", test->glsltype, type, test->type);
		size_and_type_ok = false;
	}
	glDeleteProgram(prog);
}

void
piglit_init(int argc, char **argv)
{
	int i;

	/* Set up test */
	piglit_require_GLSL_version(150);
	piglit_require_transform_feedback();
	piglit_require_extension("GL_ARB_gpu_shader_fp64");

	for (i = 0; i < ARRAY_SIZE(tests); i++)
		run_test(&tests[i]);

	if (size_and_type_ok == false)
		piglit_report_result(PIGLIT_FAIL);
	piglit_report_result(PIGLIT_PASS);
}

enum piglit_result piglit_display(void)
{
	return PIGLIT_PASS;
}
