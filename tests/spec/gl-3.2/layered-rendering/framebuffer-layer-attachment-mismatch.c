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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/** \file
 *
 * From section 4.4.4 (Framebuffer Completeness) of the GL 3.2 spec,
 * under the "Whole Framebuffer Completeness" heading:
 *
 *     If any framebuffer attachment is layered, all populated
 *     attachments must be layered.  Additionally, all populated color
 *     attachments must be from textures of the same target.
 *
 *     { FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS }
 *
 * This test verifies that if two layered framebuffer attachments use
 * different texture targets, then the framebuffer is incomplete, even
 * if the two attachments have the same number of layer.  We test this
 * by using a cube map texture and a 2D array texture containing 6
 * layers.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 32;
	config.supports_gl_core_version = 32;

PIGLIT_GL_TEST_CONFIG_END


#define TEX_SIZE 32


static const GLenum cube_map_faces[6] = {
	GL_TEXTURE_CUBE_MAP_POSITIVE_X,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
};


void
piglit_init(int argc, char **argv)
{
	GLuint textures[2];
	GLuint fbo;
	int i;
	GLenum fbstatus;

	glGenTextures(2, textures);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textures[0]);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	for (i = 0; i < 6; i++) {
		glTexImage2D(cube_map_faces[i], 0 /* level */, GL_RGBA,
			     TEX_SIZE, TEX_SIZE, 0 /* border */, GL_RGBA,
			     GL_FLOAT, NULL);
	}
	glBindTexture(GL_TEXTURE_2D_ARRAY, textures[1]);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0 /* level */, GL_RGBA, TEX_SIZE,
		     TEX_SIZE, 6, 0 /* border */, GL_RGBA, GL_FLOAT, NULL);
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textures[0],
			     0 /* level */);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, textures[1],
			     0 /* level */);
	fbstatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (fbstatus != GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS) {
		printf("Expected GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS, got "
		       "%s\n", piglit_get_gl_enum_name(fbstatus));
		piglit_report_result(PIGLIT_FAIL);
	}
	piglit_report_result(PIGLIT_PASS);
}


enum piglit_result
piglit_display(void)
{
	/* Should never be reached */
	return PIGLIT_FAIL;
}
