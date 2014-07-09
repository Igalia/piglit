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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "piglit-util-gl.h"
#include "xfb3_common.h"

/**
 * @file ext_interleaved_two_bufs.c 
 *
 * Record two interleaved sets of attributes, one holding two attributes and the
 * other holding three, into two separate buffers. The test can be executed in
 * three different ways: recording vertex shader, recording single geometry
 * shader invocation or recording maximum amount of geometry shader invocations
 * per pipeline stage.
 *
 * Recording both vertex shader and geometry shader outputs in parallel is not
 * supported by the extension, the spec:
 *
 * "When a geometry shader is active (see section 2.12), transform feedback
 *  records the values of the selected geometry shader output variables from the
 *  emitted vertices. Otherwise, the values of the selected vertex shader output
 *  variables are recorded."
 * 
 * This test uses the "EXT"-style GLSL transform feedback.
 */

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 32;
	config.supports_gl_core_version = 32;

PIGLIT_GL_TEST_CONFIG_END

static const char vs_two_sets_text[] =
	"#version 150\n"
	"out float x1_out;\n"
	"out vec2 x2_out;\n"
	"out vec3 x3_out;\n"
	"out float y1_out;\n"
	"out vec4 y2_out;\n"
	"void main() {\n"
	"  gl_Position = vec4(0.0);\n"
	"  x1_out = 1.0;\n"
	"  x2_out = vec2(2.0, 3.0);\n"
	"  x3_out = vec3(4.0, 5.0, 6.0);\n"
	"  y1_out = 7.0;\n"
	"  y2_out = vec4(8.0, 9.0, 10.0, 11.0);\n"
	"}";

static const char gs_text_two_sets_tmpl[] =
	"#version 150\n"
	"#extension GL_ARB_gpu_shader5 : enable\n"
	"#define INVOCATION_MAX_N %u\n"
	"layout(points, invocations = INVOCATION_MAX_N) in;\n"
	"layout(points, max_vertices = 1) out;\n"
	"out float x1_out;\n"
	"out vec2 x2_out;\n"
	"out vec3 x3_out;\n"
	"out float y1_out;\n"
	"out vec4 y2_out;\n"
	"void main() {\n"
	"  gl_Position = gl_in[0].gl_Position;\n"
	"  x1_out = 1.0 + gl_InvocationID;\n"
	"  x2_out = vec2(2.0 + gl_InvocationID, 3.0 + gl_InvocationID);\n"
	"  x3_out = vec3(4.0 + gl_InvocationID, 5.0 + gl_InvocationID,\n"
	"                6.0 + gl_InvocationID);\n"
	"  y1_out = 7.0 + gl_InvocationID;\n"
	"  y2_out = vec4(8.0 + gl_InvocationID, 9.0 + gl_InvocationID,\n"
	"                10.0 + gl_InvocationID, 11.0 + gl_InvocationID);\n"
	"  EmitVertex();\n"
	"  EndPrimitive();\n"
	"}";

#define BUF_1_FLOAT_N 6
#define BUF_2_FLOAT_N 5

/**
 * The spec for ARB_transform_feedback3 says:
 *
 * If a string in <varyings> is "gl_NextBuffer", it does not identify a
 * varying variable, but instead serves as a buffer separator value to 
 * direct subsequent varyings at the next transform feedback binding point.
 */
static const char *varyings[] = {
	"x1_out", "x2_out", "x3_out", "gl_NextBuffer",
	"y1_out", "y2_out"
};

static void
print_usage_and_exit(const char *prog_name)
{
	printf("Usage: %s <subtest>\n"
	       "  where <subtest> is one of the following:\n"
	       "    vs (vertex shader only)\n"
	       "    gs (with geometry shader invoked once per stage)\n"
	       "    gs_max (with geometry shader invoked max times per "
	       "stage)\n", prog_name);
	piglit_report_result(PIGLIT_FAIL);
}

