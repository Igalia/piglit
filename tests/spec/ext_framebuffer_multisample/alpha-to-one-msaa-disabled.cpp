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
 * \file alpha-to-one-msaa-disabled.cpp
 *
 * Verify that alpha values are not modified if GL_SAMPLE_ALPHA_TO_ONE is
 * enabled and GL_MULTISAMPLE is disabled in a multisample buffer.
 *
 * This test operates by drawing a pattern in multisample FBO to generate
 * reference and test image. Reference image is drawn to right half of window
 * system frame buffer and test image to left half.
 *
 * Compare the left and right halves of window system frame buffer to verify
 * the test image.
 *
 * Author: Anuj Phogat <anuj.phogat@gmail.com>
 */

PIGLIT_GL_TEST_MAIN(512 /*window_width*/,
		    256 /*window_height*/,
		    GLUT_DOUBLE | GLUT_RGBA | GLUT_ALPHA)

void
print_usage_and_exit(char *prog_name)
{
        printf("Usage: %s <num_samples>\n", prog_name);
	piglit_report_result(PIGLIT_FAIL);
}

void
piglit_init(int argc, char **argv)
{
	int num_attachments = 1;
	int samples;

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
	shader_compile(false /* sample_alpha_to_coverage */,
		       false /* dual_src_blend */);
}

enum piglit_result
piglit_display()
{
	bool pass = true;
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	allocate_data_arrays();

	/* Set sample_alpha_to_one = false to generate the reference image */
	draw_reference_image(false /* sample_alpha_to_coverage */,
			     false /* sample_alpha_to_one */);

	/* Test multisample fbo with GL_SAMPLE_ALPHA_TO_ONE enabled but
	 * GL_MULTISAMPLE disabled */
	glDisable(GL_MULTISAMPLE);

	draw_test_image(false /* sample_alpha_to_coverage */,
			true /* sample_alpha_to_one */);

	glEnable(GL_MULTISAMPLE);

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	pass = piglit_probe_rect_halves_equal_rgba(0, 0,
						   piglit_width,
						   piglit_height)
	       && pass;
	/* Free the memory allocated for data arrays */
	free_data_arrays();

	if (!piglit_automatic)
		piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
