/*
 * Copyright © 2013 Intel Corporation
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


/** @file blit.c
 *
 * Section 4.3.2(Reading and Copying Pixels) From GL spec 3.2 core:
 *   If the read framebuffer is layered (see section 4.4.7), pixel values are
 * read from layer zero. If the draw framebuffer is layered, pixel values are
 * written to layer zero. If both read and draw framebuffers are layered, the
 * blit operation is still performed only on layer zero.
 *
 * Test Layout
 * *-------*-------*    test1:
 * |       |       |      Source tex is layered, destination tex is layered
 * | test3 | test4 |    test2:
 * |       |       |      Source tex is layered, destination tex is not layered
 * *-------*-------*    test3:
 * |       |       |      Source tex is not layered, destination tex is layered
 * | test1 | test2 |    test4:
 * |       |       |      Source tex is not layered, destination tex is not layered
 * *-------*-------*
 *
 *    src dst           Each Test
 *   *---*---*             Display source tex layers on left
 *   |   |   | layer 1     Blit source tex to destination tex
 *   *---*---*             Display resulting layers
 *   |   |   | layer 2
 *   *---*---*
 */

#include "piglit-util-gl-common.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 32;
	config.supports_gl_core_version = 32;

	config.window_width  = 128;
	config.window_height = 128;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

/* Values Set in piglit init */
const int texWidth  = 32;
const int texHeight = 32;
const int texDepth  = 2;
const int texelsPerLayer = 32 * 32;
const int floatPerLayer  = 32 * 32 * 3;

static const float srcColors[2][3] = {
	{1, 0, 0}, {0, 1, 0}
};

static const float dstColors[2][3] = {
	{0, 0, 1}, {0, 1, 1}
};

/*
 * Blit the passed texture to the screen. If texture is layered,
 * loops through each layer and blit it to the screen.
 */
bool
display_texture(int x, int y, int w, int h,
		     GLuint tex, int layers)
{
	GLuint tempFBO;
	int i, dx1, dy1, dx2, dy2;
	GLenum fbStatus;

	dx1 = x;
	dx2 = x+w;

	/* Gen temp fbo to work with */
	glGenFramebuffers(1, &tempFBO);

	if(layers == 1) {
		dy1 = y;
		dy2 = y+h;

		glBindFramebuffer(GL_FRAMEBUFFER, tempFBO);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
				       GL_TEXTURE_2D, tex, 0);

		/* Blit layer to screen */
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, tempFBO);
		glBlitFramebuffer(0, 0, texWidth, texHeight,
				  dx1, dy1, dx2, dy2,
				  GL_COLOR_BUFFER_BIT, GL_NEAREST);
	} else {
		/* loop through each layer */
		for( i = 0; i < layers; i++) {
			dy1 = y + i*(h/layers);
			dy2 = y + (i+1)*(h/layers);

			/* Bind new layer to display */
			glBindFramebuffer(GL_FRAMEBUFFER, tempFBO);
			glFramebufferTextureLayer(GL_FRAMEBUFFER,
						  GL_COLOR_ATTACHMENT0,
						  tex, 0, i);

			fbStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
			if(fbStatus != GL_FRAMEBUFFER_COMPLETE) {
				printf("Framebuffer Status: %s\n",
				       piglit_get_gl_enum_name(fbStatus));
				return false;
			}

			/* Blit layer to screen */
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
			glBindFramebuffer(GL_READ_FRAMEBUFFER, tempFBO);
			glBlitFramebuffer(0, 0, texWidth, texHeight,
					  dx1, dy1, dx2, dy2,
					  GL_COLOR_BUFFER_BIT, GL_NEAREST);
		}
	}

	/* Cleanup temp fbo */
	glBindFramebuffer(GL_FRAMEBUFFER, piglit_winsys_fbo);
	glDeleteFramebuffers(1, &tempFBO);

	return piglit_check_gl_error(GL_NO_ERROR);
}

void
gen_color_data(float *colorData, int texelsPerLayer, int layers, bool useSrcTex)
{
	int i, j;
	for(j = 0; j < layers; j++) {
		for(i = 0; i < texelsPerLayer; i++) {
			int offset = j * texelsPerLayer * 3 + i * 3;
			if(useSrcTex) {
				colorData[offset + 0] = srcColors[j][0];
				colorData[offset + 1] = srcColors[j][1];
				colorData[offset + 2] = srcColors[j][2];
			} else {
				colorData[offset + 0] = dstColors[j][0];
				colorData[offset + 1] = dstColors[j][1];
				colorData[offset + 2] = dstColors[j][2];
			}
		}
	}
}

