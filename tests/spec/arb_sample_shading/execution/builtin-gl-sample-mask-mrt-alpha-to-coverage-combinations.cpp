/*
 * Copyright © 2019 Intel Corporation
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
/*
 * \file builtin-gl-sample-mask-mrt-alpha-to-coverage-combinations.cpp
 *
 * Test verifies correct work of alpha-to-coverage with every variation of
 * sample mask, all levels of MSAA and different number of render targets.
 *
 * \author Andrii Kryvytskyi <andrii.o.kryvytskyi@globallogic.com>
 */

#include "piglit-fbo.h"
using namespace piglit_util_fbo;
PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_core_version = 40;

	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

	/* 4 possible alphas and 2^16 possible sample masks */
	config.window_height = 512;
	config.window_width = 512;

PIGLIT_GL_TEST_CONFIG_END

static Fbo
make_fbo(int tex_attachments_num, int samples_num)
{
	FboConfig config(samples_num, piglit_width, piglit_height);
	config.num_rb_attachments = 0;

	config.num_tex_attachments = tex_attachments_num;

	for (int i = 0; i < tex_attachments_num; i++)
		config.tex_attachment[i] = GL_COLOR_ATTACHMENT0 + i;

	Fbo fbo;
	fbo.setup(config);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	return fbo;
}

static GLuint test_prog;
static GLuint check_prog;
static GLint max_samples_num;
static GLint max_attachments_num;

static bool should_all_be_checked;
static int num_samples_to_check;
static int num_render_targets_to_check;

static const char *test_vs =
	"#version 130\n"
	"in vec4 piglit_vertex;\n"
	"\n"
	"void main()\n"
	"{\n"
	"    gl_Position = piglit_vertex;\n"
	"}\n";

static const char *test_fs =
	"#version 400\n"
	"#extension GL_ARB_sample_shading : enable\n"
	"\n"
	"layout(pixel_center_integer) in vec4 gl_FragCoord;\n"
	"uniform int render_targets;\n"
	"uniform int samples_num;\n"
	"uniform int screen_width;\n"
	"out vec4 out_color[gl_MaxDrawBuffers];\n"
	"\n"
	"void main()\n"
	"{\n"
	"\n"
	"    float fragment_index = gl_FragCoord.x +"
	"        gl_FragCoord.y * screen_width;\n"
	"	int sample_permutations = int(exp2(samples_num));\n"
	"\n"
	"    vec4 color = vec4(\n"
	"                        mod(gl_FragCoord.x, 2),\n"
	"                        mod(gl_FragCoord.y, 2),\n"
	"                        mod(fragment_index, 2),\n"
	"                        mod(gl_FragCoord.x, 4) - 1\n"
	"                 );\n"
	"\n"
	"    gl_SampleMask[0] = int(mod("
	"                        fragment_index / 4,"
	"                        sample_permutations)"
	"                       );\n"
	"\n"
	"    out_color[0] = color;\n"
	"    for (int i = 1; i < render_targets; i++) {\n"
	"    color = vec4(\n"
	"                mod(gl_FragCoord.y, 2),\n"
	"                mod(fragment_index, 2),\n"
	"                mod(gl_FragCoord.x, 2),\n"
	"                0.0\n"
	"            );\n"
	"\n"
	"    out_color[i] = color;\n"
	"    }\n"
	"}\n";

static const char *check_vs =
	"#version 130\n"
	"in vec4 piglit_vertex;\n"
	"\n"
	"void main()\n"
	"{\n"
	"    gl_Position = piglit_vertex;\n"
	"}\n";

