/*
 * Copyright (c) 2014 Intel Corporation
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
 * @file xfb-streams.c
 *
 * This test uses geometry shader multiple stream support from
 * GL_ARB_gpu_shader5 and GL_ARB_transform_feedback3 to capture
 * transform feedback from 4 streams into 4 buffers. (GL_ARB_gpu_shader5
 * requires support for 4 GS streams.)
 */

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 32;
	config.supports_gl_core_version = 32;

PIGLIT_GL_TEST_CONFIG_END

static const char vs_pass_thru_text[] =
	"#version 150\n"
	"void main() {\n"
	"  gl_Position = vec4(0.0);\n"
	"}\n";

static const char gs_tmpl[] =
	"#version 150\n"
	"#extension GL_ARB_gpu_shader5 : enable\n"
	"layout(points, invocations = %d) in;\n"
	"layout(points, max_vertices = 4) out;\n"
	"out float stream0_0_out;\n"
	"layout(stream = 1) out vec2 stream1_0_out;\n"
	"layout(stream = 2) out float stream2_0_out;\n"
	"layout(stream = 3) out vec3 stream3_0_out;\n"
	"layout(stream = 1) out vec3 stream1_1_out;\n"
	"layout(stream = 2) out vec4 stream2_1_out;\n"
	"void main() {\n"
	"  gl_Position = gl_in[0].gl_Position;\n"

	"  stream0_0_out = 1.0 + gl_InvocationID;\n"
	"  EmitVertex();\n"
	"  EndPrimitive();\n"

	"  stream3_0_out = vec3(12.0 + gl_InvocationID, 13.0 + gl_InvocationID,\n"
	"                       14.0 + gl_InvocationID);\n"
	"  EmitStreamVertex(3);\n"
	"  EndStreamPrimitive(3);\n"

	"  stream2_0_out = 7.0 + gl_InvocationID;\n"
	"  stream2_1_out = vec4(8.0 + gl_InvocationID, 9.0 + gl_InvocationID,\n"
	"                       10.0 + gl_InvocationID, 11.0 + gl_InvocationID);\n"
	"  EmitStreamVertex(2);\n"
	"  EndStreamPrimitive(2);\n"

	"  stream1_0_out = vec2(2.0 + gl_InvocationID, 3.0 + gl_InvocationID);\n"
	"  stream1_1_out = vec3(4.0 + gl_InvocationID, 5.0 + gl_InvocationID,\n"
	"                       6.0 + gl_InvocationID);\n"
	"  EmitStreamVertex(1);\n"
	"  EndStreamPrimitive(1);\n"
	"}";

const char *stream_names[] = { "first", "second", "third", "forth" };
int stream_float_counts[] = { 1, 5, 5, 3 };

#define STREAMS ARRAY_SIZE(stream_names)

static const char *varyings[] = {
	"stream0_0_out", "gl_NextBuffer",
	"stream1_0_out", "stream1_1_out", "gl_NextBuffer",
	"stream2_0_out", "stream2_1_out", "gl_NextBuffer",
	"stream3_0_out"
};

static void
build_and_use_program(GLint gs_invocation_n)
{
	GLuint prog;

	char *gs_text;

	asprintf(&gs_text, gs_tmpl, gs_invocation_n);
	prog = piglit_build_simple_program_multiple_shaders(
				GL_VERTEX_SHADER, vs_pass_thru_text,
				GL_GEOMETRY_SHADER, gs_text, 0);
	free(gs_text);

	glTransformFeedbackVaryings(prog, ARRAY_SIZE(varyings), varyings,
				GL_INTERLEAVED_ATTRIBS);

	glLinkProgram(prog);
	if (!piglit_link_check_status(prog))
		piglit_report_result(PIGLIT_FAIL);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	glUseProgram(prog);
}

