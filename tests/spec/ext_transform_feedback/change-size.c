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
 * \file change-size.c
 *
 * Confirm that transform feedback properly handles a change in the
 * size of a transform feedback buffer after it is bound but before it
 * is used.
 *
 * In particular, this test verifies the following behaviours, from
 * the GL 4.3 spec, section 6.1.1 ("Binding Buffer Objects to Indexed
 * Targets"):
 *
 *   BindBufferBase binds the entire buffer, even when the size of the buffer
 *   is changed after the binding is established. It is equivalent to calling
 *   BindBufferRange with offset zero, while size is determined by the size of
 *   the bound buffer at the time the binding is used.
 *
 *   Regardless of the size specified with BindBufferRange, or indirectly with
 *   BindBufferBase, the GL will never read or write beyond the end of a bound
 *   buffer. In some cases this constraint may result in visibly different
 *   behavior when a buffer overflow would otherwise result, such as described
 *   for transform feedback operations in section 13.2.2.
 *
 * This test verifies that the expected number of primitives are
 * written after a change to the size of the transform feedback
 * buffer, using both a GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN query
 * and by looking at the contents of the buffer itself.  We run
 * transform feedback in GL_TRIANGLES mode and use a buffer size that
 * is not a multiple of 3, so that we can look at the last element in
 * the transform feedback buffer and verify that transform feedback
 * didn't overwrite it.
 *
 * The test performs the following operations:
 *
 * 1. Create a transform feedback buffer using glBufferData().
 *
 * 2. Bind the buffer for transform feedback using either
 *    glBindBufferBase, glBindBufferRange, or glBindBufferOffsetEXT
 *    (if supported).
 *
 * 3. Change the size of the bound buffer using glBufferData().  A
 *    non-null data pointer is passed to glBufferData() to store a
 *    known pattern in the buffer, so that in step 6 we'll be able to
 *    determine which parts of the buffer were overwritten.
 *
 * 4. Draw some triangles, feeding back a single float from each
 *    vertex.
 *
 * 5. Verify, using a GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN query,
 *    that the expected number of primitives were written to the
 *    buffer.
 *
 * 6. Verify, using glMapBuffer, that the expected data was written to
 *    the buffer.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

/**
 * Maximum buffer size--used for declaraing static arrays.  Measured
 * in multiples of sizeof(GLfloat).
 */
#define MAX_BUFFER_SIZE_FLOATS 10

static GLuint prog;
static GLuint xfb_buf;
static GLuint query;

const struct test_case
{
	/**
	 * Name of the test case.  NULL is used as a sentinel to mark
	 * the end of the list of test cases.
	 */
	const char *name;

	/**
	 * Size that the buffer should have before binding.  Measured
	 * in multiples of sizeof(GLfloat).
	 */
	unsigned initial_size;

	/**
	 * Offset to pass to glBindBufferRange/glBindBufferOffsetEXT,
	 * or zero if glBindBufferBase should be used.  Measured in
	 * multiples of sizeof(GLfloat).
	 */
	unsigned bind_offset;

	/**
	 * Size to pass to glBindBufferRange, or zero if
	 * glBindBufferOffsetEXT/glBindBufferBase should be used.
	 * Measured in multiples of sizeof(GLfloat).
	 */
	unsigned bind_size;

	/**
	 * Size of the buffer that should be passed to
	 * glBindBufferData after the buffer is bound.  Measured in
	 * multiples of sizeof(GLfloat).
	 */
	unsigned new_size;

	/**
	 * Number of triangles to draw.
	 */
	unsigned num_draw_triangles;

	/**
	 * Number of primitives that are expected to be written to the
	 * buffer.
	 */
	unsigned num_feedback_triangles;
} test_cases[] = {
	/* name            initial  bind    bind  new   num tris:
	 *                 size     offset  size  size  draw  feedback */
	{ "base-shrink",   7,       0,      0,    4,    2,    1 },
	{ "base-grow",     4,       0,      0,    7,    2,    2 },
	{ "offset-shrink", 10,      3,      0,    7,    2,    1 },
	{ "offset-grow",   7,       3,      0,    10,   2,    2 },
	{ "range-shrink",  10,      3,      7,    7,    2,    1 },
	{ "range-grow",    7,       3,      4,    10,   2,    1 },
	{ NULL,            0,       0,      0,    0,    0,    0 }
};

const struct test_case *selected_test;

/**
 * Vertex shader, which simply copies its input attribute to its
 * output varying, adding 100 in the process.
 */
static const char *vstext =
	"#version 120\n"
	"attribute float input_value;\n"
	"varying float output_value;\n"
	"\n"
	"void main()\n"
	"{\n"
	"  gl_Position = vec4(0.0);\n"
	"  output_value = 100.0 + input_value;\n"
	"}\n";

static void
print_usage_and_exit(const char *prog_name)
{
	unsigned i;
	printf("Usage: %s <test_case>\n"
	       "  where <test_case> is one of the following:\n", prog_name);
	for (i = 0; test_cases[i].name != NULL; i++)
		printf("    %s\n", test_cases[i].name);
	exit(1);
}

static const struct test_case *
interpret_test_case_arg(const char *arg)
{
	unsigned i;
	for (i = 0; test_cases[i].name != NULL; i++) {
		if (strcmp(test_cases[i].name, arg) == 0)
			return &test_cases[i];
	}
	return NULL;
}

void
piglit_init(int argc, char **argv)
{
	const char *varying_name = "output_value";

	/* Parse args */
	if (argc != 2)
		print_usage_and_exit(argv[0]);
	selected_test = interpret_test_case_arg(argv[1]);
	if (selected_test == NULL)
		print_usage_and_exit(argv[0]);

	/* Make sure required GL features are present */
	piglit_require_GLSL_version(120);
	piglit_require_transform_feedback();
	if (selected_test->bind_offset != 0 && selected_test->bind_size == 0) {
		/* Test requires glBindBufferOffsetEXT, which is in
		 * EXT_transform_feedback, but was never adopted into
		 * OpenGL.
		 */
		piglit_require_extension("GL_EXT_transform_feedback");
	}

	/* Create program and buffer */
	prog = piglit_build_simple_program_unlinked(vstext, NULL);
	glTransformFeedbackVaryings(prog, 1, &varying_name,
				    GL_INTERLEAVED_ATTRIBS);
	glLinkProgram(prog);
	if (!piglit_link_check_status(prog))
		piglit_report_result(PIGLIT_FAIL);
	glGenBuffers(1, &xfb_buf);
	glGenQueries(1, &query);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);
}