static const char *check_fs =
	"#version 400\n"
	"#extension GL_ARB_texture_multisample : require\n"
	"\n"
	"layout(pixel_center_integer) in vec4 gl_FragCoord;\n"
	"uniform int screen_width;\n"
	"uniform int screen_height;\n"
	"uniform int samples_num;\n"
	"uniform int render_target;\n"
	"uniform sampler2DMS tex;\n"
	"\n"
	"out vec4 out_color;\n"
	"\n"
	"vec4 get_expected_color()\n"
	"{\n"
	"    float fragment_index = gl_FragCoord.x +"
	"                            gl_FragCoord.y * screen_width;\n"
	"    vec4 expected = vec4(0.0);\n"
	"    float alpha = clamp(mod(gl_FragCoord.x, 4) - 1, 0, 1);\n"
	"\n"
	"    if (alpha == 0.0f) {\n"
	"    expected = vec4(0.0);\n"
	"    } else if (render_target == 0) {\n"
	"    expected = vec4(\n"
	"                    mod(gl_FragCoord.x, 2),\n"
	"                    mod(gl_FragCoord.y, 2),\n"
	"                    mod(fragment_index, 2),\n"
	"                    alpha\n"
	"               );\n"
	"    } else {\n"
	"    expected = vec4(\n"
	"                    mod(gl_FragCoord.y, 2),\n"
	"                    mod(fragment_index, 2),\n"
	"                    mod(gl_FragCoord.x, 2),\n"
	"                    0.0\n"
	"               );\n"
	"    }\n"
	"\n"
	"    return expected;\n"
	"}\n"
	"\n"
	"void main()\n"
	"{\n"
	"    vec4 expected = get_expected_color();\n"
	"    bool pass = true;\n"
	"\n"
	"    int fragment_index = int(gl_FragCoord.x +"
	"                            gl_FragCoord.y * screen_width);\n"
	"    int sample_permutations = int(exp2(samples_num));\n"
	"    int sample_mask = int(mod("
	"                        fragment_index / 4,"
	"                        sample_permutations)"
	"                      );\n"
	"\n"
	"    for (int i = 0; i < samples_num; i++) {\n"
	"    ivec2 texelToFetch = ivec2(gl_FragCoord.x, gl_FragCoord.y);\n"
	"    vec4 currentColor = texelFetch(tex, texelToFetch, i);\n"
	"\n"
	"    if ((sample_mask & (1 << i)) != 0)\n"
	"        pass = pass && (expected == currentColor);\n"
	"    else\n"
	"        pass = pass && (vec4(0.0) == currentColor);"
	"    }\n"
	"\n"
	"    if (pass)\n"
	"        out_color = vec4(0.0f, 1.0f, 0.0f, 1.0f);\n"
	"    else\n"
	"        out_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);\n"
	"}\n";

/**
 * Render to all specified texture attachments of the multisample fbo
 * enabling alpha-to-coverage to make the implementation pass the additional
 * alpha component from the first attachment when rendering into the
 * second. Alpha value is being counted using current fragment coordinates
 * to cover more possible variations of sample mask and alpha.
 * The resulting sample mask will still be such as specified in
 * the fragment shader because an alpha value of 1.0 maps to the
 * coverage mask ~0.
 */
static bool
run_test(Fbo &fbo, int render_targets, int samples_num)
{
	glUseProgram(test_prog);
	glUniform1i(glGetUniformLocation(test_prog, "render_targets"),
		render_targets);
	glUniform1i(glGetUniformLocation(test_prog, "samples_num"),
		samples_num);
	glUniform1i(glGetUniformLocation(test_prog, "screen_width"),
		piglit_width);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo.handle);
	glDrawBuffers(fbo.config.num_tex_attachments,
		fbo.config.tex_attachment);

	glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
	fbo.set_viewport();

	glClear(GL_COLOR_BUFFER_BIT);
	piglit_draw_rect(-1, -1, 2, 2);

	glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);

	return piglit_check_gl_error(GL_NO_ERROR);
}

/**
 * Resolve the green component of each sample in case all checks are
 * done and equal to expected values.
 */
