/*
 * Copyright © 2014 VMware, Inc.
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
 * Test GL_EXT_packed_depth_stencil with glRead/DrawPixels
 * Based on an original Glean test written by Brian Paul.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 12;
	config.window_visual = (PIGLIT_GL_VISUAL_RGBA |
				PIGLIT_GL_VISUAL_DEPTH |
				PIGLIT_GL_VISUAL_STENCIL);
PIGLIT_GL_TEST_CONFIG_END


static bool
test_readdrawpixels(void)
{
	/*reference image data */
	static const GLuint image[4] = {
		0x00000000,
		0x000000ff,
		0xffffff00,
		0xffffffff
	};
	GLuint readback[4];
	GLuint stencilMap[2] = { 2, 2 };  /* map all stencil values to 2 */
	int i;

	glWindowPos2i(0, 0);
	glDrawPixels(2, 2, GL_DEPTH_STENCIL_EXT,
		     GL_UNSIGNED_INT_24_8_EXT, image);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		return false;

	glReadPixels(0, 0, 2, 2, GL_DEPTH_STENCIL_EXT,
		     GL_UNSIGNED_INT_24_8_EXT, readback);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		return false;

	for (i = 0; i < 4; i++) {
		if (image[i] != readback[i]) {
			printf("Image returned by glReadPixels didn't match"
			       " the expected result (0x%x != 0x%x)",
			       readback[i], image[i]);
			return false;
		}
	}

	/* test depth scale/bias and stencil mapping (in a trivial way) */
	glPixelTransferf(GL_DEPTH_SCALE, 0.0);  /* map all depths to 1.0 */
	glPixelTransferf(GL_DEPTH_BIAS, 1.0);
	glPixelMapuiv(GL_PIXEL_MAP_S_TO_S, 2, stencilMap);
	glPixelTransferi(GL_MAP_STENCIL, 1);
	glReadPixels(0, 0, 2, 2, GL_DEPTH_STENCIL_EXT,
		     GL_UNSIGNED_INT_24_8_EXT, readback);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		return false;

	for (i = 0; i < 4; i++) {
		if (readback[i] != 0xffffff02) {
			printf("Image returned by glReadPixels didn't match"
			       " the expected result (0x%x != 0xffffff02)",
			       readback[i]);
			return false;
		}
	}
	glPixelTransferf(GL_DEPTH_SCALE, 1.0);
	glPixelTransferf(GL_DEPTH_BIAS, 0.0);
	glPixelTransferi(GL_MAP_STENCIL, 0);

	return true;
}


void
piglit_init(int argc, char **argv)
{
	bool pass = true;

	piglit_require_extension("GL_EXT_packed_depth_stencil");

	pass = test_readdrawpixels() && pass;

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}


enum piglit_result
piglit_display(void)
{
	/* unused */
	return PIGLIT_FAIL;
}