enum piglit_result
piglit_display(void)
{
	GLint input_index = glGetAttribLocation(prog, "input_value");
	GLfloat canary_data[MAX_BUFFER_SIZE_FLOATS];
	GLfloat input_data[MAX_BUFFER_SIZE_FLOATS];
	GLfloat expected_data[MAX_BUFFER_SIZE_FLOATS];
	GLfloat *output_data;
	GLuint query_result;
	GLboolean pass = GL_TRUE;
	unsigned i;

	glUseProgram(prog);

	/* Create a transform feedback buffer. */
	glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, xfb_buf);
	glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER,
		     selected_test->initial_size * sizeof(GLfloat), NULL,
		     GL_STREAM_READ);

	/* Bind the buffer for transform feedback. */
	if (selected_test->bind_size != 0) {
		glBindBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, 0, xfb_buf,
				  selected_test->bind_offset * sizeof(GLfloat),
				  selected_test->bind_size * sizeof(GLfloat));
	} else if (selected_test->bind_offset != 0) {
		glBindBufferOffsetEXT(GL_TRANSFORM_FEEDBACK_BUFFER, 0,
				      xfb_buf,
				      selected_test->bind_offset
				      * sizeof(GLfloat));
	} else {
		glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, xfb_buf);
	}

	/* Change the size of the bound buffer. */
	for (i = 0; i < MAX_BUFFER_SIZE_FLOATS; i++)
		canary_data[i] = -1;
	glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER,
		     selected_test->new_size * sizeof(GLfloat), canary_data,
		     GL_STREAM_READ);

	/* Draw some triangles. */
	for (i = 0; i < MAX_BUFFER_SIZE_FLOATS; i++)
		input_data[i] = i + 1;
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glVertexAttribPointer(input_index, 1, GL_FLOAT, GL_FALSE,
			      sizeof(GLfloat), input_data);
	glEnableVertexAttribArray(input_index);
	glBeginTransformFeedback(GL_TRIANGLES);
	glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, query);
	glDrawArrays(GL_TRIANGLES, 0, selected_test->num_draw_triangles * 3);
	glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);
	glEndTransformFeedback();

	/* Verify that the expected number of primitives were
	 * written.
	 */
	glGetQueryObjectuiv(query, GL_QUERY_RESULT, &query_result);
	printf("PRIMITIVES_WRITTEN: expected=%u, actual=%u\n",
	       selected_test->num_feedback_triangles, query_result);
	if (query_result != selected_test->num_feedback_triangles)
		pass = GL_FALSE;

	/* Verify that the expected data was written. */
	for (i = 0; i < selected_test->new_size; i++) {
		if (i >= selected_test->bind_offset &&
		    i < (3 * selected_test->num_feedback_triangles
			 + selected_test->bind_offset)) {
			expected_data[i] = 100.0
				+ input_data[i - selected_test->bind_offset];
		} else {
			expected_data[i] = canary_data[i];
		}
	}
	output_data = glMapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, GL_READ_ONLY);
	for (i = 0; i < selected_test->new_size; ++i) {
		printf("data[%u]: expected=%f, actual=%f\n", i,
		       expected_data[i], output_data[i]);
		if (expected_data[i] != output_data[i])
			pass = GL_FALSE;
	}
	glUnmapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
