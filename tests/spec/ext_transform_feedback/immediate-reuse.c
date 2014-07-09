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
 * \file immediate-reuse.c
 *
 * Verify that if a transform feedback output buffer is immediately
 * re-used as a transform feedback input (changing no GL settings
 * except for buffer bindings), rendering is correct.
 *
 * The test operates by using a shader whose transform feedback
 * outputs are the same as its inputs, except with positions and
 * colors offset by a constant value.  It draws a pair of triangles on
 * the left side of the screen, then cycles the transform feedback
 * output back through as vertex input 15 times; this should result in
 * a stepped gradient being drawn.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 256;
	config.window_height = 16;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

static const char *vstext =
	"attribute vec4 in_position;\n"
	"attribute vec4 in_color;\n"
	"varying vec4 xfb_position;\n"
	"varying vec4 xfb_color;\n"
	"varying vec4 out_color;\n"
	"\n"
	"void main()\n"
	"{\n"
	"  gl_Position = in_position;\n"
	"  out_color = in_color;\n"
	"  xfb_position = in_position + vec4(0.125, 0.0, 0.0, 0.0);\n"
	"  xfb_color = in_color + vec4(0.0625, -0.0625, 0.0625, 0.0);\n"
	"}\n";

static const char *fstext =
	"varying vec4 out_color;\n"
	"\n"
	"void main()\n"
	"{\n"
	"  gl_FragColor = out_color;\n"
	"}\n";

static const char *varyings[] = { "xfb_position", "xfb_color" };

static GLuint bufs[2];
static GLuint prog;

void
piglit_init(int argc, char **argv)
{
	GLuint vs, fs;

	piglit_require_GLSL();
	piglit_require_transform_feedback();

	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vstext);
	fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, fstext);
	prog = glCreateProgram();
	glAttachShader(prog, vs);
	glAttachShader(prog, fs);
	glBindAttribLocation(prog, 0, "in_position");
	glBindAttribLocation(prog, 1, "in_color");
	glTransformFeedbackVaryings(prog, 2, varyings, GL_INTERLEAVED_ATTRIBS);
	glLinkProgram(prog);
	if (!piglit_link_check_status(prog)) {
		glDeleteProgram(prog);
		piglit_report_result(PIGLIT_FAIL);
	}

	glGenBuffers(2, bufs);
}

enum piglit_result
piglit_display(void)
{
	int i;
	GLboolean pass = GL_TRUE;
	static const float initial_vertex_data[6][8] = {
		/* position XYZW            color RGBA */
		{ -1.0,   -1.0, 0.0, 1.0,   0.0, 1.0, 0.0, 1.0 },
		{ -0.875, -1.0, 0.0, 1.0,   0.0, 1.0, 0.0, 1.0 },
		{ -1.0,    1.0, 0.0, 1.0,   0.0, 1.0, 0.0, 1.0 },
		{ -0.875, -1.0, 0.0, 1.0,   0.0, 1.0, 0.0, 1.0 },
		{ -0.875,  1.0, 0.0, 1.0,   0.0, 1.0, 0.0, 1.0 },
		{ -1.0,    1.0, 0.0, 1.0,   0.0, 1.0, 0.0, 1.0 }
	};
	static const float initial_dummy_data[6][8] = {
		/* position XYZW            color RGBA */
		{ -1.0,   -1.0, 0.0, 1.0,   1.0, 0.0, 0.0, 1.0 },
		{ -0.875, -1.0, 0.0, 1.0,   1.0, 0.0, 0.0, 1.0 },
		{ -1.0,    1.0, 0.0, 1.0,   1.0, 0.0, 0.0, 1.0 },
		{ -0.875, -1.0, 0.0, 1.0,   1.0, 0.0, 0.0, 1.0 },
		{ -0.875,  1.0, 0.0, 1.0,   1.0, 0.0, 0.0, 1.0 },
		{ -1.0,    1.0, 0.0, 1.0,   1.0, 0.0, 0.0, 1.0 }
	};

	/* Setup program and initial buffer contents */
	glBindBuffer(GL_ARRAY_BUFFER, bufs[0]);
	glBufferData(GL_ARRAY_BUFFER,
		     sizeof(initial_vertex_data), initial_vertex_data,
		     GL_STREAM_COPY);
	glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, bufs[1]);
	glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER,
		     sizeof(initial_dummy_data), initial_dummy_data,
		     GL_STREAM_COPY);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glUseProgram(prog);
	glClear(GL_COLOR_BUFFER_BIT);

	/* Draw 16 times, swapping transform feedback and vertex data
	 * so that transform feedback output is fed back to vertex
	 * input.
	 */
	for (i = 0; i < 16; ++i) {
		glBindBuffer(GL_ARRAY_BUFFER, bufs[i % 2]);
		glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0,
				 bufs[(i + 1) % 2]);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE,
				      8 * sizeof(float), (void *) 0);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE,
				      8 * sizeof(float),
				      (void *) (4 * sizeof(float)));
		glBeginTransformFeedback(GL_TRIANGLES);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glEndTransformFeedback();
	}

	/* Check that the proper gradient was drawn */
	for (i = 0; i < 16; ++i) {
		float expected_color[3] = {i/16.0, (16 - i)/16.0, i/16.0 };
		pass = piglit_probe_rect_rgb(16 * i, 0, 16, 16,
					     expected_color) && pass;
	}

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
