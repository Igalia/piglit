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


/** @file clear-color.c
 *
 * Section 4.4.7(Framebuffer Objects) From GL spec 3.2 core:
 * When the Clear or ClearBuffer* commands are used to clear a layered
 * framebuffer attachment, all layers of the attachment are cleared.
 *
 * Test Layout
 *         Tex1     Tex2
 *      *--------*--------*    Each Layer for both tex1 and tex2 will be
 *      | layer3 | layer3 |   different colors.
 *      *--------*--------*
 *      | layer2 | layer2 |    Tex1 will be cleared using glClear()
 *      *--------*--------*
 *      | layer1 | layer1 |    Tex2 will be cleared using glClearBuffer()
 *      *--------*--------*
 *
 *      Result:
 *        Layer 1-3 of both tex1 and tex2 should be the clearColor
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 32;
	config.supports_gl_core_version = 32;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static GLuint fbo[2];
static GLuint texture[2];
static const int layers = 3;

bool
display_layered_texture(int x, int y, int w, int h, int texWidth, int texHeight,
			GLenum textureType, GLuint texture, int layers) {
	GLuint tempFBO;
	int i;
	int dx1, dy1, dx2, dy2;

	dx1 = x;
	dx2 = x+w;

	/* Gen temp fbo */
	glGenFramebuffers(1, &tempFBO);

	/* Loop through each layer, attaching the individual layer to the
	 * temp fbo, then blit fbo to the correct location on screen
	 */
	for(i = 0; i < layers; i++) {
		GLenum framebufferStatus;

		dy1 = y + (i)  *(h/layers);
		dy2 = y + (i+1)*(h/layers);

		glBindFramebuffer(GL_FRAMEBUFFER, tempFBO);
		glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
					  texture, 0, i);

		framebufferStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if(framebufferStatus != GL_FRAMEBUFFER_COMPLETE) {
			printf("Framebuffer Status: %s\n",
			       piglit_get_gl_enum_name(framebufferStatus));
			return false;
		}

		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, tempFBO);
		glBlitFramebuffer(0, 0, texWidth, texHeight,
				  dx1, dy1, dx2, dy2, GL_COLOR_BUFFER_BIT,
				  GL_NEAREST);
	}

	/* Cleanup temp fbo */
	glBindFramebuffer(GL_FRAMEBUFFER, piglit_winsys_fbo);
	glDeleteFramebuffers(1, &tempFBO);

	return piglit_check_gl_error(GL_NO_ERROR);
}

void
piglit_init(int argc, char **argv)
{
	int i, j;
	GLenum fbstatus;
	float *colorLayers =
		malloc(sizeof(float) * layers *
		       piglit_width * piglit_height * 3);
	float colors[3][3] = {
		{0.0, 0.0, 1.0},
		{0.0, 1.0, 0.0},
		{1.0, 0.0, 0.0}
	};

	/* Create color data for texture */
	for(j = 0; j < layers; j++) {
		float *thisLayer =
			&colorLayers[j * piglit_width * piglit_height * 3];
		for(i = 0; i < piglit_width*piglit_height; i++) {
			thisLayer[i*3+0] = colors[j][0];
			thisLayer[i*3+1] = colors[j][1];
			thisLayer[i*3+2] = colors[j][2];
		}
	}

	glGenTextures(2, texture);
	glGenFramebuffers(2, fbo);
	for(i = 0; i < 2; i++) {
		/* Create texture */
		glBindTexture(GL_TEXTURE_2D_ARRAY, texture[i]);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_R, GL_REPEAT);
		glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGB, piglit_width, piglit_height,
			     layers, 0, GL_RGB, GL_FLOAT, colorLayers);

		/* Gen Framebuffer */
		glBindFramebuffer(GL_FRAMEBUFFER, fbo[i]);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
				     texture[i], 0);

		fbstatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if(fbstatus != GL_FRAMEBUFFER_COMPLETE){
			printf("%s\n", piglit_get_gl_enum_name(fbstatus));
			piglit_report_result(PIGLIT_FAIL);
		}
	}

	if(!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);
}


enum piglit_result
piglit_display(void)
{
	bool pass = true;
	const float clearColor[3] = { 1, 1, 0 };

	/* Clear Defualt Framebuffer */
	glBindFramebuffer(GL_FRAMEBUFFER, piglit_winsys_fbo);
	glClearColor(1,1,0,1);
	glClear(GL_COLOR_BUFFER_BIT);

	/* Clear texture[0] with glClear() */
	glBindFramebuffer(GL_FRAMEBUFFER, fbo[0]);
	glClearColor(clearColor[0], clearColor[1], clearColor[2], 1);
	glClear(GL_COLOR_BUFFER_BIT);

	/* Clear texture[1] with glClearBuffer() */
	glBindFramebuffer(GL_FRAMEBUFFER, fbo[1]);
	glClearBufferfv(GL_COLOR, 0, clearColor);

	/* Display glClear texture */
	if(!display_layered_texture(0, 0, piglit_width/2, piglit_height,
				    piglit_width, piglit_height, GL_TEXTURE_2D_ARRAY,
				    texture[0], layers)) {
		printf("Failed to display layered texture for glClear\n");
		pass = false;
	}

	/* Display glClearBuffer texture */
	if(!display_layered_texture(piglit_width/2, 0, piglit_width/2,
				    piglit_height, piglit_width, piglit_height,
				    GL_TEXTURE_2D_ARRAY, texture[1], layers)) {
		printf("Failed to display layered texture for glClearBuffer\n");
		pass = false;
	}

	/* Check for passing conditions for glClear*/
	if(!piglit_probe_rect_rgb(0, 0, piglit_width/2, piglit_height,
				  clearColor)) {
		printf("Incorrect probe value for glClear test.\n");
		pass = false;
	}

	/* Check for passing conditions for glClearBuffer*/
	if(!piglit_probe_rect_rgb(piglit_width/2, 0, piglit_width/2,
				  piglit_height, clearColor)) {
		printf("Incorrect probe value for glClearBuffer test.\n");
		pass = false;
	}

	if(!piglit_check_gl_error(GL_NO_ERROR))
		pass = false;

	piglit_present_results();
	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
