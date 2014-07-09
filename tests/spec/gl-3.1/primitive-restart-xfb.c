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
 * \file primitive-restart-xfb.c
 *
 * Test interactions between primitive restart and transform feedback
 * primitive counting behaviour.
 *
 * This test makes a single glDrawElements(GL_TRIANGLE_STRIP, 9, ...)
 * draw call, where the index buffer specifies 4 normal vertices, the
 * primitive restart index, and then 4 more normal vertices.  It
 * verifies that the implementation correctly counts this as drawing 4
 * triangles (rather than 7, which would be the behaviour of if
 * primitive restart were not in use).
 *
 * The test can be run in three ways (selectable by a command line
 * argument):
 *
 * - "generated" verifies that the GL_PRIMITIVES_GENERATED query
 *   counts the primitives correctly.
 *
 * - "written" verifies that the
 *   GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN query counts the
 *   primitives correctly.
 *
 * - "flush" verifies that if these 4 triangles are followed by a
 *   glFlush() and then more primitives further drawing, transform
 *   feedback for the latter primitives is placed at the correct
 *   location in the transform feedback buffer.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_core_version = 31;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;
PIGLIT_GL_TEST_CONFIG_END


enum test_mode_enum {
	TEST_MODE_GENERATED,
	TEST_MODE_WRITTEN,
	TEST_MODE_FLUSH,
};


static const char vs_text[] =
	"#version 130\n"
	"in int x_in;\n"
	"flat out int x_out;\n"
	"void main()\n"
	"{\n"
	"  gl_Position = vec4(0.0);\n"
	"  x_out = x_in;\n"
	"}\n";


static const GLchar *varyings[] = { "x_out" };


/**
 * Indices used for the test.
 */
static const GLubyte indices[] = {
	/* For the main draw call */
	0, 1, 2, 3, 0xff, 4, 5, 6, 7,

	/* After the glFlush() call (when in TEST_MODE_FLUSH) */
	8, 9, 10, 11
};


static const GLint vertex_attrs[] = { 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3 };


/**
 * Expected transform feedback result when in TEST_MODE_FLUSH:
 * - 2 triangles with attribute 1 and 2 triangles with attribute value 2 (from
 *   the first draw call)
 * - 2 triangles with attribute 3 (from the second draw call, after the flush)
 */
static const GLint expected_xfb_result[] =
	{ 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3 };


static void
print_usage_and_exit(const char *prog_name)
{
	printf("Usage: %s <subtest>\n"
	       "  where <subtest> is one of the following:\n"
	       "    generated\n"
	       "    written\n"
	       "    flush\n", prog_name);
	piglit_report_result(PIGLIT_FAIL);
}


static bool
check_query_result(GLuint query, GLuint expected)
{
	GLuint result;

	glGetQueryObjectuiv(query, GL_QUERY_RESULT, &result);
	if (result != expected) {
		printf("Query result: %u, expected: %u\n", result, expected);
		return false;
	}
	return true;
}


static bool
check_xfb_result()
{
	bool pass = true;
	int i;
	const GLint *readback =
		glMapBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, 0,
				 sizeof(expected_xfb_result), GL_MAP_READ_BIT);
	for (i = 0; i < ARRAY_SIZE(expected_xfb_result); i++) {
		if (readback[i] != expected_xfb_result[i]) {
			printf("XFB[%i] == %i, expected %i\n", i, readback[i],
			       expected_xfb_result[i]);
			pass = false;
		}
	}
	glUnmapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER);
	return pass;
}


void
piglit_init(int argc, char **argv)
{
	GLuint buf;
	void *initial_data;
	bool pass = true;
	enum test_mode_enum test_mode;
	GLuint prog;
	GLuint vao, vbo_attrs, vbo_indices;
	GLuint query;

	if (argc != 2)
		print_usage_and_exit(argv[0]);
	if (strcmp(argv[1], "generated") == 0)
		test_mode = TEST_MODE_GENERATED;
	else if (strcmp(argv[1], "written") == 0)
		test_mode = TEST_MODE_WRITTEN;
	else if (strcmp(argv[1], "flush") == 0)
		test_mode = TEST_MODE_FLUSH;
	else
		print_usage_and_exit(argv[0]);

	prog = piglit_build_simple_program_unlinked(vs_text, NULL);
	glTransformFeedbackVaryings(prog, 1, varyings, GL_INTERLEAVED_ATTRIBS);
	glBindAttribLocation(prog, 0, "x_in");
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

	/* Set up VAO/VBO */
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(1, &vbo_attrs);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_attrs);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_attrs), vertex_attrs,
		     GL_STREAM_DRAW);
	glVertexAttribIPointer(0, 1, GL_INT, 0, NULL);
	glEnableVertexAttribArray(0);
	glGenBuffers(1, &vbo_indices);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_indices);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
		     GL_STREAM_DRAW);

	/* Misc setup */
	glEnable(GL_RASTERIZER_DISCARD);
	glEnable(GL_PRIMITIVE_RESTART);
	glPrimitiveRestartIndex(0xff);
	glGenQueries(1, &query);

	switch (test_mode) {
	case TEST_MODE_GENERATED:
		glBeginQuery(GL_PRIMITIVES_GENERATED, query);
		glBeginTransformFeedback(GL_TRIANGLES);
		glDrawElements(GL_TRIANGLE_STRIP, 9, GL_UNSIGNED_BYTE, NULL);
		glEndTransformFeedback();
		glEndQuery(GL_PRIMITIVES_GENERATED);
		pass = check_query_result(query, 4);
		break;
	case TEST_MODE_WRITTEN:
		glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, query);
		glBeginTransformFeedback(GL_TRIANGLES);
		glDrawElements(GL_TRIANGLE_STRIP, 9, GL_UNSIGNED_BYTE, NULL);
		glEndTransformFeedback();
		glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);
		pass = check_query_result(query, 4);
		break;
	case TEST_MODE_FLUSH:
		glBeginTransformFeedback(GL_TRIANGLES);
		glDrawElements(GL_TRIANGLE_STRIP, 9, GL_UNSIGNED_BYTE, NULL);
		glFlush();
		glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_BYTE,
			       (void *) (9 * sizeof(GLubyte)));
		glEndTransformFeedback();
		pass = check_xfb_result();
		break;
	}

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}


enum piglit_result
piglit_display(void)
{
	/* Should never be reached */
	return PIGLIT_FAIL;
}
