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
 * This test exercises an AMD driver bug where if we enable
 * GL_FRAMEBUFFER_SRGB, gamma corrected colors are written to the render
 * target even if the target is not sRGB buffer.  The buffer should get
 * linear colors.
 *
 * Known to be
 *      -- Present in : AMD Linux driver - 13.12
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


static bool
test_srgb_clear(void)
{
	static const GLfloat clearColor[4] = { 0.25, 0.5, 0.75, 1.0 };
	const float tolerance = 2.0f / 255.0f;
	GLfloat texColor[4];
	GLenum target = GL_TEXTURE_2D;
	GLuint texData[TEX_NUMPIXELS];
	GLuint tex, fbo, i;
        GLint val;

	/* init tex data */
	for (i = 0; i < TEX_NUMPIXELS; ++i) {
		texData[i] = RED;
	}

	/* Create RGBA texture */
	glGenTextures(1, &tex);
	glActiveTextureARB(GL_TEXTURE0);
	glBindTexture(target, tex);
	glTexImage2D(target, 0, GL_RGBA8, TEX_WIDTH, TEX_HEIGHT,
		     0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, texData);

	/* Create framebuffer and attach the texture */
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			       GL_TEXTURE_2D, tex, 0);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) !=
	    GL_FRAMEBUFFER_COMPLETE) {
		printf("incomplete framebuffer at line %d\n", __LINE__);
		return false;
	}

	/* Check color encoding */
	glGetFramebufferAttachmentParameterivEXT(GL_FRAMEBUFFER,
				    GL_COLOR_ATTACHMENT0,
				    GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING,
				    &val);
	if (val != GL_LINEAR) {
		printf("Unexpected color encoding.  Expected GL_LINEAR, "
		       "found %s\n",
		       piglit_get_gl_enum_name(val));
		return false;
	}

	/* Issue a clear */
	glViewport(0, 0, TEX_WIDTH, TEX_HEIGHT);
	glEnable(GL_FRAMEBUFFER_SRGB);

	glClearColor(clearColor[0],
		     clearColor[1],
		     clearColor[2],
		     clearColor[3]);

	glClear(GL_COLOR_BUFFER_BIT);

	/* reset texData to black */
	memset(texData, 0, sizeof(texData));

	/* Get tex image data from the RGBA texture */
	glGetTexImage(target, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, texData);

	/* Convert a texel to float */
	texColor[0] = ((texData[0] >>  0) & 0xff) / 255.0f;
	texColor[1] = ((texData[0] >>  8) & 0xff) / 255.0f;
	texColor[2] = ((texData[0] >> 16) & 0xff) / 255.0f;
	texColor[3] = ((texData[0] >> 24) & 0xff) / 255.0f;

	if (fabs(texColor[0] - clearColor[0]) > tolerance ||
	    fabs(texColor[1] - clearColor[1]) > tolerance ||
	    fabs(texColor[2] - clearColor[2]) > tolerance ||
	    fabs(texColor[3] - clearColor[3]) > tolerance) {
		printf("Expected (%g, %g, %g, %g) but found (%g, %g, %g, %g)\n",
		       clearColor[0],
		       clearColor[1],
		       clearColor[2],
		       clearColor[3],
		       texColor[0],
		       texColor[1],
		       texColor[2],
		       texColor[3]);
		/* clean up */
		glDeleteTextures(1, &tex);
		glDeleteFramebuffers(1, &fbo);
		return false;
	}

	/* should have been no errors */
	if (!piglit_check_gl_error(GL_NO_ERROR))
		return false;

	/* if we get here, we passed */
	glDeleteTextures(1, &tex);
	glDeleteFramebuffers(1, &fbo);
	return true;
}


enum piglit_result
piglit_display(void)
{
	bool pass = test_srgb_clear();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


void
piglit_init(int argc, char *argv[])
{
	piglit_require_extension("GL_ARB_framebuffer_sRGB");
}
