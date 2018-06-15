/*
 * Copyright (c) 2018 Intel Corporation
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

/**
 * \file xfb-varyings.c
 *
 * Ref: https://bugs.freedesktop.org/show_bug.cgi?id=106907
 *
 * We test that querying transform feedback varying information via
 * glGetProgramiv works correctly after restoring a program binary.
 */

#include "piglit-util-gl.h"
#include "gpb-common.h"
#include <stdlib.h>

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 30;
	config.window_visual = PIGLIT_GL_VISUAL_RGB;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

static bool
get_programiv(GLuint prog, GLenum param, int expected)
{
	int val;
	glGetProgramiv(prog, param, &val);
	if (val != expected)
		fprintf(stderr, "error: got %d, expected %d\n", expected, val);
	return val == expected;
}

void
piglit_init(int argc, char **argv)
{
	GLsizei bin_length;
	GLenum bin_format;
	void *binary;
	int num_formats = 0;
	bool pass = true;
	GLuint prog;

	char name[256];
	GLenum type;
	GLsizei length;
	GLsizei size;

	static const char *varyings[] = { "xfb1", "xfb2", NULL };
	static const char vs_source[] =
		"varying vec4 xfb1;\n"
		"varying vec4 xfb2;\n"
		"void main()\n"
		"{\n"
		"    gl_Position = gl_Vertex;\n"
		"    xfb1 = vec4(1.0);\n"
		"    xfb2 = vec4(0.0);\n"
		"}\n";

	piglit_require_extension("GL_ARB_get_program_binary");

	glGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS, &num_formats);
	if (num_formats == 0)
		piglit_report_result(PIGLIT_SKIP);

	prog = piglit_build_simple_program_unlinked(vs_source, NULL);

	glTransformFeedbackVaryings(prog, 2, varyings, GL_SEPARATE_ATTRIBS);

	glLinkProgram(prog);
	if (!piglit_link_check_status(prog))
		piglit_report_result(PIGLIT_FAIL);

	if (!gpb_save_program(prog, &binary, &bin_length, &bin_format)) {
		fprintf(stderr,
			"failed to save program with GetProgramBinary\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	/* Delete program and create empty one. */
	glDeleteProgram(prog);
	prog = glCreateProgram();

	/* Restore original program. */
	if (!gpb_restore_program(prog, binary, bin_length, bin_format)) {
		free(binary);
		fprintf(stderr, "failed to restore binary program\n");
		piglit_report_result(PIGLIT_FAIL);
	}
	free(binary);

	/* Query XFB varying information. */
	if (!get_programiv(prog, GL_TRANSFORM_FEEDBACK_BUFFER_MODE,
			   GL_SEPARATE_ATTRIBS))
		piglit_report_result(PIGLIT_FAIL);

	if (!get_programiv(prog, GL_TRANSFORM_FEEDBACK_VARYINGS,
			   ARRAY_SIZE(varyings) - 1))
		piglit_report_result(PIGLIT_FAIL);

	/* Check that names of varyings match. */
	for (unsigned i = 0; i < 2; i++) {
		glGetTransformFeedbackVarying(prog, i, sizeof(name), &length,
					      &size, &type, name);
		if (strcmp(varyings[i], name) != 0) {
			fprintf(stderr, "expected %s, got %s\n",
				varyings[i], name);
			piglit_report_result(PIGLIT_FAIL);
		}
	}

	glDeleteProgram(prog);

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}
