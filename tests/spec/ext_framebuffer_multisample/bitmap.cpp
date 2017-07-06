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

#include "piglit-fbo.h"
#include "piglit-test-pattern.h"
using namespace piglit_util_fbo;
using namespace piglit_util_test_pattern;

/**
 * \file bitmap.cpp
 *
 * This test case verifies the functionality of glBitmap() with multisample
 * FBO and assumes that MSAA accuracy test already passes. glBitmap() is
 * expected to work exactly the same way on multisample FBO as it works on
 * single sample FBO.
 *
 * Test operates by drawing a test pattern in a single sample FBO which
 * generates a reference image in right half of default framebuffer.
 *
 * Draw the same test pattern in multisample buffer and blit it in to a single
 * sample FBO (resolve_fbo). Then blit the resolve_fbo to left half of window
 * system framebuffer. This is the test image.
 *
 * Compare the two halves of default framebuffer.
 */

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 512;
	config.window_height = 256;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

const int pattern_width = 256; const int pattern_height = 256;

static Fbo ms_fbo, resolve_fbo;
static GLint num_samples;

static GLubyte bitmap[] =
{
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xc0, 0x00, 0x00, 0x03, 0xc0, 0x00, 0x00, 0x03,
	0xcf, 0xff, 0xff, 0xf3, 0xcf, 0xff, 0xff, 0xf3,
	0xcc, 0x00, 0x00, 0x33, 0xcc, 0x00, 0x00, 0x33,
	0xcc, 0xff, 0xff, 0x33, 0xcc, 0xff, 0xff, 0x33,
	0xcc, 0xc0, 0x03, 0x33, 0xcc, 0xc0, 0x03, 0x33,
	0xcc, 0xcf, 0xf3, 0x33, 0xcc, 0xcf, 0xf3, 0x33,
	0xcc, 0xcf, 0xf3, 0x33, 0xcc, 0xcf, 0xf3, 0x33,
	0xcc, 0xcf, 0xf3, 0x33, 0xcc, 0xcf, 0xf3, 0x33,
	0xcc, 0xcf, 0xf3, 0x33, 0xcc, 0xcf, 0xf3, 0x33,
	0xcc, 0xc0, 0x03, 0x33, 0xcc, 0xc0, 0x03, 0x33,
	0xcc, 0xff, 0xff, 0x33, 0xcc, 0xff, 0xff, 0x33,
	0xcc, 0x00, 0x00, 0x33, 0xcc, 0x00, 0x00, 0x33,
	0xcf, 0xff, 0xff, 0xf3, 0xcf, 0xff, 0xff, 0xf3,
	0xc0, 0x00, 0x00, 0x03, 0xc0, 0x00, 0x00, 0x03,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
};

void
draw_pattern(void)
{
	const int w  = 32, h = 32;

	glClear(GL_COLOR_BUFFER_BIT);

	glRasterPos2i (0, 0);
	glColor3f(1.0, 1.0 , 1.0);
	for (int i = 0; i < pattern_width / w; i++)
		glBitmap(w, h, 0.0, 0.0, w, h, bitmap);

	glRasterPos2i (0, pattern_height - h);
	for (int i = 0; i < pattern_width / w; i++)
		glBitmap(w, h, 0.0, 0.0, w, -h, bitmap);
}

void
print_usage_and_exit(char *prog_name)
{
	printf("Usage: %s <num_samples>\n", prog_name);
	piglit_report_result(PIGLIT_FAIL);
}

bool
test_multisample_bitmap()
{
	bool result = true;

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, ms_fbo.handle);
	draw_pattern();

	/* Blit ms_fbo to resolve_fbo to resolve multisample buffer */
	glBindFramebuffer(GL_READ_FRAMEBUFFER, ms_fbo.handle);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolve_fbo.handle);
	glBlitFramebuffer(0, 0, pattern_width, pattern_height,
			  0, 0, pattern_width, pattern_height,
			  GL_COLOR_BUFFER_BIT, GL_NEAREST);

	/* Blit resolve_fbo to the left half of window system framebuffer.
	 * This is the test image.
	 */
	glBindFramebuffer(GL_READ_FRAMEBUFFER, resolve_fbo.handle);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
	glBlitFramebuffer(0, 0, pattern_width, pattern_height,
			  0, 0, pattern_width, pattern_height,
			  GL_COLOR_BUFFER_BIT, GL_NEAREST);

	/* Check that the left and right halves of the screen match */
	glBindFramebuffer(GL_READ_FRAMEBUFFER, piglit_winsys_fbo);
	result = piglit_probe_rect_halves_equal_rgba(0, 0, piglit_width,
						     piglit_height)
		 && result;

	result = piglit_check_gl_error(GL_NO_ERROR) && result;
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
	return result;
}


void
piglit_init(int argc, char **argv)
{
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

	piglit_ortho_projection(pattern_width, pattern_height, GL_TRUE);

	/* Skip the test if num_samples > GL_MAX_SAMPLES */
	GLint max_samples;
	glGetIntegerv(GL_MAX_SAMPLES, &max_samples);
	if (num_samples > max_samples)
		piglit_report_result(PIGLIT_SKIP);

	ms_fbo.setup(FboConfig(num_samples, pattern_width, pattern_height));
	resolve_fbo.setup(FboConfig(0, pattern_width, pattern_height));
}

enum piglit_result
piglit_display()
{
	bool pass = true;
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	/* Draw test pattern in single sample resolve_fbo */
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolve_fbo.handle);
	resolve_fbo.set_viewport();
	draw_pattern();

	/* Blit resolve_fbo to the right half of window system framebuffer. This
	 * is a reference image.
	 */
	glBindFramebuffer(GL_READ_FRAMEBUFFER, resolve_fbo.handle);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
	glBlitFramebuffer(0, 0, pattern_width, pattern_height,
			  pattern_width, 0, 2 * pattern_width, pattern_height,
			  GL_COLOR_BUFFER_BIT, GL_NEAREST);

	/* Test drawing bitmap in multisample FBO */
	pass = test_multisample_bitmap() && pass;

	if (!piglit_automatic)
		piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
