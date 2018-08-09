/*
 * Copyright © 2016 Intel Corporation
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

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 32;
	config.supports_gl_core_version = 32;

PIGLIT_GL_TEST_CONFIG_END

static const char vs_pass_thru_text[] =
	"#version 150\n"
	"void main() {\n"
	"  gl_Position = vec4(0.0);\n"
	"}\n";

#define VS_TWO_SETS_NAME "vs_two_sets.shader_source"

#define VS_TWO_SETS_IFC_NAME "vs_two_sets_ifc.shader_source"

#define VS_TWO_SETS_NAMED_IFC_NAME "vs_two_sets_named_ifc.shader_source"

#define VS_TWO_SETS_STRUCT_NAME "vs_two_sets_struct.shader_source"

#define VS_DOUBLE_NAME "vs_double.shader_source"

static const char gs_text_two_sets_tmpl[] =
	"#version 150\n"
	"#extension GL_ARB_enhanced_layouts: require\n"
	"#extension GL_ARB_gpu_shader5 : enable\n"
	"#define INVOCATION_MAX_N %u\n"
	"layout(points, invocations = INVOCATION_MAX_N) in;\n"
	"layout(points, max_vertices = 1) out;\n"
	"\n"
	"layout(xfb_offset = 0) out float x1_out;\n"
	"layout(xfb_offset = 4) out vec2 x2_out;\n"
	"layout(xfb_offset = 12) out vec3 x3_out;\n"
	"out vec3 not_captured1;\n"
	"layout(xfb_buffer = 2) out;\n"
	"layout(xfb_offset = 0) out float y1_out;\n"
	"layout(xfb_offset = 4) out vec4 y2_out;\n"
	"layout(xfb_buffer = 2) out vec3 not_captured2;\n"
	"void main() {\n"
	"  gl_Position = gl_in[0].gl_Position;\n"
	"  x1_out = 1.0 + gl_InvocationID;\n"
	"  x2_out = vec2(2.0 + gl_InvocationID, 3.0 + gl_InvocationID);\n"
	"  x3_out = vec3(4.0 + gl_InvocationID, 5.0 + gl_InvocationID,\n"
	"                6.0 + gl_InvocationID);\n"
	"  y1_out = 7.0 + gl_InvocationID;\n"
	"  y2_out = vec4(8.0 + gl_InvocationID, 9.0 + gl_InvocationID,\n"
	"                10.0 + gl_InvocationID, 11.0 + gl_InvocationID);\n"
	"  not_captured1 = vec3(1.0);"
	"  not_captured2 = vec3(1.0);"
	"  EmitVertex();\n"
	"  EndPrimitive();\n"
	"}";

struct test_config {
	const char *shader_source_filename;
	bool spirv;
	GLint gs_invocation_n;
	bool doubles;
};

#define BUF_1_FLOAT_N 6
#define BUF_2_FLOAT_N 5
#define BUF_DOUBLE_N 7


static void
print_usage_and_exit(const char *prog_name)
{
	printf("Usage: %s <subtest> [spirv]\n"
	       "  where <subtest> is one of the following:\n"
	       "    vs (vertex shader only)\n"
	       "    vs_ifc (vertex shader only, with interface block)\n"
	       "    vs_named_ifc (vertex shader only, with named interface block)\n"
	       "    vs_struct (vertex shader only, with structs)\n"
	       "    vs_double (vertex shader only, using doubles)\n"
	       "    gs (with geometry shader invoked once per stage)\n"
	       "    gs_max (with geometry shader invoked max times per "
	       "stage)\n"
	       "  add “spirv” to the command line to use SPIR-V shaders "
	       "instead of GLSL. Only vs* tests suport SPIR-V shaders.\n",
	       prog_name);
	piglit_report_result(PIGLIT_FAIL);
}

static GLuint
compile_spirv_program(GLenum shader_type,
		      const unsigned spirv_asm_size,
		      const char *spirv_asm)
{
	GLuint shader, prog;

	shader = piglit_assemble_spirv(shader_type,
				       spirv_asm_size,
				       spirv_asm);

	glSpecializeShader(shader,
			   "main",
			   0, /* numSpecializationConstants */
			   NULL /* pConstantIndex */,
			   NULL /* pConstantValue */);

	prog = glCreateProgram();
	glAttachShader(prog, shader);
	glDeleteShader(shader);

	return prog;
}

static void
build_and_use_program(const struct test_config *config)
{
	GLuint prog;

	if (config->gs_invocation_n == 0) {
		char filepath[4096];
		char *source;
		unsigned source_size;

		piglit_join_paths(filepath,
				  sizeof(filepath),
				  6, /* num parts */
				  piglit_source_dir(),
				  "tests",
				  "spec",
				  "arb_enhanced_layouts",
				  "shader_source",
				  config->shader_source_filename);

		piglit_load_source_from_shader_test(filepath,
						    GL_VERTEX_SHADER,
						    config->spirv,
						    &source,
						    &source_size);

		if (config->spirv) {
			prog = compile_spirv_program(GL_VERTEX_SHADER,
						     source_size,
						     source);
		} else {
			prog = piglit_build_simple_program_multiple_shaders(
				GL_VERTEX_SHADER, source, 0);
		}

		free(source);
	} else {
		if (config->spirv) {
			fprintf(stderr, "SPIR-V not supported for this subtest\n");
			piglit_report_result(PIGLIT_FAIL);
		}

		char *gs_text;

		(void)!asprintf(&gs_text,
				gs_text_two_sets_tmpl,
				config->gs_invocation_n);
		prog = piglit_build_simple_program_multiple_shaders(
				GL_VERTEX_SHADER, vs_pass_thru_text,
				GL_GEOMETRY_SHADER, gs_text, 0);
		free(gs_text);
	}

	glLinkProgram(prog);
	if (!piglit_link_check_status(prog))
		piglit_report_result(PIGLIT_FAIL);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	glUseProgram(prog);
}

