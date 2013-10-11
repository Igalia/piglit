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

/** \file builtin-gl-sample-id.cpp
 *  This test verifies that using gl_SampleID in fragment shader program
 *  works as per ARB_sample_shading specification.
 *
 **/

#include "piglit-fbo.h"
using namespace piglit_util_fbo;

const int pattern_width = 128; const int pattern_height = 128;

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 21;
	config.supports_gl_core_version = 31;

	config.window_width = pattern_width;
	config.window_height = pattern_height;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

static int num_samples;
static unsigned prog_0, prog_1;
static Fbo multisampled_tex;

static void
print_usage_and_exit(char *prog_name)
{
	printf("Usage: %s <num_samples>\n", prog_name);
	piglit_report_result(PIGLIT_FAIL);
}

void
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
		"uniform int samples;\n"
		"out vec4 out_color;\n"
		"void main()\n"
		"{\n"
		"  if (samples == 0)\n"
		"    out_color = vec4(0.0, 1.0, 0.0, 1.0);\n"
		"  else\n"
		"    out_color = vec4(0.0, float(gl_SampleID) / samples, 0.0, 1.0);\n"
		"}\n";

	static const char *frag_1 =
		"#version 130\n"
		"#extension GL_ARB_texture_multisample : require\n"
		"uniform sampler2DMS ms_tex;\n"
		"uniform int samples;\n"
		"out vec4 out_color;\n"
		"void main()\n"
		"{\n"
		"  int i;\n"
		"  bool pass = true;\n"
		"  for (i = 0; i < samples; i++) {\n"
		"    vec4 sample_color =\n"
		"    texelFetch(ms_tex, ivec2(gl_FragCoord.xy), i);\n"
		"  float sample_id_float = sample_color.g * samples;\n"
		"  int sample_id_int = int(round(sample_id_float));\n"
		"  if (sample_id_int != i)\n"
		"    pass = false;\n"
		"  }\n"
		"\n"
		"  if (pass)\n"
		"    out_color = vec4(0.0, 1.0, 0.0, 1.0);\n"
		"  else\n"
		"    out_color = vec4(1.0, 0.0, 0.0, 1.0);\n"
		"}\n";

	/* Compile program */
	GLint vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vert);
	GLint fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, frag_0);
	piglit_check_gl_error(GL_NO_ERROR);
	prog_0 = piglit_link_simple_program(vs, fs);
	if (!piglit_link_check_status(prog_0)) {
		piglit_report_result(PIGLIT_FAIL);
	}

	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vert);
	fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, frag_1);
	piglit_check_gl_error(GL_NO_ERROR);
	prog_1 = piglit_link_simple_program(vs, fs);
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

	piglit_require_extension("GL_ARB_texture_multisample");
	piglit_require_extension("GL_ARB_sample_shading");
	piglit_require_GLSL_version(130);

	/* Skip the test if num_samples > GL_MAX_SAMPLES */
	GLint max_samples;
	glGetIntegerv(GL_MAX_SAMPLES, &max_samples);
	if (num_samples > max_samples)
		piglit_report_result(PIGLIT_SKIP);

	FboConfig msConfig(num_samples, pattern_width, pattern_height);
        msConfig.attach_texture = true;
	multisampled_tex.setup(msConfig);

	compile_shader();
	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		piglit_report_result(PIGLIT_FAIL);
	}
}

enum piglit_result
piglit_display()
{
	bool pass = true;
	int samples;
        float expected[4] = {0.0, 1.0, 0.0, 1.0};

	glUseProgram(prog_0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, multisampled_tex.handle);
	glGetIntegerv(GL_SAMPLES, &samples);
	glClear(GL_COLOR_BUFFER_BIT);
	glUniform1i(glGetUniformLocation(prog_0, "samples"), samples);
        piglit_draw_rect(-1, -1, 2, 2);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, multisampled_tex.handle);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
	glClear(GL_COLOR_BUFFER_BIT);
	if (samples == 0) {
		glBlitFramebuffer(0, 0,
				  pattern_width, pattern_height,
				  0, 0,
				  pattern_width, pattern_height,
				  GL_COLOR_BUFFER_BIT,
				  GL_NEAREST);
	} else {
		glUseProgram(prog_1);
		glUniform1i(glGetUniformLocation(prog_1, "ms_tex"), 0);
		glUniform1i(glGetUniformLocation(prog_1, "samples"), samples);
		piglit_draw_rect(-1, -1, 2, 2);
	}
	glBindFramebuffer(GL_READ_FRAMEBUFFER, piglit_winsys_fbo);
	pass = piglit_probe_rect_rgba(0, 0, pattern_width,
                                      pattern_width, expected)
               && pass;
	piglit_present_results();
	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
