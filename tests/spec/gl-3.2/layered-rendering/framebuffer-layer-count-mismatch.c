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
 * ARB_geometry_shader4 doesn't permit layered framebuffers to have
 * mismatched layer counts.  From ARB_geometry_shader4, under the
 * heading "add to the list of conditions necessary for completeness":
 *
 *     * If any framebuffer attachment is layered, all attachments
 *       must have the same layer count.  For three-dimensional
 *       textures, the layer count is the depth of the attached
 *       volume.  For cube map textures, the layer count is always
 *       six.  For one- and two-dimensional array textures, the layer
 *       count is simply the number of layers in the array texture.
 *       { FRAMEBUFFER_INCOMPLETE_LAYER_COUNT_ARB }
 *
 * However, this restriction was lifted when geometry shaders were
 * adopted into OpenGL 3.2.  Instead, OpenGL 3.2 states, in section
 * 4.4.7 (Layered Framebuffers):
 *
 *     When fragments are written to a layered framebuffer, the
 *     fragment’s layer number selects an image from the array of
 *     images at each attachment point to use for the stencil test
 *     (see section 4.1.5), depth buffer test (see section 4.1.6), and
 *     for blending and color buffer writes (see section 4.1.8).  If
 *     the fragment’s layer number is negative, or greater than the
 *     minimum number of layers of any attachment, the effects of the
 *     fragment on the framebuffer contents are undefined.
 *
 * This test verifies that a framebuffer is considered complete even
 * if two different attachments have different layer counts.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 32;
	config.supports_gl_core_version = 32;

PIGLIT_GL_TEST_CONFIG_END


#define TEX_SIZE 32


void
piglit_init(int argc, char **argv)
{
	GLuint textures[2];
	GLuint fbo;
	GLenum fbstatus;

	glGenTextures(2, textures);
	glBindTexture(GL_TEXTURE_2D_ARRAY, textures[0]);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0 /* level */, GL_RGBA, TEX_SIZE,
		     TEX_SIZE, 2, 0 /* border */, GL_RGBA, GL_FLOAT, NULL);
	glBindTexture(GL_TEXTURE_2D_ARRAY, textures[1]);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0 /* level */, GL_RGBA, TEX_SIZE,
		     TEX_SIZE, 4, 0 /* border */, GL_RGBA, GL_FLOAT, NULL);
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textures[0],
			     0 /* level */);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, textures[1],
			     0 /* level */);
	fbstatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (fbstatus != GL_FRAMEBUFFER_COMPLETE) {
		printf("Framebuffer incomplete: %s\n",
		       piglit_get_gl_enum_name(fbstatus));
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
