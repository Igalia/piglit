/*
 * Copyright (c) 2016 Intel Corporation
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

/**
 * @file basic.c
 *
 * This test verifies the basic functionality of
 * ARB_transform_feedback_overflow_query: that it detects overflow for specific
 * streams, and on any stream too if requested. It does so by causing overflow
 * first on stream 0, and then on stream 1.
 */

#define BUFFER_OFFSET(i) ((void *)((char *)NULL + i))

const struct piglit_subtest overflow_query_subtests[];

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 32;
	config.supports_gl_core_version = 32;
	config.subtests = overflow_query_subtests;

PIGLIT_GL_TEST_CONFIG_END

static const char vs_pass_thru_text[] =
	"#version 150\n"
	"void main() {\n"
	"  gl_Position = vec4(gl_VertexID);\n"
	"}\n";

static const char gs_overflow_single[] =
	"#version 150\n"
	"layout(points) in;\n"
	"layout(points, max_vertices = 1) out;\n"
	"out vec2 stream0_out;\n"
	"void main() {\n"
	"  gl_Position = gl_in[0].gl_Position;\n"

	"  stream0_out = vec2(gl_Position[0], gl_Position[1]);\n"
	"  EmitVertex();\n"
	"  EndPrimitive();\n"
	"}";

static const char gs_overflow_multi[] =
	"#version 150\n"
	"#extension GL_ARB_gpu_shader5 : enable\n"
	"layout(points) in;\n"
	"layout(points, max_vertices = 4) out;\n"
	"layout(stream = 0) out vec2 stream0_out;\n"
	"layout(stream = 1) out vec2 stream1_out;\n"
	"void main() {\n"
	"  gl_Position = gl_in[0].gl_Position;\n"

	"  stream0_out = vec2(gl_Position[0], gl_Position[1]);\n"
	"  EmitStreamVertex(0);\n"
	"  EndStreamPrimitive(0);\n"

	"  stream1_out = vec2(gl_Position[0], gl_Position[1]) + 20;\n"
	"  EmitStreamVertex(1);\n"
	"  EndStreamPrimitive(1);\n"
	"}";

static const char *varyings_single[] = { "stream0_out" };

static const char *varyings_multi[] = {
	"stream0_out", "gl_NextBuffer", "stream1_out",
};

#define STREAMS 2

static const char *program_in_use = NULL;

static bool
build_and_use_program(const char *gs_text, const char **gs_varyings,
		      int array_size)
{
	GLuint prog;

	if (program_in_use == gs_text)
		return true;

	prog = piglit_build_simple_program_multiple_shaders(
				GL_VERTEX_SHADER, vs_pass_thru_text,
				GL_GEOMETRY_SHADER, gs_text, 0);

	glTransformFeedbackVaryings(prog, array_size, gs_varyings,
				GL_INTERLEAVED_ATTRIBS);

	glLinkProgram(prog);
	if (!piglit_link_check_status(prog))
		return false;
	if (!piglit_check_gl_error(GL_NO_ERROR))
		return false;

	glUseProgram(prog);

	program_in_use = gs_text;

	return true;
}

static enum piglit_result
simple_query(GLuint query, bool expected)
{
	enum piglit_result pass = PIGLIT_PASS;
	GLuint value;

	glGetQueryObjectuiv(query, GL_QUERY_RESULT, &value);
	if (value != expected) {
		printf("Wrong value for query. expected: %d, value: %d\n",
		       expected, value);
		pass = PIGLIT_FAIL;
	}

	return pass;
}

static enum piglit_result
conditional_render(GLuint query, bool inverted, bool expected)
{
	enum piglit_result pass = PIGLIT_PASS;
	bool rendered, render_expected;
	GLuint generated_q, value;
	GLuint wait = inverted ? GL_QUERY_WAIT_INVERTED : GL_QUERY_WAIT;

	glGenQueries(1, &generated_q);

	glBeginQuery(GL_PRIMITIVES_GENERATED, generated_q);
	glBeginTransformFeedback(GL_POINTS);
	glBeginConditionalRender(query, wait);
	glDrawArrays(GL_POINTS, 0, 1);
	glEndConditionalRender();
	glEndTransformFeedback();
	glEndQuery(GL_PRIMITIVES_GENERATED);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		return PIGLIT_FAIL;

	glGetQueryObjectuiv(generated_q, GL_QUERY_RESULT, &value);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		return PIGLIT_FAIL;

	rendered = value == 1;
	render_expected = inverted ? !expected : expected;
	if (rendered != render_expected) {
		printf("Error: expect to render? %d, rendered? %d\n",
		       render_expected, rendered);
		pass = PIGLIT_FAIL;
	}

	return pass;
}

