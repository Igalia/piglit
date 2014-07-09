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
 * This test exercises an NVIDIA driver bug where using glGetTexImage
 * to read a cubemap face into a PBO fails.  It appears that glGetTexImage
 * always reads from the +X face.
 */


#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 15;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA;
PIGLIT_GL_TEST_CONFIG_END


#undef NUMCOLORS
#define NUMCOLORS        7
#define TEX_WIDTH        32
#define TEX_HEIGHT       32
#define TEX_NUMPIXELS    (TEX_WIDTH * TEX_HEIGHT)


static const GLuint Colors[NUMCOLORS] = {
	0xFF0000FF,	 /* red */
	0x00FF00FF,	 /* green */
	0x0000FFFF,	 /* blue */
	0x00FFFFFF,	 /* cyan */
	0xFF00FFFF,	 /* magenta */
	0xFFFF00FF,	 /* yellow */
	0x7F7F7FFF,	 /* gray */
};


/**
 * Test one cube map face to see if glGetTexImage from a cube face into
 * a PBO works correctly.
 */
static bool
test_face(GLuint face)
{
	const GLenum cubeFaceTarget = GL_TEXTURE_CUBE_MAP_POSITIVE_X + face;
	const GLuint expectedColor = Colors[face];
	GLuint texData[TEX_NUMPIXELS];
	GLuint cubeTex, fbo, packPBO;
	GLuint f, i;
	void *ptr;

	/* Create the cubemap texture. */
	glGenTextures(1, &cubeTex);
	glActiveTextureARB(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeTex);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, TEX_WIDTH);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	for (f = 0; f < 6; ++f) {
		for (i = 0; i < TEX_NUMPIXELS; ++i) {
			texData[i] = Colors[f];
		}
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + f, 0,
			     GL_SRGB8_ALPHA8, TEX_WIDTH, TEX_HEIGHT, 0, GL_BGRA,
			     GL_UNSIGNED_INT_8_8_8_8_REV, texData);
	}

	/* Setup the FBO. */
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			       cubeFaceTarget, cubeTex, 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) !=
	    GL_FRAMEBUFFER_COMPLETE) {
		printf("Incomplete framebuffer object\n");
		return false;
	}
	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	/* Read back cubemap face into PBO */
	glGenBuffers(1, &packPBO);
	glBindBuffer(GL_PIXEL_PACK_BUFFER, packPBO);
	glBufferData(GL_PIXEL_PACK_BUFFER, TEX_NUMPIXELS * sizeof(GLuint),
						NULL, GL_STREAM_READ);
	glPixelStorei(GL_PACK_ROW_LENGTH, TEX_WIDTH);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glGetTexImage(cubeFaceTarget, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV,
		      NULL);

	/* Map pack PBO to get results. */
	ptr = glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
	if (!ptr) {
		printf("failed to map PBO\n");
		return false;
	}
	for (i = 0; i < TEX_NUMPIXELS; ++i) {
		texData[i] = Colors[6];	 /* gray */
	}

	memcpy(texData, ptr, TEX_NUMPIXELS * sizeof(GLuint));

	glUnmapBuffer(GL_PIXEL_PACK_BUFFER);

	if (expectedColor != texData[0]) {
		printf("Colors don't match for face %u\n", face);
		printf("Expected 0x%08x but found 0x%08x \n",
		       expectedColor, texData[0]);
		return false;
	}

	glDeleteTextures(1, &cubeTex);
	glDeleteFramebuffers(1, &fbo);
	glDeleteBuffers(1, &packPBO);

	/* if we get here we passed */
	return true;
}


enum piglit_result
piglit_display(void)
{
	GLuint i;
	bool pass = true;
	for (i = 0; i < 6; i++) {
		pass = test_face(i) && pass;
	}
	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


void
piglit_init(int argc, char *argv[])
{
	piglit_require_extension("GL_ARB_texture_cube_map");
	piglit_require_extension("GL_ARB_pixel_buffer_object");
	piglit_require_extension("GL_ARB_framebuffer_object");
}
