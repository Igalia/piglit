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
 * \file no-color.cpp
 *
 * This test verifies proper operation of multisampled FBOs that lack
 * a color buffer.  It operates by performing identical draw
 * operations in an FBO that lacks a color buffer and an FBO that has
 * a color buffer, and verifying that the resulting output is the
 * same.
 *
 * The test can run in any of the following modes:
 *
 * - depth: test depth buffer behaviour, using a fragment shader that
 *   does not compute depth.
 *
 * - depth-computed: test depth buffer behaviour, using a fragment
 *   shader that *does* compute depth.
 *
 * - stencil: test stencil buffer beahviour.
 *
 * It can also be configured to use either a combined depth/stencil
 * buffer, separate depth/stencil buffers, or just a single
 * depth/stencil buffer depending on the type of test.
 *
 * The test operates by performing the following steps:
 *
 * 1. Draw a test pattern to a multisampled FBO that lacks a color
 *    buffer (let's call this the "test FBO").
 *
 * 2. Blit the test pattern to a multisampled FBO that has a full
 *    complement of color, depth, and stencil buffers (let's call this
 *    the "manifest FBO").
 *
 * 3. Do a "manifest" operation to cause colors to be drawn that are
 *    dependent upon the contents of the depth or stencil buffer.
 *
 * 4. Blit the color buffer from the manifest FBO to the screen.  This
 *    is the test image, and is shown in the left half of the piglit
 *    window.
 *
 * 5. Draw the test pattern again, but this time draw it directly to
 *    the manifest FBO.
 *
 * 6. Do a "manifest" operation again.
 *
 * 7. Blit the color buffer to the screen.  This is the reference
 *    image, and is shown in the right half of the piglit window.
 *
 * 8. Compare the test and reference images to make sure they match.
 */

#include "piglit-test-pattern.h"
#include "piglit-fbo.h"
using namespace piglit_util_fbo;
using namespace piglit_util_test_pattern;

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 512;
	config.window_height = 256;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

namespace {

const int pattern_width = 256, pattern_height = 256;

GLenum buffer_to_test;
Fbo test_fbo;
Fbo manifest_fbo;
ManifestProgram *manifest_program;
TestPattern *test_pattern;

void
print_usage_and_exit(char *prog_name)
{
	printf("Usage: %s <num_samples> <test_type> <buffer_config?\n"
	       "  where <test_type> is one of:\n"
	       "    depth: test fixed pipeline depth\n"
	       "    depth-computed: test depth value computed by a shader\n"
	       "    stencil: test stencil\n"
	       "  and <buffer_config> is one of:\n"
	       "    combined: use a single combined depth/stencil buffer\n"
	       "    separate: use separate depth and stencil buffers\n"
	       "    single: use just a single buffer (depth or stencil)\n",
	       prog_name);
	piglit_report_result(PIGLIT_FAIL);
}

extern "C" void
piglit_init(int argc, char **argv)
{
	if (argc != 4)
		print_usage_and_exit(argv[0]);

	/* 1st arg: num_samples */
	char *endptr = NULL;
	int num_samples = strtol(argv[1], &endptr, 0);
	if (endptr != argv[1] + strlen(argv[1]))
		print_usage_and_exit(argv[0]);

	/* 2nd arg: test_type */
	if (strcmp(argv[2], "depth") == 0) {
		buffer_to_test = GL_DEPTH_BUFFER_BIT;
		manifest_program = new ManifestDepth;
		test_pattern = new DepthSunburst(false /* compute_depth */);
	} else if (strcmp(argv[2], "depth-computed") == 0) {
		buffer_to_test = GL_DEPTH_BUFFER_BIT;
		manifest_program = new ManifestDepth;
		test_pattern = new DepthSunburst(true /* compute_depth */);
	} else if (strcmp(argv[2], "stencil") == 0) {
		buffer_to_test = GL_STENCIL_BUFFER_BIT;
		manifest_program = new ManifestStencil;
		test_pattern = new StencilSunburst;
	} else {
		print_usage_and_exit(argv[0]);
	}

	/* 3rd arg: buffer_config */
	FboConfig test_fbo_config(num_samples, pattern_width, pattern_height);
	test_fbo_config.color_internalformat = GL_NONE;
	if (strcmp(argv[3], "combined") == 0) {
		test_fbo_config.combine_depth_stencil = true;
	} else if (strcmp(argv[3], "separate") == 0) {
		test_fbo_config.combine_depth_stencil = false;
	} else if (strcmp(argv[3], "single") == 0) {
		test_fbo_config.combine_depth_stencil = false;
		if (buffer_to_test == GL_DEPTH_BUFFER_BIT)
			test_fbo_config.stencil_internalformat = GL_NONE;
		else
			test_fbo_config.depth_internalformat = GL_NONE;
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

	test_fbo.setup(test_fbo_config);
	manifest_fbo.setup(FboConfig(num_samples, pattern_width,
				     pattern_height));
	manifest_program->compile();
	test_pattern->compile();

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);
}

extern "C" enum piglit_result
piglit_display()
{
	bool pass = true;

	/* Draw the test pattern into test_fbo. */
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, test_fbo.handle);
	test_fbo.set_viewport();
	test_pattern->draw(TestPattern::no_projection);

	/* Blit the test pattern to manifest_fbo. */
	glBindFramebuffer(GL_READ_FRAMEBUFFER, test_fbo.handle);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, manifest_fbo.handle);
	glBlitFramebuffer(0, 0, pattern_width, pattern_height,
			  0, 0, pattern_width, pattern_height,
			  buffer_to_test, GL_NEAREST);

	/* Manifest the pattern so that it is reflected in color
	 * values in manifest_fbo.
	 */
	manifest_program->run();

	/* Blit the color buffer from manifest_fbo to the screen.
	 * This is the test image.
	 */
	glBindFramebuffer(GL_READ_FRAMEBUFFER, manifest_fbo.handle);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
	glBlitFramebuffer(0, 0, pattern_width, pattern_height,
			  0, 0, pattern_width, pattern_height,
			  GL_COLOR_BUFFER_BIT, GL_NEAREST);

	/* Draw the test pattern into manifest_fbo. */
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, manifest_fbo.handle);
	manifest_fbo.set_viewport();
	test_pattern->draw(TestPattern::no_projection);

	/* Manifest the pattern so that it is reflected in color
	 * values in manifest_fbo.
	 */
	manifest_program->run();

	/* Blit the color buffer from manifest_fbo to the screen.
	 * This is the reference image.
	 */
	glBindFramebuffer(GL_READ_FRAMEBUFFER, manifest_fbo.handle);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
	glBlitFramebuffer(0, 0, pattern_width, pattern_height,
			  pattern_width, 0, 2*pattern_width, pattern_height,
			  GL_COLOR_BUFFER_BIT, GL_NEAREST);

	/* Compare the test and reference images */
	glBindFramebuffer(GL_READ_FRAMEBUFFER, piglit_winsys_fbo);
	pass = piglit_probe_rect_halves_equal_rgba(0, 0, 2*pattern_width,
						   pattern_height) && pass;

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

} /* Anonymous namespace */
