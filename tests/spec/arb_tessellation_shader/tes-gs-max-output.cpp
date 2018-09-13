/*
 * Copyright (c) 2018 Advanced Micro Devices
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
 * \file tes-gs-max-output.cpp
 *
 * Stress the limits of what tessellation + geometry shaders can output using
 * generic shaders with points as input and output primitives, allowing
 * arbitrary:
 * - number of input instances (instanced draws)
 * - number of input patches per instance
 * - (integer) tessellation factors
 * - number of invocations (GS instances)
 * - number of output vertices per invocation
 * - number of output components per vertex
 *
 * Verification works by rendering points and writing to an SSBO from the
 * fragment shader.
 */

#include "piglit-util-gl.h"

#include <algorithm>
#include <map>
#include <set>
#include <vector>

#define WINDOW_SIZE 256

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 32;
	config.supports_gl_core_version = 32;
	config.window_width = WINDOW_SIZE;
	config.window_height = WINDOW_SIZE;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

#define PASTE(x) #x
#define STR(x) PASTE(x)

struct testcase {
	unsigned num_instances; /* draw instances */
	unsigned num_patches; /* draw size / count */
	unsigned tessfactor_u;
	unsigned tessfactor_v;
	unsigned num_invocations; /* GS invocations / instances */
	unsigned num_outputs; /* # vertex ouput per GS invocation */
	unsigned num_extra_components; /* # extra components per GS output vertex */

	bool operator<(const testcase &o) const {
		return std::make_tuple(num_instances, num_patches, tessfactor_u,
				       tessfactor_v, num_invocations, num_outputs,
				       num_extra_components)
			<
		       std::make_tuple(o.num_instances, o.num_patches, o.tessfactor_u,
				       o.tessfactor_v, o.num_invocations, o.num_outputs,
				       o.num_extra_components);
	}
};

struct fragmentshaderkey {
	unsigned num_extra_components;

	bool operator<(const fragmentshaderkey &o) const {
		return num_extra_components < o.num_extra_components;
	}
};

struct geometryshaderkey {
	unsigned num_invocations;
	unsigned num_outputs;
	unsigned num_extra_components;

	bool operator<(const geometryshaderkey &o) const {
		if (num_invocations < o.num_invocations)
			return true;
		if (num_invocations > o.num_invocations)
			return false;
		if (num_outputs < o.num_outputs)
			return true;
		if (num_outputs > o.num_outputs)
			return false;
		return num_extra_components < o.num_extra_components;
	}
};

static std::map<fragmentshaderkey, GLuint> fragmentshaders;
static std::map<geometryshaderkey, GLuint> testprograms;

static const struct testcase default_testcase = {
	.num_instances = 1,
	.num_patches = 1,
	.tessfactor_u = 1,
	.tessfactor_v = 1,
	.num_invocations = 1,
	.num_outputs = 1,
	.num_extra_components = 0,
};

static int32_t *buffer_copy;

static const unsigned max_final_points = 2 * 1024 * 1024; /* requires 16 MiB buffer */
static bool small = false;
static GLuint vs_shader;
static GLuint tcs_shader;
static GLuint tes_shader;
static GLuint vao;
static GLuint ssbo;
static std::vector<testcase> testcases;
static std::set<testcase> testcases_set;
static GLuint max_tessfactor;
static GLuint max_gs_invocations;
static GLuint max_gs_out_vertices;
static GLuint max_gs_total_out_components;
static GLuint max_gs_out_components;
static unsigned max_gs_out_vertices_real;

static const char vs_text[] =
	"#version 150\n"
	"\n"
	"uniform int u_verts_per_instance;\n"
	"\n"
	"out int vs_tcs_id;\n"
	"\n"
	"void main() {\n"
	"  vs_tcs_id = gl_InstanceID * u_verts_per_instance + gl_VertexID;\n"
	"}\n";

