/*
 * Copyright (c) 2014 VMware, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NON-INFRINGEMENT.  IN NO EVENT SHALL VMWARE AND/OR THEIR SUPPLIERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */


/**
 * This test exercises an NVIDIA driver bug where copying from
 * a sRGBA texture to another RGBA texture using ARB_copy_image
 * followed by a GetTexImage() on the RGBA texture results in
 * swapping of red and blue channels.
 *
 * Known to be
 *      -- Present in : Nvidia GTX 650, driver - 319.32
 *      -- Fixed in   :
 */


#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 15;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA;
PIGLIT_GL_TEST_CONFIG_END


#define TEX_WIDTH        32
#define TEX_HEIGHT       32
#define TEX_NUMPIXELS    (TEX_WIDTH * TEX_HEIGHT)

#define RED 0xFF0000FF
#define GRAY 0x7F7F7FFF


static bool
test_srgb_copy(void)
{
	GLenum target = GL_TEXTURE_2D;
	GLuint texData[TEX_NUMPIXELS];
	GLuint texRGBA, texSRGBA, i;

	/* init tex data */
	for (i = 0; i < TEX_NUMPIXELS; ++i) {
		texData[i] = RED;
	}

	/* Create sRGBA texture */
	glGenTextures(1, &texSRGBA);
	glActiveTextureARB(GL_TEXTURE0);
	glBindTexture(target, texSRGBA);
	glTexStorage2D(target, 1, GL_SRGB8_ALPHA8, TEX_WIDTH, TEX_HEIGHT);
	glTexSubImage2D(target, 0, 0, 0, TEX_WIDTH, TEX_HEIGHT,
			GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, texData);

	/* Create RGB texture */
	glGenTextures(1, &texRGBA);
	glBindTexture(target, texRGBA);
	glTexStorage2D(target, 1, GL_RGBA8, TEX_WIDTH, TEX_HEIGHT);

	/* Copy data from sRGBA to RGBA texture using arb_copy_image */
	glCopyImageSubData(texSRGBA, target, 0, 0, 0, 0,
			   texRGBA,  target, 0, 0, 0, 0,
			   TEX_WIDTH, TEX_HEIGHT, 1);

	/* reset texData to gray */
	for (i = 0; i < TEX_NUMPIXELS; ++i) {
		texData[i] = GRAY;
	}

	/* Get tex image data from the RGBA texture */
	glGetTexImage(target, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, texData);

	if (texData[0] != RED) {
		printf("Expected 0x%08x but found 0x%08x \n",
			RED, texData[0]);
		/* clean up */
		glDeleteTextures(1, &texSRGBA);
		glDeleteTextures(1, &texRGBA);
		return false;
	}

	/* should have been no errors */
	if (!piglit_check_gl_error(GL_NO_ERROR))
		return false;

	/* if we get here, we passed */
	glDeleteTextures(1, &texSRGBA);
	glDeleteTextures(1, &texRGBA);
	return true;
}


enum piglit_result
piglit_display(void)
{
	bool pass = test_srgb_copy();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


void
piglit_init(int argc, char *argv[])
{
	piglit_require_extension("GL_ARB_copy_image");
	piglit_require_extension("GL_ARB_texture_storage");
	piglit_require_extension("GL_EXT_texture_sRGB");
}
