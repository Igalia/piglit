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
 * \file clear.cpp
 *
 * Verify that the implementation ignores multisample fragment
 * operations when performing clears.
 *
 * This test checks that the following state variables (from GL 3.0
 * section 4.1.3 "Multisample Fragment Operations") do not apply when
 * performing clears:
 *
 * - GL_SAMPLE_ALPHA_TO_COVERAGE
 *
 * - GL_SAMPLE_ALPHA_TO_ONE
 *
 * - GL_SAMPLE_COVERAGE
 *
 * - GL_SAMPLE_COVERAGE_VALUE
 *
 * - GL_SAMPLE_COVERAGE_INVERT
 *
 * The test operates by setting the above state variables in a way
 * that would reduce the sample coverage (for normal GL draw
 * operations).  Then it performs a glClear() and verifies that all
 * samples of all pixels were cleared.
 *
 * The test can be run in 3 modes: color, depth, and stencil.
 *
 * In depth and stencil modes, extra work is required to verify that
 * all samples are properly cleared.  Since a typical MSAA resolve
 * retains only one sample from each pixel for the depth and stencil
 * buffers, we need to convert depth/stencil values into colors, then
 * blit to the screen and check that the resulting color is correct.
 * The extra work of converting depth/stencil values into colors is
 * done using the ManifestDepth and ManifestStencil programs from
 * piglit-test-pattern.h.
 */

#include "piglit-test-pattern.h"
#include "piglit-fbo.h"
using namespace piglit_util_fbo;
using namespace piglit_util_test_pattern;

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 256;
	config.window_height = 256;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

namespace {

const int pattern_width = 256; const int pattern_height = 256;

Fbo multisampled_fbo;
ManifestProgram *manifest_program = NULL;
GLbitfield buffer_to_test;


void
print_usage_and_exit(char *prog_name)
{
	printf("Usage: %s <num_samples> <buffer_type>\n"
	       "  where <buffer_type> is one of:\n"
	       "    color\n"
	       "    depth\n"
	       "    stencil\n",
	       prog_name);
	piglit_report_result(PIGLIT_FAIL);
}


extern "C" void
piglit_init(int argc, char **argv)
{
	if (argc != 3)
		print_usage_and_exit(argv[0]);

	/* 1st arg: num_samples */
	char *endptr = NULL;
	int num_samples = strtol(argv[1], &endptr, 0);
	if (endptr != argv[1] + strlen(argv[1]))
		print_usage_and_exit(argv[0]);

	/* 2nd arg: buffer_type */
	if (strcmp(argv[2], "color") == 0) {
		buffer_to_test = GL_COLOR_BUFFER_BIT;
	} else if (strcmp(argv[2], "depth") == 0) {
		buffer_to_test = GL_DEPTH_BUFFER_BIT;
		manifest_program = new ManifestDepth();
	} else if (strcmp(argv[2], "stencil") == 0) {
		buffer_to_test = GL_STENCIL_BUFFER_BIT;
		manifest_program = new ManifestStencil();
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

	multisampled_fbo.setup(FboConfig(num_samples, pattern_width,
					 pattern_height));
	if (manifest_program)
		manifest_program->compile();

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);
}


bool
test_clear(const float r, const float g, const float b,
	   const float a, const bool fast_clear_compatible)
{
	bool pass = true;

	/* Clear all buffers of the multisampled fbo to default values
	 * (color={0,0,0,0}, depth=1, stencil=0), with no special
	 * coverage settings set.
	 */
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, multisampled_fbo.handle);
	multisampled_fbo.set_viewport();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT |
		GL_STENCIL_BUFFER_BIT);

	/* Set all the clear values to non-default settings.  We use
	 * an alpha value other than 1.0 so verify that
	 * GL_SAMPLE_ALPHA_TO_COVERAGE and GL_SAMPLE_ALPHA_TO_ONE
	 * don't take effect.
	 */
	glClearColor(r, g, b, a);
	glClearDepth(0.5);
	glClearStencil(1);

	/* Enable the mulitsample fragment operations that glClear()
	 * is supposed to ignore.
	 */
	glEnable(GL_SAMPLE_COVERAGE);
	glSampleCoverage(0.5, GL_TRUE);
	glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
	glEnable(GL_SAMPLE_ALPHA_TO_ONE);

	/* Clear the buffer under test. */
	glClear(buffer_to_test);

	/* Reset the multisample fragment operations and clear values
	 * to their default settings.
	 */
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClearDepth(1.0);
	glClearStencil(0);
	glDisable(GL_SAMPLE_COVERAGE);
	glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
	glDisable(GL_SAMPLE_ALPHA_TO_ONE);

	/* If we are testing the depth or stencil buffer, use
	 * manifest_program to convert depth/stencil values to
	 * colors.
	 */
	if (manifest_program)
		manifest_program->run();

	/* Blit the color values from the multisampled FBO to the
	 * screen, forcing a resolve.
	 */
	glBindFramebuffer(GL_READ_FRAMEBUFFER, multisampled_fbo.handle);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
	glBlitFramebuffer(0, 0, pattern_width, pattern_height,
			  0, 0, pattern_width, pattern_height,
			  GL_COLOR_BUFFER_BIT, GL_NEAREST);

	/* Figure out what color we expect to be drawn, depending on
	 * which buffer was tested:
	 *
	 * - If we tested the color buffer, we should have gotten the
         *   clear color back verbatim.
	 *
	 * - If we tested the depth buffer, we should have gotten red,
         *   since the ManifestDepth program converts a depth value of
         *   0.5 to red.
	 *
	 * - If we tested the stencil buffer, we should have gotten
         *   blue, since the ManifestStencil program converts a
         *   stencil value of 1 to blue.
	 */
	const float expected_color[4] = { r, g, b, a };
	static const float expected_depth[4] = { 1.0, 0.0, 0.0, 1.0 };
	static const float expected_stencil[4] = { 0.0, 0.0, 1.0, 1.0 };
	const float *expected = NULL;
	switch (buffer_to_test) {
	case GL_COLOR_BUFFER_BIT:
		expected = expected_color;
		break;
	case GL_DEPTH_BUFFER_BIT:
		expected = expected_depth;
		break;
	case GL_STENCIL_BUFFER_BIT:
		expected = expected_stencil;
		break;
	default:
		printf("Unexpected value in buffer_to_test\n");
		piglit_report_result(PIGLIT_FAIL);
		break;
	}

	/* Test that the appropriate color was drawn.  Since the
	 * resolve operation averaged together all the color samples
	 * corresponding to each pixel, this effectively verifies that
	 * all samples of every pixel were correctly cleared.
	 */
	glBindFramebuffer(GL_READ_FRAMEBUFFER, piglit_winsys_fbo);
	pass = piglit_probe_rect_rgba(0, 0, piglit_width, piglit_height,
				      expected) && pass;

	if (buffer_to_test == GL_COLOR_BUFFER_BIT)
		printf("fast_clear_compatible = %s, result = %s\n",
		       fast_clear_compatible ? "true" : "false",
		       pass ? "pass" : "fail");

	piglit_present_results();

	return pass;
}

extern "C" enum piglit_result
piglit_display()
{
	bool pass =true;

	/* Non 'fast clear' path. */
	pass = test_clear(1.0, 1.0, 1.0, 0.5, false) && pass;

	/* Test with color values compatible with Intel's i965 driver's
	 * 'fast clear' constraints. It varifies the 'fast clear' path
	 * if supported by the implementation.
	 */
	pass = test_clear(1.0, 1.0, 1.0, 0.0, true) && pass;

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

} /* Anonymous namespace */