static const char tcs_text[] =
	"#version 150\n"
	"#extension GL_ARB_tessellation_shader : require\n"
	"layout(vertices = 1) out;\n"
	"\n"
	"in int vs_tcs_id[];\n"
	"\n"
	"out int tcs_tes_id[];\n"
	"\n"
	"uniform int u_tessfactor_u;\n"
	"uniform int u_tessfactor_v;\n"
	"\n"
	"void main() {\n"
	"  tcs_tes_id[gl_InvocationID] = vs_tcs_id[gl_InvocationID];\n"
	"  gl_TessLevelOuter[0] = u_tessfactor_v;\n"
	"  gl_TessLevelOuter[1] = u_tessfactor_u;\n"
	"}\n";

static const char tes_text[] =
	"#version 150\n"
	"#extension GL_ARB_tessellation_shader : require\n"
	"layout(isolines, equal_spacing) in;\n"
	"\n"
	"in int tcs_tes_id[];\n"
	"\n"
	"out int tes_gs_id;\n"
	"\n"
	"void main() {\n"
	"  tes_gs_id = tcs_tes_id[0];\n"
	"  gl_Position.x = gl_TessCoord[0];\n"
	"  gl_Position.y = gl_TessCoord[1];\n"
	"}\n";

/* Those numbers really don't matter much for what we're trying to do here. */
#define GEN_SEQUENCE \
	"int seq_next(int x) {\n" \
	"  x = (x + 1) * 709900053;\n" \
	"  x = x ^ (x >> 17);\n" \
	"  return x;\n" \
	"}\n"

static const char gs_text[] =
	"#version 150\n"
	"#extension GL_ARB_gpu_shader5 : require\n"
	"\n"
	"#define NUM_INVOCATIONS %d\n"
	"#define NUM_OUT_VERTICES %d\n"
	"#define NUM_EXTRA_COMPONENTS %d\n"
	"\n"
	"layout(lines, invocations = NUM_INVOCATIONS) in;\n"
	"layout(points, max_vertices = NUM_OUT_VERTICES) out;\n"
	"\n"
	"uniform int u_tessfactor_u;\n"
	"uniform int u_tessfactor_v;\n"
	"\n"
	"in int tes_gs_id[];\n"
	"\n"
	"flat out int gs_ps_data[1 + NUM_EXTRA_COMPONENTS];\n"
	"\n"
	GEN_SEQUENCE
	"\n"
	"void main() {\n"
	"  int in_id = tes_gs_id[0] * u_tessfactor_u * u_tessfactor_v;\n"
	"  float v = gl_in[0].gl_Position.y;\n"
	"  in_id += u_tessfactor_u * int(v * u_tessfactor_v + 0.5);\n"
	"  float u = min(gl_in[0].gl_Position.x, gl_in[1].gl_Position.x);\n"
	"  in_id += int(u * u_tessfactor_u + 0.5);\n"
	"\n"
	"  for (int i = 0; i < NUM_OUT_VERTICES; ++i) {\n"
	"    uint id = (in_id * NUM_INVOCATIONS + gl_InvocationID) * NUM_OUT_VERTICES + i;\n"
	"    uint x = id %% " STR(WINDOW_SIZE) "u;\n"
	"    uint y = (id / " STR(WINDOW_SIZE) "u) %% " STR(WINDOW_SIZE) "u;\n"
	"    gl_Position.x = (float(x) + 0.5) / " STR(WINDOW_SIZE) " * 2.0 - 1.0;\n"
	"    gl_Position.y = (float(y) + 0.5) / " STR(WINDOW_SIZE) " * 2.0 - 1.0;\n"
	"    gl_Position.z = 0.0;\n"
	"    gl_Position.w = 1.0;\n"
	"\n"
	"    int val = int(id);\n"
	"    for (int j = 0; j <= NUM_EXTRA_COMPONENTS; ++j) {\n"
	"      gs_ps_data[j] = val;\n"
	"      val = seq_next(val);\n"
	"    }\n"
	"\n"
	"    EmitVertex();\n"
	"  }\n"
	"}\n";

