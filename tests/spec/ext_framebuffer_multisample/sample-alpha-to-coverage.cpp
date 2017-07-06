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
 * \file sample-alpha-to-coverage.cpp
 *
 * Verify sample alpha to coverage with multisample FBO
 *
 * When rendering to multisample FBO, fragment's alpha value should be
 * used to determine the coverage value.
 *
 * This test operates by drawing a pattern in multisample FBO to generate
 * reference and test image. Reference image is drawn to right half of window
 * system frame buffer and test image to left half.
 *
 * Compute the expected color / depth values.
 *
 * Probe color / depth buffer blitted to downsampled FBO (resolve_fbo) and
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

static GLenum buffer_to_test;

void
print_usage_and_exit(char *prog_name)
{
	printf("Usage: %s <num_samples> <test_type>\n"
	       "  where <test_type> is one of:\n"
	       "    color\n"
	       "    depth\n",
	       prog_name);
	piglit_report_result(PIGLIT_FAIL);
}

void
piglit_init(int argc, char **argv)
{
	const int num_attachments = 1;
	int samples;

	piglit_require_gl_version(21);
	piglit_require_extension("GL_ARB_framebuffer_object");
	piglit_require_extension("GL_ARB_vertex_array_object");

	if (argc < 3)
		print_usage_and_exit(argv[0]);
	{
		char *endptr = NULL;
		samples = strtol(argv[1], &endptr, 0);
		if (endptr != argv[1] + strlen(argv[1]))
			print_usage_and_exit(argv[0]);
	}
	if (strcmp(argv[2], "color") == 0) {
		buffer_to_test = GL_COLOR_BUFFER_BIT;
	} else if (strcmp(argv[2], "depth") == 0) {
		buffer_to_test = GL_DEPTH_BUFFER_BIT;
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_ALWAYS);
	} else
		print_usage_and_exit(argv[0]);

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
				      buffer_to_test,
				      GL_RGBA);
	shader_compile(true /* sample_alpha_to_coverage */,
		       false /* dual_src_blend */,
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

	/* Reference image drawn here doesn't represent an expected image.
	 * Reference image is drawn only to visualize the image difference
	 * caused by enabling sample_alpha_to_coverage in test image.
	 */
	if(buffer_to_test == GL_COLOR_BUFFER_BIT)
		draw_reference_image(true /* sample_alpha_to_coverage */,
				     false /* sample_alpha_to_one */);

	draw_test_image(true /* sample_alpha_to_coverage */,
			false /* sample_alpha_to_one */);

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	/* Probe all the test image blitted to resolve fbo and compare with
	 * expected color values.  This method of verification is appropriate
	 * for tests with sample-alpha-to-coverage enabled. Possibility of
	 * dithering effect when the coverage value is not a strict multiple
	 * of 1 / num_samples makes image compare (test / reference image)
	 * unsuitable for this test.
	 */
	if(buffer_to_test == GL_COLOR_BUFFER_BIT)
		pass = probe_framebuffer_color() && pass;
	else if (buffer_to_test == GL_DEPTH_BUFFER_BIT)
		pass = probe_framebuffer_depth() && pass;

	/* Free the memory allocated for data arrays */
	free_data_arrays();

	if (!piglit_automatic && buffer_to_test == GL_COLOR_BUFFER_BIT)
		piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
