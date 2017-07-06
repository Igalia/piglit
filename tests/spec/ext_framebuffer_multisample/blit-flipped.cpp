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

/** \file blit-flipped.cpp
 *
 * From the GL 4.3 spec, section 18.3.1 "Blitting Pixel Rectangles":
 *
 *     If SAMPLE_BUFFERS for either the read framebuffer or draw
 *     framebuffer is greater than zero, no copy is performed and an
 *     INVALID_OPERATION error is generated if the dimensions of the
 *     source and destination rectangles provided to BlitFramebuffer
 *     are not identical, or if the formats of the read and draw
 *     framebuffers are not identical.
 *
 * It is not clear from the spec whether "dimensions" should mean both
 * sign and magnitude, or just magnitude.  However, Y flips are likely
 * to be commonplace in OpenGL applications that have been ported from
 * DirectX applications, as a result of the fact that DirectX and
 * OpenGL differ in their orientation of the Y axis.  Furthermore, at
 * least one commercial driver (nVidia) permits Y flips, and L4D2
 * relies on them being permitted.  So it seems prudent to assume that
 * "dimensions" means just magnitude, not sign.
 *
 * This test verifies that a blit from a multisampled buffer to a
 * single-sampled buffer is permitted to flip either in the X or Y
 * direction, and that the resulting image is the same as what would
 * be obtained by doing a non-flipped blit to a single-sampled buffer,
 * and then a second blit that performs a flip.
 */

#include "piglit-test-pattern.h"
#include "piglit-fbo.h"
using namespace piglit_util_fbo;
using namespace piglit_util_test_pattern;

const int pattern_width = 256; const int pattern_height = 256;

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = pattern_width*2;
	config.window_height = pattern_height;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

static Fbo multisampled_fbo, singlesampled_fbo;
static TestPattern *test_pattern;
static GLint srcX0, srcY0, srcX1, srcY1;

static void
print_usage_and_exit(char *prog_name)
{
	printf("Usage: %s <num_samples> <flip_direction>\n"
	       "  where <flip_direction> is either x or y\n",
	       prog_name);
	piglit_report_result(PIGLIT_FAIL);
}

void
piglit_init(int argc, char **argv)
{
	if (argc != 3)
		print_usage_and_exit(argv[0]);

	/* 1st arg: num_samples */
	char *endptr = NULL;
	int num_samples = strtol(argv[1], &endptr, 0);
	if (endptr != argv[1] + strlen(argv[1]))
		print_usage_and_exit(argv[0]);

	/* 2nd arg: flip_direction */
	if (strcmp(argv[2], "x") == 0) {
		srcX0 = pattern_width;
		srcX1 = 0;
		srcY0 = 0;
		srcY1 = pattern_height;
	} else if (strcmp(argv[2], "y") == 0) {
		srcX0 = 0;
		srcX1 = pattern_width;
		srcY0 = pattern_height;
		srcY1 = 0;
	} else {
		print_usage_and_exit(argv[0]);
	}

	piglit_require_gl_version(21);
	piglit_require_extension("GL_ARB_framebuffer_object");
	piglit_require_extension("GL_ARB_vertex_array_object");

	/* Skip the test if num_samples > GL_MAX_SAMPLES */
	GLint max_samples;
	glGetIntegerv(GL_MAX_SAMPLES, &max_samples);
	if (num_samples > max_samples)
		piglit_report_result(PIGLIT_SKIP);

	singlesampled_fbo.setup(FboConfig(0,
					  pattern_width,
					  pattern_height));

	multisampled_fbo.setup(FboConfig(num_samples,
					 pattern_width,
					 pattern_height));

	test_pattern = new Triangles();
	test_pattern->compile();

	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		piglit_report_result(PIGLIT_FAIL);
	}
}

enum piglit_result
piglit_display()
{
	bool pass = true;

	/* Draw the test pattern into the multisampled buffer. */
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, multisampled_fbo.handle);
	multisampled_fbo.set_viewport();
	test_pattern->draw(TestPattern::no_projection);

	/* Blit it to a single-sampled buffer, flipping the
	 * appropriate coordinate.  This will only work if the
	 * implementation allows multisampled blits to be flipped.
	 */
	glBindFramebuffer(GL_READ_FRAMEBUFFER, multisampled_fbo.handle);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, singlesampled_fbo.handle);
	glBlitFramebuffer(srcX0, srcY0, srcX1, srcY1,
			  0, 0, pattern_width, pattern_height,
			  GL_COLOR_BUFFER_BIT, GL_NEAREST);
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	/* Blit the resulting image to the screen, performing no
	 * additional flip.  This is the test image.
	 */
	glBindFramebuffer(GL_READ_FRAMEBUFFER, singlesampled_fbo.handle);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
	glBlitFramebuffer(0, 0, pattern_width, pattern_height,
			  0, 0, pattern_width, pattern_height,
			  GL_COLOR_BUFFER_BIT, GL_NEAREST);

	/* Blit the test pattern into the single-sampled buffer with
	 * no flip.  This should always work.
	 */
	glBindFramebuffer(GL_READ_FRAMEBUFFER, multisampled_fbo.handle);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, singlesampled_fbo.handle);
	glBlitFramebuffer(0, 0, pattern_width, pattern_height,
			  0, 0, pattern_width, pattern_height,
			  GL_COLOR_BUFFER_BIT, GL_NEAREST);

	/* Blit the resulting image to the screen, flipping the
	 * appropriate coordinate, to produce the reference image.
	 * This should always work (since it is blitting from
	 * single-sampled to single-sampled).
	 */
	glBindFramebuffer(GL_READ_FRAMEBUFFER, singlesampled_fbo.handle);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
	glBlitFramebuffer(srcX0, srcY0, srcX1, srcY1,
			  pattern_width, 0, 2*pattern_width, pattern_height,
			  GL_COLOR_BUFFER_BIT, GL_NEAREST);

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	/* Compare the test and reference images. */
	glBindFramebuffer(GL_READ_FRAMEBUFFER, piglit_winsys_fbo);
	pass = piglit_probe_rect_halves_equal_rgba(0, 0, 2*pattern_width,
						   pattern_height) && pass;

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