static bool
probe_buffers(const GLuint *xfb, const GLuint *queries, unsigned primitive_n)
{
	bool pass;
	unsigned i;
	GLuint query_result;
	float *expected[STREAMS];
	int expected_n[STREAMS];

	for (i = 0; i < STREAMS; i++) {
		expected_n[i] = stream_float_counts[i] * primitive_n;
	}

	for (i = 0; i < STREAMS; i++) {
		glGetQueryObjectuiv(queries[i], GL_QUERY_RESULT, &query_result);
		if (query_result != primitive_n) {
			printf("Expected %u primitives generated, got %u\n",
			       primitive_n, query_result);
			piglit_report_result(PIGLIT_FAIL);
		}
		glGetQueryObjectuiv(queries[STREAMS+i], GL_QUERY_RESULT, &query_result);
		if (query_result != primitive_n) {
			printf("Expected %u TF primitives written, got %u\n",
			       primitive_n, query_result);
			piglit_report_result(PIGLIT_FAIL);
		}
	}

	for (i = 0; i < STREAMS; i++) {
		expected[i] = malloc(expected_n[i] * sizeof(float));
	}

	for (i = 0; i < primitive_n; ++i) {
		expected[0][i * stream_float_counts[0] + 0] = i + 1.0; /* stream0_0 */

		expected[1][i * stream_float_counts[1] + 0] = i + 2.0; /* stream1_0[0] */
		expected[1][i * stream_float_counts[1] + 1] = i + 3.0; /* stream1_0[1] */
		expected[1][i * stream_float_counts[1] + 2] = i + 4.0; /* stream1_1[0] */
		expected[1][i * stream_float_counts[1] + 3] = i + 5.0; /* stream1_1[1] */
		expected[1][i * stream_float_counts[1] + 4] = i + 6.0; /* stream1_1[2] */

		expected[2][i * stream_float_counts[2] + 0] = i +  7.0; /* stream2_0 */
		expected[2][i * stream_float_counts[2] + 1] = i +  8.0; /* stream2_1[0] */
		expected[2][i * stream_float_counts[2] + 2] = i +  9.0; /* stream2_1[1] */
		expected[2][i * stream_float_counts[2] + 3] = i + 10.0; /* stream2_1[2] */
		expected[2][i * stream_float_counts[2] + 4] = i + 11.0; /* stream2_1[3] */

		expected[3][i * stream_float_counts[3] + 0] = i + 12.0; /* stream3_0[0] */
		expected[3][i * stream_float_counts[3] + 1] = i + 13.0; /* stream3_0[1] */
		expected[3][i * stream_float_counts[3] + 2] = i + 14.0; /* stream3_0[2] */
	}

	for (i = 0; i < STREAMS; ++i) {
		char *name;
		asprintf(&name, "stream%d", i);
		pass = piglit_probe_buffer(xfb[i], GL_TRANSFORM_FEEDBACK_BUFFER,
					   name, 1, expected_n[i], expected[i]);
		free(name);
	}

	for (i = 0; i < STREAMS; i++) {
		free(expected[i]);
	}

	return pass;
}

void
piglit_init(int argc, char **argv)
{
	bool pass;
	unsigned primitive_n;
	GLint gs_invocation_n;
	GLuint queries[2*STREAMS];
	GLuint xfb[STREAMS];
	GLuint vao;
	unsigned i;

	piglit_require_extension("GL_ARB_gpu_shader5");
	piglit_require_extension("GL_ARB_transform_feedback3");

	glGetIntegerv(GL_MAX_GEOMETRY_SHADER_INVOCATIONS, &gs_invocation_n);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	if (gs_invocation_n <= 0) {
		printf("Maximum amount of geometry shader invocations "
		       "needs to be positive (%u).\n", gs_invocation_n);
		piglit_report_result(PIGLIT_FAIL);
	}

	primitive_n = gs_invocation_n;

	build_and_use_program(gs_invocation_n);

	/* Set up the transform feedback buffers. */
	glGenBuffers(ARRAY_SIZE(xfb), xfb);
	for (i = 0; i < ARRAY_SIZE(xfb); i++) {
		unsigned float_n = primitive_n * stream_float_counts[i];
		glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, i, xfb[i]);
		glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER,
			     float_n * sizeof(float), NULL,
			     GL_STREAM_READ);
	}

	/* Test only records using transform feedback. */
	glEnable(GL_RASTERIZER_DISCARD);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	glGenQueries(ARRAY_SIZE(queries), queries);
	for (i = 0; i < STREAMS; i++) {
		glBeginQueryIndexed(GL_PRIMITIVES_GENERATED, i, queries[i]);
		glBeginQueryIndexed(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN,
				    i, queries[STREAMS + i]);
	}

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	/* Draw and record */
	glBeginTransformFeedback(GL_POINTS);
	glDrawArrays(GL_POINTS, 0, 1);
	for (i = 0; i < STREAMS; i++) {
		glEndQueryIndexed(GL_PRIMITIVES_GENERATED, i);
		glEndQueryIndexed(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN,
				  i);
	}
	glEndTransformFeedback();
	glDeleteVertexArrays(1, &vao);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	pass = probe_buffers(xfb, queries, primitive_n);

	glDeleteBuffers(ARRAY_SIZE(xfb), xfb);
	glDeleteQueries(ARRAY_SIZE(queries), queries);

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result
piglit_display(void)
{
	/* Should never be reached */
	return PIGLIT_FAIL;
}
