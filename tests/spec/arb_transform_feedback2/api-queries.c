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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

/**
 * \file api-queries.c
 * Verify a handful of API queries.
 *
 * None of these subtests is large enough to warrant a separate test case.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_RGB;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

static const char vstext[] =
	"varying vec4 x;\n"
	"void main()\n"
	"{\n"
	"    gl_Position = vec4(0);\n"
	"    x = vec4(0);\n"
	"}"
	;

static bool check_bool(const GLenum param, bool expect)
{
	GLboolean value;

	glGetBooleanv(param, &value);

	if (expect != value) {
		fprintf(stderr,
			"%s has incorrect state.\n"
			"Got %s, expected %s.\n",
			piglit_get_gl_enum_name(param),
			value ? "true" : "false",
			expect ? "true" : "false");
		return false;
	}

	return true;
}

static bool check_int(const GLenum param, int expect)
{
	int value;

	glGetIntegerv(param, &value);

	if (expect != value) {
		fprintf(stderr,
			"%s has incorrect state.\n"
			"Got %d, expected %d.\n",
			piglit_get_gl_enum_name(param), value, expect);
		return false;
	}

	return true;
}

void piglit_init(int argc, char **argv)
{
	GLuint buf;
	GLuint id;
	GLuint prog;
	GLuint vs;
	const char *varyings[] = {"x"};
	bool pass = true;

	piglit_require_transform_feedback();
	piglit_require_GLSL();
	piglit_require_extension("GL_ARB_transform_feedback2");

	pass = check_int(GL_TRANSFORM_FEEDBACK_BINDING, 0) && pass;

	/* This is all just the boot-strap work for the test.
	 */
	glGenBuffers(1, &buf);
	glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, buf);
	glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, 1024, NULL, GL_STREAM_READ);

	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vstext);
	prog = glCreateProgram();
	glAttachShader(prog, vs);

	glTransformFeedbackVaryings(prog, 1, varyings,
				    GL_INTERLEAVED_ATTRIBS);
	glLinkProgram(prog);
	if (!piglit_link_check_status(prog)) {
		pass = false;
		goto done;
	}

	glUseProgram(prog);

	glGenTransformFeedbacks(1, &id);

	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, id);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, buf);

	/* Verify the initial state of transform feedback object queires.
	 */
	pass = check_int(GL_TRANSFORM_FEEDBACK_BINDING, id) && pass;

	pass = check_bool(GL_TRANSFORM_FEEDBACK_BUFFER_PAUSED, false) && pass;

	pass = check_bool(GL_TRANSFORM_FEEDBACK_BUFFER_ACTIVE, false) && pass;

	/* Make active and verify.
	 */
	glBeginTransformFeedback(GL_TRIANGLES);
	pass = piglit_check_gl_error(0) && pass;

	pass = check_bool(GL_TRANSFORM_FEEDBACK_BUFFER_ACTIVE, true) && pass;

	/* Pause and verify.
	 */
	glPauseTransformFeedback();
	pass = piglit_check_gl_error(0) && pass;

	pass = check_bool(GL_TRANSFORM_FEEDBACK_BUFFER_PAUSED, true) && pass;

	/* Resume and verify.
	 */
	glResumeTransformFeedback();
	pass = piglit_check_gl_error(0) && pass;

	pass = check_bool(GL_TRANSFORM_FEEDBACK_BUFFER_PAUSED, false) && pass;

	/* End and verify.
	 */
	glEndTransformFeedback();
	pass = piglit_check_gl_error(0) && pass;

	pass = check_bool(GL_TRANSFORM_FEEDBACK_BUFFER_ACTIVE, false) && pass;

	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);

done:
	glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, 0);
	glDeleteBuffers(1, &buf);

	glDeleteTransformFeedbacks(1, &id);

	glUseProgram(0);
	glDeleteShader(vs);
	glDeleteProgram(prog);

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
