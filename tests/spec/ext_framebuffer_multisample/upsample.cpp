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
 * \file upsample.cpp
 *
 * Verify the accuracy of upsampling from a non-MSAA buffer to an MSAA
 * buffer.
 *
 * This test operates by drawing a test image in the window system
 * framebuffer (which is non-MSAA), and then blitting it to an MSAA
 * framebuffer, forcing the implementation to upsample it.
 *
 * To verify that upsampling has properly replicated each pixel value
 * in each of its samples, we blit from the MSAA buffer back to the
 * window system framebuffer.  This causes all of the samples for each
 * pixel to be blended, so if any of the pixels were not upsampled
 * correctly, we would expect the downsampled image to be different
 * from the original image.
 *
 * When testing depth and stencil buffers, we need to modify this
 * procedure slightly, since downsampling depth and stencil buffers
 * doesn't cause the pixels to be blended.  So, after the first blit,
 * we execute a "manifest pass" to translate the depth or stencil
 * image into a color image.  This is done independently in both the
 * MSAA and non-MSAA buffers.  Then we downsample the resulting color
 * image as before.
 *
 * Note: this test relies on proper functioning of the MSAA buffer and
 * the downsample blit.  These are already adequately tested by
 * accuracy.cpp.
 */

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 512;
	config.window_height = 256;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DEPTH | PIGLIT_GL_VISUAL_STENCIL;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

const int pattern_width = 256; const int pattern_height = 256;

Fbo multisample_fbo;
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

	multisample_fbo.setup(FboConfig(num_samples, pattern_width,
					pattern_height));
}

enum piglit_result
piglit_display()
{
	bool pass = true;

	/* Draw the full test pattern on the right half of the piglit
	 * window, as a reference image.
	 *
	 * To map the full test pattern to the right half of the
	 * windows, we need a projection matrix that multiplies the X
	 * coordinate by 0.5 and adds 0.5.
	 */
	float proj[4][4] = {
		{ 0.5, 0, 0, 0.5 },
		{ 0,   1, 0, 0   },
		{ 0,   0, 1, 0   },
		{ 0,   0, 0, 1   }
	};
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
	glViewport(0, 0, piglit_width, piglit_height);
	test_pattern->draw(proj);

	/* Blit the test pattern to multisample_fbo, forcing the
	 * implementation to upsample it.
	 */
	glBindFramebuffer(GL_READ_FRAMEBUFFER, piglit_winsys_fbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, multisample_fbo.handle);
	glBlitFramebuffer(pattern_width, 0, pattern_width*2, pattern_height,
			  0, 0, pattern_width, pattern_height,
			  buffer_to_test, filter_mode);

	if (manifest_program) {
		/* Manifest the test pattern in the main framebuffer. */
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
		manifest_program->run();

		/* Manifest the test pattern in the multisample
		 * framebuffer.
		 */
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, multisample_fbo.handle);
		multisample_fbo.set_viewport();
		manifest_program->run();
	}

	/* Blit the manifested test pattern to the left half of the
	 * main framebuffer, forcing the implementation to downsample
	 * it.
	 */
	glBindFramebuffer(GL_READ_FRAMEBUFFER, multisample_fbo.handle);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
	glBlitFramebuffer(0, 0, pattern_width, pattern_height,
			  0, 0, pattern_width, pattern_height,
			  GL_COLOR_BUFFER_BIT, GL_NEAREST);

	/* Check that the left and right halves of the screen match.
	 * If they don't, then there is either a problem with
	 * upsampling or downsampling.  Since downsampling is already
	 * tested by accuracy.cpp, we'll assume that any problem we
	 * see here is due to upsampling.
	 */
	glBindFramebuffer(GL_READ_FRAMEBUFFER, piglit_winsys_fbo);
	pass = piglit_probe_rect_halves_equal_rgba(0, 0, piglit_width,
						   piglit_height) && pass;

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
