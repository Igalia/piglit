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
#include "piglit-test-pattern.h"
#include "piglit-fbo.h"
using namespace piglit_util_fbo;
using namespace piglit_util_test_pattern;

/**
 * \file multisample-blit.cpp
 *
 * Verify the accuracy of blitting from an MSAA buffer to another MSAA
 * buffer.
 *
 * This test operates by drawing a test image in an MSAA buffer,
 * blitting it to a second MSAA buffer, and then blitting it to the
 * window system framebuffer (which is non-MSAA).
 *
 * To verify that the MSAA-to-MSAA blit worked properly, we also do a
 * blit straight from the MSAA buffer to the window system
 * framebuffer--this should produce the same image.
 */
PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 512;
	config.window_height = 256;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DEPTH | PIGLIT_GL_VISUAL_STENCIL;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

const int pattern_width = 256; const int pattern_height = 256;

Fbo src_fbo;
Fbo dst_fbo;
TestPattern *test_pattern = NULL;
ManifestProgram *manifest_program = NULL;
GLbitfield buffer_to_test;
GLenum filter_mode = GL_NEAREST;

void
print_usage_and_exit(char *prog_name)
{
	printf("Usage: %s <num_samples> <buffer_type>\n"
	       "  where <buffer_type> is one of:\n"
	       "    color\n"
	       "    stencil\n"
	       "    depth\n"
	       "Available options:\n"
	       "    linear: use GL_LINEAR filter mode\n",
	       prog_name);
	piglit_report_result(PIGLIT_FAIL);
}

void
piglit_init(int argc, char **argv)
{
	int num_samples;
	if (argc < 3)
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

	if (strcmp(argv[2], "color") == 0) {
		test_pattern = new Triangles();
		buffer_to_test = GL_COLOR_BUFFER_BIT;
	} else if (strcmp(argv[2], "depth") == 0) {
		test_pattern = new DepthSunburst();
		manifest_program = new ManifestDepth();
		buffer_to_test = GL_DEPTH_BUFFER_BIT;
	} else if (strcmp(argv[2], "stencil") == 0) {
		test_pattern = new StencilSunburst();
		manifest_program = new ManifestStencil();
		buffer_to_test = GL_STENCIL_BUFFER_BIT;
	} else {
		print_usage_and_exit(argv[0]);
	}

	for (int i = 3; i < argc; i++) {
		if (strcmp(argv[i], "linear") == 0)
			filter_mode = GL_LINEAR;
		else
			print_usage_and_exit(argv[0]);
	}

	test_pattern->compile();
	if (manifest_program)
		manifest_program->compile();

	src_fbo.setup(FboConfig(num_samples, pattern_width, pattern_height));
	dst_fbo.setup(FboConfig(num_samples, pattern_width, pattern_height));
}

enum piglit_result
piglit_display()
{
	bool pass = true;

	/* Draw the test pattern in src_fbo. */
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, src_fbo.handle);
	src_fbo.set_viewport();
	test_pattern->draw(TestPattern::no_projection);

	/* Blit from src_fbo to dst_fbo. */
	glBindFramebuffer(GL_READ_FRAMEBUFFER, src_fbo.handle);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dst_fbo.handle);
	glBlitFramebuffer(0, 0, pattern_width, pattern_height,
			  0, 0, pattern_width, pattern_height,
			  buffer_to_test, filter_mode);

	/* If necessary, manifest the depth/stencil image in dst_fbo
	 * into a color image.  This ensures that the blit that
	 * follows will depend on all samples of each pixel.
	 */
	dst_fbo.set_viewport();
	if (manifest_program)
		manifest_program->run();

	/* Blit from dst_fbo to the left half of the window system
	 * framebuffer.  This is the test image.
	 */
	glBindFramebuffer(GL_READ_FRAMEBUFFER, dst_fbo.handle);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
	glBlitFramebuffer(0, 0, pattern_width, pattern_height,
			  0, 0, pattern_width, pattern_height,
			  GL_COLOR_BUFFER_BIT, GL_NEAREST);

	/* Now manifest the image in src_fbo and blit it directly to
	 * the window system framebuffer.  This is the reference
	 * image.
	 */
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, src_fbo.handle);
	src_fbo.set_viewport();
	if (manifest_program)
		manifest_program->run();
	glBindFramebuffer(GL_READ_FRAMEBUFFER, src_fbo.handle);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
	glBlitFramebuffer(0, 0, pattern_width, pattern_height,
			  pattern_width, 0, 2*pattern_width, pattern_height,
			  GL_COLOR_BUFFER_BIT, GL_NEAREST);

	/* Check that the left and right halves of the screen match.
	 * If they don't, then there must have been a problem blitting
	 * from src_fbo to dst_fbo.
	 */
	glBindFramebuffer(GL_READ_FRAMEBUFFER, piglit_winsys_fbo);
	pass = piglit_probe_rect_halves_equal_rgba(0, 0, piglit_width,
						   piglit_height) && pass;

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
