/*
 * Copyright 2017 Advanced Micro Devices, Inc.
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/** \file samplemask.c
 *
 * Test two properties of gl_SampleMask and gl_SampleMaskIn for different
 * sample rates, while rendering a slightly off-center triangle fan that
 * covers the entire window, to thoroughly exercise cases where pixels are
 * partially covered:
 *
 * 1. Setting all bits of gl_SampleMask in all fragment shader invocations will
 *    cause all samples to be written exactly once. I.e., setting
 *    bits outside the rasterization coverage has no effect.
 * 2. The bits of gl_SampleMaskIn over all fragment shader invocations form a
 *    partition of the set of samples. This subtest requires
 *    ARB_shader_atomic_counters to disambiguate between fragment shader
 *    invocations. (Also verifies sampleID is 0 when msaa is disabled.)
 * Additionally, there's a test to just verify gl_SampleMaskIn is 1 when 
 * msaa is disabled (regardless of per-sample frequency shader or sample
 * shading). (Omitted from test 2 because it's difficult to track down
 * what's going wrong if drivers fail too many parts of the test.)
 *
 * The sample rate is controlled in one of two ways: Either glMinSampleShading
 * or a fragment shader variant that uses gl_SampleID is used.
 */

#include "piglit-fbo.h"

using namespace piglit_util_fbo;

/* Produce lots of very narrow triangles, but some fully covered pixels as well. */
#define WINDOW_SIZE 256
#define VERTICES_PER_EDGE 80

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_core_version = 31;

	config.window_width = WINDOW_SIZE;
	config.window_height = WINDOW_SIZE;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

#define STR(x) #x
#define STRINGIFY(x) STR(x)

enum rate_mode {
	rate_mode_sampleshading,
	rate_mode_sampleid,
	rate_mode_sampleid_noms,
};

static int num_samples;
static int actual_num_samples;
static bool partition_check_supported;
static bool mask_in_one_supported;
static const char *procname;
static const char *testname;
static const char *sample_rate;
static unsigned prog_fix_sample_mask[2];
static unsigned prog_fix_check;
static unsigned prog_mask_in_one[2];
static unsigned prog_partition_write[2];
static unsigned prog_partition_check;
static unsigned prog_partition_check_have_sampleid;
static unsigned prog_partition_check_msaa_disabled;
static Fbo ms_fbo;
static Fbo ms_ifbo;

static void
print_usage_and_exit(const char *prog_name)
{
	printf("Usage: %s <num_samples> <rate> {fix|partition|mask_in_one|all}\n"
	       "where <rate> is either a floating point MinSampleShading value\n"
	       "             or 'sample', 'noms', or 'all'\n",
	       prog_name);
	piglit_report_result(PIGLIT_FAIL);
}

static void
build_program_variants(unsigned prog[2], const char *vert, const char *frag_template)
{
	char *frag;

	if (asprintf(&frag, frag_template, "0") < 0)
		piglit_report_result(PIGLIT_FAIL);
	prog[0] = piglit_build_simple_program(vert, frag);
	free(frag);

	if (asprintf(&frag, frag_template, "gl_SampleID") < 0)
		piglit_report_result(PIGLIT_FAIL);
	prog[1] = piglit_build_simple_program(vert, frag);
	free(frag);
}

