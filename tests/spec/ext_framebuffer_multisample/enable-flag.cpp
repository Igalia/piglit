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
 * \file enable-flag.cpp
 *
 * This test verifies the proper functioning of the GL_MULTISAMPLE
 * flag.  From the GL 3.0 spec (p.116):
 *
 *     "Multisample rasterization is enabled or disabled by calling
 *     Enable or Disable with the symbolic constant MULTISAMPLE."
 *
 * Elsewhere in the spec, where multisample rasterization is described
 * (sections 3.4.3, 3.5.4, and 3.6.6), the following text is
 * consistently used:
 *
 *     "If MULTISAMPLE is enabled, and the value of SAMPLE_BUFFERS is
 *     one, then..."
 *
 * So, in other words, disabling GL_MULTISAMPLE should prevent
 * multisample rasterization from occurring, even if the draw
 * framebuffer is multisampled.
 *
 *
 * This test operates by performing the following operations:
 *
 * 1. Verify that the default state of GL_MULTISAMPLE is enabled.
 *
 * 2. Draw a test image into a multisampled buffer, with
 *    GL_MULTISAMPLE disabled.
 *
 * 3. Blit this image to the left half of the piglit window (which is
 *    not multisampled) to resolve it.
 *
 * 4. Draw the same test image into a single-sampled buffer.
 *
 * 5. Blit this image to the right half of the piglit window.
 *
 * 6. Verify that the two halves of the piglit window match.  If they
 *    don't, then presumably the disabling of GL_MULTISAMPLE failed to
 *    take effect.
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

const int pattern_width = 256; const int pattern_height = 256;

Fbo singlesampled_fbo;
Fbo multisampled_fbo;
Triangles triangles;

extern "C" void
piglit_init(int argc, char **argv)
{
	piglit_require_gl_version(21);
	piglit_require_extension("GL_ARB_framebuffer_object");
	piglit_require_extension("GL_ARB_vertex_array_object");

	GLint max_samples;
	glGetIntegerv(GL_MAX_SAMPLES, &max_samples);

	singlesampled_fbo.setup(FboConfig(0, pattern_width,
					  pattern_height));
	multisampled_fbo.setup(FboConfig(max_samples, pattern_width,
					 pattern_height));
	triangles.compile();
}

extern "C" enum piglit_result
piglit_display()
{
	bool pass = true;

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
	glViewport(0, 0, piglit_width, piglit_height);
	glClear(GL_COLOR_BUFFER_BIT);

	/* Verify that the default state of GL_MULTISAMPLE is
	 * enabled.
	 */
	pass = glIsEnabled(GL_MULTISAMPLE) && pass;

	/* Draw a test image into a multisampled buffer, with
	 * GL_MULTISAMPLE disabled.
	 */
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, multisampled_fbo.handle);
	multisampled_fbo.set_viewport();
	glDisable(GL_MULTISAMPLE);
	triangles.draw(TestPattern::no_projection);
	glEnable(GL_MULTISAMPLE);

	/* Blit this image to the left half of the piglit window
	 * (which is not multisampled) to resolve it.
	 */
	glBindFramebuffer(GL_READ_FRAMEBUFFER, multisampled_fbo.handle);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
	glBlitFramebuffer(0, 0, pattern_width, pattern_height,
			  0, 0, pattern_width, pattern_height,
			  GL_COLOR_BUFFER_BIT, GL_NEAREST);

	/* Draw the same test image into a single-sampled buffer. */
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, singlesampled_fbo.handle);
	singlesampled_fbo.set_viewport();
	triangles.draw(TestPattern::no_projection);

	/* Blit this image to the right half of the piglit window. */
	glBindFramebuffer(GL_READ_FRAMEBUFFER, singlesampled_fbo.handle);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
	glBlitFramebuffer(0, 0, pattern_width, pattern_height,
			  pattern_width, 0, 2*pattern_width, pattern_height,
			  GL_COLOR_BUFFER_BIT, GL_NEAREST);

	/* Verify that the two halves of the piglit window match.  If
	 * they don't, then presumably the disabling of GL_MULTISAMPLE
	 * failed to take effect.
	 */
	glBindFramebuffer(GL_READ_FRAMEBUFFER, piglit_winsys_fbo);
	pass = piglit_probe_rect_halves_equal_rgba(0, 0, 2*pattern_width,
						   pattern_height) && pass;

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

} /* Anonymous namespace */
