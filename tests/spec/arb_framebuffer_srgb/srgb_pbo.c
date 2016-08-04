/*
 * Copyright © 2016 VMware, Inc.
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
 *
 * Authors:
 *    Neha Bhende <bhenden@vmware.com>
 */

/**
 * \file srgb-pbo.c
 *
 * To test a bug in glClear() with GL_FRAMEBUFFER_SRGB enabled on nVidia GPUs.
 * nVidia GPUs seems to disable GL_FRAMEBUFFER_SRGB after using a PBO to
 * define a GL_BGRA4 texture.
 * This test creates fbo and the clear it with GL_FRAMEBUFFER_SRGB.
 * Before reading fbo pixels, test performs some PBO operations and creates
 * GL_RGBA4 texture. So when we readback fbo pixels, it has GL_FRAMEBUFFER_SRGB
 * disabled values.
 * For example, we clear the framebuffer with (R, G, B, A)=(0, 0, 127, 0)
 * with GL_FRAMEBUFFER_SRGB enabled, we should get (0, 0, 187, 0) but we get
 * (0, 0, 127, 0).
 */

#include <stdio.h>
#include <inttypes.h>
#include <math.h>
#include "piglit-util-gl.h"


PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 13;
	config.window_visual = PIGLIT_GL_VISUAL_RGB;
PIGLIT_GL_TEST_CONFIG_END

const unsigned int texWidth = 256;
const unsigned int texHeight = 256;
const unsigned int level = 0;
const unsigned int numLevels = 1;
const GLenum texFormat = GL_BGRA;
const GLenum texInternalFormat = GL_SRGB8_ALPHA8;
const GLenum texType = GL_UNSIGNED_INT_8_8_8_8_REV;

struct BGRA4
{
	unsigned blue:4;
	unsigned green:4;
	unsigned red:4;
	unsigned alpha:4;
};

struct BGRA8
{
	unsigned char blue;
	unsigned char green;
	unsigned char red;
	unsigned char alpha;
};


static GLuint
createTexture2D(const GLenum internalFormat, unsigned int numMipmapLevels,
		unsigned int width, unsigned int height)
{
	GLuint tex;

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);

	glTexStorage2D(GL_TEXTURE_2D, numMipmapLevels, internalFormat,
			width, height);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	return tex;
}


enum piglit_result
test2dTexelAt(const GLuint tex2D, unsigned int mipLevel, unsigned int x,
	      unsigned int y, struct BGRA8 expected, unsigned int width,
	      unsigned int height)
{
	struct BGRA8 *texData = (struct BGRA8 *)
		malloc(width * height *sizeof(struct BGRA8));
	enum piglit_result status = PIGLIT_PASS;

	glBindTexture(GL_TEXTURE_2D, tex2D);
	glPixelStorei(GL_PACK_ROW_LENGTH, width);
	glPixelStorei(GL_PACK_IMAGE_HEIGHT, height);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);

	glGetTexImage(GL_TEXTURE_2D, mipLevel, texFormat, texType, texData);

	const unsigned int offset = y * width + x;
	struct BGRA8 pixel = texData[offset];

	if (memcmp(&expected, &pixel, sizeof(struct BGRA8))) {
		status = PIGLIT_FAIL;
		fprintf(stdout," texel mismatch at position (%u, %u): \n", x,y);
		fprintf(stdout, "expected {%u, %u, %u, %u},"
			" found {%u, %u, %u, %u}\n",
			expected.red, expected.green,
			expected.blue, expected.alpha,
			pixel.red, pixel.green, pixel.blue, pixel.alpha);
	}

	free(texData);
	return status;
}


/** Borrowed from Mesa */
static float
linear_to_srgb(float cl)
{
	if (cl <= 0.0f)
		return 0.0f;
	else if (cl < 0.0031308f)
		return 12.92f * cl;
	else if (cl < 1.0f)
		return 1.055f * powf(cl, 0.41666f) - 0.055f;
	else
		return 1.0f;
}


enum piglit_result
piglit_display(void)
{
	/* not reached */
	return PIGLIT_FAIL;
}


void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_ARB_texture_storage");
	piglit_require_extension("GL_ARB_framebuffer_object");
	piglit_require_extension("GL_ARB_pixel_buffer_object");
	piglit_require_extension("GL_ARB_framebuffer_sRGB");

	GLuint tex2D = createTexture2D(texInternalFormat, numLevels,
				       texWidth, texHeight);
	const struct BGRA8 clearVal = { 127, 63, 192, 0 };
	struct BGRA8 sRGBVal;

	/* compute expected sRGB value */
	sRGBVal.red = (unsigned) (255 * linear_to_srgb(clearVal.red / 255.0));
	sRGBVal.green = (unsigned) (255 * linear_to_srgb(clearVal.green / 255.0));
	sRGBVal.blue = (unsigned) (255 * linear_to_srgb(clearVal.blue / 255.0));
	sRGBVal.alpha = 0;

	const unsigned int numPixels = texWidth * texHeight;
	GLuint fbo;
	enum piglit_result status = PIGLIT_PASS;

	/* Create a FBO */
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	/* using 2D texture as frame buffer texture */
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
				GL_TEXTURE_2D, tex2D, level);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) !=
	    GL_FRAMEBUFFER_COMPLETE) {
		fprintf(stdout, "Error: Cannot attach tex2D to FBO!\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) !=
	    GL_FRAMEBUFFER_COMPLETE) {
		fprintf(stderr, "Error: Cannot set Draw Buffer!\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	/* clearing texture tex2D with GL_FRAMEBUFFER_SRGB */
	glViewport(0, 0, texWidth, texHeight);
	glEnable(GL_FRAMEBUFFER_SRGB);
	glClearColor(clearVal.red / 255.0f, clearVal.green / 255.0f,
		     clearVal.blue / 255.0f, clearVal.alpha / 255.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	/* following code basically triggers the issue.
	 * We have sRGB fbo. PBO operations are done and GL_RGBA4
	 * texture is created, before readback.
	 * When we readback framebuffer pixels, it doesn't have expected
	 * sRGB values.
	 */

	GLuint tex;
	tex = createTexture2D(GL_RGBA4, 1, texWidth, texHeight);

	/* Create a PBO and initialize it to zeros */
	GLuint unpackPbo;
	const unsigned int texDataLength = numPixels * sizeof(struct BGRA4);

	glGenBuffers(1, &unpackPbo);
	glBindBuffer(GL_COPY_READ_BUFFER, unpackPbo);
	glBufferData(GL_COPY_READ_BUFFER, texDataLength, 0, GL_STREAM_DRAW);
	void *ptr =
		glMapBufferRange(GL_COPY_READ_BUFFER, 0, texDataLength,
				 GL_MAP_WRITE_BIT);

	if (!ptr) {
		fprintf(stderr, "Error: Failed to map PBO!\n");
		piglit_report_result(PIGLIT_FAIL);
	}
	memset(ptr, 0, texDataLength);
	glUnmapBuffer(GL_COPY_READ_BUFFER);

	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, unpackPbo);

	glBindTexture(GL_TEXTURE_2D, tex);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, texWidth);
	glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, texHeight);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glTexSubImage2D(GL_TEXTURE_2D, level,
			0, 0,
			texWidth, texHeight,
			GL_BGRA, GL_UNSIGNED_SHORT_4_4_4_4_REV,
			0);

	/* now recheck 2D texture tex2D data */
	status = test2dTexelAt(tex2D, level, 0, 0, sRGBVal,
			       texWidth, texHeight);

	piglit_report_result(status);
}
