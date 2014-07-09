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

/** \file
 *
 * Verify that when transform feedback is applied to a program
 * containing both a geometry shader and a vertex shader, the size and
 * type of the data captured (as well as the data itself) are
 * determined by the geometry shader and not the vertex shader.
 *
 * This test creates a geometry and a vertex shader which both output
 * the following variables:
 *
 * - foo
 * - gl_ClipDistance
 *
 * but declare them to have different types and array sizes, and
 * output different data to them.
 *
 * Then it verifies that:
 *
 * - glGetTransformFeedbackVarying() returns information based on the
 *   types and array sizes declared in the geometry shader.
 *
 * - The data captured by transform feedback is consistent with the
 *   declarations in the geometry shader.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 32;
	config.supports_gl_core_version = 32;
PIGLIT_GL_TEST_CONFIG_END

/**
 * This vertex shader should generate the following outputs (assuming
 * that 2 vertices are processed):
 *
 * foo        gl_ClipDistance
 * [0]  [1]   [0]  [1]  [2]
 * 1001 1003  1007 1008 1009
 * 1011 1013  1017 1018 1019
 */
static const char *vstext =
	"#version 150\n"
	"out VsOuts {\n"
	"  float foo[2];\n"
	"};\n"
	"out gl_PerVertex {\n"
	"  float gl_ClipDistance[3];\n"
	"};\n"
	"void main()\n"
	"{\n"
	"  float x = float(gl_VertexID * 10 + 1000);\n"
	"  foo[0] = x + 1.0;\n"
	"  foo[1] = x + 3.0;\n"
	"  gl_ClipDistance[0] = x + 7.0;\n"
	"  gl_ClipDistance[1] = x + 8.0;\n"
	"  gl_ClipDistance[2] = x + 9.0;\n"
	"}\n";

/**
 * When coupled with the vertex shader above, this geometry shader
 * should generate the following outputs:
 *
 * foo                         gl_ClipDistance
 * [0]      [1]      [2]       [0] [1] [2] [3]
 * (1, 2)   (3, 4)   (5, 6)    7   8   9   10
 * (11, 12) (13, 14) (15, 16)  17  18  19  20
 */
static const char *gstext =
	"#version 150\n"
	"layout(points) in;\n"
	"layout(points, max_vertices = 1) out;\n"
	"in VsOuts {\n"
	"  float foo[2];\n"
	"} vs_outs[1];\n"
	"in gl_PerVertex {\n"
	"  float gl_ClipDistance[3];\n"
	"} gl_in[];\n"
	"out vec2 foo[3];\n"
	"out float gl_ClipDistance[4];\n"
	"void main()\n"
	"{\n"
	"  foo[0] = vs_outs[0].foo[0] - 1000.0 + vec2(0.0, 1.0);\n"
	"  foo[1] = vs_outs[0].foo[1] - 1000.0 + vec2(0.0, 1.0);\n"
	"  foo[2] = vs_outs[0].foo[1] - 1000.0 + vec2(2.0, 3.0);\n"
	"  gl_ClipDistance[0] = gl_in[0].gl_ClipDistance[0] - 1000.0;\n"
	"  gl_ClipDistance[1] = gl_in[0].gl_ClipDistance[1] - 1000.0;\n"
	"  gl_ClipDistance[2] = gl_in[0].gl_ClipDistance[2] - 1000.0;\n"
	"  gl_ClipDistance[3] = gl_in[0].gl_ClipDistance[2] - 1000.0 + 1.0;\n"
	"  EmitVertex();\n"
	"}\n";

/**
 * When this set of varyings is captured from the geometry shader
 * above, the output should be a sequence of floating point numbers
 * counting from 1 to 20.
 */
static const char *varyings[] = { "foo", "gl_ClipDistance" };

#define EXPECTED_NUM_OUTPUT_FLOATS 20


static bool
check_varying(GLuint prog, GLuint index, const char *expected_name, GLsizei expected_size, GLenum expected_type)
{
	GLsizei length, size;
	GLenum type;
	char name[100];
	bool pass = true;
	glGetTransformFeedbackVarying(prog, index, ARRAY_SIZE(name), &length,
				      &size, &type, name);
	name[ARRAY_SIZE(name) - 1] = '\0';
	if (strcmp(name, expected_name) != 0) {
		printf("Varying %d: expected name '%s', got '%s'\n", index,
		       expected_name, name);
		pass = false;
	}
	if (expected_size != size) {
		printf("varying %d: expected size %d, got %d\n", index,
		       expected_size, size);
		pass = false;
	}
	if (expected_type != type) {
		printf("varying %d: expected type %d (%s), got %d (%s)\n",
		       index,
		       expected_type, piglit_get_gl_enum_name(expected_type),
		       type, piglit_get_gl_enum_name(type));
		pass = false;
	}
	return pass;
}


void
piglit_init(int argc, char **argv)
{
	GLuint prog, vao, xfb_buf;
	const GLfloat *readback;
	int i;
	bool pass = true;

	prog = piglit_build_simple_program_unlinked_multiple_shaders(
		GL_VERTEX_SHADER, vstext,
		GL_GEOMETRY_SHADER, gstext,
		0, NULL);
	glTransformFeedbackVaryings(prog, ARRAY_SIZE(varyings), varyings,
				    GL_INTERLEAVED_ATTRIBS);
	glLinkProgram(prog);
	if (!piglit_link_check_status(prog)) {
		glDeleteProgram(prog);
		piglit_report_result(PIGLIT_FAIL);
	}
	glUseProgram(prog);

	/* Check that glGetTransformFeedbackVarying() returns the
	 * correct values.
	 */
	pass = check_varying(prog, 0, "foo", 3, GL_FLOAT_VEC2) && pass;
	pass = check_varying(prog, 1, "gl_ClipDistance", 4, GL_FLOAT) && pass;

	/* Setup GL state necessary for drawing */
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(1, &xfb_buf);
	glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, xfb_buf);
	glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER,
		     EXPECTED_NUM_OUTPUT_FLOATS * sizeof(GLfloat), NULL,
		     GL_STREAM_READ);
	glBindBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, 0, xfb_buf, 0,
			  EXPECTED_NUM_OUTPUT_FLOATS * sizeof(GLfloat));
	glEnable(GL_RASTERIZER_DISCARD);

	/* Do drawing */
	glBeginTransformFeedback(GL_POINTS);
	glDrawArrays(GL_POINTS, 0, 2);
	glEndTransformFeedback();

	/* Check that the correct data was written into the transform
	 * feedback buffer.
	 */
	readback = glMapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, GL_READ_ONLY);
	for (i = 0; i < EXPECTED_NUM_OUTPUT_FLOATS; i++) {
		GLfloat expected = i + 1;
		if (readback[i] != expected) {
			printf("Incorrect data for output %d."
			       "  Expected %f, got %f.\n", i, expected,
			       readback[i]);
			pass = false;
		}
	}
	glUnmapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER);

	/* Check for errors */
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result
piglit_display(void)
{
	/* Should never be reached */
	return PIGLIT_FAIL;
}