static const char fs_text[] =
	"#version 150\n"
	"#extension GL_ARB_shader_storage_buffer_object : require\n"
	"\n"
	"#define NUM_EXTRA_COMPONENTS %d\n"
	"\n"
	"flat in int gs_ps_data[1 + NUM_EXTRA_COMPONENTS];\n"
	"out vec4 out_color;\n"
	"\n"
	"layout(std430, binding = 0) buffer SSBO {\n"
	"  ivec2 data[];\n"
	"} ssbo;\n"
	"\n"
	GEN_SEQUENCE
	"\n"
	"void main() {\n"
	"  int id = gs_ps_data[0];\n"
	"  int screen_id = int(gl_FragCoord.y) * " STR(WINDOW_SIZE) " + int(gl_FragCoord.x);\n"
	"  if (screen_id != id %% (" STR(WINDOW_SIZE * WINDOW_SIZE) ")) {\n"
	"    ssbo.data[id].x = 1000;\n"
	"    ssbo.data[id].y = screen_id;\n"
	"    out_color = vec4(0.1, 0, 0, 1);\n"
	"    return;\n"
	"  }\n"
	"\n"
	"  int val = id;\n"
	"  for (int j = 0; j <= NUM_EXTRA_COMPONENTS; ++j) {\n"
	"    if (val != gs_ps_data[j]) {\n"
	"      ssbo.data[id].x = 2000 + j;\n"
	"      ssbo.data[id].y = gs_ps_data[j];\n"
	"      out_color = vec4(0, 0.1, 0, 1);\n"
	"      return;\n"
	"    }\n"
	"    val = seq_next(val);\n"
	"  }\n"
	"\n"
	"  ssbo.data[id].x = 1;\n"
	"  out_color = vec4(0, 0, 0, 1);\n"
	"}\n";

static void
print_testcase(const struct testcase *tc)
{
	printf("Case: instances = %u patches = %u tessfactor = %u,%u invocations = %u "
	       "outputs = %u extra_components = %u\n",
	       tc->num_instances, tc->num_patches, tc->tessfactor_u, tc->tessfactor_v,
	       tc->num_invocations, tc->num_outputs, tc->num_extra_components);
}

static void
add_testcase(const struct testcase *tc)
{
	if (!testcases_set.insert(*tc).second)
		return;

	if (tc->num_instances > 64 * 1024 ||
	    tc->num_patches > 64 * 1024 ||
	    tc->tessfactor_u > 64 * 1024 ||
	    tc->tessfactor_v > 64 * 1024 ||
	    tc->num_invocations > 64 * 1024 ||
	    tc->num_outputs > 64 * 1024 ||
	    tc->num_extra_components > 64 * 1024) {
		fprintf(stderr, "Excessive test case size. Are you sure?\n");
		print_testcase(tc);
		exit(1);
	}

	/* Multiple separate multiplies to avoid integer wraparound */
	if ((uint64_t)tc->num_instances * tc->num_patches * tc->tessfactor_u > max_final_points ||
	    (uint64_t)tc->tessfactor_v * tc->num_invocations * tc->num_outputs > max_final_points ||
	    (uint64_t)tc->num_instances * tc->num_patches * tc->tessfactor_u *
	    tc->tessfactor_v * tc->num_invocations * tc->num_outputs > max_final_points) {
		fprintf(stderr, "Test case has more than %u final points.\n", max_final_points);
		print_testcase(tc);
		exit(1);
	}

	/* Check against implementation-defined limits. */
	if (tc->tessfactor_u > max_tessfactor || tc->tessfactor_v > max_tessfactor) {
		fprintf(stderr, "Tessellation factor too high (max: %u)\n",
			max_tessfactor);
		print_testcase(tc);
		exit(1);
	}
	if (tc->num_outputs > max_gs_out_vertices) {
		fprintf(stderr, "Too many output vertices (max: %d)\n",
			max_gs_out_vertices);
		print_testcase(tc);
		exit(1);
	}
	if (tc->num_outputs * (5 + tc->num_extra_components) > max_gs_total_out_components) {
		fprintf(stderr, "Too many output components (max: %d)\n",
			max_gs_total_out_components);
		print_testcase(tc);
		exit(1);
	}
	if (tc->num_invocations > max_gs_invocations) {
		fprintf(stderr, "Too many GS invocations (max: %d)\n",
			max_gs_invocations);
		print_testcase(tc);
		exit(1);
	}

	/* Compile GS shader and link program */
	geometryshaderkey gskey;
	gskey.num_invocations = tc->num_invocations;
	gskey.num_outputs = tc->num_outputs;
	gskey.num_extra_components = tc->num_extra_components;
	if (testprograms.find(gskey) == testprograms.end()) {
		char *text;

		fragmentshaderkey fskey;
		fskey.num_extra_components = tc->num_extra_components;
		auto fsit = fragmentshaders.find(fskey);
		if (fsit == fragmentshaders.end()) {
			if (asprintf(&text, fs_text, tc->num_extra_components) < 0)
				abort();
			GLuint fs_shader =
				piglit_compile_shader_text(GL_FRAGMENT_SHADER, text);
			free(text);

			fsit = fragmentshaders.insert(std::make_pair(fskey, fs_shader)).first;
		}

		if (asprintf(&text, gs_text, tc->num_invocations, tc->num_outputs,
			tc->num_extra_components) < 0)
			abort();
		GLuint gs_shader =
			piglit_compile_shader_text(GL_GEOMETRY_SHADER, text);
		free(text);

		GLuint prog =  glCreateProgram();
		glAttachShader(prog, vs_shader);
		glAttachShader(prog, tcs_shader);
		glAttachShader(prog, tes_shader);
		glAttachShader(prog, gs_shader);
		glAttachShader(prog, fsit->second);
		glLinkProgram(prog);
		if (!piglit_link_check_status(prog))
			piglit_report_result(PIGLIT_FAIL);

		glDeleteShader(gs_shader);

		testprograms.insert(std::make_pair(gskey, prog));
	}

	testcases.push_back(*tc);
}

