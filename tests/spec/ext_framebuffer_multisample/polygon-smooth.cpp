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
using namespace piglit_util_fbo;
using namespace piglit_util_test_pattern;

/**
 * \file polygon-smooth.cpp
 *
 * Page 134 (in the PDF) of the OpenGL 3.0 spec says:
 * "If MULTISAMPLE is enabled, and the value of SAMPLE BUFFERS is one,
 * then polygons are rasterized using the following algorithm, regardless
 * of whether polygon antialias-ing (POLYGON_SMOOTH) is enabled or disabled".
 *
 * This test operates by drawing a test pattern in to multisample fbo with
 * GL_POLYGON_SMOOTH disabled. Blits it in to right half of window system
 * framebuffer. This is used as reference image.
 *
 * Draw the same test pattern for the second time in multisample buffer with
 * GL_POLYGON_SMOOTH enabled. Blit it in to left half of window system
 * framebuffer. This is the test image.
 *
 * To verify that GL_POLYGON_SMOOTH is ignored during MSAA, compare the two
 * halves of default framebuffer. They are expected to match.
 */

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 512;
	config.window_height = 256;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DEPTH | PIGLIT_GL_VISUAL_STENCIL;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

const int pattern_width = 256; const int pattern_height = 256;

Fbo ms_fbo;
Test *test = NULL;
GLbitfield buffer_to_test;
TestPattern *test_pattern = NULL;

void
print_usage_and_exit(char *prog_name)
{
	printf("Usage: %s <num_samples>\n",
	       prog_name);
	piglit_report_result(PIGLIT_FAIL);
}

void
piglit_init(int argc, char **argv)
{
	int num_samples;
	if (argc < 2)
		print_usage_and_exit(argv[0]);
	{
		char *endptr = NULL;
		num_samples = strtol(argv[1], &endptr, 0);
		if (endptr != argv[1] + strlen(argv[1]))
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

	buffer_to_test = GL_COLOR_BUFFER_BIT;

	test_pattern = new Triangles();
	test_pattern->compile();

	ms_fbo.setup(FboConfig(num_samples, pattern_width, pattern_height));

	/* Enable blending to test GL_POLYGON_SMOOTH */
	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA_SATURATE, GL_ONE);
}

enum piglit_result
piglit_display()
{
	bool pass = true;
	float proj[4][4] = {
		{ 1, 0, 0, 0 },
		{ 0, 1, 0, 0 },
		{ 0, 0, 1, 0 },
		{ 0, 0, 0, 1 }
	};

	/* Draw test pattern in  multisample ms_fbo with GL_POLYGON_SMOOTH
	 * disabled.
	 */
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, ms_fbo.handle);
	ms_fbo.set_viewport();
	test_pattern->draw(proj);

	/* Blit ms_fbo to the right half of window system framebuffer. This
	 * is a reference image to test MSAA with polygon smooth.
	 */
	glBindFramebuffer(GL_READ_FRAMEBUFFER, ms_fbo.handle);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
	glBlitFramebuffer(0, 0, pattern_width, pattern_height,
			  pattern_width, 0, 2 * pattern_width, pattern_height,
			  GL_COLOR_BUFFER_BIT, GL_NEAREST);

	/* Now draw test pattern in mulisample ms_fbo with GL_POLYGON_SMOOTH
	 * enabled
	 */
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_POLYGON_SMOOTH);
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, ms_fbo.handle);
	ms_fbo.set_viewport();

	glClear(buffer_to_test);
	test_pattern->draw(proj);

	glDisable(GL_POLYGON_SMOOTH);

	/* Blit ms_fbo to the left half of window system framebuffer. This
	 * is the test image.
	 */
	glBindFramebuffer(GL_READ_FRAMEBUFFER, ms_fbo.handle);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
	glBlitFramebuffer(0, 0, pattern_width, pattern_height,
			  0, 0, pattern_width, pattern_height,
			  GL_COLOR_BUFFER_BIT, GL_NEAREST);

	/* Check that the left and right halves of the screen match. If they
	 * don't, then GL_POLYGON_SMOOTH is not ignored with multisample
	 * rendering.
	 */
	glBindFramebuffer(GL_READ_FRAMEBUFFER, piglit_winsys_fbo);
	pass = piglit_probe_rect_halves_equal_rgba(0, 0, piglit_width,
						   piglit_height) && pass;
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	piglit_present_results();
	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
