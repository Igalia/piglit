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
 * \file gs-max-output.cpp
 *
 * Stress the limits of what a geometry shader can output using a generic
 * geometry shader with points as input and output primitives, allowing
 * arbitrary:
 * - number of input instances (instanced draws)
 * - number of input points per instance
 * - number of invocations (GS instances)
 * - number of output vertices per invocation
 * - number of output components per vertex
 */

#include "piglit-util-gl.h"

#include <map>
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
	unsigned num_points; /* draw size / count */
	unsigned num_invocations; /* GS invocations / instances */
	unsigned num_outputs; /* # vertex ouput per GS invocation */
	unsigned num_components; /* # extra components per GS output vertex */


};

struct fragmentshaderkey {
	unsigned num_components;

	bool operator<(const fragmentshaderkey &o) const {
		return num_components < o.num_components;
	}
};

struct geometryshaderkey {
	unsigned num_invocations;
	unsigned num_outputs;
	unsigned num_components;

	bool operator<(const geometryshaderkey &o) const {
		if (num_invocations < o.num_invocations)
			return true;
		if (num_invocations > o.num_invocations)
			return false;
		if (num_outputs < o.num_outputs)
			return true;
		if (num_outputs > o.num_outputs)
			return false;
		return num_components < o.num_components;
	}
};

static std::map<fragmentshaderkey, GLuint> fragmentshaders;
static std::map<geometryshaderkey, GLuint> testprograms;

static const struct testcase default_testcase = {
	.num_instances = 1,
	.num_points = 1,
	.num_invocations = 1,
	.num_outputs = 1,
	.num_components = 0,
};

static GLuint vs_shader;
static GLuint vao;
static std::vector<testcase> testcases;
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
	"out int vs_gs_id;\n"
	"\n"
	"void main() {\n"
	"  vs_gs_id = gl_InstanceID * u_verts_per_instance + gl_VertexID;\n"
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
	"#define NUM_PAYLOAD_COMPONENTS %d\n"
	"\n"
	"layout(points, invocations = NUM_INVOCATIONS) in;\n"
	"layout(points, max_vertices = NUM_OUT_VERTICES) out;\n"
	"\n"
	"in int vs_gs_id[];\n"
	"#if NUM_PAYLOAD_COMPONENTS\n"
	"flat out int gs_ps_data[NUM_PAYLOAD_COMPONENTS];\n"
	"#endif\n"
	"\n"
	GEN_SEQUENCE
	"\n"
	"void main() {\n"
	"  for (int i = 0; i < NUM_OUT_VERTICES; ++i) {\n"
	"    int id = (vs_gs_id[0] * NUM_INVOCATIONS + gl_InvocationID) * NUM_OUT_VERTICES + i;\n"
	"    int x = id %% " STR(WINDOW_SIZE) ";\n"
	"    int y = id / " STR(WINDOW_SIZE) ";\n"
	"    gl_Position.x = (float(x) + 0.5) / " STR(WINDOW_SIZE) " * 2.0 - 1.0;\n"
	"    gl_Position.y = (float(y) + 0.5) / " STR(WINDOW_SIZE) " * 2.0 - 1.0;\n"
	"    gl_Position.z = 0.0;\n"
	"    gl_Position.w = 1.0;\n"
	"\n"
	"#if NUM_PAYLOAD_COMPONENTS\n"
	"    int val = id;\n"
	"    for (int j = 0; j < NUM_PAYLOAD_COMPONENTS; ++j) {\n"
	"      gs_ps_data[j] = val;\n"
	"      val = seq_next(val);\n"
	"    }\n"
	"#endif\n"
	"\n"
	"    EmitVertex();\n"
	"  }\n"
	"}\n";

static const char fs_text[] =
	"#version 150\n"
	"\n"
	"#define NUM_PAYLOAD_COMPONENTS %d\n"
	"\n"
	"#if NUM_PAYLOAD_COMPONENTS\n"
	"flat in int gs_ps_data[NUM_PAYLOAD_COMPONENTS];\n"
	"#endif\n"
	"out vec4 out_color;\n"
	"\n"
	GEN_SEQUENCE
	"\n"
	"void main() {\n"
	"#if NUM_PAYLOAD_COMPONENTS\n"
	"  int id = int(gl_FragCoord.y) * " STR(WINDOW_SIZE) " + int(gl_FragCoord.x);\n"
	"  int val = id;\n"
	"  for (int j = 0; j < NUM_PAYLOAD_COMPONENTS; ++j) {\n"
	"    if (val != gs_ps_data[j]) {\n"
	"      out_color.x = 1.0;\n"
	"      out_color.y = float(j) / (NUM_PAYLOAD_COMPONENTS - 1);\n"
	"      out_color.z = float(val & 0xff) / 255;\n"
	"      out_color.w = float(gs_ps_data[j] & 0xff) / 255;\n"
	"      return;\n"
	"    }\n"
	"    val = seq_next(val);\n"
	"  }\n"
	"#endif\n"
	"  out_color = vec4(0, 1, 0, 1);\n"
	"}\n";