static void
compile_shaders(void)
{
	static const char vert_passthrough[] =
		"#version 130\n"
		"in vec4 piglit_vertex;\n"
		"void main()\n"
		"{\n"
		"  gl_Position = piglit_vertex;\n"
		"}\n";
	static const char vert_fan[] =
		"#version 130\n"
		"void main()\n"
		"{\n"
		/* Calculate vertices of the triangle fan based on gl_VertexID */
		"  if (gl_VertexID == 0) {\n"
		"    gl_Position = vec4(0.01, 0.011, 0, 1);\n"
		"  } else {\n"
		"    int edge = ((gl_VertexID - 1) / " STRINGIFY(VERTICES_PER_EDGE) ") % 4;\n"
		"    int vertex = (gl_VertexID - 1) % " STRINGIFY(VERTICES_PER_EDGE) ";\n"
		"    float t = 2.0 / " STRINGIFY(VERTICES_PER_EDGE) " * vertex;\n"
		"    if (edge == 0)\n"
		"      gl_Position = vec4(-1 + t, -1, 0, 1);\n"
		"    else if (edge == 1)\n"
		"      gl_Position = vec4(1, -1 + t, 0, 1);\n"
		"    else if (edge == 2)\n"
		"      gl_Position = vec4(1 - t, 1, 0, 1);\n"
		"    else\n"
		"      gl_Position = vec4(-1, 1 - t, 0, 1);\n"
		"  }\n"
		"}\n";

	static const char frag_fix_sample_mask[] =
		"#version 130\n"
		"#extension GL_ARB_sample_shading : enable\n"
		"out vec4 out_color;\n"
		"void main()\n"
		"{\n"
		"  gl_SampleMask[0] = ~0;\n"
		"  out_color = vec4(0.1, 0.0, %s, 0.0);\n"
		"}\n";
	static const char frag_fix_check[] =
		"#version 130\n"
		"#extension GL_ARB_texture_multisample : require\n"
		"uniform sampler2DMS tex;\n"
		"uniform int num_samples;\n"
		"out vec4 out_color;\n"
		"void main()\n"
		"{\n"
		"  out_color = vec4(0.0, 1.0, 0.0, 1.0);\n"
		"  for (int i = 0; i < num_samples; ++i) {\n"
		"    float v = texelFetch(tex, ivec2(gl_FragCoord.xy), i).x;\n"
		"    if (abs(v - 0.1) > 0.01)\n"
		"      out_color = vec4(1.0, float(i) / 255, v, 0.0);\n"
		"  }\n"
		"}\n";

	static const char frag_mask_in_one[] =
		"#version 130\n"
		"#extension GL_ARB_gpu_shader5 : enable\n"
		"#extension GL_ARB_sample_shading : enable\n"
		"out vec4 out_color;\n"
		"void main()\n"
		"{\n"
		"  out_color = vec4(float(gl_SampleMaskIn[0]) / 10.0, 0.0, %s, 0.0);\n"
		"}\n";

	static const char frag_partition_write[] =
		"#version 140\n"
		"#extension GL_ARB_gpu_shader5 : enable\n"
		"#extension GL_ARB_sample_shading : enable\n"
		"#extension GL_ARB_shader_atomic_counters : enable\n"
		"layout(binding = 0, offset = 0) uniform atomic_uint counter;\n"
		"out ivec4 out_color;\n"
		"void main()\n"
		"{\n"
		"  int invocation = int(atomicCounterIncrement(counter));\n"
		"  out_color = ivec4(gl_SampleMaskIn[0], invocation, %s, 0);\n"
		"}\n";
	static const char frag_partition_check[] =
		"#version 130\n"
		"#extension GL_ARB_texture_multisample : require\n"
		"uniform isampler2DMS tex;\n"
		"uniform int num_samples;\n"
		"uniform bool have_sampleid;\n"
		"uniform bool msaa_disabled;\n"
		"out vec4 out_color;\n"
		"void main()\n"
		"{\n"
		"  out_color = vec4(0, 1, 0, 1);\n"
		"  for (int i = 0; i < num_samples; ++i) {\n"
		"    ivec4 di = texelFetch(tex, ivec2(gl_FragCoord.xy), i);\n"
		"    if (msaa_disabled) {\n"
		"      /* omit di.x == 1 test here, drivers fail multiple parts already... */\n"
		"      if (di.z != 0)\n"
		"        out_color = vec4(0.2, float(i) / 255, float(di.z) / 255, 0);\n"
		"    } else {\n"
		"      if ((di.x & (1 << i)) == 0)\n"
		"        out_color = vec4(0.1, float(i) / 255, float(di.x) / 255, 0);\n"
		"      if (have_sampleid && di.z != i)\n"
		"        out_color = vec4(0.2, float(i) / 255, float(di.z) / 255, 0);\n"
		"    };\n"
		"    for (int j = i + 1; j < num_samples; ++j) {\n"
		"      ivec2 dj = texelFetch(tex, ivec2(gl_FragCoord.xy), j).xy;\n"
		"      bool overlap = (di.x & dj.x) != 0;\n"
		"      bool equal = di.x == dj.x;\n"
		"      bool same_invoc = di.y == dj.y;\n"
		"      if (same_invoc && !equal)\n"
		"        out_color = vec4(0.5, float(i) / 255, float(j) / 255, 0);\n"
		"      if (!same_invoc && overlap)\n"
		"        out_color = vec4(0.6, float(i) / 255, float(j) / 255, 0);\n"
		"    }\n"
		"  }\n"
		"}\n";

	build_program_variants(prog_fix_sample_mask, vert_fan, frag_fix_sample_mask);
	prog_fix_check = piglit_build_simple_program(vert_passthrough, frag_fix_check);
	glUseProgram(prog_fix_check);
	glUniform1i(glGetUniformLocation(prog_fix_check, "tex"), 0);
	glUniform1i(glGetUniformLocation(prog_fix_check, "num_samples"), actual_num_samples);

	if (mask_in_one_supported) {
		build_program_variants(prog_mask_in_one, vert_fan, frag_mask_in_one);
	}

	if (partition_check_supported) {
		build_program_variants(prog_partition_write, vert_fan, frag_partition_write);
		prog_partition_check = piglit_build_simple_program(vert_passthrough, frag_partition_check);
		glUseProgram(prog_partition_check);
		glUniform1i(glGetUniformLocation(prog_partition_check, "tex"), 0);
		glUniform1i(glGetUniformLocation(prog_partition_check, "num_samples"),
			    actual_num_samples);
		prog_partition_check_have_sampleid =
			glGetUniformLocation(prog_partition_check, "have_sampleid");
		prog_partition_check_msaa_disabled =
			glGetUniformLocation(prog_partition_check, "msaa_disabled");
	}
}

