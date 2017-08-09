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

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 32;
	config.supports_gl_core_version = 32;
	config.khr_no_error_support = PIGLIT_HAS_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

/**
 * Verify that glBeginQueryIndexed emits correct error when an invalid index is
 * used.
 *
 * From the ARB_transform_feedback_overflow_query spec:
 *     An INVALID_VALUE error is generated if <target> is SAMPLES_PASSED ...
 *     TIME_ELAPSED, or TRANSFORM_FEEDBACK_OVERFLOW_ARB, and <index> is not
 *     zero.
 */
static enum piglit_result
test_begin_index_non_zero(void *test_data)
{
	enum piglit_result pass = PIGLIT_PASS;
	GLuint query;

	glGenQueries(1, &query);
	glBeginQueryIndexed(GL_TRANSFORM_FEEDBACK_OVERFLOW_ARB, 1, query);

	if (!piglit_check_gl_error(GL_INVALID_VALUE))
		pass = PIGLIT_FAIL;

	glDeleteQueries(1, &query);

	return pass;
}

/**
 * Verify that glBeginQueryIndexed emits correct error when an invalid index is
 * used.
 *
 * From the ARB_transform_feedback_overflow_query spec:
 *     An INVALID_VALUE error is generated if <target> is PRIMITIVES_GENERATED,
 *     TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, or
 *     TRANSFORM_FEEDBACK_STREAM_OVERFLOW_ARB, and <index> is not in the range
 *     zero to the value of MAX_VERTEX_STREAMS minus one.
 */
static enum piglit_result
test_begin_index_invalid(void *test_data)
{
	enum piglit_result pass = PIGLIT_PASS;
	GLint max_stream;
	GLuint query;

	glGetIntegerv(GL_MAX_VERTEX_STREAMS, &max_stream);
	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		printf("failed to resolve the maximum number of streams\n");
		pass = PIGLIT_FAIL;
		goto err_del_query;
	}

	glGenQueries(1, &query);
	glBeginQueryIndexed(GL_TRANSFORM_FEEDBACK_STREAM_OVERFLOW_ARB, max_stream, query);
	if (!piglit_check_gl_error(GL_INVALID_VALUE))
		pass = PIGLIT_FAIL;

err_del_query:
	glDeleteQueries(1, &query);

	return pass;
}

/**
 * Verify that glEndQueryIndexed emits correct error when an invalid index is
 * used.
 *
 * From the ARB_transform_feedback_overflow_query spec:
 *     An INVALID_VALUE error is generated if <target> is SAMPLES_PASSED, ...
 *     TIME_ELAPSED, or TRANSFORM_FEEDBACK_OVERFLOW_ARB, and <index> is not
 *     zero.
 */
static enum piglit_result
test_end_index_non_zero(void *test_data)
{
	enum piglit_result pass = PIGLIT_PASS;

	glEndQueryIndexed(GL_TRANSFORM_FEEDBACK_OVERFLOW_ARB, 1);

	if (!piglit_check_gl_error(GL_INVALID_VALUE))
		pass = PIGLIT_FAIL;

	return pass;
}

/**
 * Verify that glEndQueryIndexed emits correct error when an invalid index is
 * used.
 *
 * From the ARB_transform_feedback_overflow_query spec:
 *     An INVALID_VALUE error is generated if <target> is PRIMITIVES_GENERATED,
 *     TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, or
 *     TRANSFORM_FEEDBACK_STREAM_OVERFLOW_ARB, and <index> is not in the range
 *     zero to the value of MAX_VERTEX_STREAMS minus one.
 */
static enum piglit_result
test_end_index_invalid(void *test_data)
{
	enum piglit_result pass = PIGLIT_PASS;
	GLint max_stream;

	glGetIntegerv(GL_MAX_VERTEX_STREAMS, &max_stream);
	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		printf("failed to resolve the maximum number of streams\n");
		pass = PIGLIT_FAIL;
		goto end;
	}

	glEndQueryIndexed(GL_TRANSFORM_FEEDBACK_STREAM_OVERFLOW_ARB, max_stream);
	if (!piglit_check_gl_error(GL_INVALID_VALUE))
		pass = PIGLIT_FAIL;