static void
print_testcase(const struct testcase *tc)
{
	printf("Case: instances = %u points = %u invocations = %u "
	       "outputs = %u components = %u\n",
	       tc->num_instances, tc->num_points, tc->num_invocations,
	       tc->num_outputs, tc->num_components);
}

static void
add_testcase(const struct testcase *tc)
{
	if (tc->num_instances > 64 * 1024 ||
	    tc->num_points > 64 * 1024 ||
	    tc->num_invocations > 64 * 1024 ||
	    tc->num_outputs > 64 * 1024 ||
	    tc->num_components > 64 * 1024) {
		fprintf(stderr, "Excessive test case size. Are you sure?\n");
		print_testcase(tc);
		exit(1);
	}

	/* Check against implementation-defined limits. */
	if (tc->num_outputs > (unsigned)max_gs_out_vertices) {
		fprintf(stderr, "Too many output vertices (max: %d)\n",
			max_gs_out_vertices);
		print_testcase(tc);
		exit(1);
	}
	if (tc->num_outputs * (4 + tc->num_components) > (unsigned)max_gs_total_out_components) {
		fprintf(stderr, "Too many output components (max: %d)\n",
			max_gs_total_out_components);
		print_testcase(tc);
		exit(1);
	}
	if (tc->num_invocations > (unsigned)max_gs_invocations) {
		fprintf(stderr, "Too many GS invocations (max: %d)\n",
			max_gs_invocations);
		print_testcase(tc);
		exit(1);
	}

	/* Compile GS shader and link program */
	geometryshaderkey gskey;
	gskey.num_invocations = tc->num_invocations;
	gskey.num_outputs = tc->num_outputs;
	gskey.num_components = tc->num_components;
	if (testprograms.find(gskey) == testprograms.end()) {
		char *text;

		fragmentshaderkey fskey;
		fskey.num_components = tc->num_components;
		std::map<fragmentshaderkey, GLuint>::const_iterator fsit =
			fragmentshaders.find(fskey);
		if (fsit == fragmentshaders.end()) {
			if (asprintf(&text, fs_text, tc->num_components) < 0)
				abort();
			GLuint fs_shader =
				piglit_compile_shader_text(GL_FRAGMENT_SHADER, text);
			free(text);

			fsit = fragmentshaders.insert(std::make_pair(fskey, fs_shader)).first;
		}

		if (asprintf(&text, gs_text, tc->num_invocations, tc->num_outputs,
			tc->num_components) < 0)
			abort();
		GLuint gs_shader =
			piglit_compile_shader_text(GL_GEOMETRY_SHADER, text);
		free(text);

		GLuint prog =  glCreateProgram();
		glAttachShader(prog, vs_shader);
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
	print_testcase(tc);

	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	geometryshaderkey gskey;
	gskey.num_invocations = tc->num_invocations;
	gskey.num_outputs = tc->num_outputs;
	gskey.num_components = tc->num_components;
	std::map<geometryshaderkey, GLuint>::const_iterator  progit =
		testprograms.find(gskey);
	assert(progit != testprograms.end());

	glUseProgram(progit->second);
	glUniform1i(glGetUniformLocation(progit->second, "u_verts_per_instance"),
		    tc->num_points);

	glDrawArraysInstanced(GL_POINTS, 0, tc->num_points, tc->num_instances);

	float expected[WINDOW_SIZE * WINDOW_SIZE * 4];
	unsigned num_total =
		tc->num_instances * tc->num_points * tc->num_invocations * tc->num_outputs;
	memset(expected, 0, sizeof(expected));

	for (unsigned i = 0; i < WINDOW_SIZE * WINDOW_SIZE; ++i) {
		if (i < num_total)
			expected[4 * i + 1] = 1.0;
		expected[4 * i + 3] = 1.0;
	}

	return piglit_probe_image_rgba(0, 0, WINDOW_SIZE, WINDOW_SIZE, expected);
}

static void
generate_testcases_max(const testcase &tc, bool explicit_instances, bool explicit_points)
{
	unsigned amplify = tc.num_invocations * tc.num_outputs;
	double target_in_points = double(WINDOW_SIZE * WINDOW_SIZE) / amplify;

	if (!explicit_instances) {
		testcase tc1 = tc;
		tc1.num_instances = MAX2(1, (unsigned)(target_in_points / tc1.num_points));
		add_testcase(&tc1);
	}

	if (!explicit_points) {
		testcase tc1 = tc;
		tc1.num_points = MAX2(1, (unsigned)(target_in_points / tc1.num_instances));
		add_testcase(&tc1);
	}

	if (!explicit_instances && !explicit_points) {
		testcase tc1 = tc;
		tc1.num_instances = MAX2(1, (unsigned)sqrt(target_in_points));
		tc1.num_points = MAX2(1, (unsigned)(target_in_points / tc1.num_instances));
		add_testcase(&tc1);
	}

	if (explicit_instances && explicit_points)
		add_testcase(&tc);
}

void
piglit_init(int argc, char **argv)
{
	bool explicit_instances = false;
	bool explicit_points = false;
	bool explicit_invocations = false;
	bool explicit_outputs = false;
	bool explicit_components = false;
	bool scan_mode = false;
	unsigned scan_seed = 0;
	unsigned scan_count = 0;
	struct testcase explicit_testcase;

	memcpy(&explicit_testcase, &default_testcase, sizeof(explicit_testcase));

	int i;
	for (i = 1; i + 1 < argc; ++i) {
		if (!strcmp(argv[i], "-instances")) {
			explicit_testcase.num_instances = atoi(argv[i + 1]);
			explicit_instances = true;
			i++;
		} else if (!strcmp(argv[i], "-points")) {
			explicit_testcase.num_points = atoi(argv[i + 1]);
			explicit_points = true;
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
			explicit_testcase.num_components = atoi(argv[i + 1]);
			explicit_components = true;
			i++;
		} else if (!strcmp(argv[i], "-scan")) {
			if (i + 2 >= argc) {
				fprintf(stderr, "-scan: too few arguments\n");
				exit(1);
			}
			scan_seed = atoi(argv[i + 1]);
			scan_count = atoi(argv[i + 2]);
			scan_mode = true;
			i += 2;
		} else
			break;
	}
	if (i < argc) {
		fprintf(stderr, "Unknown argument: %s\n", argv[i]);
		exit(1);
	}

	vs_shader = piglit_compile_shader_text(GL_VERTEX_SHADER, vs_text);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	/* Various other GL objects needed by the test */
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGetIntegerv(GL_MAX_GEOMETRY_OUTPUT_VERTICES, (GLint*)&max_gs_out_vertices);
	glGetIntegerv(GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS,
		      (GLint*)&max_gs_total_out_components);
	glGetIntegerv(GL_MAX_GEOMETRY_OUTPUT_COMPONENTS,
		      (GLint*)&max_gs_out_components);
	glGetIntegerv(GL_MAX_GEOMETRY_SHADER_INVOCATIONS, (GLint*)&max_gs_invocations);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	max_gs_out_vertices_real = MIN2(max_gs_out_vertices,
					max_gs_total_out_components / 4);

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
				tc1.num_components = MIN2(max_gs_total_out_components / tc1.num_outputs,
							  max_gs_out_components) - 4;
			} else {
				tc1.num_outputs = MIN2(max_gs_total_out_components / (4 + tc1.num_components),
						       max_gs_out_vertices_real);
			}

			generate_testcases_max(tc1, explicit_instances, explicit_points);
		}

		if (!explicit_components) {
			testcase tc1 = tc0;

			if (!explicit_outputs) {
				tc1.num_components = max_gs_out_components - 4;
				tc1.num_outputs = MIN2(max_gs_total_out_components / (4 + tc1.num_components),
						       max_gs_out_vertices_real);
			} else {
				tc1.num_components = MIN2(max_gs_total_out_components / tc1.num_outputs,
							  max_gs_out_components) - 4;
			}

			generate_testcases_max(tc1, explicit_instances, explicit_points);
		}

		if (explicit_outputs && explicit_components)
			generate_testcases_max(tc0, explicit_instances, explicit_points);

		/* Generate additional tests randomly */
		while (testcases.size() < scan_count) {
			testcase tc = explicit_testcase;

			if (!explicit_outputs || !explicit_components) {
				if (explicit_outputs || rand() & 1) {
					unsigned max_components =
						MIN2(max_gs_total_out_components / tc.num_outputs,
						     max_gs_out_components) - 4;
					tc.num_components = rand() % (max_components + 1);

					if (!explicit_outputs) {
						unsigned max_outputs =
							MIN2(max_gs_total_out_components / (4 + tc.num_components),
							     max_gs_out_vertices_real);
						tc.num_outputs = 1 + rand() % max_outputs;
					}
				} else {
					unsigned max_outputs =
						MIN2(max_gs_total_out_components / (4 + tc.num_components),
						     max_gs_out_vertices_real);
					tc.num_outputs = 1 + rand() % max_outputs;

					if (!explicit_components) {
						unsigned max_components =
							MIN2(max_gs_total_out_components / tc.num_outputs,
							      max_gs_out_components) - 4;
						tc.num_components = rand() % (max_components + 1);
					}
				}
			}

			if (!explicit_invocations)
				tc.num_invocations = 1 + rand() % max_gs_invocations;

			unsigned amplify = tc.num_invocations * tc.num_outputs;
			unsigned target_in_points =
				MAX2(1, (WINDOW_SIZE * WINDOW_SIZE + amplify - 1) / amplify);

			switch (rand() % 4) {
			case 0:
				tc.num_points = 1 + rand() % target_in_points;
				tc.num_instances = 1 + rand() % (1 + target_in_points / tc.num_points);
				break;
			case 1:
				tc.num_instances = 1 + rand() % target_in_points;
				tc.num_points = 1 + rand() % (1 + target_in_points / tc.num_instances);
				break;
			default: {
				unsigned min = MAX2(1, sqrt(target_in_points) / 2);
				unsigned max = MIN2(sqrt(target_in_points) * 3 / 2,
						    target_in_points);
				tc.num_instances = min + rand() % (max - min + 1);
				tc.num_points = 1 + rand() % (1 + target_in_points / tc.num_instances);
				break;
			}
			}

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
