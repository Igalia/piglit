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

/**
 * \file clip-and-scissor-blit.cpp
 *
 * Verify the accuracy of blits incolving MSAA buffers when the blit
 * coordinates are clipped to the edges of the source or destination
 * surface, or scissored.
 *
 * The test starts by creating a source framebuffer and populating it
 * with a simple image.  It also creates a destination framebuffer.
 *
 * Then, it executes the following sequence of steps several times in
 * a loop:
 *
 * 1. Clear the destination framebuffer to gray.
 *
 * 2. Blit from the source framebuffer to the destination framebuffer,
 *    using clipping or scissoring to limit the amount of data that is
 *    blitted.
 *
 * 3. Do a simple (unclipped, unscissored) blit from the destination
 *    framebuffer to the screen.  This produces a test image.
 *
 * 4. Clear the destination framebuffer to gray.
 *
 * 5. Blit from the source framebuffer to the destination framebuffer,
 *    this time adjusting the coordinates to limit the amount of data
 *    that is blitted.
 *
 * 6. Do a simple (unclipped, unscissored) blit from the destination
 *    framebuffer to the screen.  This produces a reference image.
 *
 * 7. Verify that the test and reference images match.
 */
#include "piglit-fbo.h"
using namespace piglit_util_fbo;

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 600;
	config.window_height = 320;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

