/*
 * Copyright © 2013 Intel Corporation
 * Copyright © 2014 Advanced Micro Devices, Inc.
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

/** \file builtin-gl-sample-mask-simple.cpp
 *  This test verifies that supplying a value to gl_SampleMask[]
 *  in fragment shader program works as per ARB_sample_shading
 *  specification.
 **/

#include "piglit-fbo.h"
using namespace piglit_util_fbo;

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 21;
	config.supports_gl_core_version = 31;

	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

static int  num_samples;
static unsigned prog_0, prog_1;
static Fbo multisampled_tex;

static void
print_usage_and_exit(char *prog_name)
{
	printf("Usage: %s <num_samples>\n", prog_name);
	piglit_report_result(PIGLIT_FAIL);
}

static void
compile_shader(void)
{
	static const char *vert =
		"#version 130\n"
		"in vec4 piglit_vertex;\n"
		"void main()\n"
		"{\n"
		"  gl_Position = piglit_vertex;\n"
		"}\n";
	static const char *frag_0 =
		"#version 130\n"
		"#extension GL_ARB_sample_shading : enable\n"
		"out vec4 out_color;\n"
		"void main()\n"
		"{\n"
		"  gl_SampleMask[0] = int(gl_FragCoord.x);\n"
		"  out_color = vec4(1.0, 0.0, 0.0, 0.0);\n"
		"}\n";

	static const char *frag_template =
		"#version 130 \n"
		"%s \n"
		"uniform %s tex; \n"
		"#define fetch(i) (texelFetch(tex, ivec2(int(gl_FragCoord.y/8) * 16 + int(gl_FragCoord.x/8) % 16, 0)%s)) \n"
		"uniform int samples; \n"
		"out vec4 out_color; \n"
		"void main() \n"
		"{ \n"
		"  vec4 outv = vec4(0.0, 0.0, 0.0, 0.0); \n"
		/* encode the sample mask to RGBA */
		"  outv.x += fetch(0).x * 0.6; \n"
		"  if (1 < samples) outv.y += fetch(1).x * 0.6; \n"
		"  if (2 < samples) outv.z += fetch(2).x * 0.6; \n"
		"  if (3 < samples) outv.w += fetch(3).x * 0.6; \n"
		"  if (4 < samples) outv.x += fetch(4).x * 0.4; \n"
		"  if (5 < samples) outv.y += fetch(5).x * 0.4; \n"
		"  if (6 < samples) outv.z += fetch(6).x * 0.4; \n"
		"  if (7 < samples) outv.w += fetch(7).x * 0.4; \n"
		"  out_color = outv;\n"
		"} \n";

	/* Compile program */
	prog_0 = piglit_build_simple_program(vert, frag_0);
	if (!piglit_link_check_status(prog_0)) {
		piglit_report_result(PIGLIT_FAIL);
	}

	char *frag_1;
	if (num_samples)
	        asprintf(&frag_1, frag_template,
			 "#extension GL_ARB_texture_multisample : require",
			 "sampler2DMS", ", i");
	else
	        asprintf(&frag_1, frag_template, "", "sampler2DRect", "");

	prog_1 = piglit_build_simple_program(vert, frag_1);
	if (!piglit_link_check_status(prog_1)) {
		piglit_report_result(PIGLIT_FAIL);
	}
}

void
piglit_init(int argc, char **argv)
{
	if (argc != 2)
		print_usage_and_exit(argv[0]);

	/* 1st arg: num_samples */
	char *endptr = NULL;
	num_samples = strtol(argv[1], &endptr, 0);
	if (endptr != argv[1] + strlen(argv[1]))
		print_usage_and_exit(argv[0]);

	if (num_samples > 8) {
		puts("This test only supports 8 samples.");
		piglit_report_result(PIGLIT_SKIP);
	}

	piglit_require_extension("GL_ARB_texture_multisample");
	piglit_require_extension("GL_ARB_sample_shading");
	piglit_require_GLSL_version(130);

	/* Skip the test if num_samples > GL_MAX_SAMPLES */
	GLint max_samples;
	glGetIntegerv(GL_MAX_SAMPLES, &max_samples);
	if (num_samples > max_samples)
		piglit_report_result(PIGLIT_SKIP);

	FboConfig msConfig(num_samples, 1 << MAX2(num_samples, 1), 1);
	msConfig.num_rb_attachments = 0;
	msConfig.num_tex_attachments = 1;
	multisampled_tex.setup(msConfig);

	compile_shader();
	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		piglit_report_result(PIGLIT_FAIL);
	}
}

static bool
equal(float a, float b)
{
	return fabs(a - b) < piglit_tolerance[0];
}

enum piglit_result
piglit_display()
{
	bool pass = true;
	int samples, i, j;

	glUseProgram(prog_0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, multisampled_tex.handle);
	multisampled_tex.set_viewport();
	glGetIntegerv(GL_SAMPLES, &samples);
	samples = MAX2(samples, 1);

	glClear(GL_COLOR_BUFFER_BIT);
	glUniform1i(glGetUniformLocation(prog_0, "samples"), samples);
        piglit_draw_rect(-1, -1, 2, 2);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
	glViewport(0, 0, piglit_width, piglit_height);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(prog_1);
	glUniform1i(glGetUniformLocation(prog_1, "tex"), 0);
	glUniform1i(glGetUniformLocation(prog_1, "samples"), samples);
	if (samples <= 4)
		piglit_draw_rect(-1, -1,
				 8*(1 << samples) * (2.0/piglit_width),
				 8 * (2.0/piglit_height));
	else if (samples == 8)
		piglit_draw_rect(-1, -1,
				 8*16 * (2.0/piglit_width),
				 8*16 * (2.0/piglit_height));
	else
		assert(0 && "Unimplemented");

	for (i = 0; i < multisampled_tex.config.width; i++) {
		float color[4];
		unsigned full_mask = (1 << samples) - 1;
		unsigned expected_mask = i & full_mask;
		unsigned observed_mask = 0;

		if (multisampled_tex.config.num_samples == 0)
			expected_mask = full_mask;

		glReadPixels((i % 16) * 8 + 4,
			     (i / 16) * 8 + 4,
			     1, 1, GL_RGBA, GL_FLOAT, color);

		for (j = 0; j < 4; j++) {
			if (equal(color[j], 1) || equal(color[j], 0.6))
				observed_mask |= 1 << (j + 0);
			if (equal(color[j], 1) || equal(color[j], 0.4))
				observed_mask |= 1 << (j + 4);
		}

		if (expected_mask != observed_mask) {
			printf("Test failed, samples = %u\n"
			       "  Expected sample mask: 0x%x\n"
			       "  Observed sample mask: 0x%x\n",
			       samples, expected_mask, observed_mask);
			pass = false;
		}
	}

	piglit_present_results();
	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