static void
probe_queries(const GLuint *queries,
	      unsigned primitive_n)
{
	GLuint query_result;

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
}


static bool
probe_buffers_float(const GLuint *xfb,
		    unsigned primitive_n)
{
	bool pass;
	unsigned i;
	float *first;
	float *second;

	const unsigned first_n = primitive_n * BUF_1_FLOAT_N;
	const unsigned second_n = primitive_n * BUF_2_FLOAT_N;

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

static bool
probe_buffers_double(const GLuint *xfb,
		     unsigned primitive_n)
{
	bool pass;
	unsigned i, j;
	double *first;

	const unsigned first_n = primitive_n * BUF_DOUBLE_N;

	first = malloc(first_n * sizeof(double));

	for (i = 0; i < primitive_n; ++i) {
		for (j = 0; j < BUF_DOUBLE_N; j++)
			first[i * BUF_DOUBLE_N + j] = i + j + 1;
	}

	pass = piglit_probe_buffer_doubles(xfb[0], GL_TRANSFORM_FEEDBACK_BUFFER,
					   "first", 1, first_n, first);

	free(first);

	return pass;
}

static void
parse_args(int argc, char **argv, struct test_config *config)
{
	int i, j;
	bool option_was_handled = false;
	static const struct {
		const char *name;
		const char *shader_source_filename;
		unsigned gs_invocation_n;
		bool doubles;
	} test_types[] = {
		{
			"vs",
			"vs_two_sets.shader_source",
			0,
			false
		},
		{
			"vs_ifc",
			"vs_two_sets_ifc.shader_source",
			0,
			false
		},
		{
			"vs_named_ifc",
			"vs_two_sets_named_ifc.shader_source",
			0,
			false
		},
		{
			"vs_struct",
			"vs_two_sets_struct.shader_source",
			0,
			false
		},
		{
			"vs_double",
			"vs_double.shader_source",
			0,
			true,
		},
		{
			"gs",
			NULL,
			1,
			false
		},
		{
			"gs_max",
			NULL,
			INT_MAX,
			false
		}
	};

	memset(config, 0, sizeof *config);

	for (i = 1; i < argc; i++) {
		for (j = 0; j < ARRAY_SIZE(test_types); j++) {
			if (!strcmp(argv[i], test_types[j].name)) {
				config->shader_source_filename =
					test_types[j].shader_source_filename,
				config->gs_invocation_n =
					test_types[j].gs_invocation_n;
				config->doubles =
					test_types[j].doubles;
				option_was_handled = true;
				goto option_handled;
			}
		}

		if (!strcmp(argv[i], "spirv")) {
			config->spirv = true;
			printf("Running on SPIR-V mode\n");
		}

	option_handled:
		continue;
	}

	if (!option_was_handled)
		print_usage_and_exit(argv[0]);
}

void
piglit_init(int argc, char **argv)
{
	bool pass;
	unsigned primitive_n;
	GLuint queries[2];
	GLuint xfb[2];
	GLuint vao;
	struct test_config config;

	piglit_require_GLSL_version(150);
	piglit_require_extension("GL_ARB_transform_feedback3");
	piglit_require_extension("GL_ARB_enhanced_layouts");

	parse_args(argc, argv, &config);

	if (config.gs_invocation_n > 0) {
		piglit_require_extension("GL_ARB_gpu_shader5");

		if (config.gs_invocation_n == INT_MAX) {
			glGetIntegerv(GL_MAX_GEOMETRY_SHADER_INVOCATIONS,
				      &config.gs_invocation_n);
			if (config.gs_invocation_n <= 0) {
				printf("Maximum amount of geometry shader "
				       "invocations needs to be positive "
				       "(%u).\n",
				       config.gs_invocation_n);
				piglit_report_result(PIGLIT_FAIL);
			}
		}
	}

	if (config.spirv) {
		piglit_require_extension("GL_ARB_gl_spirv");

		if (config.gs_invocation_n > 0) {
			printf("Geometry shader invocations is not supported "
			       "with SPIR-V\n");
			piglit_report_result(PIGLIT_FAIL);
		}
	}

	if (config.doubles)
	   piglit_require_GLSL_version(450);

	/* Zero invocations means the feedback is produced by vertex shader */
	primitive_n = config.gs_invocation_n ? config.gs_invocation_n : 1;

	build_and_use_program(&config);

	/* Set up the transform feedback buffers. */
	glGenBuffers(ARRAY_SIZE(xfb), xfb);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, xfb[0]);
	glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER,
		     primitive_n *
		     MAX2(BUF_1_FLOAT_N * sizeof(float),
			  BUF_DOUBLE_N * sizeof(double)),
		     NULL,
		     GL_STREAM_READ);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 2, xfb[1]);
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

	probe_queries(queries, primitive_n);
	if (config.doubles)
		pass = probe_buffers_double(xfb, primitive_n);
	else
		pass = probe_buffers_float(xfb, primitive_n);

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