static bool
run_testcase(const struct testcase *tc)
{
	unsigned final_points = tc->num_instances * tc->num_patches * tc->tessfactor_u *
				tc->tessfactor_v * tc->num_invocations * tc->num_outputs;
	unsigned bufsize = 2 * sizeof(int32_t) * final_points;

	print_testcase(tc);

	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	geometryshaderkey gskey;
	gskey.num_invocations = tc->num_invocations;
	gskey.num_outputs = tc->num_outputs;
	gskey.num_extra_components = tc->num_extra_components;
	auto progit = testprograms.find(gskey);
	assert(progit != testprograms.end());

	glUseProgram(progit->second);
	glPatchParameteri(GL_PATCH_VERTICES, 1);
	glUniform1i(glGetUniformLocation(progit->second, "u_tessfactor_u"),
		    tc->tessfactor_u);
	glUniform1i(glGetUniformLocation(progit->second, "u_tessfactor_v"),
		    tc->tessfactor_v);
	glUniform1i(glGetUniformLocation(progit->second, "u_verts_per_instance"),
		    tc->num_patches);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);

	memset(buffer_copy, 0, bufsize);
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, bufsize, buffer_copy);

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);

	glDrawArraysInstanced(GL_PATCHES, 0, tc->num_patches, tc->num_instances);

	glDisable(GL_BLEND);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		return false;

	static const float expected[] = { 0, 0, 0, 1 };
	bool ok = true;

	if (!piglit_probe_rect_rgba(0, 0, WINDOW_SIZE, WINDOW_SIZE, expected))
		ok = false;

	glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, bufsize, buffer_copy);

	for (unsigned i = 0; i < final_points; ++i) {
		if (buffer_copy[2 * i] != 1) {
			printf("Error @ %d: %d %d\n", i,
			       buffer_copy[2 * i], buffer_copy[2 * i + 1]);
			ok = false;
		}
	}

	return ok;
}


