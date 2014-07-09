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

/**
 * \file nonflat-interal.c
 *
 * Test that non-flat integral vertex shader outputs can be captured
 * with transform feedback.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 30;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;
PIGLIT_GL_TEST_CONFIG_END


static const char vs_text[] =
	"#version 130\n"
	"out int out_int;\n"
	"out ivec2 out_ivec2;\n"
	"out ivec3 out_ivec3;\n"
	"out ivec4 out_ivec4;\n"
	"out uint out_uint;\n"
	"out uvec2 out_uvec2;\n"
	"out uvec3 out_uvec3;\n"
	"out uvec4 out_uvec4;\n"
	"void main()\n"
	"{\n"
	"  gl_Position = vec4(0.0);\n"
	"  out_int = 11;\n"
	"  out_ivec2 = ivec2(21, 22);\n"
	"  out_ivec3 = ivec3(31, 32, 33);\n"
	"  out_ivec4 = ivec4(41, 42, 43, 44);\n"
	"  out_uint = 51u;\n"
	"  out_uvec2 = uvec2(61u, 62u);\n"
	"  out_uvec3 = uvec3(71u, 72u, 73u);\n"
	"  out_uvec4 = uvec4(81u, 82u, 83u, 84u);\n"
	"}\n";


static const GLchar *varyings[] = {
	"out_int", "out_ivec2", "out_ivec3", "out_ivec4",
	"out_uint", "out_uvec2", "out_uvec3", "out_uvec4"
};


static const GLint expected_xfb_result[] = {
	11, 21, 22, 31, 32, 33, 41, 42, 43, 44,
	51, 61, 62, 71, 72, 73, 81, 82, 83, 84
};


void
piglit_init(int argc, char **argv)
{
	bool pass = true;
	const GLint *readback;
	GLuint buf;
	void *initial_data;
	int i;
	GLuint prog = piglit_build_simple_program_unlinked(vs_text, NULL);
	glTransformFeedbackVaryings(prog, ARRAY_SIZE(varyings), varyings,
				    GL_INTERLEAVED_ATTRIBS);
	glLinkProgram(prog);
	if (!piglit_link_check_status(prog) ||
	    !piglit_check_gl_error(GL_NO_ERROR)) {
		piglit_report_result(PIGLIT_FAIL);
	}
	glUseProgram(prog);

	/* Create transform feedback buffer and pre-load it with
	 * garbage.
	 */
	glGenBuffers(1, &buf);
	initial_data = malloc(sizeof(expected_xfb_result));
	memset(initial_data, 0xcc, sizeof(expected_xfb_result));
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, buf);
	glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, sizeof(expected_xfb_result),
		     initial_data, GL_STREAM_READ);
	free(initial_data);

	/* Run the test */
	glEnable(GL_RASTERIZER_DISCARD);
	glBeginTransformFeedback(GL_POINTS);
	glDrawArrays(GL_POINTS, 0, 1);
	glEndTransformFeedback();

	/* Check output */
	readback = glMapBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, 0,
				    sizeof(expected_xfb_result),
				    GL_MAP_READ_BIT);
	for (i = 0; i < ARRAY_SIZE(expected_xfb_result); i++) {
		if (readback[i] != expected_xfb_result[i]) {
			printf("XFB[%i] == %i, expected %i\n", i, readback[i],
			       expected_xfb_result[i]);
			pass = false;
		}
	}
	glUnmapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER);

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}


enum piglit_result
piglit_display(void)
{
	/* Should never be reached */
	return PIGLIT_FAIL;
}
