/*
 * Copyright (c) 2017 Advanced Micro Devices, Inc.
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

/**
 * \file qbo-coherency.c
 * Test coherency of ARB_query_buffer_object results with pre-shader pipeline
 * stages:
 *  1. Indirect draw
 *     Write the qbo result to the 'first' member of the indirect draw
 *     structure, and write the gl_VertexID to a transform feedback buffer.
 *  2. Index buffer fetch
 *     Write the qbo result to the index buffer, and write the gl_VertexID to
 *     a transform feedback buffer.
 *  3. Indirect draw count
 *     Write the qbo result to the 'drawcount' value for an
 *     GL_ARB_indirect_parameters multi-draw, and increment an atomic counter
 *     in the vertex shader.
 *  4. Indirect dispatch
 *     Write the qbo result to the number of groups, and count the groups using
 *     an atomic counter.
 */

#include "common.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 32;
	config.supports_gl_core_version = 32;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE |
		PIGLIT_GL_VISUAL_DEPTH;
PIGLIT_GL_TEST_CONFIG_END

#define BUFFER_OFFSET(i) ((void *)((char *)NULL + i))

#define DRAW_COUNT_CLAMP_MAX 50

static GLuint prog_compute;
static GLuint prog_xfb;
static GLuint prog_vs_atomic;
static GLuint empty_vao;
static GLuint indirect_draw_count_data_bo;

static const char *arg_consumer_mode;
static const char *arg_query_type;

struct consumer_mode {
	const char *name;
	unsigned (*run)(unsigned query);
	const char *extensions[2];
	bool amplify;
	bool clamped;
	unsigned clamp_max;
};

static unsigned
indirect_draw(unsigned query)
{
	static const GLuint indirect_data[] = {
		1, /* count */
		1, /* instanceCount */
		999, /* first */
		0, /* baseInstance */
	};
	unsigned indirect_bo;
	unsigned xfb_bo;

	glGenBuffers(1, &indirect_bo);
	glBindBuffer(GL_QUERY_BUFFER, indirect_bo);
	glBufferData(GL_QUERY_BUFFER, sizeof(indirect_data), indirect_data, GL_STATIC_DRAW);
	glGetQueryObjectuivARB(query, GL_QUERY_RESULT, BUFFER_OFFSET(2 * sizeof(GLuint)));

	glUseProgram(prog_xfb);
	glBindVertexArray(empty_vao);

	glGenBuffers(1, &xfb_bo);
	glBindBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, 0, xfb_bo, 0, sizeof(GLuint));
	glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, sizeof(GLuint), NULL, GL_STREAM_READ);

	glEnable(GL_RASTERIZER_DISCARD);
	glBeginTransformFeedback(GL_POINTS);

	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indirect_bo);
	glDrawArraysIndirect(GL_POINTS, BUFFER_OFFSET(0));

	glEndTransformFeedback();
	glDisable(GL_RASTERIZER_DISCARD);

	unsigned result;
	glGetBufferSubData(GL_TRANSFORM_FEEDBACK_BUFFER, 0, sizeof(result), &result);

	glDeleteBuffers(1, &indirect_bo);
	glDeleteBuffers(1, &xfb_bo);
	piglit_check_gl_error(GL_NO_ERROR);

	return result;
}

static unsigned
index_buffer(unsigned query)
{
	static const GLuint index_data[] = {
		999,
	};
	unsigned index_bo;
	unsigned xfb_bo;

	glGenBuffers(1, &index_bo);
	glBindBuffer(GL_QUERY_BUFFER, index_bo);
	glBufferData(GL_QUERY_BUFFER, sizeof(index_data), index_data, GL_STATIC_DRAW);
	glGetQueryObjectuivARB(query, GL_QUERY_RESULT, BUFFER_OFFSET(0));

	glUseProgram(prog_xfb);
	glBindVertexArray(empty_vao);

	glGenBuffers(1, &xfb_bo);
	glBindBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, 0, xfb_bo, 0, sizeof(GLuint));
	glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, sizeof(GLuint), NULL, GL_STREAM_READ);

	glEnable(GL_RASTERIZER_DISCARD);
	glBeginTransformFeedback(GL_POINTS);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_bo);
	glDrawElements(GL_POINTS, 1, GL_UNSIGNED_INT, BUFFER_OFFSET(0));

	glEndTransformFeedback();
	glDisable(GL_RASTERIZER_DISCARD);

	unsigned result;
	glGetBufferSubData(GL_TRANSFORM_FEEDBACK_BUFFER, 0, sizeof(result), &result);

	glDeleteBuffers(1, &index_bo);
	glDeleteBuffers(1, &xfb_bo);
	piglit_check_gl_error(GL_NO_ERROR);

	return result;
}