static bool
check(const Fbo &fbo, unsigned render_target,
	int render_targets_num, int samples_num)
{
	bool pass = true;
	const float expected[] = { 0.0f, 1.0f, 0.0f, 1.0f };

	glUseProgram(check_prog);
	glUniform1i(glGetUniformLocation(check_prog, "tex"), 0);
	glUniform1i(glGetUniformLocation(check_prog, "samples_num"),
		samples_num);
	glUniform1i(glGetUniformLocation(check_prog, "screen_width"),
		piglit_width);
	glUniform1i(glGetUniformLocation(check_prog, "screen_height"),
		piglit_height);
	glUniform1i(glGetUniformLocation(check_prog, "render_target"),
		render_target);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, fbo.color_tex[render_target]);
	glViewport(0, 0, piglit_width, piglit_height);

	glClear(GL_COLOR_BUFFER_BIT);
	piglit_draw_rect(-1, -1, 2, 2);

	pass &= piglit_check_gl_error(GL_NO_ERROR);
	pass &= piglit_probe_rect_rgba(0, 0, piglit_width, piglit_height,
			expected);

	if (!pass)
		printf("Test failed with %d samples, %d render targets, "
			"%d render target.\n",
			samples_num, render_targets_num, render_target);

	piglit_present_results();

	return pass;
}

static bool
run_check(int samples_num, int render_targets_num)
{
	bool pass = true;

	Fbo fbo = make_fbo(render_targets_num, samples_num);

	pass &= run_test(fbo, render_targets_num, samples_num);

	for (int render_target_to_check = render_targets_num - 1;
			render_target_to_check >= 0; render_target_to_check--) {
		pass &= check(fbo, render_target_to_check, render_targets_num,
			samples_num);
	}

	return pass;
}

static void
print_usage_and_exit(char *prog_name)
{
	printf("%s Should be used without arguments or with: <samples_num>"
		" <render_targets_num>\n",
		prog_name);

	piglit_report_result(PIGLIT_FAIL);
}

void delete_programs()
{
	glDeleteProgram(test_prog);
	glDeleteProgram(check_prog);
}

void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_ARB_texture_multisample");
	piglit_require_extension("GL_ARB_sample_shading");

	glGetIntegerv(GL_MAX_SAMPLES, &max_samples_num);
	glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &max_attachments_num);

	/* 32 will have too many permutations */
	max_samples_num = max_samples_num > 16 ? 16 : max_samples_num;

	if (argc == 1) {
		should_all_be_checked = true;
	} else if (argc == 3) {
		should_all_be_checked = false;

		/* arg1 - samples_num */
		char *end_ptr = NULL;
		num_samples_to_check = strtol(argv[1], &end_ptr, 0);
		if (end_ptr != argv[1] + strlen(argv[1]))
			print_usage_and_exit(argv[0]);

		/* arg2 - render_targets_num */
		end_ptr = NULL;
		num_render_targets_to_check = strtol(argv[2], &end_ptr, 0);
		if (end_ptr != argv[2] + strlen(argv[2]))
			print_usage_and_exit(argv[0]);

		if (num_samples_to_check > max_samples_num ||
			num_render_targets_to_check > max_attachments_num) {
			printf("Error max supported samples = %d, max supported "
				"color attachments = %d \n",
				max_samples_num, max_attachments_num);
			piglit_report_result(PIGLIT_FAIL);
		}
	} else {
		print_usage_and_exit(argv[0]);
	}

	test_prog = piglit_build_simple_program(test_vs, test_fs);
	check_prog = piglit_build_simple_program(check_vs, check_fs);

	atexit(delete_programs);
}

piglit_result
piglit_display()
{
	bool pass = true;

	if (should_all_be_checked) {
		for (int samples_num = 1; samples_num <= max_samples_num;
			samples_num *= 2) {
			pass &= run_check(samples_num, 1) &&
					run_check(samples_num, 2) &&
					run_check(samples_num, max_attachments_num);
		}
	} else {
		pass &= run_check(num_samples_to_check, num_render_targets_to_check);
	}

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