GLuint
create_bind_texture(GLenum textureType, bool useSrcTex)
{
	GLuint texture = 0;
	float *colorData = NULL;

	piglit_check_gl_error(GL_NO_ERROR);

	glGenTextures(1, &texture);
	glBindTexture(textureType, texture);

	glTexParameteri(textureType, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(textureType, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(textureType, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(textureType, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(textureType, GL_TEXTURE_WRAP_R, GL_REPEAT);

	switch(textureType) {
	case GL_TEXTURE_2D:
		colorData = malloc(floatPerLayer * sizeof(float));
		gen_color_data(colorData, texelsPerLayer, 1, useSrcTex);
		glTexImage2D(textureType, 0, GL_RGB, texWidth, texHeight, 0,
			     GL_RGB, GL_FLOAT, colorData);
		break;
	case GL_TEXTURE_3D:
		colorData = malloc(texDepth * floatPerLayer * sizeof(float));
		gen_color_data(colorData, texelsPerLayer, texDepth, useSrcTex);
		glTexImage3D(textureType, 0, GL_RGB, texWidth, texHeight,
			     texDepth, 0, GL_RGB, GL_FLOAT, colorData);
		break;
	}

	if(!piglit_check_gl_error(GL_NO_ERROR)) {
		glDeleteTextures(1, &texture);
		texture = 0;
	}

	if(colorData != NULL)
		free(colorData);

	return texture;
}

bool
testFramebufferBlitLayered(int x, int y, int w, int h,
			   bool srcLayered, bool dstLayered)
{
	bool pass = true;
	GLuint srcFBO, dstFBO;
	GLuint srcTex, dstTex;
	GLenum fbStatus;
	int srclayers = (srcLayered) ? (texDepth) : (1);
	int dstlayers = (dstLayered) ? (texDepth) : (1);

	/* Set up source fbo */
	glGenFramebuffers(1, &srcFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, srcFBO);

	piglit_check_gl_error(GL_NO_ERROR);
	if(srcLayered) {
		srcTex = create_bind_texture(GL_TEXTURE_3D, true);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
				     srcTex, 0);
	} else {
		srcTex = create_bind_texture(GL_TEXTURE_2D, true);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
				     GL_TEXTURE_2D, srcTex, 0);
	}

	/* Check source framebuffer status */
	fbStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if(fbStatus != GL_FRAMEBUFFER_COMPLETE) {
		printf("testFramebufferBlitLayered srcFBO Status: %s\n",
		       piglit_get_gl_enum_name(fbStatus));
		return false;
	}

	/* Set up dstt fbo */
	glGenFramebuffers(1, &dstFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, dstFBO);

	if(dstLayered) {
		dstTex = create_bind_texture(GL_TEXTURE_3D, false);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
				     dstTex, 0);
	} else {
		dstTex = create_bind_texture(GL_TEXTURE_2D, false);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
				       GL_TEXTURE_2D, dstTex, 0);
	}

	/* Check destination framebuffer status */
	fbStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if(fbStatus != GL_FRAMEBUFFER_COMPLETE) {
		printf("testFramebufferBlitLayered dstFBO Status: %s\n",
		       piglit_get_gl_enum_name(fbStatus));
		return false;
	}

	/* Check for if any errors have occured */
	if(!piglit_check_gl_error(GL_NO_ERROR)) {
		printf("Error setting up framebuffers for test.\n");
		return false;
	}

	/* Blit from source to destination framebuffers */
	glBindFramebuffer(GL_READ_FRAMEBUFFER, srcFBO);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dstFBO);
	glBlitFramebuffer(0, 0, texWidth, texHeight,
			  0, 0, texWidth, texHeight,
			  GL_COLOR_BUFFER_BIT, GL_LINEAR);

	/* Display the results */
	display_texture(x, y, w/2, h, srcTex, srclayers);
	display_texture(x+w/2, y, w/2, h, dstTex, dstlayers);

	/* Check for pass condition */
	if(dstLayered) {
		pass = piglit_probe_rect_rgb(x+w/2, y, w/2, h/2, srcColors[0])
		       && pass;
		pass = piglit_probe_rect_rgb(x+w/2, y+h/2, w/2, h/2,
					     dstColors[1])
		       && pass;
	} else {
		pass = piglit_probe_rect_rgb(x+w/2, y, w/2, h, srcColors[0])
		       && pass;
	}

	/* Clean up */
	glBindFramebuffer(GL_FRAMEBUFFER, piglit_winsys_fbo);
	glDeleteFramebuffers(1, &srcFBO);
	glDeleteFramebuffers(1, &dstFBO);
	glDeleteTextures(1, &srcTex);
	glDeleteTextures(1, &dstTex);

	/* Check for if any errors have occured */
	if(!piglit_check_gl_error(GL_NO_ERROR)) {
		printf("Error setting up framebuffers for test.\n");
		return false;
	}

	return pass;
}

void
piglit_init(int argc, char **argv)
{

}

enum piglit_result
piglit_display(void)
{
	bool pass = true;
	int hw = piglit_width/2;
	int hh = piglit_height/2;

	glBindFramebuffer(GL_FRAMEBUFFER, piglit_winsys_fbo);
	glClearColor(1,1,1,1);
	glClear(GL_COLOR_BUFFER_BIT);

	/* Source is layered, destination is layered */
	pass = testFramebufferBlitLayered( 0,  0, hw, hh, true,  true)  && pass;

	/* Source is layered, destination is not layered */
	pass = testFramebufferBlitLayered(hw,  0, hw, hh, true,  false) && pass;

	/* Source not is layered, destination is layered */
	pass = testFramebufferBlitLayered( 0, hh, hw, hh, false, true)  && pass;

	/* Source not is layered, destination is not layered */
	pass = testFramebufferBlitLayered(hw, hh, hw, hh, false, false) && pass;

	/* Check for if any errors have occured */
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