static unsigned
indirect_draw_count(unsigned query)
{
	static const unsigned zero = 0;
	static const unsigned count_default = 999;
	unsigned indirect_count_bo;
	unsigned atomic_bo;

	glGenBuffers(1, &indirect_count_bo);
	glBindBuffer(GL_QUERY_BUFFER, indirect_count_bo);
	glBufferData(GL_QUERY_BUFFER, sizeof(count_default), &count_default, GL_STATIC_DRAW);
	glGetQueryObjectuivARB(query, GL_QUERY_RESULT, BUFFER_OFFSET(0));

	glUseProgram(prog_vs_atomic);
	glBindVertexArray(empty_vao);

	glGenBuffers(1, &atomic_bo);
	glBindBufferRange(GL_ATOMIC_COUNTER_BUFFER, 0, atomic_bo, 0, 4);
	glBufferData(GL_ATOMIC_COUNTER_BUFFER, 4, &zero, GL_STATIC_DRAW);

	glEnable(GL_RASTERIZER_DISCARD);

	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indirect_draw_count_data_bo);
	glBindBuffer(GL_PARAMETER_BUFFER_ARB, indirect_count_bo);
	glMultiDrawArraysIndirectCountARB(GL_POINTS, BUFFER_OFFSET(0), 0,
					  DRAW_COUNT_CLAMP_MAX, 0);

	glDisable(GL_RASTERIZER_DISCARD);

	unsigned result;
	glGetBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(result), &result);

	glDeleteBuffers(1, &indirect_count_bo);
	glDeleteBuffers(1, &atomic_bo);
	piglit_check_gl_error(GL_NO_ERROR);

	return result;
}

static unsigned
indirect_dispatch(unsigned query)
{
	static const GLuint indirect_data[] = {
		999, /* num_groups_x */
		1, /* num_groups_y */
		1, /* num_groups_z */
	};
	static const unsigned zero = 0;
	unsigned indirect_bo;
	unsigned atomic_bo;

	glGenBuffers(1, &indirect_bo);
	glBindBuffer(GL_QUERY_BUFFER, indirect_bo);
	glBufferData(GL_QUERY_BUFFER, sizeof(indirect_data), indirect_data, GL_STATIC_DRAW);
	glGetQueryObjectuivARB(query, GL_QUERY_RESULT, BUFFER_OFFSET(0));

	glUseProgram(prog_compute);

	glGenBuffers(1, &atomic_bo);
	glBindBufferRange(GL_ATOMIC_COUNTER_BUFFER, 0, atomic_bo, 0, 4);
	glBufferData(GL_ATOMIC_COUNTER_BUFFER, 4, &zero, GL_STATIC_DRAW);

	glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER, indirect_bo);
	glDispatchComputeIndirect(0);

	unsigned result;
	glGetBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(result), &result);

	glDeleteBuffers(1, &indirect_bo);
	glDeleteBuffers(1, &atomic_bo);
	piglit_check_gl_error(GL_NO_ERROR);

	return result;
}

static const struct consumer_mode consumer_modes[] = {
	{ "indirect-draw",       &indirect_draw,
		{ "GL_ARB_draw_indirect", NULL } },
	{ "index-buffer",        &index_buffer,        { NULL, } },
	{ "indirect-draw-count", &indirect_draw_count,
		{ "GL_ARB_indirect_parameters", "GL_ARB_shader_atomic_counters" },
		.clamped = true, .clamp_max = DRAW_COUNT_CLAMP_MAX },
	{ "indirect-dispatch",   &indirect_dispatch,
		{ "GL_ARB_compute_shader", NULL },
		.amplify = true },
};

static enum piglit_result
run_subtest(const struct consumer_mode *cm, const struct query_type_desc *qdesc)
{
	GLuint query;
	bool exact;
	unsigned expected, result;

	get_query_values(qdesc, &exact, &expected);

	glGenQueries(1, &query);

	run_query(query, qdesc);
	result = cm->run(query);

	glDeleteQueries(1, &query);

	piglit_check_gl_error(GL_NO_ERROR);

	if (cm->clamped)
		expected = MIN2(expected, cm->clamp_max);

	if (result != expected && (exact || result < expected)) {
		fprintf(stderr, "Result: %u\nExpected: %u\n", result, expected);
		return PIGLIT_FAIL;
	}

	return PIGLIT_PASS;
}