static void
draw_fan(enum rate_mode mode, float sample_rate, bool msaa_force_disable)
{
	if (mode == rate_mode_sampleid_noms || msaa_force_disable) {
		glDisable(GL_MULTISAMPLE);
	}
	if (mode == rate_mode_sampleshading) {
		glEnable(GL_SAMPLE_SHADING);
		glMinSampleShading(sample_rate);
	}
	glDrawArrays(GL_TRIANGLE_FAN, 0, 2 + 4 * VERTICES_PER_EDGE);
	glDisable(GL_SAMPLE_SHADING);
	glEnable(GL_MULTISAMPLE);
}

static enum piglit_result
test_fix(enum rate_mode mode, float sample_rate)
{
	glClearColor(0.0, 0.0, 0.0, 0.0);

	/* 1. Draw everything with gl_SampleMask = ~0; */
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, ms_fbo.handle);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(prog_fix_sample_mask[mode != rate_mode_sampleshading]);

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);

	draw_fan(mode, sample_rate, false);

	glDisable(GL_BLEND);

	/* 2. Use the check shader to check correctness. */
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(prog_fix_check);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, ms_fbo.color_tex[0]);

	piglit_draw_rect(-1, -1, 2, 2);

	static const float expected[4] = { 0, 1, 0, 1 };
	if (!piglit_probe_rect_rgba(0, 0, WINDOW_SIZE, WINDOW_SIZE, expected))
		return PIGLIT_FAIL;

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	return PIGLIT_PASS;
}

static enum piglit_result
test_mask_in_one(enum rate_mode mode, float sample_rate)
{
	glClearColor(0.0, 0.0, 0.0, 0.0);

	/* 1. Draw everything outputting gl_SampleMaskIn, with msaa disabled; */
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, ms_fbo.handle);
	glClear(GL_COLOR_BUFFER_BIT);

	/*
	 * We'll abuse the sampleid_noms mode here and use the prog without
	 * sample id to so we still have 3 somewhat meaningful modes - of
	 * course with msaa always disabled it should always be the same.
	 */
	glUseProgram(prog_mask_in_one[mode == rate_mode_sampleid]);

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);

	draw_fan(mode, sample_rate, true);

	glDisable(GL_BLEND);

	/* 2. Use the check shader to check correctness. */
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(prog_fix_check);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, ms_fbo.color_tex[0]);

	piglit_draw_rect(-1, -1, 2, 2);

	static const float expected[4] = { 0, 1, 0, 1 };
	if (!piglit_probe_rect_rgba(0, 0, WINDOW_SIZE, WINDOW_SIZE, expected))
		return PIGLIT_FAIL;

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	return PIGLIT_PASS;
}

static enum piglit_result
test_partition(enum rate_mode mode, float sample_rate)
{
	if (!partition_check_supported)
		return PIGLIT_SKIP;

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, ms_ifbo.handle);

	glUseProgram(prog_partition_write[mode != rate_mode_sampleshading]);

	draw_fan(mode, sample_rate, false);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(prog_partition_check);
	glUniform1i(prog_partition_check_have_sampleid, mode == rate_mode_sampleid);
	glUniform1i(prog_partition_check_msaa_disabled, mode == rate_mode_sampleid_noms);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, ms_ifbo.color_tex[0]);

	piglit_draw_rect(-1, -1, 2, 2);

	static const float expected[4] = { 0, 1, 0, 1 };
	if (!piglit_probe_rect_rgba(0, 0, WINDOW_SIZE, WINDOW_SIZE, expected))
		return PIGLIT_FAIL;

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	return PIGLIT_PASS;
}

