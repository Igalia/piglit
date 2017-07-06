/*
 * Copyright © 2012 Intel Corporation
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
 * \file alpha-to-coverage-dual-src-blend.cpp
 *
 * Verify sample-alpha-to-coverage with dual source blending.
 *
 * This test operates by drawing a pattern in single sample FBO to generate
 * a reference and test image. Reference image is drawn to the right half of
 * window system draw buffer and test image to the left half.
 *
 * Compute the expected color values. Probe window system color buffer and
 * compare against expected color values.
 *
 * Author: Anuj Phogat <anuj.phogat@gmail.com>
 */

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 512;
	config.window_height = 256;
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
	const int num_attachments = 1;

	if (argc < 2)
		print_usage_and_exit(argv[0]);
	{
		char *endptr = NULL;
		samples = strtol(argv[1], &endptr, 0);
		if (endptr != argv[1] + strlen(argv[1]))
			print_usage_and_exit(argv[0]);
	}

	piglit_require_gl_version(30);

	int pattern_width = piglit_width / 2;
	int pattern_height = piglit_height / num_attachments;

	piglit_ortho_projection(pattern_width,
				pattern_height,
				GL_TRUE);

	/* Skip the test if samples > GL_MAX_SAMPLES */
	GLint max_samples;
	glGetIntegerv(GL_MAX_SAMPLES, &max_samples);

	if (samples > max_samples)
		piglit_report_result(PIGLIT_SKIP);

	ms_fbo_and_draw_buffers_setup(samples,
				      pattern_width,
				      pattern_height,
				      num_attachments,
				      GL_COLOR_BUFFER_BIT,
				      GL_RGBA);

	shader_compile(true /* sample_alpha_to_coverage */,
		       true /* dual_src_blend */,
		       true /* frag_out_zero_write */);
}

enum piglit_result
piglit_display()
{
	bool pass = true;
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	allocate_data_arrays();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC1_ALPHA);

	/* Reference image drawn when sample_alpha_to_coverage is enabled,
	 * doesn't represent an expected image. Reference image is drawn only
	 * to visualize the image difference caused by enabling
	 * sample_alpha_to_coverage
	 */
	draw_reference_image(true /* sample_alpha_to_coverage */,
			     false /* sample_alpha_to_one */);

	draw_test_image(true /* sample_alpha_to_coverage */,
			false /* sample_alpha_to_one */);

	glDisable(GL_BLEND);
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	/* Probe test images of all the draw_buffers blitted to resolve fbo
	 * and compare with expected color values.  This method of verification
	 * is appropriate for tests with sample-alpha-to-coverage enabled.
	 * Possibility of dithering effect when the coverage value is not a
	 * strict multiple of 1 / num_samples makes image compare (test /
	 * reference image) unsuitable for this test.
	 */
	pass = probe_framebuffer_color() && pass;

	/* Free the memory allocated for data arrays */
	free_data_arrays();

	if (!piglit_automatic)
		piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