namespace {

const int src_size[2] = { 30, 20 };
const int dst_size[2] = { 50, 40 };
const int cut_amount[2] = { 10, 7 };

enum test_type_enum
{
	TEST_TYPE_SRC,
	TEST_TYPE_DST,
	TEST_TYPE_SCISSOR,
	NUM_TEST_TYPES
};

Fbo src_fbo;
Fbo dst_fbo;

/**
 * From the GL 3.3 spec (section 4.3.2 Copying Pixels):
 *
 * If SAMPLE_BUFFERS for either the read framebuffer or draw
 * framebuffer is greater than zero, no copy is performed and an
 * INVALID_OPERATION error is generated if the dimensions of the
 * source and destination rectangles provided to BlitFramebuffer are
 * not identical, if the formats of the read and draw framebuffers are
 * not identical, or if the values of SAMPLES for the read and draw
 * buffers are not identical.
 *
 * It is not 100% clear whether "the dimensions of the source and
 * destination rectangles" are meant in a signed sense or an unsigned
 * sense--in other words, if SAMPLE_BUFFERS is greater than zero for
 * either the read or draw framebuffer, and abs(srcX0 - srcX1) ==
 * abs(dstX0 - dstX1), but (srcX0 - srcX1) and (dstX0 - dstX1) have
 * opposite signs (so that the image is being mirrored in the X
 * direction), should an INVALID_OPERATION error be generated?
 *
 * Some implementations have interpreted the answer to be yes, so we
 * only test clipping of mirrored blits when SAMPLE_BUFFERS is 0 for
 * both the read and draw framebuffers.
 *
 * This boolean is true if we should test clipping of mirrored blits.
 */
bool test_mirrored_blits = false;

void
draw_simple_src_image()
{
	glColor4f(1.0, 0.0, 0.0, 0.0);
	piglit_draw_rect(-1, -1, 1, 1);

	glColor4f(0.0, 1.0, 0.0, 0.25);
	piglit_draw_rect(0, -1, 1, 1);

	glColor4f(0.0, 0.0, 1.0, 0.5);
	piglit_draw_rect(-1, 0, 1, 1);

	glColor4f(1.0, 1.0, 1.0, 1.0);
	piglit_draw_rect(0, 0, 1, 1);
}

bool
do_test(int coord, bool clip_low, test_type_enum test_type,
	bool flip_src, bool flip_dst)
{
	/* If this test flips src but not dst (or vice versa), then it
	 * is unclear from the spec whether it should be allowed for
	 * multisampled blits, so skip it unless test_mirrored_blits
	 * is true.
	 */
	if (flip_src != flip_dst && !test_mirrored_blits)
		return true;

	/* Figure out where to draw the images */
	int display_x = (6 * coord + 2 * test_type) * dst_size[0];
	int display_y = ((clip_low ? 4 : 0) + (flip_src ? 2 : 0) +
			 (flip_dst ? 1 : 0)) * dst_size[1];

	static const char * const test_type_strings[] = {
		"clip src",
		"clip dst",
		"scissor"
	};
	printf("Testing %s %s%s%s%s at (%d, %d)\n",
	       test_type_strings[test_type],
	       clip_low ? "-" : "+",
	       coord ? "y" : "x",
	       flip_src ? " (flip src)" : "",
	       flip_dst ? " (flip dst)" : "",
	       display_x, display_y);

	/* Number of pixels we'll try to cut out of the blit by
	 * clipping or scissoring.
	 */
	int cut = cut_amount[coord];

	/* Amount by which the blits must be offset to produce an
	 * image in the center of the destination fbo.
	 */
	int dx = (dst_size[0] - src_size[0]) / 2;
	int dy = (dst_size[1] - src_size[1]) / 2;

	/* Set up blit and scissor parameters for both the test and
	 * reference blits
	 */
	int test_src[2][2] = { /* E.g. test_src[1][0] == srcY0 */
		{ 0, src_size[0] }, { 0, src_size[1] } };
	int test_dst[2][2] = { /* E.g. test_dst[1][0] == dstY0 */
		{ dx, src_size[0] + dx }, { dy, src_size[1] + dy } };
	int ref_src[2][2] = { /* E.g. test_src[1][0] == srcY0 */
		{ 0, src_size[0] }, { 0, src_size[1] } };
	int ref_dst[2][2] = { /* E.g. test_dst[1][0] == dstY0 */
		{ dx, src_size[0] + dx }, { dy, src_size[1] + dy } };
	int scissor[2][2] = { /* E.g. scissor[0] = { left, right } */
		{ 0, 0 }, { 0, 0 } };
	switch (test_type) {
	case TEST_TYPE_SRC:
		if (clip_low) {
			test_src[coord][0] += cut;
			test_src[coord][1] += cut;
			ref_src[coord][0] += cut;
			ref_dst[coord][1] -= cut;
		} else {
			test_src[coord][0] -= cut;
			test_src[coord][1] -= cut;
			ref_src[coord][1] -= cut;
			ref_dst[coord][0] += cut;
		}
		break;
	case TEST_TYPE_DST:
		if (clip_low) {
			test_dst[coord][0] = -cut;
			test_dst[coord][1] =
				test_dst[coord][0] + src_size[coord];
			ref_src[coord][0] = cut;
			ref_dst[coord][0] = 0;
			ref_dst[coord][1] = test_dst[coord][1];
		} else {
			test_dst[coord][1] = dst_size[coord] + cut;
			test_dst[coord][0] =
				test_dst[coord][1] - src_size[coord];
			ref_src[coord][1] = src_size[coord] - cut;
			ref_dst[coord][0] = test_dst[coord][0];
			ref_dst[coord][1] = dst_size[coord];
		}
		break;
	case TEST_TYPE_SCISSOR:
		if (clip_low) {
			scissor[coord][0] = test_dst[coord][0] + cut;
			scissor[coord][1] = dst_size[coord];
			ref_src[coord][0] += cut;
			ref_dst[coord][0] += cut;
		} else {
			scissor[coord][0] = 0;
			scissor[coord][1] = test_dst[coord][1] - cut;
			ref_src[coord][1] -= cut;
			ref_dst[coord][1] -= cut;
		}
		scissor[1-coord][0] = 0;
		scissor[1-coord][1] = dst_size[1-coord];
		break;
	default:
		printf("Unexpected test type\n");
		piglit_report_result(PIGLIT_FAIL);
		break;
	}

	/* Flip coordinates if requested */
	if (flip_src) {
		test_src[coord][0] = src_size[coord] - test_src[coord][0];
		test_src[coord][1] = src_size[coord] - test_src[coord][1];
		ref_src[coord][0] = src_size[coord] - ref_src[coord][0];
		ref_src[coord][1] = src_size[coord] - ref_src[coord][1];
	}
	if (flip_dst) {
		test_dst[coord][0] = dst_size[coord] - test_dst[coord][0];
		test_dst[coord][1] = dst_size[coord] - test_dst[coord][1];
		ref_dst[coord][0] = dst_size[coord] - ref_dst[coord][0];
		ref_dst[coord][1] = dst_size[coord] - ref_dst[coord][1];
		int tmp = scissor[coord][0];
		scissor[coord][0] = dst_size[coord] - scissor[coord][1];
		scissor[coord][1] = dst_size[coord] - tmp;
	}

	/* Clear the destination framebuffer to gray */
	glBindFramebuffer(GL_READ_FRAMEBUFFER, src_fbo.handle);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dst_fbo.handle);
	glClearColor(0.5, 0.5, 0.5, 0.5);
	glClear(GL_COLOR_BUFFER_BIT);

	/* Set up scissor */
	glScissor(scissor[0][0],
		  scissor[1][0],
		  scissor[0][1] - scissor[0][0],
		  scissor[1][1] - scissor[1][0]);
	if (test_type == TEST_TYPE_SCISSOR)
		glEnable(GL_SCISSOR_TEST);
	else
		glDisable(GL_SCISSOR_TEST);

