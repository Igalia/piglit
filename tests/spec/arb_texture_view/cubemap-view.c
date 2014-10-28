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
 * With Nvidia OpenGL drivers, if we create a TextureView from a cubemap
 * face other than GL_TEXTURE_CUBE_MAP_POSITIVE_X and attach it to an FBO,
 * we cannot read back the correct data in the original cubemap texture
 * by glGetTexImage() with a system memory pointer right after a clearing
 * or drawing call.
 *
 * Known to be
 *      -- Present in : Nvidia GTX 650, driver - 325.15
 *      -- Fixed in   :
 */


#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_core_version = 32;
	config.supports_gl_compat_version = 32;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

#define WIDTH 32
#define HEIGHT 32
#define LEVELS 6
#define COLOR_RED        0xFF0000FF
#define COLOR_GREEN      0x00FF00FF
#define COLOR_BLUE       0x0000FFFF
#define COLOR_CYAN       0x00FFFFFF
#define COLOR_MAGENTA    0xFF00FFFF
#define COLOR_YELLOW     0xFFFF00FF
#define COLOR_GRAY       0x7F7F7FFF
#define CLEAR_COLOR      0x000033FF
#define NUM_VERTICES     4
#define VIEW_LEVEL       3
#define NUM_FACES        6

static bool
test_cubemap_view(void)
{
	const GLuint white = 0xffffffff;
	const unsigned int colors[LEVELS] = {COLOR_RED, COLOR_GREEN,
					     COLOR_BLUE, COLOR_CYAN,
					     COLOR_CYAN, COLOR_MAGENTA};
	const unsigned int numPixels = WIDTH * HEIGHT;
	GLuint texData[WIDTH * HEIGHT];
	GLuint i, cubeTex, view, fbo, f;

	for (i = 0; i < numPixels; ++i) {
		texData[i] = white;
	}

	/* Create a cubemap textures */
	glGenTextures(1, &cubeTex);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeTex);
	glTexStorage2D(GL_TEXTURE_CUBE_MAP, 1, GL_RGBA8, WIDTH, HEIGHT);

	for (f = 0; f < NUM_FACES; f++) {
		for (i = 0; i < numPixels; i++) {
			texData[i] = colors[f];
		}

		glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + f,
				0, 0, 0, WIDTH, HEIGHT,
				GL_RGBA, GL_UNSIGNED_INT_8_8_8_8,
				texData);
	}

	/* Create a texture view from the negative X face of the cubemap */
	glGenTextures(1, &view);
	glTextureView(view, GL_TEXTURE_2D, cubeTex, GL_RGBA8,
		      0, 1, 1 /* -X face */, 1);
	glBindTexture(GL_TEXTURE_2D, view);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		return false;

	/* Setup the FBO */
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			       GL_TEXTURE_2D, view, 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) !=
	    GL_FRAMEBUFFER_COMPLETE) {
		printf("incomplete framebuffer at line %d\n", __LINE__);
		return false;
	}

	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) !=
	    GL_FRAMEBUFFER_COMPLETE) {
		printf("incomplete framebuffer at line %d\n", __LINE__);
		return false;
	}

	/* Clear and draw */
	glViewport(0, 0, WIDTH, HEIGHT);
	glClearColor(((CLEAR_COLOR >> 24) & 0xFF) / 255.0f,
		     ((CLEAR_COLOR >> 16) & 0xFF) / 255.0f,
		     ((CLEAR_COLOR >> 8)  & 0xFF) / 255.0f,
		     ((CLEAR_COLOR) & 0xFF) / 255.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	/* Read back */
	glPixelStorei(GL_PACK_ROW_LENGTH, WIDTH);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	memset(texData, 0, sizeof(texData));
	glGetTexImage(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0,
		      GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, texData);

	if (texData[0] != CLEAR_COLOR) {
		printf("At pixel (0,0) expected 0x%x but found 0x%x\n",
			CLEAR_COLOR, texData[0]);
		/* clean up */
		glDeleteTextures(1, &cubeTex);
		glDeleteTextures(1, &view);
		glDeleteFramebuffers(1, &fbo);
		return false;
	}

	if (!piglit_check_gl_error(GL_NO_ERROR))
		return false;

	glDeleteTextures(1, &cubeTex);
	glDeleteTextures(1, &view);
	glDeleteFramebuffers(1, &fbo);
	return true;
}


enum piglit_result
piglit_display(void)
{
	bool pass = test_cubemap_view();
	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_ARB_texture_storage");
	piglit_require_extension("GL_ARB_texture_view");
}