static void
generate_testcases_max2(const testcase &tc, bool explicit_instances, bool explicit_patches)
{
	unsigned amplify = tc.tessfactor_u * tc.tessfactor_v * tc.num_invocations * tc.num_outputs;
	unsigned target_in = max_final_points / amplify;

	if (small)
		target_in = MIN2(4, target_in);

	if (!explicit_instances) {
		testcase tc1 = tc;
		tc1.num_instances = MAX2(1, target_in / tc1.num_patches);
		add_testcase(&tc1);
	}

	if (!explicit_patches) {
		testcase tc1 = tc;
		tc1.num_patches = MAX2(1, target_in / tc1.num_instances);
		add_testcase(&tc1);
	}

	if (!explicit_instances && !explicit_patches) {
		testcase tc1 = tc;
		tc1.num_instances = MAX2(1, (unsigned)sqrt(target_in));
		tc1.num_patches = MAX2(1, target_in / tc1.num_instances);
		add_testcase(&tc1);
	}

	if (explicit_instances && explicit_patches)
		add_testcase(&tc);
}

static void
generate_testcases_max1(const testcase &tc, bool explicit_instances, bool explicit_patches,
			bool explicit_tessfactor_u, bool explicit_tessfactor_v,
			unsigned tess_out_max)
{
	if (!explicit_tessfactor_u) {
		testcase tc1 = tc;
		tc1.tessfactor_u = MIN2(MAX2(1, tess_out_max / tc1.tessfactor_v),
					max_tessfactor);
		generate_testcases_max2(tc1, explicit_instances, explicit_patches);
	}

	if (!explicit_tessfactor_v) {
		testcase tc1 = tc;
		tc1.tessfactor_v = MIN2(MAX2(1, tess_out_max / tc1.tessfactor_u),
					max_tessfactor);
		generate_testcases_max2(tc1, explicit_instances, explicit_patches);
	}

	if (!explicit_tessfactor_u && !explicit_tessfactor_v) {
		testcase tc1 = tc;
		tc1.tessfactor_u = MIN2(MAX2(1, (unsigned)sqrt(tess_out_max)),
					max_tessfactor);
		tc1.tessfactor_v = MIN2(MAX2(1, tess_out_max / tc1.tessfactor_u),
					max_tessfactor);
		generate_testcases_max2(tc1, explicit_instances, explicit_patches);
	}

	if (explicit_tessfactor_u && explicit_tessfactor_v)
		generate_testcases_max2(tc, explicit_instances, explicit_patches);
}

static void
generate_testcases_max(const testcase &tc, bool explicit_instances, bool explicit_patches,
		       bool explicit_tessfactor_u, bool explicit_tessfactor_v)
{
	unsigned amplify = tc.num_invocations * tc.num_outputs;
	unsigned tess_out_max = max_final_points / amplify;

	if (small) {
		generate_testcases_max1(tc, explicit_instances, explicit_patches,
					explicit_tessfactor_u, explicit_tessfactor_v,
					MIN2(4, tess_out_max));
	} else {
		generate_testcases_max1(tc, explicit_instances, explicit_patches,
					explicit_tessfactor_u, explicit_tessfactor_v,
					tess_out_max);
		while (tess_out_max > 4) {
			tess_out_max = sqrt(tess_out_max);
			generate_testcases_max1(tc, explicit_instances, explicit_patches,
						explicit_tessfactor_u, explicit_tessfactor_v,
						tess_out_max);
		}
	}
}

static float
rand_subdivide(int partitions)
{
	double x = 1.0;

	for (int i = 1; i < partitions; ++i)
		x = std::min(x, (double)rand() / ((double)RAND_MAX + 1));

	return x;
}