static enum piglit_result
overflow_buffer_object(GLuint query, bool expected)
{
	enum piglit_result pass = PIGLIT_PASS;
	GLuint queryBuffer;
	const GLuint *readback;

	// Create a buffer object for the query result
	glGenBuffers(1, &queryBuffer);
	glBindBuffer(GL_QUERY_BUFFER, queryBuffer);
	glBufferData(GL_QUERY_BUFFER, sizeof(GLuint),
		     NULL, GL_DYNAMIC_COPY);

	// Get query results to buffer object
	glBindBuffer(GL_QUERY_BUFFER, queryBuffer);
	glGetQueryObjectuiv(query, GL_QUERY_RESULT, BUFFER_OFFSET(0));

	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		pass = PIGLIT_FAIL;
		goto err_del_buffer;
	}

	readback = glMapBuffer(GL_QUERY_BUFFER, GL_READ_ONLY);
	if (readback[0] != expected) {
		printf("Query buffer object error. Expected: %u, read: %u\n",
		       expected, readback[0]);
		pass = PIGLIT_FAIL;
	}

	if (!piglit_check_gl_error(GL_NO_ERROR))
		pass = PIGLIT_FAIL;

	glUnmapBuffer(GL_QUERY_BUFFER);
	glBindBuffer(GL_QUERY_BUFFER, 0);
err_del_buffer:
	glDeleteBuffers(1, &queryBuffer);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		pass = PIGLIT_FAIL;

	return pass;
}

static bool
check_multistream_extensions(void)
{
	if (!piglit_is_extension_supported("GL_ARB_gpu_shader5")) {
		piglit_loge("context does not support GL_ARB_gpu_shader5; "
			    "skipping test");
		return false;
	}

	if (!piglit_is_extension_supported("GL_ARB_transform_feedback3")) {
		piglit_loge("context does not support "
			    "GL_ARB_transform_feedback3; skipping test");
		return false;
	}

	return true;
}

static enum piglit_result
run_subtest(int n_streams, int array_sizes[], int stream, GLuint query_type,
		  bool inverted, bool expected, const char *test_type)
{
	enum piglit_result pass = PIGLIT_PASS;
	GLuint query;
	GLuint *xfb;
	GLuint vao;
	int varyings_size;
	const char *gs_text;
	const char **gs_varyings;

	if (!strcmp(test_type, "buffer_object") &&
	    !piglit_is_extension_supported("GL_ARB_query_buffer_object")) {
		piglit_loge("context does not support "
			    "GL_ARB_query_buffer_object; skipping test");
	    return PIGLIT_SKIP;
	}

	if (n_streams > 1) {
		if (!check_multistream_extensions())
			return PIGLIT_SKIP;

		gs_text = gs_overflow_multi;
		gs_varyings = varyings_multi;
		varyings_size = ARRAY_SIZE(varyings_multi);
	} else {
		gs_text = gs_overflow_single;
		gs_varyings = varyings_single;
		varyings_size = ARRAY_SIZE(varyings_single);
	}

	if (!build_and_use_program(gs_text, gs_varyings, varyings_size)) {
		printf("Could not build and link program.\n");
		return PIGLIT_FAIL;
	}

	xfb = malloc(sizeof(*xfb) * n_streams);

	/* Set up the transform feedback buffers. */
	glGenBuffers(n_streams, xfb);
	for (int i = 0; i < n_streams; i++) {
		glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, i, xfb[i]);
		glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER,
			     array_sizes[i] * sizeof(float), NULL,
			     GL_STREAM_READ);
	}

	/* Test only records using transform feedback. */
	glEnable(GL_RASTERIZER_DISCARD);

	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		pass = PIGLIT_FAIL;
		goto err_del_buffers;
	}

	glGenQueries(1, &query);
	glBeginQueryIndexed(query_type, stream, query);

	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		pass = PIGLIT_FAIL;
		goto err_del_queries;
	}

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		pass = PIGLIT_FAIL;
		goto err_del_vao;
	}

	/* Draw and record */
	glBeginTransformFeedback(GL_POINTS);
	glDrawArrays(GL_POINTS, 0, 3);
	glEndQueryIndexed(query_type, stream);
	glEndTransformFeedback();

	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		pass = PIGLIT_FAIL;
		goto err_del_vao;
	}

	if (!strcmp(test_type, "simple_query")) {
		pass = simple_query(query, expected);
	} else if (!strcmp(test_type, "conditional_render")) {
		pass = conditional_render(query, inverted, expected);
	} else if (!strcmp(test_type, "buffer_object")) {
		pass = overflow_buffer_object(query, expected);
	} else {
		printf("Unkown test.\n");
		pass = PIGLIT_FAIL;
	}

