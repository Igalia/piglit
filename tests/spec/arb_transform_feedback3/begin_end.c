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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "piglit-util-gl.h"
#include "xfb3_common.h"

/**
 * @file begin_end.c 
 *
 * This tests for a bug in the gallium state tracker which asserted with
 * state_tracker/st_cb_xformfb.c:194: st_transform_feedback_get_draw_target: Assertion `0' failed.
 * This was being hit by ogl conform as well.
 */

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 32;
	config.supports_gl_core_version = 32;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

void
piglit_init(int argc, char **argv)
{
	bool pass = true;
	GLuint prog;
	GLint max_attrib_n;
	GLuint buffer;
	const char * outputVaryings[] = {"gl_Position"};
	piglit_require_extension("GL_ARB_transform_feedback3");

	glGetIntegerv(GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS,
		&max_attrib_n);
	if (!max_attrib_n) {
		printf("Maximum number of separete attributes is zero\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	prog = piglit_build_simple_program_multiple_shaders(
			    GL_VERTEX_SHADER, vs_pass_thru_text, 0);
	glTransformFeedbackVaryings(prog, 1, &outputVaryings[0], GL_INTERLEAVED_ATTRIBS);
	glLinkProgram(prog);
	glUseProgram(prog);

	glGenBuffers(1, &buffer);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, buffer);
	glBeginTransformFeedback(GL_POINTS);
	glEndTransformFeedback();

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result
piglit_display(void)
{
	/* Should never be reached */
	return PIGLIT_FAIL;
}