	/* Do the test blit */
	glBlitFramebuffer(test_src[0][0], test_src[1][0],
			  test_src[0][1], test_src[1][1],
			  test_dst[0][0], test_dst[1][0],
			  test_dst[0][1], test_dst[1][1],
			  GL_COLOR_BUFFER_BIT, GL_NEAREST);

	/* Disable scissoring */
	glDisable(GL_SCISSOR_TEST);

	/* Transfer the test image to the screen */
	glBindFramebuffer(GL_READ_FRAMEBUFFER, dst_fbo.handle);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
	glBlitFramebuffer(0, 0, dst_size[0], dst_size[1],
			  display_x, display_y,
			  display_x + dst_size[0], display_y + dst_size[1],
			  GL_COLOR_BUFFER_BIT, GL_NEAREST);

	/* Clear the destination framebuffer to gray */
	glBindFramebuffer(GL_READ_FRAMEBUFFER, src_fbo.handle);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dst_fbo.handle);
	glClearColor(0.5, 0.5, 0.5, 0.5);
	glClear(GL_COLOR_BUFFER_BIT);

	/* Do the reference blit */
	glBlitFramebuffer(ref_src[0][0], ref_src[1][0],
			  ref_src[0][1], ref_src[1][1],
			  ref_dst[0][0], ref_dst[1][0],
			  ref_dst[0][1], ref_dst[1][1],
			  GL_COLOR_BUFFER_BIT, GL_NEAREST);

	/* Transfer the reference image to the screen */
	glBindFramebuffer(GL_READ_FRAMEBUFFER, dst_fbo.handle);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
	glBlitFramebuffer(0, 0, dst_size[0], dst_size[1],
			  display_x + dst_size[0], display_y,
			  display_x + 2 * dst_size[0], display_y + dst_size[1],
			  GL_COLOR_BUFFER_BIT, GL_NEAREST);

	/* Compare the test and reference images */
	glBindFramebuffer(GL_READ_FRAMEBUFFER, piglit_winsys_fbo);
	return piglit_probe_rect_halves_equal_rgba(display_x, display_y,
						   2 * dst_size[0],
						   dst_size[1]);
}

NORETURN void
print_usage_and_exit(char *prog_name)
{
	printf("Usage: %s <num_samples> <blit_type>\n"
	       "  where <blit_type> is one of:\n"
	       "    msaa\n"
	       "    upsample\n"
	       "    downsample\n"
	       "    normal\n",
	       prog_name);
	piglit_report_result(PIGLIT_FAIL);
}

extern "C" void
piglit_init(int argc, char **argv)
{
	piglit_require_gl_version(21);
	piglit_require_extension("GL_ARB_framebuffer_object");
	piglit_require_extension("GL_ARB_vertex_array_object");

	if (argc < 3)
		print_usage_and_exit(argv[0]);

	int num_samples;
	{
		char *endptr = NULL;
		num_samples = strtol(argv[1], &endptr, 0);
		if (endptr != argv[1] + strlen(argv[1]))
			print_usage_and_exit(argv[0]);
	}

	/* Skip the test if num_samples > GL_MAX_SAMPLES */
	GLint max_samples;
	glGetIntegerv(GL_MAX_SAMPLES, &max_samples);
	if (num_samples > max_samples)
		piglit_report_result(PIGLIT_SKIP);

	int src_samples;
	int dst_samples;
	if (strcmp(argv[2], "msaa") == 0) {
		src_samples = dst_samples = num_samples;
	} else if (strcmp(argv[2], "upsample") == 0) {
		src_samples = 0;
		dst_samples = num_samples;
	} else if (strcmp(argv[2], "downsample") == 0) {
		src_samples = num_samples;
		dst_samples = 0;
	} else if (strcmp(argv[2], "normal") == 0) {
		src_samples = dst_samples = 0;
		test_mirrored_blits = true;
	} else {
		print_usage_and_exit(argv[0]);
	}

	src_fbo.setup(FboConfig(src_samples, src_size[0], src_size[1]));
	dst_fbo.setup(FboConfig(dst_samples, dst_size[0], dst_size[1]));
}

extern "C" enum piglit_result
piglit_display()
{
	/* Draw a simple image in the source buffer */
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, src_fbo.handle);
	src_fbo.set_viewport();
	draw_simple_src_image();

	bool pass = true;
	for (int coord = 0; coord < 2; ++coord) {
		for (int clip_low = 0; clip_low < 2; ++clip_low) {
			for (int test_type = 0; test_type < NUM_TEST_TYPES; ++test_type) {
				for (int flip_src = 0; flip_src < 2; ++flip_src) {
					for (int flip_dst = 0; flip_dst < 2; ++flip_dst) {
						pass = do_test(coord, clip_low, test_type_enum(test_type),
							       flip_src, flip_dst) && pass;
					}
				}
			}
		}
	}

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

} /* anonymous namespace */