err_del_vao:
	glDeleteVertexArrays(1, &vao);
err_del_queries:
	glDeleteQueries(1, &query);
err_del_buffers:
	glDeleteBuffers(n_streams, xfb);
	free(xfb);

	return pass;
}

/**
 * inverted = false, overflow = true.
 */
static enum piglit_result
test_overflow_single(void *test_data)
{
	GLuint query_type = GL_TRANSFORM_FEEDBACK_OVERFLOW_ARB;
	int array_sizes[] = { 5 };

	return run_subtest(1, array_sizes, 0, query_type, false, true, test_data);
}

/**
 * inverted = false, expected overflow: false
 */
static enum piglit_result
test_no_overflow_single(void *test_data)
{
	GLuint query_type = GL_TRANSFORM_FEEDBACK_OVERFLOW_ARB;
	int array_sizes[] = { 6 };

	return run_subtest(1, array_sizes, 0, query_type, false, false, test_data);
}

/**
 * Overflow on stream 0.
 * Query for overflow on stream 0.
 * inverted = false, expected overflow: true.
 */
static enum piglit_result
test_overflow_stream_0(void *test_data)
{
	GLuint query_type = GL_TRANSFORM_FEEDBACK_STREAM_OVERFLOW_ARB;
	int array_sizes[] = { 5, 6 };

	return run_subtest(2, array_sizes, 0, query_type, false, true, test_data);
}

/**
 * Overflow on stream 1.
 * Query for overflow on stream 0.
 * inverted = true, expected overflow: false.
 */
static enum piglit_result
test_overflow_stream_1(void *test_data)
{
	GLuint query_type = GL_TRANSFORM_FEEDBACK_STREAM_OVERFLOW_ARB;
	int array_sizes[] = { 6, 5 };

	return run_subtest(2, array_sizes, 0, query_type, true, false, test_data);
}

/**
 * Overflow on stream 1.
 * Query for overflow on stream 1.
 * inverted = true, expected overflow: true.
 */
static enum piglit_result
test_overflow_stream_2(void *test_data)
{
	GLuint query_type = GL_TRANSFORM_FEEDBACK_STREAM_OVERFLOW_ARB;
	int array_sizes[] = { 6, 5 };

	return run_subtest(2, array_sizes, 1, query_type, true, true, test_data);
}

/**
 * Overflow on stream 1.
 * Query for overflow on any stream.
 * inverted = false, expected overflow: true.
 */
static enum piglit_result
test_overflow_stream_any(void *test_data)
{
	GLuint query_type = GL_TRANSFORM_FEEDBACK_OVERFLOW_ARB;
	int array_sizes[] = { 6, 5 };

	return run_subtest(2, array_sizes, 0, query_type, false, true, test_data);
}

/**
 * Overflow on stream 1.
 * Query for overflow on any stream.
 * inverted = true, expected overflow: true.
 */
static enum piglit_result
test_overflow_stream_any_inverted(void *test_data)
{
	GLuint query_type = GL_TRANSFORM_FEEDBACK_OVERFLOW_ARB;
	int array_sizes[] = { 6, 5 };

	return run_subtest(2, array_sizes, 0, query_type, true, true, test_data);
}

/**
 * No overflow.
 * Query for overflow on any stream.
 * inverted = false, expected overflow: false.
 */
static enum piglit_result
test_no_overflow_stream_any(void *test_data)
{
	GLuint query_type = GL_TRANSFORM_FEEDBACK_OVERFLOW_ARB;
	int array_sizes[] = { 6, 6 };

	return run_subtest(2, array_sizes, 0, query_type, false, false,
			   test_data);
}