static void
build_and_use_program(unsigned gs_invocation_n)
{
	GLuint prog;

	if (gs_invocation_n == 0) {
		prog = piglit_build_simple_program_multiple_shaders(
				GL_VERTEX_SHADER, vs_two_sets_text, 0);
	} else {
		char *gs_text;

		asprintf(&gs_text, gs_text_two_sets_tmpl, gs_invocation_n);
		prog = piglit_build_simple_program_multiple_shaders(
				GL_VERTEX_SHADER, vs_pass_thru_text,
				GL_GEOMETRY_SHADER, gs_text, 0);
		free(gs_text);
	}

	/**
	 * In the EXT-style the recorded varyings need to be set before linking.
	 *
	 * Also it should be noticed that when mixed mode is used, i.e., where
	 * one records multiple attributes per buffer but also uses separate
	 * buffers, the mode must be set to interleaved.
	 */
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
	float *first;
	float *second;

	const unsigned first_n = primitive_n * BUF_1_FLOAT_N;
	const unsigned second_n = primitive_n * BUF_2_FLOAT_N;

	glGetQueryObjectuiv(queries[0], GL_QUERY_RESULT, &query_result);
	if (query_result != primitive_n) {
		printf("Expected %u primitives written, got %u\n",
			primitive_n, query_result);
		piglit_report_result(PIGLIT_FAIL);
	}

	glGetQueryObjectuiv(queries[1], GL_QUERY_RESULT, &query_result);
	if (query_result != primitive_n) {
		printf("Expected %u primitives generated, got %u\n",
			primitive_n, query_result);
		piglit_report_result(PIGLIT_FAIL);
	}

	first = malloc(first_n * sizeof(float));
	second = malloc(second_n * sizeof(float));

	for (i = 0; i < primitive_n; ++i) {
		first[i * BUF_1_FLOAT_N + 0] = i + 1.0; /* x1 */
		first[i * BUF_1_FLOAT_N + 1] = i + 2.0; /* x2[0] */
		first[i * BUF_1_FLOAT_N + 2] = i + 3.0; /* x2[1] */
		first[i * BUF_1_FLOAT_N + 3] = i + 4.0; /* x3[0] */
		first[i * BUF_1_FLOAT_N + 4] = i + 5.0; /* x3[1] */
		first[i * BUF_1_FLOAT_N + 5] = i + 6.0; /* x3[2] */

		second[i * BUF_2_FLOAT_N + 0] = i +  7.0; /* y1 */
		second[i * BUF_2_FLOAT_N + 1] = i +  8.0; /* y2[0] */
		second[i * BUF_2_FLOAT_N + 2] = i +  9.0; /* y2[1] */
		second[i * BUF_2_FLOAT_N + 3] = i + 10.0; /* y2[2] */
		second[i * BUF_2_FLOAT_N + 4] = i + 11.0; /* y2u3] */
	}

	pass = piglit_probe_buffer(xfb[0], GL_TRANSFORM_FEEDBACK_BUFFER,
			"first", 1, first_n, first);
	pass = piglit_probe_buffer(xfb[1], GL_TRANSFORM_FEEDBACK_BUFFER,
			"second", 1, second_n, second) &&
			pass;

	free(first);
	free(second);

	return pass;
}

static unsigned
parse_args(int argc, char **argv)
{
	GLint gs_invocation_n;

	if (argc != 2)
		print_usage_and_exit(argv[0]);

	if (strcmp(argv[1], "vs") == 0)
		return 0;

	piglit_require_extension("GL_ARB_gpu_shader5");

	if (strcmp(argv[1], "gs") == 0)
		return 1;

	if (strcmp(argv[1], "gs_max") != 0)
		print_usage_and_exit(argv[0]);

	glGetIntegerv(GL_MAX_GEOMETRY_SHADER_INVOCATIONS, &gs_invocation_n);
	if (gs_invocation_n <= 0) {
		printf("Maximum amount of geometry shader invocations "
		       "needs to be positive (%u).\n", gs_invocation_n);
		piglit_report_result(PIGLIT_FAIL);
	}

	return gs_invocation_n;
}

void
piglit_init(int argc, char **argv)
{
	bool pass;
	unsigned primitive_n, gs_invocation_n;
	GLuint queries[2];
	GLuint xfb[2];
	GLuint vao;

	piglit_require_GLSL_version(150);
	piglit_require_extension("GL_ARB_transform_feedback3");

	gs_invocation_n = parse_args(argc, argv);

	/* Zero invocations means the feedback is produced by vertex shader */
	primitive_n = gs_invocation_n ? gs_invocation_n : 1;

	build_and_use_program(gs_invocation_n);

	/* Set up the transform feedback buffers. */
	glGenBuffers(ARRAY_SIZE(xfb), xfb);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, xfb[0]);
	glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER,
		primitive_n * BUF_1_FLOAT_N * sizeof(float), NULL,
		GL_STREAM_READ);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 1, xfb[1]);
	glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER,
		primitive_n * BUF_2_FLOAT_N * sizeof(float), NULL,
		GL_STREAM_READ);

	/* Test only records using transform feedback. */
	glEnable(GL_RASTERIZER_DISCARD);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	glGenQueries(2, queries);
	glBeginQuery(GL_PRIMITIVES_GENERATED, queries[0]);
	glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, queries[1]);

	/* Test is run under desktop OpenGL 3.2 -> use of VAOs is required */
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	/* Draw and record */
	glBeginTransformFeedback(GL_POINTS);
	glDrawArrays(GL_POINTS, 0, 1);
	glEndQuery(GL_PRIMITIVES_GENERATED);
	glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);
	glEndTransformFeedback();
	glDeleteVertexArrays(1, &vao);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	pass = probe_buffers(xfb, queries, primitive_n);

	glDeleteBuffers(2, xfb);
	glDeleteQueries(2, queries);

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result
piglit_display(void)
{
	/* Should never be reached */
	return PIGLIT_FAIL;
}
