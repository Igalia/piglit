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

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 32;
	config.supports_gl_core_version = 32;

PIGLIT_GL_TEST_CONFIG_END

static const char vs_pass_thru_text[] =
	"#version 150\n"
	"void main() {\n"
	"  gl_Position = vec4(0.0);\n"
	"}\n";

static const char vs_two_sets_text[] =
	"#version 150\n"
	"#extension GL_ARB_enhanced_layouts: require\n"
	"\n"
	"layout(xfb_offset = 0) out float x1_out;\n"
	"layout(xfb_offset = 4) out float x2_out[2];\n"
	"layout(xfb_offset = 12) out vec3 x3_out;\n"
	"layout(xfb_buffer = 2) out;\n"
	"layout(xfb_offset = 0, xfb_buffer = 2) out float y1_out;\n"
	"layout(xfb_offset = 4) out vec4 y2_out;\n"
	"void main() {\n"
	"  gl_Position = vec4(0.0);\n"
	"  x1_out = 1.0;\n"
	"  x2_out[0] = 2.0;\n"
	"  x2_out[1] = 3.0;\n"
	"  x3_out = vec3(4.0, 5.0, 6.0);\n"
	"  y1_out = 7.0;\n"
	"  y2_out = vec4(8.0, 9.0, 10.0, 11.0);\n"
	"}";

static const char vs_two_sets_ifc_text[] =
	"#version 150\n"
	"#extension GL_ARB_enhanced_layouts: require\n"
	"\n"
	"out block {\n"
	"  layout(xfb_offset = 12) out float x1_out;\n"
	"  layout(xfb_offset = 16) out vec2 x2_out;\n"
	"  layout(xfb_buffer = 0) out vec3 not_captured;\n"
	"  layout(xfb_offset = 0) out vec3 x3_out;\n"
	"};"
	"layout(xfb_buffer = 2) out;\n"
	"layout(xfb_offset = 0) out block2 {\n"
	"  float y1_out;\n"
	"  vec4 y2_out;\n"
	"};\n"
	"void main() {\n"
	"  gl_Position = vec4(0.0);\n"
	"  x1_out = 4.0;\n"
	"  x2_out = vec2(5.0, 6.0);\n"
	"  x3_out = vec3(1.0, 2.0, 3.0);\n"
	"  y1_out = 7.0;\n"
	"  y2_out = vec4(8.0, 9.0, 10.0, 11.0);\n"
	"}";

static const char vs_two_sets_named_ifc_text[] =
	"#version 150\n"
	"#extension GL_ARB_enhanced_layouts: require\n"
	"\n"
	"out block {\n"
	"  layout(xfb_offset = 0) out float x1_out;\n"
	"  layout(xfb_offset = 4) out vec2 x2_out;\n"
	"  layout(xfb_buffer = 0) out vec3 not_captured;\n"
	"  layout(xfb_offset = 12) out vec3 x3_out;\n"
	"} x;"
	"layout(xfb_buffer = 2) out;\n"
	"layout(xfb_offset = 0) out block2 {\n"
	"  float y1_out;\n"
	"  vec4 y2_out;\n"
	"} y;\n"
	"void main() {\n"
	"  gl_Position = vec4(0.0);\n"
	"  x.x1_out = 1.0;\n"
	"  x.x2_out = vec2(2.0, 3.0);\n"
	"  x.x3_out = vec3(4.0, 5.0, 6.0);\n"
	"  y.y1_out = 7.0;\n"
	"  y.y2_out = vec4(8.0, 9.0, 10.0, 11.0);\n"
	"}";

static const char vs_two_sets_struct_text[] =
	"#version 150\n"
	"#extension GL_ARB_enhanced_layouts: require\n"
	"\n"
	"struct Array {\n"
	"  float x2_out;\n"
	"};\n"
	"struct AoA {\n"
	"  Array x2_Array[2];\n"
	"};\n"
	"struct S {\n"
	"  float x1_out;\n"
	"  AoA x2_AoA[2];\n"
	"  float x3_out;\n"
	"};"
	"layout(xfb_offset = 0) out S s1;\n"
	"layout(xfb_offset = 0, xfb_buffer = 2) out struct S2 {\n"
	"  float y1_out;\n"
	"  vec4 y2_out;\n"
	"} s2;\n"
	"void main() {\n"
	"  gl_Position = vec4(0.0);\n"
	"  s1.x1_out = 1.0;\n"
	"  s1.x2_AoA[0].x2_Array[0].x2_out = 2.0;\n"
	"  s1.x2_AoA[0].x2_Array[1].x2_out = 3.0;\n"
	"  s1.x2_AoA[1].x2_Array[0].x2_out = 4.0;\n"
	"  s1.x2_AoA[1].x2_Array[1].x2_out = 5.0;\n"
	"  s1.x3_out = 6.0;\n"
	"  s2.y1_out = 7.0;\n"
	"  s2.y2_out = vec4(8.0, 9.0, 10.0, 11.0);\n"
	"}";

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
	const char *glsl_source;
	const char *spirv_filename;
	bool spirv;
	GLint gs_invocation_n;
};

