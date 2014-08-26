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
#include "common.h"

/**
 * \file accuracy.c
 *
 * Verify the accuracy of multisample antialiasing.
 *
 * This test utilizes the functions defined in common.cpp to verfify the
 * accuracy of MSAA.
 *
 * The test also accepts the following flags:
 *
 * - "small": Causes the MSAA image to be renedered in extremely tiny
 *   (16x16) tiles that are then stitched together.  This verifies
 *   that MSAA works properly on very small buffers (a critical corner
 *   case on i965).
 *
 * - "depthstencil": Causes the framebuffers to use a combined
 *   depth/stencil buffer (as opposed to separate depth and stencil
 *   buffers).  On some implementations (e.g. the nVidia proprietary
 *   driver for Linux) this is necessary for framebuffer completeness.
 *   On others (e.g. i965), this is an important corner case to test.
 */

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 512;
	config.window_height = 256;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

const int pattern_width = 256; const int pattern_height = 256;
const int supersample_factor = 16;
int num_samples, max_samples;
bool small = false, combine_depth_stencil = false;
bool all_samples = false;
GLenum filter_mode = GL_NEAREST;
test_type_enum test_type;
Test *test = NULL;

NORETURN void
print_usage_and_exit(char *prog_name)
{
	printf("Usage: %s <sample_arg> <test_type> [options]\n"
	       "  where <sample_arg> is one of:\n"
	       "    <num_samples>: test supplied sample count\n"
	       "    all_samples: test all power of 2 samples\n"
	       "  where <test_type> is one of:\n"
	       "    color: test downsampling of color buffer\n"
	       "    srgb: test downsampling of srgb color buffer\n"
	       "    stencil_draw: test drawing using MSAA stencil buffer\n"
	       "    stencil_resolve: test resolve of MSAA stencil buffer\n"
	       "    depth_draw: test drawing using MSAA depth buffer\n"
	       "    depth_resolve: test resolve of MSAA depth buffer\n"
	       "Available options:\n"
	       "    small: use a very small (16x16) MSAA buffer\n"
	       "    depthstencil: use a combined depth/stencil buffer\n"
	       "    linear: use GL_LINEAR filter mode\n",
	       prog_name);
	piglit_report_result(PIGLIT_FAIL);
}

void
piglit_init(int argc, char **argv)
{
	int i;

	if (argc < 3)
		print_usage_and_exit(argv[0]);
	{
		char *endptr = NULL;
		if (streq(argv[1], "all_samples"))
			all_samples = true;
		else {
			num_samples = strtol(argv[1], &endptr, 0);
			if (endptr != argv[1] + strlen(argv[1]))
				print_usage_and_exit(argv[0]);
		}
	}

	for (i = 3; i < argc; ++i) {
		if (strcmp(argv[i], "small") == 0) {
			small = true;
		} else if (strcmp(argv[i], "depthstencil") == 0) {
			combine_depth_stencil = true;
		} else if (strcmp(argv[i], "linear") == 0) {
			filter_mode = GL_LINEAR;
		} else {
			print_usage_and_exit(argv[0]);
		}
	}

	piglit_require_gl_version(21);
	piglit_require_extension("GL_ARB_framebuffer_object");
	piglit_require_extension("GL_ARB_vertex_array_object");

	/* Skip the test if num_samples > GL_MAX_SAMPLES */
	glGetIntegerv(GL_MAX_SAMPLES, &max_samples);
	if (num_samples > max_samples)
		piglit_report_result(PIGLIT_SKIP);

	if (strcmp(argv[2], "color") == 0) {
		test_type = TEST_TYPE_COLOR;
	} else if (strcmp(argv[2], "srgb") == 0) {
		test_type = TEST_TYPE_SRGB;
	} else if (strcmp(argv[2], "stencil_draw") == 0) {
		test_type = TEST_TYPE_STENCIL_DRAW;
	} else if (strcmp(argv[2], "stencil_resolve") == 0) {
		test_type = TEST_TYPE_STENCIL_RESOLVE;
	} else if (strcmp(argv[2], "depth_draw") == 0) {
		test_type = TEST_TYPE_DEPTH_DRAW;
	} else if (strcmp(argv[2], "depth_resolve") == 0) {
		test_type = TEST_TYPE_DEPTH_RESOLVE;
	} else {
		print_usage_and_exit(argv[0]);
	}
}

bool
test_create_and_execute()
{
	test = create_test(test_type, num_samples, small,
			   combine_depth_stencil,
			   pattern_width, pattern_height, supersample_factor,
			   filter_mode);
	return test->run();
}

enum piglit_result
piglit_display()
{
	bool pass = true, result = pass;

	if (!all_samples) {
		pass = test_create_and_execute() && pass;
		pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
		piglit_present_results();
		return pass ? PIGLIT_PASS : PIGLIT_FAIL;
	}

	for (num_samples = 0; num_samples <= max_samples; ) {
		result = test_create_and_execute();
		result = piglit_check_gl_error(GL_NO_ERROR) && result;
		printf("Samples = %d, Result = %s\n\n",
		       num_samples, result ? "pass" : "fail");
		pass = result && pass;
		/* Test only power of 2 samples */
		num_samples = num_samples ? num_samples << 1: num_samples + 2;
		piglit_present_results();
	}

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