void
piglit_init(int argc, char **argv)
{
	bool explicit_instances = false;
	bool explicit_patches = false;
	bool explicit_tessfactor_u = false;
	bool explicit_tessfactor_v = false;
	bool explicit_invocations = false;
	bool explicit_outputs = false;
	bool explicit_components = false;
	bool scan_mode = false;
	unsigned scan_seed = 0;
	unsigned scan_count = 0;
	struct testcase explicit_testcase;

	piglit_require_extension("GL_ARB_tessellation_shader");
	piglit_require_extension("GL_ARB_shader_storage_buffer_object");

	memcpy(&explicit_testcase, &default_testcase, sizeof(explicit_testcase));

	int i;
	for (i = 1; i < argc; ++i) {
		if (!strcmp(argv[i], "-small")) {
			small = true;
		} else {
			if (i + 1 >= argc)
				break;

			if (!strcmp(argv[i], "-instances")) {
				explicit_testcase.num_instances = atoi(argv[i + 1]);
				explicit_instances = true;
				i++;
			} else if (!strcmp(argv[i], "-patches")) {
				explicit_testcase.num_patches = atoi(argv[i + 1]);
				explicit_patches = true;
				i++;
			} else if (!strcmp(argv[i], "-tessfactor_u")) {
				explicit_testcase.tessfactor_u = atoi(argv[i + 1]);
				explicit_tessfactor_u = true;
				i++;
			} else if (!strcmp(argv[i], "-tessfactor_v")) {
				explicit_testcase.tessfactor_v = atoi(argv[i + 1]);
				explicit_tessfactor_v = true;
				i++;
			} else if (!strcmp(argv[i], "-invocations")) {
				explicit_testcase.num_invocations = atoi(argv[i + 1]);
				explicit_invocations = true;
				i++;
			} else if (!strcmp(argv[i], "-outputs")) {
				explicit_testcase.num_outputs = atoi(argv[i + 1]);
				explicit_outputs = true;
				i++;
			} else if (!strcmp(argv[i], "-components")) {
				explicit_testcase.num_extra_components = atoi(argv[i + 1]);
				explicit_components = true;
				i++;
			} else {
				if (i + 2 >= argc)
					break;

				if (!strcmp(argv[i], "-scan")) {
					scan_seed = atoi(argv[i + 1]);
					scan_count = atoi(argv[i + 2]);
					scan_mode = true;
					i += 2;
				} else
					break;
			}
		}
	}
	if (i < argc) {
		fprintf(stderr, "Unknown argument or too few params: %s\n", argv[i]);
		exit(1);
	}

	/* Various GL objects needed by the test */
	vs_shader = piglit_compile_shader_text(GL_VERTEX_SHADER, vs_text);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	tcs_shader = piglit_compile_shader_text(GL_TESS_CONTROL_SHADER, tcs_text);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	tes_shader = piglit_compile_shader_text(GL_TESS_EVALUATION_SHADER, tes_text);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
	glBufferData(GL_SHADER_STORAGE_BUFFER, max_final_points * 8, NULL, GL_DYNAMIC_READ);

	buffer_copy = (int32_t *)calloc(2 * sizeof(int32_t), max_final_points);

	glGetIntegerv(GL_MAX_TESS_GEN_LEVEL, (GLint*)&max_tessfactor);
	glGetIntegerv(GL_MAX_GEOMETRY_OUTPUT_VERTICES, (GLint*)&max_gs_out_vertices);
	glGetIntegerv(GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS,
		      (GLint*)&max_gs_total_out_components);
	glGetIntegerv(GL_MAX_GEOMETRY_OUTPUT_COMPONENTS,
		      (GLint*)&max_gs_out_components);
	glGetIntegerv(GL_MAX_GEOMETRY_SHADER_INVOCATIONS, (GLint*)&max_gs_invocations);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	max_gs_out_vertices_real = MIN2(max_gs_out_vertices,
					max_gs_total_out_components / 5);

	if (scan_mode) {
		srand(scan_seed);

		/* First, generate test cases that max out each of the dimensions */
		testcase tc0 = explicit_testcase;
		if (!explicit_invocations)
			tc0.num_invocations = max_gs_invocations;

		if (!explicit_outputs) {
			testcase tc1 = tc0;

			if (!explicit_components) {
				tc1.num_outputs = max_gs_out_vertices_real;
				tc1.num_extra_components =
					MIN2(max_gs_total_out_components / tc1.num_outputs,
					     max_gs_out_components) - 5;
			} else {
				tc1.num_outputs =
					MIN2(max_gs_total_out_components /
					     (5 + tc1.num_extra_components),
					     max_gs_out_vertices_real);
			}

			generate_testcases_max(tc1, explicit_instances, explicit_patches,
					       explicit_tessfactor_u, explicit_tessfactor_v);
		}

		if (!explicit_components) {
			testcase tc1 = tc0;

			if (!explicit_outputs) {
				tc1.num_extra_components = max_gs_out_components - 5;
				tc1.num_outputs =
					MIN2(max_gs_total_out_components /
					     (5 + tc1.num_extra_components),
					     max_gs_out_vertices_real);
			} else {
				tc1.num_extra_components =
					MIN2(max_gs_total_out_components / tc1.num_outputs,
					     max_gs_out_components) - 4;
			}

			generate_testcases_max(tc1, explicit_instances, explicit_patches,
					       explicit_tessfactor_u, explicit_tessfactor_v);
		}

		if (explicit_outputs && explicit_components)
			generate_testcases_max(tc0, explicit_instances, explicit_patches,
					       explicit_tessfactor_u, explicit_tessfactor_v);

		/* Generate additional tests randomly.
		 *
		 * Attempt to generate a random distribution that isn't too
		 * lop-sided, but admittedly this is all just hand-wavey
		 * heuristics.
		 */
		while (testcases.size() < scan_count) {
			testcase tc = explicit_testcase;

			if (!explicit_outputs || !explicit_components) {
				if (explicit_outputs || rand() & 1) {
					unsigned max_components =
						MIN2(max_gs_total_out_components / tc.num_outputs,
						     max_gs_out_components) - 5;
					tc.num_extra_components = rand() % (max_components + 1);

					if (!explicit_outputs) {
						unsigned max_outputs =
							MIN2(max_gs_total_out_components /
							     (5 + tc.num_extra_components),
							     max_gs_out_vertices_real);
						tc.num_outputs = 1 + rand() % max_outputs;
					}
				} else {
					unsigned max_outputs =
						MIN2(max_gs_total_out_components /
						     (5 + tc.num_extra_components),
						     max_gs_out_vertices_real);
					tc.num_outputs = 1 + rand() % max_outputs;

					if (!explicit_components) {
						unsigned max_components =
							MIN2(max_gs_total_out_components / tc.num_outputs,
							     max_gs_out_components) - 5;
						tc.num_extra_components = rand() % (max_components + 1);
					}
				}
			}

			unsigned amplify = tc.num_outputs;

			if (explicit_invocations)
				amplify *= tc.num_invocations;
			if (explicit_tessfactor_u)
				amplify *= tc.tessfactor_u;
			if (explicit_tessfactor_v)
				amplify *= tc.tessfactor_v;
			if (explicit_patches)
				amplify *= tc.num_patches;
			if (explicit_instances)
				amplify *= tc.num_instances;

			unsigned target = max_final_points / amplify;

			if (small)
				target = MIN2(32, target);

			if (!explicit_tessfactor_u) {
				float tf_log_weight = logf(target) * rand_subdivide(6);
				tc.tessfactor_u = MIN2(expf(tf_log_weight), max_tessfactor);
				target /= tc.tessfactor_u;
			}
			if (!explicit_tessfactor_v) {
				float tf_log_weight = logf(target) * rand_subdivide(6);
				tc.tessfactor_v = MIN2(expf(tf_log_weight), max_tessfactor);
				target /= tc.tessfactor_v;
			}
			if (!explicit_invocations) {
				float log_weight = logf(target);
				if (!explicit_instances || !explicit_patches)
					log_weight *= rand_subdivide(2);
				tc.num_invocations = MIN2(expf(log_weight), max_gs_invocations);
				target /= tc.num_invocations;
			}
			if (!explicit_instances) {
				float log_weight = logf(target);
				if (!explicit_patches)
					log_weight *= rand_subdivide(2);
				tc.num_instances = expf(log_weight);
				target /= tc.num_instances;
			}
			if (!explicit_patches)
				tc.num_patches = 1 + rand() % target;

			add_testcase(&tc);
		}
	} else {
		add_testcase(&explicit_testcase);
	}
}

enum piglit_result
piglit_display(void)
{
	bool pass = true;

	for (unsigned i = 0; i < testcases.size(); ++i) {
		if (!run_testcase(&testcases[i]))
			pass = false;
	}

	if (!piglit_check_gl_error(GL_NO_ERROR))
		pass = false;

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
