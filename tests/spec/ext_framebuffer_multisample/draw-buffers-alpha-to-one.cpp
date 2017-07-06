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
 * \file draw-buffers-alpha-to-one.cpp
 *
 * Verify sample alpha to one with multiple draw buffers
 *
 * When rendering to multiple draw buffers, GL_SAMPLE_ALPHA_TO_ONE should
 * modify the alpha values sent to each draw buffer. OpenGL 3.3 specification's
 * section 4.1.3 (page 196 of pdf) is little unclear about the exact behavior
 * in above mentioned case:
 *
 * "All alpha values in this section refer only to the alpha component of the
 * fragment shader output linked to color number zero, index zero (see section
 * 3.9.2) if a fragment shader is in use, or the alpha component of the result
 * of fixed-function fragment shading. If the fragment shader does not write to
 * this output, the alpha value is undefined."
 *
 * And later in the same section it is stated that:
 *
 * "Next, if SAMPLE_ALPHA_TO_ONE is enabled, each alpha value is replaced by
 * the maximum representable alpha value for fixed-point color buffers, or by
 * 1.0 for floating-point buffers. Otherwise, the alpha values are not changed."
 *
 * By reading above two references together, specification seems to suggest that
 * alpha values for only draw buffer zero will be modified when GL_SAMPLE_TO_ONE.
 * is enabled. But with NVIDIA's proprietary drivers this test verifies that
 * alpha values for all the draw buffers will be modified. In my opinion, this
 * section needs clarification from khronos.
 *
 * At present test is based on the behavior observed with NVIDIA's proprietary
 * drivers.
 *
 * This test operates by drawing a pattern in multisample FBO to generate
 * reference and test images for all the draw buffers. Reference images are drawn
 * to right half of window system draw buffer and test images to left half.
 *
 * Compare the left and right halves of window system frame buffer to verify
 * the test image.
 *
 * Author: Anuj Phogat <anuj.phogat@gmail.com>
 */

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 512;
	config.window_height = 768;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

static int samples;

void
print_usage_and_exit(char *prog_name)
{
        printf("Usage: %s <num_samples>\n", prog_name);
	piglit_report_result(PIGLIT_FAIL);
}

void
piglit_init(int argc, char **argv)
{
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

	piglit_require_gl_version(21);
	piglit_require_extension("GL_ARB_framebuffer_object");
	piglit_require_extension("GL_ARB_vertex_array_object");

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

	draw_reference_image(false /* sample_alpha_to_coverage */,
			     true /* sample_alpha_to_one */);

	draw_test_image(false /* sample_alpha_to_coverage */,
			true /* sample_alpha_to_one */);

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	glBindFramebuffer(GL_READ_FRAMEBUFFER, piglit_winsys_fbo);
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
