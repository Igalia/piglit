/*
 * Copyright (c) 2013 VMware, Inc.
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
 * This test exercises an NVIDIA driver bug where reading back
 * a texture image via an sRGBA view returns invalid texel data.
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
test_srgb_view(GLenum view_format)
{
	GLenum target = GL_TEXTURE_2D;
	GLuint texData[TEX_NUMPIXELS];
	GLuint tex, view, i;

	/* init tex data */
	for (i = 0; i < TEX_NUMPIXELS; ++i) {
		texData[i] = RED;
	}

	/* Create RGBA texture */
	glGenTextures(1, &tex);
	glActiveTextureARB(GL_TEXTURE0);
	glBindTexture(target, tex);
	glTexStorage2D(target, 1, GL_RGBA8, TEX_WIDTH, TEX_HEIGHT);
	glTexSubImage2D(target, 0, 0, 0, TEX_WIDTH, TEX_HEIGHT,
			GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, texData);

	/* Create sRGB texture view */
	glGenTextures(1, &view);
	glTextureView(view, target, tex, view_format, 0, 1, 0, 1);
	glBindTexture(target, view);
	glTexParameteri(target, GL_TEXTURE_BASE_LEVEL, 0);

	/* reset texData to gray */
	for (i = 0; i < TEX_NUMPIXELS; ++i) {
		texData[i] = GRAY;
	}

	/* Get tex image data from the view */
	glGetTexImage(target, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, texData);

	if (texData[0] != RED) {
		printf("Wrong color for %s texture view.\n",
		       piglit_get_gl_enum_name(view_format));
		printf("Expected 0x%08x but found 0x%08x \n",
		       RED, texData[0]);
		return false;
	}

	/* should have been no errors */
	if (!piglit_check_gl_error(GL_NO_ERROR))
		return false;

	/* if we get here, we passed */
	glDeleteTextures(1, &view);
	glDeleteTextures(1, &tex);
	return true;
}


enum piglit_result
piglit_display(void)
{
	bool pass = true;

	pass = test_srgb_view(GL_RGBA8) && pass;
	pass = test_srgb_view(GL_SRGB8_ALPHA8) && pass;

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


void
piglit_init(int argc, char *argv[])
{
	piglit_require_extension("GL_ARB_texture_storage");
	piglit_require_extension("GL_ARB_texture_view");
	piglit_require_extension("GL_EXT_texture_sRGB");
}