#define BUF_1_FLOAT_N 6
#define BUF_2_FLOAT_N 5

static void
print_usage_and_exit(const char *prog_name)
{
	printf("Usage: %s <subtest> [spirv]\n"
	       "  where <subtest> is one of the following:\n"
	       "    vs (vertex shader only)\n"
	       "    vs_ifc (vertex shader only, with interface block)\n"
	       "    vs_named_ifc (vertex shader only, with named interface block)\n"
	       "    vs_struct (vertex shader only, with structs)\n"
	       "    gs (with geometry shader invoked once per stage)\n"
	       "    gs_max (with geometry shader invoked max times per "
	       "stage)\n"
	       "  add “spirv” to the command line to use SPIR-V shaders "
	       "instead of GLSL\n",
	       prog_name);
	piglit_report_result(PIGLIT_FAIL);
}

static GLuint
compile_spirv_program(const char *filename)
{
	char filepath[4096];
	unsigned size;
	char *source;
	GLuint shader, prog;

	piglit_join_paths(filepath,
			  sizeof(filepath),
			  6, /* num parts */
			  piglit_source_dir(),
			  "tests",
			  "spec",
			  "arb_enhanced_layouts",
			  "spirv",
			  filename);

	source = piglit_load_raw_file(filepath, &size);

	if (source == NULL) {
		printf("Failed to load %s\n", filepath);
		piglit_report_result(PIGLIT_FAIL);
	}

        shader = glCreateShader(GL_VERTEX_SHADER);

        glShaderBinary(1, &shader,
		       GL_SHADER_BINARY_FORMAT_SPIR_V_ARB,
		       source,
		       size);

	free(source);

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
		if (config->spirv) {
			prog = compile_spirv_program(config->spirv_filename);
		} else {
			prog = piglit_build_simple_program_multiple_shaders(
				GL_VERTEX_SHADER, config->glsl_source, 0);
		}
	} else {
		char *gs_text;

		(void)!asprintf(&gs_text,
				config->glsl_source,
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

static void
parse_args(int argc, char **argv, struct test_config *config)
{
	int i, j;
	static const struct {
		const char *name;
		const char *glsl_source;
		const char *spirv_filename;
		unsigned gs_invocation_n;
	} test_types[] = {
		{
			"vs",
			vs_two_sets_text,
			"vs_two_sets_text.spirv",
			0
		},
		{
			"vs_ifc",
			vs_two_sets_ifc_text,
			"vs_two_sets_ifc_text.spirv",
			0
		},
		{
			"vs_named_ifc",
			vs_two_sets_named_ifc_text,
			"vs_two_sets_named_ifc_text.spirv",
			0
		},
		{
			"vs_struct",
			vs_two_sets_struct_text,
			"vs_two_sets_struct_text.spirv",
			0
		},
		{
			"gs",
			gs_text_two_sets_tmpl,
			NULL,
			1
		},
		{
			"gs_max",
			gs_text_two_sets_tmpl,
			NULL,
			INT_MAX
		}
	};

	memset(config, 0, sizeof *config);

	for (i = 1; i < argc; i++) {
		for (j = 0; j < ARRAY_SIZE(test_types); j++) {
			if (!strcmp(argv[i], test_types[j].name)) {
				if (config->glsl_source != NULL)
					print_usage_and_exit(argv[0]);
				config->glsl_source =
					test_types[j].glsl_source;
				config->spirv_filename =
					test_types[j].spirv_filename;
				config->gs_invocation_n =
					test_types[j].gs_invocation_n;
				goto option_handled;
			}
		}

		if (!strcmp(argv[i], "spirv"))
			config->spirv = true;

	option_handled:
		continue;
	}

	if (config->glsl_source == NULL)
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

	/* Zero invocations means the feedback is produced by vertex shader */
	primitive_n = config.gs_invocation_n ? config.gs_invocation_n : 1;

	build_and_use_program(&config);

	/* Set up the transform feedback buffers. */
	glGenBuffers(ARRAY_SIZE(xfb), xfb);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, xfb[0]);
	glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER,
		primitive_n * BUF_1_FLOAT_N * sizeof(float), NULL,
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