const struct piglit_subtest overflow_query_subtests[] = {
	{
		"arb_transform_feedback_overflow_query-simple_query_single",
		"arb_transform_feedback_overflow_query-simple_query_single",
		test_overflow_single,
		"simple_query"
	},
	{
		"arb_transform_feedback_overflow_query-simple_query_no_overflow_single",
		"arb_transform_feedback_overflow_query-simple_query_no_overflow_single",
		test_no_overflow_single,
		"simple_query"
	},
	{
		"arb_transform_feedback_overflow_query-conditional_render_single",
		"arb_transform_feedback_overflow_query-conditional_render_single",
		test_overflow_single,
		"conditional_render"
	},
	{
		"arb_transform_feedback_overflow_query-conditional_render_no_overflow_single",
		"arb_transform_feedback_overflow_query-conditional_render_no_overflow_single",
		test_no_overflow_single,
		"conditional_render"
	},
	{
		"arb_transform_feedback_overflow_query-buffer_object_single",
		"arb_transform_feedback_overflow_query-buffer_object_single",
		test_overflow_single,
		"buffer_object"
	},
	{
		"arb_transform_feedback_overflow_query-buffer_object_no_overflow_single",
		"arb_transform_feedback_overflow_query-buffer_object_no_overflow_single",
		test_no_overflow_single,
		"buffer_object"
	},
	{
		"arb_transform_feedback_overflow_query-simple_query_0",
		"arb_transform_feedback_overflow_query-simple_query_0",
		test_overflow_stream_0,
		"simple_query"
	},
	{
		"arb_transform_feedback_overflow_query-simple_query_1",
		"arb_transform_feedback_overflow_query-simple_query_1",
		test_overflow_stream_1,
		"simple_query"
	},
	{
		"arb_transform_feedback_overflow_query-simple_query_2",
		"arb_transform_feedback_overflow_query-simple_query_2",
		test_overflow_stream_2,
		"simple_query"
	},
	{
		"arb_transform_feedback_overflow_query-simple_query_any",
		"arb_transform_feedback_overflow_query-simple_query_any",
		test_overflow_stream_any,
		"simple_query"
	},
	{
		"arb_transform_feedback_overflow_query-simple_query_no_overflow",
		"arb_transform_feedback_overflow_query-simple_query_no_overflow",
		test_no_overflow_stream_any,
		"simple_query"
	},
	{
		"arb_transform_feedback_overflow_query-conditional_render_0",
		"arb_transform_feedback_overflow_query-conditional_render_0",
		test_overflow_stream_0,
		"conditional_render"
	},
	{
		"arb_transform_feedback_overflow_query-conditional_render_1",
		"arb_transform_feedback_overflow_query-conditional_render_1",
		test_overflow_stream_1,
		"conditional_render"
	},
	{
		"arb_transform_feedback_overflow_query-conditional_render_2",
		"arb_transform_feedback_overflow_query-conditional_render_2",
		test_overflow_stream_2,
		"conditional_render"
	},
	{
		"arb_transform_feedback_overflow_query-conditional_render_any",
		"arb_transform_feedback_overflow_query-conditional_render_any",
		test_overflow_stream_any,
		"conditional_render"
	},
	{
		"arb_transform_feedback_overflow_query-conditional_render_any_inverted",
		"arb_transform_feedback_overflow_query-conditional_render_any_inverted",
		test_overflow_stream_any_inverted,
		"conditional_render"
	},
	{
		"arb_transform_feedback_overflow_query-conditional_render_no_overflow",
		"arb_transform_feedback_overflow_query-conditional_render_no_overflow",
		test_no_overflow_stream_any,
		"conditional_render"
	},
	{
		"arb_transform_feedback_overflow_query-buffer_object_0",
		"arb_transform_feedback_overflow_query-buffer_object_0",
		test_overflow_stream_0,
		"buffer_object"
	},
	{
		"arb_transform_feedback_overflow_query-buffer_object_1",
		"arb_transform_feedback_overflow_query-buffer_object_1",
		test_overflow_stream_1,
		"buffer_object"
	},
	{
		"arb_transform_feedback_overflow_query-buffer_object_2",
		"arb_transform_feedback_overflow_query-buffer_object_2",
		test_overflow_stream_2,
		"buffer_object"
	},
	{
		"arb_transform_feedback_overflow_query-buffer_object_any",
		"arb_transform_feedback_overflow_query-buffer_object_any",
		test_overflow_stream_any,
		"buffer_object"
	},
	{
		"arb_transform_feedback_overflow_query-buffer_object_no_overflow",
		"arb_transform_feedback_overflow_query-buffer_object_no_overflow",
		test_no_overflow_stream_any,
		"buffer_object"
	},
	{0},
};

void
piglit_init(int argc, char **argv)
{
	enum piglit_result result = PIGLIT_SKIP;
	const char **selected_subtests = NULL;
	size_t num_selected_subtests = 0;
	const struct piglit_subtest *subtests = overflow_query_subtests;

	piglit_require_extension("GL_ARB_transform_feedback_overflow_query");

	/* Strip common piglit args. */
	piglit_strip_arg(&argc, argv, "-fbo");
	piglit_strip_arg(&argc, argv, "-auto");
	num_selected_subtests = piglit_get_selected_tests(&selected_subtests);

	if (argc > 1) {
		fprintf(stderr, "usage error\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	result = piglit_run_selected_subtests(subtests, selected_subtests,
					      num_selected_subtests, result);
	piglit_report_result(result);
}

enum piglit_result
piglit_display(void)
{
	/* Should never be reached */
	return PIGLIT_FAIL;
}
