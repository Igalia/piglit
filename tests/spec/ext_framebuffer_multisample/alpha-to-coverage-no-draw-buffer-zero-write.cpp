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

#include "draw-buffers-common.h"

/**
 * \file alpha-to-coverage-no-draw-buffer-zero-write.cpp
 *
 * Verify sample alpha to coverage with multiple draw buffers when nothing
 * is written to draw buffer zero.
 *
 * When nothing is written to draw buffer zero, GL_SAMPLE_ALPHA_TO_COVERAGE
 * usage shouldn't hang the system. The alpha value used to determine
 * coverage will be undefined which will result in to pixels with undefined
 * colors. So, pixels can't be probed for color in this test.
 *
 * From OpenGL 2.1 specification:
 * "If a fragment shader writes to neither gl FragColor nor gl FragData,
 * the values of the fragment colors following shader execution are
 * undefined, and may differ for each fragment color."
 *
 * It is a significant edge case for i965 driver.
 */

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 21;

	config.window_width = 512;
	config.window_height = 768;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

void
print_usage_and_exit(char *prog_name)
{
	printf("Usage: %s <num_samples>\n", prog_name);
	piglit_report_result(PIGLIT_FAIL);
}

void
piglit_init(int argc, char **argv)
{
	int samples;
	/* At present fragment shader supports only fixed number of
	 * attachments (3)
	 */
	int num_attachments = 3;

	if (argc < 2)
		print_usage_and_exit(argv[0]);

	{
		char *endptr = NULL;
		samples = strtol(argv[1], &endptr, 0);
		if (endptr != argv[1] + strlen(argv[1]))
			print_usage_and_exit(argv[0]);
	}

	piglit_require_extension("GL_ARB_framebuffer_object");
	piglit_require_extension("GL_ARB_vertex_array_object");
	piglit_require_extension("GL_EXT_framebuffer_multisample");

	int pattern_width = piglit_width / 2;
	int pattern_height = piglit_height / num_attachments;

	piglit_ortho_projection(pattern_width,
				pattern_height,
				GL_TRUE);

	/* Skip the test if samples > GL_MAX_SAMPLES */
	GLint max_samples;
	glGetIntegerv(GL_MAX_SAMPLES, &max_samples);

	if (samples < 1 || samples > max_samples)
		piglit_report_result(PIGLIT_SKIP);

	ms_fbo_and_draw_buffers_setup(samples,
				      pattern_width,
				      pattern_height,
				      num_attachments,
				      GL_COLOR_BUFFER_BIT,
				      GL_RGBA /* color_buffer_zero_format */);
	shader_compile(true /* sample_alpha_to_coverage */,
		       false /* dual_src_blend */,
		       false /* frag_out_zero_write */);
}

enum piglit_result
piglit_display()
{
	bool pass;

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	allocate_data_arrays();

	draw_test_image(true /* sample_alpha_to_coverage */,
			false /* sample_alpha_to_one */);

	pass = piglit_check_gl_error(GL_NO_ERROR);

	/* Free the memory allocated for data arrays */
	free_data_arrays();

	if (!piglit_automatic)
		piglit_present_results();

	/* The fragment colors are undefined in this test. So, we can't probe
	 * the pixels. If test executes to completion without error, return
	 * PIGLIT_PASS.
	 */
	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