static bool
iterate_sample_rates(const char *testname, enum piglit_result (*test)(enum rate_mode, float))
{
	bool all = !strcmp(sample_rate, "all");
	bool sample = all || !strcmp(sample_rate, "sample");
	bool noms = all || !strcmp(sample_rate, "noms");
	bool ok = true;
	enum piglit_result result;

	if (sample) {
		result = test(rate_mode_sampleid, 0);
		piglit_report_subtest_result(result, "sample %s", testname);
		if (result == PIGLIT_FAIL)
			ok = false;
	}

	if (noms) {
		result = test(rate_mode_sampleid_noms, 0);
		piglit_report_subtest_result(result, "noms %s", testname);
		if (result == PIGLIT_FAIL)
			ok = false;
	}

	if (all) {
		float rate = 1.0;
		for (;;) {
			result = test(rate_mode_sampleshading, rate);
			piglit_report_subtest_result(result, "%f %s", rate, testname);
			if (result == PIGLIT_FAIL)
				ok = false;

			if (actual_num_samples * rate <= 1)
				break;

			rate *= 0.5;
		}
	}

	if (!all && !sample && !noms) {
		float rate = atof(sample_rate);
		result = test(rate_mode_sampleshading, rate);
		piglit_report_subtest_result(result, "%f %s", rate, testname);
		if (result == PIGLIT_FAIL)
			ok = false;
	}

	return ok;
}

enum piglit_result
piglit_display()
{
	bool pass = true;
	bool run = false;
	bool all = !strcmp(testname, "all");

	if (all || !strcmp(testname, "fix")) {
		run = true;
		pass = iterate_sample_rates("fix", test_fix) && pass;
	}

	if (all || !strcmp(testname, "mask_in_one")) {
		run = true;
		pass = iterate_sample_rates("mask_in_one", test_mask_in_one) && pass;
	}

	if (all || !strcmp(testname, "partition")) {
		run = true;
		pass = iterate_sample_rates("partition", test_partition) && pass;
	}

	if (!run)
		print_usage_and_exit(procname);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	if (argc != 4)
		print_usage_and_exit(argv[0]);

	procname = argv[0];

	/* 1st arg: num_samples */
	char *endptr = NULL;
	num_samples = strtol(argv[1], &endptr, 0);
	if (endptr != argv[1] + strlen(argv[1]))
		print_usage_and_exit(argv[0]);

	/* 2nd arg: sample rate */
	sample_rate = argv[2];

	/* 3rd arg: testname */
	testname = argv[3];

	piglit_require_extension("GL_ARB_texture_multisample");
	piglit_require_extension("GL_ARB_sample_shading");
	piglit_require_GLSL_version(130);

	partition_check_supported =
		piglit_is_extension_supported("GL_ARB_gpu_shader5") &&
		piglit_is_extension_supported("GL_ARB_shader_atomic_counters");

	mask_in_one_supported =
		piglit_is_extension_supported("GL_ARB_gpu_shader5");

	/* Skip the test if num_samples > GL_MAX_SAMPLES */
	GLint max_samples;
	glGetIntegerv(GL_MAX_SAMPLES, &max_samples);
	if (num_samples > max_samples)
		piglit_report_result(PIGLIT_SKIP);

	/* Dummy vertex array */
	GLuint empty_vao;
	glGenVertexArrays(1, &empty_vao);
	glBindVertexArray(empty_vao);

	/* Multi-sample framebuffer setup */
	FboConfig fbo_config(num_samples, WINDOW_SIZE, WINDOW_SIZE);
	fbo_config.num_tex_attachments = 1;
	fbo_config.num_rb_attachments = 0;
	fbo_config.depth_internalformat = GL_NONE;
	fbo_config.stencil_internalformat = GL_NONE;
	ms_fbo.setup(fbo_config);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, ms_fbo.handle);
	glGetFramebufferParameteriv(GL_DRAW_FRAMEBUFFER, GL_SAMPLES, &actual_num_samples);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);

	printf("Requested %d samples, got %d samples\n", num_samples, actual_num_samples);

	/* Integer multi-sample framebuffer setup */
	fbo_config.color_internalformat = GL_RGBA32I;
	ms_ifbo.setup(fbo_config);

	/* Shader setup */
	compile_shaders();

	if (partition_check_supported) {
		GLuint atomic_bo;
		glGenBuffers(1, &atomic_bo);
		glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, atomic_bo);
		glBufferData(GL_ATOMIC_COUNTER_BUFFER, 4, NULL, GL_STATIC_DRAW);
	}

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);
}