end:
	return pass;
}

/**
 * Verify that glGetQueryIndexediv emits correct error when an invalid index is
 * used.
 *
 * From the ARB_transform_feedback_overflow_query spec:
 *     An INVALID_VALUE error is generated if <target> is ..., or
 *     TRANSFORM_FEEDBACK_OVERFLOW_ARB, and <index> is not zero.
 */
static enum piglit_result
test_get_query_index_non_zero(void *test_data)
{
	enum piglit_result pass = PIGLIT_PASS;
	GLint query;

	glGetQueryIndexediv(GL_TRANSFORM_FEEDBACK_OVERFLOW_ARB, 1,
			    GL_CURRENT_QUERY, &query);
	if (!piglit_check_gl_error(GL_INVALID_VALUE))
		pass = PIGLIT_FAIL;

	return pass;
}

/**
 * Verify that glGetQueryIndexediv emits correct error when an invalid index is
 * used.
 *
 * From the ARB_transform_feedback_overflow_query spec:
 *     An INVALID_VALUE error is generated if <target> is ..., or
 *     TRANSFORM_FEEDBACK_STREAM_OVERFLOW_ARB, and <index> is not in the range
 *     zero to the value of MAX_VERTEX_STREAMS minus one.
 */
static enum piglit_result
test_get_query_index_invalid(void *test_data)
{
	enum piglit_result pass = PIGLIT_PASS;
	GLint max_stream;
	GLint query;

	glGetIntegerv(GL_MAX_VERTEX_STREAMS, &max_stream);
	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		printf("failed to resolve the maximum number of streams\n");
		pass = PIGLIT_FAIL;
		goto end;
	}

	glGetQueryIndexediv(GL_TRANSFORM_FEEDBACK_STREAM_OVERFLOW_ARB,
			    max_stream, GL_CURRENT_QUERY, &query);
	if (!piglit_check_gl_error(GL_INVALID_VALUE))
		pass = PIGLIT_FAIL;

end:
	return pass;
}

const struct piglit_subtest overflow_query_subtests[] = {
	{
		"arb_transform_feedback_overflow_query-begin_index_non_zero",
		"arb_transform_feedback_overflow_query-begin_index_non_zero",
		test_begin_index_non_zero,
	},
	{
		"arb_transform_feedback_overflow_query-begin_index_invalid",
		"arb_transform_feedback_overflow_query-begin_index_invalid",
		test_begin_index_invalid,
	},
	{
		"arb_transform_feedback_overflow_query-end_index_non_zero",
		"arb_transform_feedback_overflow_query-end_index_non_zero",
		test_end_index_non_zero,
	},
	{
		"arb_transform_feedback_overflow_query-end_index_invalid",
		"arb_transform_feedback_overflow_query-end_index_invalid",
		test_end_index_invalid,
	},
	{
		"arb_transform_feedback_overflow_query-get_query_index_non_zero",
		"arb_transform_feedback_overflow_query-get_query_index_non_zero",
		test_get_query_index_non_zero,
	},
	{
		"arb_transform_feedback_overflow_query-get_query_index_invalid",
		"arb_transform_feedback_overflow_query-get_query_index_invalid",
		test_get_query_index_invalid,
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

	piglit_require_extension("GL_ARB_gpu_shader5");
	piglit_require_extension("GL_ARB_transform_feedback3");
	piglit_require_extension("GL_ARB_transform_feedback_overflow_query");

	/* Strip common piglit args. */
	piglit_strip_arg(&argc, argv, "-fbo");
	piglit_strip_arg(&argc, argv, "-auto");
	piglit_parse_subtest_args(&argc, argv, subtests, &selected_subtests,
				  &num_selected_subtests);

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
