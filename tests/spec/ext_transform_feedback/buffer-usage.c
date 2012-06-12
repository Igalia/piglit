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
 * \file buffer-usage.c
 *
 * Tests for a bug in the i965 driver where transform feedback would
 * segfault on certain buffer object allocation 'usage' arguments.
 */

#include "piglit-util.h"

#define NUM_POINTS 10002
#define SHIFT_COUNT 64

PIGLIT_GL_TEST_MAIN(
    100 /*window_width*/,
    100 /*window_height*/,
    GLUT_DOUBLE | GLUT_RGB | GLUT_ALPHA)

static const char *vstext =
	"#version 130\n"
	"out float tf;\n"
	"\n"
	"void main()\n"
	"{\n"
	"  gl_Position = vec4(0.0);\n"
	"  tf = 1.0;\n"
	"}\n";

static void
initialize_shader_and_xfb()
{
	GLuint prog, vs;
	const char *varying = "tf";

	piglit_require_gl_version(30);
	piglit_require_GLSL_version(130);
	piglit_require_transform_feedback();
	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vstext);
	prog = piglit_CreateProgram();
	piglit_AttachShader(prog, vs);
	piglit_TransformFeedbackVaryings(prog, 1, &varying,
					 GL_INTERLEAVED_ATTRIBS);
	piglit_LinkProgram(prog);
	if (!piglit_link_check_status(prog)) {
		piglit_DeleteProgram(prog);
		piglit_report_result(PIGLIT_FAIL);
	}
	piglit_UseProgram(prog);
}

static void
draw()
{
	static const GLenum bo_modes[] = {
		GL_STREAM_DRAW,
		GL_STREAM_READ,
		GL_STREAM_COPY,
		GL_STATIC_DRAW,
		GL_STATIC_READ,
		GL_STATIC_COPY,
		GL_DYNAMIC_DRAW,
		GL_DYNAMIC_READ,
		GL_DYNAMIC_COPY,
	};
	int i;
	bool pass = true;

	glEnable(GL_RASTERIZER_DISCARD);

	for (i = 0; i < ARRAY_SIZE(bo_modes); i++) {
		static GLuint xfb_buf;
		float *readback;

		/* Make a new TFB output buffer with the chosen usage
		 * mode.  Note, from ARB_vertex_buffer_object:
		 *
		 *     "The specified usage value does not constrain
		 *      the actual usage pattern of the data store."
		 */
		glGenBuffers(1, &xfb_buf);
		glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, xfb_buf);
		glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER,
			     sizeof(float), NULL, bo_modes[i]);
		piglit_BindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, xfb_buf);

		/* Do the draw call.  Here's where we segfaulted before. */
		piglit_BeginTransformFeedback(GL_POINTS);
		glDrawArrays(GL_POINTS, 0, 1);
		piglit_EndTransformFeedback();

		/* Test the output, just to be sure. */
		readback = glMapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER,
				       GL_READ_ONLY);

		if (readback[0] != 1.0) {
			fprintf(stderr, "Readback found %f, expected 1.0\n",
				readback[0]);
			pass = false;
		}
		glUnmapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER);

		glDeleteBuffers(1, &xfb_buf);
	}

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

void
piglit_init(int argc, char **argv)
{
	initialize_shader_and_xfb();
	draw();
}

enum piglit_result piglit_display(void)
{
	/* Test should finish before we reach here. */
	return PIGLIT_FAIL;
}
