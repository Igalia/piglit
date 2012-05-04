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
int piglit_width = 512; int piglit_height = 256;
int piglit_window_mode = GLUT_DOUBLE | GLUT_RGBA | GLUT_ALPHA;

const int pattern_width = 256; const int pattern_height = 256;
const int supersample_factor = 16;
Test *test = NULL;

void
print_usage_and_exit(char *prog_name)
{
	printf("Usage: %s <num_samples> <test_type> [options]\n"
	       "  where <test_type> is one of:\n"
	       "    color: test downsampling of color buffer\n"
	       "    stencil_draw: test drawing using MSAA stencil buffer\n"
	       "    stencil_resolve: test resolve of MSAA stencil buffer\n"
	       "    depth_draw: test drawing using MSAA depth buffer\n"
	       "    depth_resolve: test resolve of MSAA depth buffer\n"
	       "Available options:\n"
	       "    small: use a very small (16x16) MSAA buffer\n"
	       "    depthstencil: use a combined depth/stencil buffer\n",
	       prog_name);
	piglit_report_result(PIGLIT_FAIL);
}

void
piglit_init(int argc, char **argv)
{
	GLint max_samples;
	int i, num_samples;
	bool small = false;
	bool combine_depth_stencil = false;

	if (argc < 3)
		print_usage_and_exit(argv[0]);
	{
		char *endptr = NULL;
		num_samples = strtol(argv[1], &endptr, 0);
		if (endptr != argv[1] + strlen(argv[1]))
			print_usage_and_exit(argv[0]);
	}

	for (i = 3; i < argc; ++i) {
		if (strcmp(argv[i], "small") == 0) {
			small = true;
		} else if (strcmp(argv[i], "depthstencil") == 0) {
			combine_depth_stencil = true;
		} else {
			print_usage_and_exit(argv[0]);
		}
	}

	piglit_require_gl_version(30);
	piglit_require_GLSL_version(130);

	/* Skip the test if num_samples > GL_MAX_SAMPLES */
	glGetIntegerv(GL_MAX_SAMPLES, &max_samples);
	if (num_samples > max_samples)
		piglit_report_result(PIGLIT_SKIP);

	test_type_enum test_type;
	if (strcmp(argv[2], "color") == 0) {
		test_type = TEST_TYPE_COLOR;
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
	test = create_test(test_type, num_samples, small,
			   combine_depth_stencil,
			   pattern_width, pattern_height, supersample_factor);
}

enum piglit_result
piglit_display()
{
	enum piglit_result result = test->run() ? PIGLIT_PASS : PIGLIT_FAIL;

	piglit_present_results();

	return result;
}
