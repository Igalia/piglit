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

/**
 * \file tf-no-position.c
 *
 * Verify that we can render without specifying a gl_Position, by
 * using EXT_transform_feedback to get results.
 */

#include "piglit-util-gl.h"

#define BUFFER_SIZE 4

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.supports_gl_core_version = 31;

	config.window_width = 10;
	config.window_height = 10;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

static const char *vs_source =
	"#version 140\n"
	"in uint i;\n"
	"flat out uint o;\n"
	"\n"
	"void main()\n"
	"{\n"
	"	o = i;\n"
	"}\n";

enum piglit_result piglit_display(void)
{
	return PIGLIT_FAIL; /* UNREACHED */
}

void piglit_init(int argc, char **argv)
{
	GLuint vs;
	GLuint *readback;
	GLuint buffer[BUFFER_SIZE];
	GLuint expected[BUFFER_SIZE];
	int i;
	bool pass = true;
	GLint input_index;
	GLuint prog;
	GLuint xfb_buf, vbo;
	GLuint verts[4] = { 0, 1, 2, 3 };
	const char *varying = "o";

	piglit_require_GLSL_version(140);
	piglit_require_gl_version(30);
	piglit_require_transform_feedback();
	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vs_source);

	prog = glCreateProgram();
	glAttachShader(prog, vs);
	glTransformFeedbackVaryings(prog, 1, &varying, GL_INTERLEAVED_ATTRIBS);
	glLinkProgram(prog);
	if (!piglit_link_check_status(prog))
		piglit_report_result(PIGLIT_FAIL);
	glGenBuffers(1, &xfb_buf);
	if (!piglit_check_gl_error(0))
		piglit_report_result(PIGLIT_FAIL);

	input_index = glGetAttribLocation(prog, "i");

	glUseProgram(prog);

	if (piglit_get_gl_version() >= 31) {
                GLuint vao;
                glGenVertexArrays(1, &vao);
                glBindVertexArray(vao);
        }

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER,
		     4 * sizeof(GLuint), verts, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribIPointer(input_index, 1, GL_UNSIGNED_INT,
			       sizeof(GLuint), 0);
	glEnableVertexAttribArray(input_index);
	pass = piglit_check_gl_error(0) && pass;

	glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, xfb_buf);
	memset(buffer, 0xd0, sizeof(buffer));
	glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, sizeof(buffer), buffer,
		     GL_STREAM_READ);
	glBindBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, 0, xfb_buf,
			  0,
			  sizeof(buffer));
	glBeginTransformFeedback(GL_POINTS);
	glDrawArrays(GL_POINTS, 0, ARRAY_SIZE(verts));
	glEndTransformFeedback();

	readback = glMapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, GL_READ_ONLY);

	/* Check output */
	for (i = 0; i < BUFFER_SIZE; ++i) {
		if (verts[i] != readback[i]) {
			printf("readback[%u]: %u, expected: %u\n", i,
			       readback[i], expected[i]);
			pass = false;
		}
	}

	/* Note that rasterization occurred, but the results were
	 * undefined due to gl_Position not being written.  We do want
	 * to have rasterization occur (as opposed to just transform
	 * feedback) just to make sure the GPU didn't wedge or
	 * anything.
	 */

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
