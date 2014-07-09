/*
 * Copyright © 2014 Intel Corporation
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

/** \file transform-feedback-vertex-id.c
 *
 * This test verifies that we get expected values of 'gl_VertexID' captured
 * using transform feedback.
 *
 * Test creates a vertex shader which captures the value of 'gl_VertexID'
 * in an output variable. Then it verifies that data captured by transform
 * feedback is as expected.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 32;
	config.supports_gl_core_version = 32;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;
PIGLIT_GL_TEST_CONFIG_END

static const char *vstext =
	"#version 150\n"
	"in vec4 vertex;\n"
	"out int vertex_id;\n"
	"void main()\n"
	"{\n"
	"  gl_Position = vertex;\n"
        "  vertex_id = gl_VertexID;\n"
	"}\n";

static const char *fstext =
	"#version 150\n"
	"out vec4 color;"
	"void main()\n"
	"{\n"
	"  color = vec4(1.0);\n"
	"}\n";

static const GLfloat vertices[] = {
	-0.6f, -0.2f, 0.0f, 1.0f,
	-0.6f,  0.2f, 0.0f, 1.0f,
	-0.4f, -0.4f, 0.0f, 1.0f,
	-0.4f,  0.4f, 0.0f, 1.0f,
	 0.0f, -0.6f, 0.0f, 1.0f,
	 0.0f,  0.6f, 0.0f, 1.0f,
	 0.4f, -0.4f, 0.0f, 1.0f,
	 0.4f,  0.4f, 0.0f, 1.0f,
	 0.6f, -0.2f, 0.0f, 1.0f,
	 0.6f,  0.2f, 0.0f, 1.0f,
	 0.0f,  0.0f, 0.0f, 1.0f};

static const GLuint indices_0[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
static const GLuint indices_1[] = {2, 3, 4, 1, 5, 8, 0, 9, 6, 10, 7};

/**
 * When below set of varyings are captured from the vertex shader
 * above, the output should be a sequence of integers defined in this
 * array.
 */
static const int expected_0[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
static const int expected_1[] = {2, 3, 4, 1, 5, 8, 0, 9, 6, 10, 7};
static const int expected_2[] = {4, 5, 6, 7};

static const char *varyings[] = {"vertex_id"};

bool
setup_xfb_and_compare(const GLuint* indices, int first,
		      int count, const int *expected) {
	int i, num_output_ints;
	GLuint xfb_buf, indexBuffer;
	const GLint *readback;
	bool result = true;
	num_output_ints = count;

	/* Gen indices array buffer if required */
        glGenBuffers(1, &indexBuffer);
	if (indices != NULL) {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(GLuint),
			     indices, GL_STATIC_DRAW);
	} else {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	/* Setup transform feedback buffers */
	glGenBuffers(1, &xfb_buf);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, xfb_buf);
	glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER,
		     num_output_ints * sizeof(GLint), NULL,
		     GL_STATIC_READ);

	printf("%s with Starting index = %d, Number of indices = %d\n",
	       indices ? "glDrawElements()" : "glDrawArrays()", first, count);

	/* Do drawing */
	glBeginTransformFeedback(GL_POINTS);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPointSize(5.0);

	if (indices != NULL) {
		glDrawElements(GL_POINTS, count, GL_UNSIGNED_INT, NULL);
	} else {
		glDrawArrays(GL_POINTS, first, count);
	}
	glEndTransformFeedback();
	piglit_present_results();

	/* Check if the correct data was written into the transform feedback
	 * buffers.
	 */
	glBindBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, 0, xfb_buf, 0,
			  num_output_ints * sizeof(GLint));
	readback = glMapBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, 0,
				    num_output_ints * sizeof(GLint),
				    GL_MAP_READ_BIT);
	result = piglit_check_gl_error(GL_NO_ERROR) && result;

	for (i = 0; i < num_output_ints; i++) {
		if (readback[i] != expected[i]) {
			printf("Incorrect data for '%s' output %d."
			       "  Expected %d, got %d.\n", varyings[0],
			       i, expected[i], readback[i]);
			result = false;
		}
	}
	glUnmapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER);
	glDeleteBuffers(1, &xfb_buf);
	return result;
}

void
piglit_init(int argc, char **argv)
{
	int vertex_pos;
	GLuint vao, vertexBuffer;
	GLuint prog = piglit_build_simple_program_unlinked_multiple_shaders(
		GL_VERTEX_SHADER, vstext,
		GL_FRAGMENT_SHADER, fstext,
		0, NULL);

	glTransformFeedbackVaryings(prog, 1, varyings, GL_SEPARATE_ATTRIBS);

	glLinkProgram(prog);
	if (!piglit_link_check_status(prog)) {
		glDeleteProgram(prog);
		piglit_report_result(PIGLIT_FAIL);
	}
	glUseProgram(prog);

	/* Gen VAO */
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	/* Gen vertex array buffer */
	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, ARRAY_SIZE(vertices) * sizeof(GLfloat),
		     &vertices, GL_STATIC_DRAW);

	/* Enable vertexAttribPointer */
	vertex_pos = glGetAttribLocation(prog, "vertex");
        glEnableVertexAttribArray(vertex_pos);
        glVertexAttribPointer(vertex_pos, 4, GL_FLOAT, GL_FALSE, 0, 0);
}

enum piglit_result
piglit_display(void)
{
	bool pass = true;
	int first = 0;
	int count = ARRAY_SIZE(vertices) / 4;
	/* Draw with different 'first' and 'count' values, capture the
	 * transform feedback data and compare with expected values.
	 */
	pass = setup_xfb_and_compare(NULL, first,
				     count,
				     expected_0) && pass;

	pass = setup_xfb_and_compare(NULL, 4 /*first*/,
				     4 /*count*/,
				     expected_2) && pass;

	pass = setup_xfb_and_compare(indices_0, first,
				     count,
				     expected_0) && pass;

	pass = setup_xfb_and_compare(indices_1, first,
				     count,
				     expected_1) && pass;

	return (pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
