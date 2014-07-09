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

/**
 * @file ignore-centroid-qualifier.cpp
 *
 * Tests that all 'in' variables in fragment shader are interpolated at sample
 * positions when using per sample shading. Ignore the 'centroid' qualifier if
 * used with 'in' variable.
 *
 */
#include "piglit-util-gl.h"
#include "piglit-fbo.h"

using namespace piglit_util_fbo;
const int pattern_width = 128; const int pattern_height = 128;

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 21;
	config.supports_gl_core_version = 31;
        config.window_width = 2 * pattern_width;
        config.window_height = pattern_height;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

static Fbo multisampled_fbo;
static int sample_pos_loc, sample_id_loc, num_samples;
static int draw_prog_left, draw_prog_right, test_prog;

enum piglit_result
piglit_display(void)
{
	float pos[2];
	bool result = true, pass = true;

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, multisampled_fbo.handle);
	glClear(GL_COLOR_BUFFER_BIT);

	/* Draw test image in to left half of multisample fbo*/
	glUseProgram(draw_prog_left);
	glViewport(0, 0, pattern_width, pattern_height);
	glEnable(GL_SAMPLE_SHADING);
	glMinSampleShading(1.0);
	piglit_draw_rect(-1, -1, 2, 2);
	glDisable(GL_SAMPLE_SHADING);

	for(int i = 0; i < num_samples; i++) {
		/* Draw reference image in to right half of multisample fbo */
		glUseProgram(draw_prog_right);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, multisampled_fbo.handle);
		glGetMultisamplefv(GL_SAMPLE_POSITION, i, pos);
		glUniform2fv(sample_pos_loc, 1, pos);
		glEnable(GL_SCISSOR_TEST);
		glScissor(pattern_width, 0, pattern_width, pattern_height);
		glViewport(pattern_width, 0, pattern_width, pattern_height);
		glClear(GL_COLOR_BUFFER_BIT);
		piglit_draw_rect(-1, -1, 2, 2);
		glDisable(GL_SCISSOR_TEST);

		/* Draw sample color from multisample texture in to winsys fbo */
		glUseProgram(test_prog);
		glUniform1i(sample_id_loc, i);
		glViewport(0, 0, 2 * pattern_width, pattern_height);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
		glClear(GL_COLOR_BUFFER_BIT);
		piglit_draw_rect(-1, -1, 2, 2);

		result = piglit_probe_rect_halves_equal_rgba(0, 0,
							     piglit_width,
							     piglit_height);
		pass = pass && result;
		printf("sample_id = %d, result = %s\n", i,
		       result ? "pass" : "fail");
	}
	piglit_present_results();
	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

static void
print_usage_and_exit(char *prog_name)
{
        printf("Usage: %s <num_samples>\n", prog_name);
        piglit_report_result(PIGLIT_FAIL);
}

void
piglit_init(int argc, char**argv)
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
	if (num_samples == 0 || num_samples > max_samples)
		piglit_report_result(PIGLIT_SKIP);

	FboConfig msConfig(num_samples, piglit_width, piglit_height);
        msConfig.attach_texture = true;
	multisampled_fbo.setup(msConfig);

	/* Reduced tolerence for stricter color matching */
	piglit_set_tolerance_for_bits(16, 16, 16, 16);
	draw_prog_left = piglit_build_simple_program(
		"#version 130\n"
		"#extension GL_ARB_sample_shading: require\n"
		"in vec4 piglit_vertex;\n"
		"centroid out vec2 test;\n"
		"void main() {\n"
		"	gl_Position = piglit_vertex;\n"
		"	test = piglit_vertex.xy;\n"
		"}\n",

		"#version 130\n"
		"#extension GL_ARB_sample_shading: require\n"
		"centroid in vec2 test;\n"
		"void main() {\n"
		"	gl_FragColor = vec4(abs(test), 0, 1);\n"
		"}\n");

	draw_prog_right = piglit_build_simple_program(
		"#version 130\n"
		"uniform vec2 sample_pos;\n"
		"in vec4 piglit_vertex;\n"
		"out vec2 ref;\n"
		"void main() {\n"
		"	gl_Position = piglit_vertex;\n"
		"	ref = piglit_vertex.xy;\n"
			/* Add an offset to account for interplolation at
			 * sample position. pattern_width == pattern_height
			 * == 128, so the scaling factor between normalized
			 * device coordinates and pixels is 128/2 == 64.
			 */
		"	ref += (sample_pos - 0.5) / 64;\n"
		"}\n",

		"#version 130\n"
		"in vec2 ref;\n"
		"void main() {\n"
		"	gl_FragColor = vec4(abs(ref), 0, 1);\n"
		"}\n");
	sample_pos_loc = glGetUniformLocation(draw_prog_right, "sample_pos");

	test_prog = piglit_build_simple_program(
		"#version 130\n"
		"in vec4 piglit_vertex;\n"
		"void main() {\n"
		"	gl_Position = piglit_vertex;\n"
		"}\n",

		"#version 130\n"
		"#extension GL_ARB_texture_multisample: require\n"
		"uniform int sample_id;\n"
		"uniform sampler2DMS tex;\n"
		"void main() {\n"
		"	gl_FragColor =  texelFetch(tex, ivec2(gl_FragCoord.xy),\n"
		"				   sample_id);\n"
		"}\n");

	glUseProgram(test_prog);
	glUniform1i(glGetUniformLocation(test_prog, "tex"), 0);
	sample_id_loc = glGetUniformLocation(test_prog, "sample_id");
}