enum piglit_result
piglit_display(void)
{
	enum piglit_result result = PIGLIT_PASS;
	unsigned qnum_count = num_query_types();

	for (unsigned cnum = 0; cnum < ARRAY_SIZE(consumer_modes); cnum++) {
		const struct consumer_mode *cm = &consumer_modes[cnum];
		bool supported = true;

		if (arg_consumer_mode && strcmp(arg_consumer_mode, cm->name))
			continue;

		for (unsigned i = 0; i < ARRAY_SIZE(cm->extensions); ++i) {
			if (cm->extensions[i] &&
			    !piglit_is_extension_supported(cm->extensions[i])) {
				supported = false;
				break;
			}
		}

		for (unsigned qnum = 0; qnum < qnum_count; qnum++) {
			enum piglit_result subtest_result = PIGLIT_SKIP;
			const struct query_type_desc *qdesc = &query_types[qnum];

			if (arg_query_type &&
			    strcmp(arg_query_type, piglit_get_gl_enum_name(qdesc->type)))
				continue;

			supported = supported && is_query_supported(qdesc);

			if (cm->amplify) {
				if (qdesc->type == GL_TIMESTAMP ||
				    qdesc->type == GL_TIME_ELAPSED)
					continue;
			}

			if (supported) {
				subtest_result = run_subtest(cm, qdesc);
				if (subtest_result != PIGLIT_PASS)
					result = subtest_result;
			}

			piglit_report_subtest_result(
				subtest_result, "%s-%s",
				cm->name,
				piglit_get_gl_enum_name(qdesc->type));
		}
	}

	return result;
}

static void
prepare_prog_xfb()
{
	static const char *tf_out = "tf_out";

	prog_xfb = piglit_build_simple_program_unlinked(
		"#version 130\n"
		"\n"
		"out int tf_out;\n"
		"\n"
		"void main() {\n"
		"	tf_out = gl_VertexID;\n"
		"	gl_Position = vec4(0);\n"
		"}\n",
		NULL);
	glTransformFeedbackVaryings(prog_xfb, 1, &tf_out, GL_INTERLEAVED_ATTRIBS);
	glLinkProgram(prog_xfb);
	if (!piglit_link_check_status(prog_xfb))
		piglit_report_result(PIGLIT_FAIL);
	piglit_check_gl_error(GL_NO_ERROR);
}

static void
prepare_indirect_draw_count()
{
	prog_vs_atomic = piglit_build_simple_program(
		"#version 150\n"
		"#extension GL_ARB_shader_atomic_counters: require\n"
		"\n"
		"layout(binding = 0, offset = 0) uniform atomic_uint counter;\n"
		"\n"
		"void main() {\n"
		"	atomicCounterIncrement(counter);\n"
		"	gl_Position = vec4(0);\n"
		"}\n",
		NULL);

	static const GLuint indirect_draw_template[] = {
		1, /* count */
		1, /* instanceCount */
		0, /* first */
		0, /* baseInstance */
	};

	char *indirect_draw_count_data = malloc(DRAW_COUNT_CLAMP_MAX * sizeof(indirect_draw_template));
	char *dst = indirect_draw_count_data;
	for (unsigned i = 0; i < DRAW_COUNT_CLAMP_MAX; ++i) {
		memcpy(dst, indirect_draw_template, sizeof(indirect_draw_template));
		dst += sizeof(indirect_draw_template);
	}

	glGenBuffers(1, &indirect_draw_count_data_bo);
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indirect_draw_count_data_bo);
	glBufferData(GL_DRAW_INDIRECT_BUFFER,
		     DRAW_COUNT_CLAMP_MAX * sizeof(indirect_draw_template),
		     indirect_draw_count_data, GL_STATIC_DRAW);

	free(indirect_draw_count_data);

	piglit_check_gl_error(GL_NO_ERROR);
}

static void
prepare_prog_compute()
{
	GLuint shader = piglit_compile_shader_text(GL_COMPUTE_SHADER,
		"#version 150\n"
		"#extension GL_ARB_compute_shader: require\n"
		"#extension GL_ARB_shader_atomic_counters: require\n"
		"\n"
		"layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;\n"
		"\n"
		"layout(binding = 0, offset = 0) uniform atomic_uint counter;\n"
		"\n"
		"void main() {\n"
		"	atomicCounterIncrement(counter);\n"
		"}\n");

	prog_compute = glCreateProgram();
	glAttachShader(prog_compute, shader);
	glLinkProgram(prog_compute);
	glDeleteShader(shader);

	if (!piglit_link_check_status(prog_compute))
		piglit_report_result(PIGLIT_FAIL);
	piglit_check_gl_error(GL_NO_ERROR);
}

void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_ARB_query_buffer_object");

	if (argc > 1) {
		if (argc != 3) {
			fprintf(stderr, "usage: %s <consumer> <query_type>\n", argv[0]);
			exit(1);
		}

		arg_consumer_mode = argv[1];
		arg_query_type = argv[2];
	}

	query_common_init();

	prepare_prog_xfb();

	if (piglit_is_extension_supported("GL_ARB_compute_shader"))
		prepare_prog_compute();

	if (piglit_is_extension_supported("GL_ARB_indirect_parameters") &&
	    piglit_is_extension_supported("GL_ARB_shader_atomic_counters"))
		prepare_indirect_draw_count();

	glGenVertexArrays(1, &empty_vao);
}
