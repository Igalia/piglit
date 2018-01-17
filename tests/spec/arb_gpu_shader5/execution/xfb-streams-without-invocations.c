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
#include "piglit-shader-test.h"

/**
 * @file xfb-streams-without-invocations.c
 *
 * This test uses geometry shader multiple stream support from
 * GL_ARB_gpu_shader5 and GL_ARB_transform_feedback3 to capture
 * transform feedback from 3 streams into 2 buffers.
 *
 * Based on the work of Jordan's work in xfb-streams.c.
 */

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 32;
	config.supports_gl_core_version = 32;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

int stream_float_counts[] = { 1, 2, 5, 0 };
static bool use_spirv = false;

#define STREAMS 4

#define SHADER_TEST_FILE_NAME "xfb_streams_without_invocations.shader_test"
char shader_test_filename[4096];

static const char *varyings[] = {
	"stream0_0_out", "gl_NextBuffer",
	"stream1_0_out", "gl_NextBuffer",
	"stream2_0_out", "stream2_1_out"
};


static GLuint
assemble_spirv_shader(GLenum shader_type)
{
	char *shader_asm;
	unsigned shader_asm_size;
	GLuint shader;

	if (!piglit_load_source_from_shader_test(shader_test_filename,
						 shader_type, true,
						 &shader_asm, &shader_asm_size)) {
		piglit_report_result(PIGLIT_FAIL);
	}

	shader = piglit_assemble_spirv(shader_type,
				       shader_asm_size,
				       shader_asm);
	free(shader_asm);

	glSpecializeShader(shader,
			   "main",
			   0, /* numSpecializationConstants */
			   NULL /* pConstantIndex */,
			   NULL /* pConstantValue */);

	return shader;
}

static GLuint
build_spirv_program(void)
{
	GLuint prog, shader;

	prog = glCreateProgram();

	shader = assemble_spirv_shader(GL_VERTEX_SHADER);
	glAttachShader(prog, shader);
	glDeleteShader(shader);

	shader = assemble_spirv_shader(GL_GEOMETRY_SHADER);
	glAttachShader(prog, shader);
	glDeleteShader(shader);

	return prog;
}

static GLuint
build_glsl_program()
{
	GLuint prog;
	char *gs_text;
	char *vs_pass_thru_text;

	if (!piglit_load_source_from_shader_test(shader_test_filename,
						 GL_GEOMETRY_SHADER, false,
						 &gs_text, NULL))
		return 0;


	if (!piglit_load_source_from_shader_test(shader_test_filename,
						 GL_VERTEX_SHADER, false,
						 &vs_pass_thru_text, NULL))
		return 0;


	prog = piglit_build_simple_program_multiple_shaders(
			GL_VERTEX_SHADER, vs_pass_thru_text,
			GL_GEOMETRY_SHADER, gs_text, 0);

	glTransformFeedbackVaryings(prog, ARRAY_SIZE(varyings), varyings,
			GL_INTERLEAVED_ATTRIBS);

	free(gs_text);
	free(vs_pass_thru_text);

	return prog;
}

static void
build_and_use_program()
{
	GLuint prog;

	if (use_spirv)
		prog = build_spirv_program();
	else
		prog = build_glsl_program();

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
	/* Skip Stream = 3 as it has no transform feedback primitives written
	 * nor primitives generated
	 */
	for (i = 0; i < STREAMS-1; i++) {
		glGetQueryObjectuiv(queries[i], GL_QUERY_RESULT, &query_result);
		if (query_result != primitive_n) {
			printf("Stream = %d: Expected %u primitives generated, got %u\n",
					i, primitive_n, query_result);
			piglit_report_result(PIGLIT_FAIL);
		}
		glGetQueryObjectuiv(queries[STREAMS+i], GL_QUERY_RESULT, &query_result);
		if (query_result != primitive_n) {
			printf("Stream = %d: Expected %u TF primitives written, got %u\n",
					i, primitive_n, query_result);
			piglit_report_result(PIGLIT_FAIL);
		}
	}

	glGetQueryObjectuiv(queries[3], GL_QUERY_RESULT, &query_result);
	if (query_result != 0) {
		printf("Stream = 3: Expected 0 primitives generated, got %u\n",
				query_result);
		piglit_report_result(PIGLIT_FAIL);
	}
	glGetQueryObjectuiv(queries[STREAMS+3], GL_QUERY_RESULT, &query_result);
	if (query_result != 0) {
		printf("Stream = 3: Expected 0 primitives written, got %u\n",
				query_result);
		piglit_report_result(PIGLIT_FAIL);
	}

	for (i = 0; i < STREAMS; i++) {
		expected[i] = malloc(expected_n[i] * sizeof(float));
		memset(expected[i], 0, expected_n[i] * sizeof(float));
	}

	for (i = 0; i < primitive_n; ++i) {
		expected[0][i * stream_float_counts[1] + 0] = 0.0; /* stream0_0[0] */

		expected[1][i * stream_float_counts[1] + 0] = 0.0; /* stream1_0[0] */
		expected[1][i * stream_float_counts[1] + 1] = 1.0; /* stream1_0[1] */

		expected[2][i * stream_float_counts[2] + 0] = 0.0; /* stream2_0 */
		expected[2][i * stream_float_counts[2] + 1] = 1.0; /* stream2_1[0] */
		expected[2][i * stream_float_counts[2] + 2] = 2.0; /* stream2_1[1] */
		expected[2][i * stream_float_counts[2] + 3] = 3.0; /* stream2_1[2] */
		expected[2][i * stream_float_counts[2] + 4] = 4.0; /* stream2_1[3] */
	}

	/* Skip Stream = 3 as it has no primitives written nor generated */
	for (i = 0; i < STREAMS-1; ++i) {
		char *name;
		(void)!asprintf(&name, "stream%d", i);
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
	unsigned primitive_n = 1;
	GLuint queries[2*STREAMS];
	GLuint xfb[STREAMS];
	GLuint vao;
	unsigned i;

	piglit_require_extension("GL_ARB_gpu_shader5");
	piglit_require_extension("GL_ARB_transform_feedback3");

	for (i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "spirv"))
			use_spirv = true;
	}

	if (use_spirv)
		piglit_require_extension("GL_ARB_gl_spirv");

	piglit_join_paths(shader_test_filename,
			  sizeof(shader_test_filename),
			  7, /* num parts */
			  piglit_source_dir(),
			  "tests",
			  "spec",
			  "arb_gpu_shader5",
			  "execution",
			  "shader_test",
			  SHADER_TEST_FILE_NAME);

	build_and_use_program();

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
