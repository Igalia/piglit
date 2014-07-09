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


/** @file framebuffer-layer-complete.c
 *
 * Section 4.4.4(FRAMEBUFFER OBJECTS) From GL spec 3.2 core:
 * If any framebuffer attachment is layered, all populated attachments must be
 * layered. Additionally, all populated color attachments must be from textures
 * of the same target.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 32;
	config.supports_gl_core_version = 32;

PIGLIT_GL_TEST_CONFIG_END

const int texWidth = 30;
const int texHeight = 30;
const int texDepth = 2;

GLuint
create_bind_texture(GLenum textureType) {
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(textureType, texture);

	switch(textureType) {
	case GL_TEXTURE_2D:
		glTexParameteri(textureType, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(textureType, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(textureType, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(textureType, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexImage2D(textureType, 0, GL_RGB, texWidth, texHeight,
			     0, GL_RGB, GL_FLOAT, NULL);
		break;
	case GL_TEXTURE_3D:
		glTexParameteri(textureType, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(textureType, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(textureType, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(textureType, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(textureType, GL_TEXTURE_WRAP_R, GL_REPEAT);
		glTexImage3D(textureType, 0, GL_RGB, texWidth, texHeight,
			     texDepth, 0, GL_RGB, GL_FLOAT, NULL);
		break;
	default:
		printf("Unexpected textureType in create_bind_texture()\n");
		piglit_report_result(PIGLIT_FAIL);
		break;
	}

	return texture;
}

void
attach_texture(GLenum framebuffer, GLenum attachment, GLenum textureType,
	       GLuint texture)
{
	switch(textureType) {
		case GL_TEXTURE_2D:
			glFramebufferTexture2D(framebuffer, attachment,
					       textureType, texture, 0);
			break;
		case GL_TEXTURE_3D:
			glFramebufferTexture(framebuffer, attachment,
					     texture, 0);
			break;
	}
}

bool
CheckFramebufferStatus(GLenum expected) {
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if(status != expected) {
		printf("Expected Framebuffer status '%s', got '%s'\n",
		       piglit_get_gl_enum_name(expected),
		       piglit_get_gl_enum_name(status));
		return false;
	}
	return true;
}

bool
test_fbo_attachments_layered()
{
	bool pass = true;
	GLuint fbo, texture[2];

	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	/* Attach first texture as a layered texture */
	texture[0] = create_bind_texture(GL_TEXTURE_3D);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			     texture[0], 0);

	/* Attach a single layer of the second texture.(Non layered) */
	texture[1] = create_bind_texture(GL_TEXTURE_3D);
	glFramebufferTexture3D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1,
			       GL_TEXTURE_3D, texture[1], 0, 0);

	if(!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	pass = CheckFramebufferStatus(GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS)
		&& pass;

	glDeleteFramebuffers(1, &fbo);
	glDeleteTextures(2, texture);

	return pass;
}

bool
test_fbo_attachment_targets(GLenum texOneType, GLenum texTwoType,
				 GLenum expectedFbStatus)
{
	bool pass = true;
	GLuint fbo, texture[2];

	glGenTextures(2, texture);

	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	/* Setup texture one */
	texture[0] = create_bind_texture(texOneType);
	attach_texture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		       texOneType, texture[0]);

	/* Setup texture two */
	texture[1] = create_bind_texture(texTwoType);
	attach_texture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1,
		       texTwoType, texture[1]);

	if(!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	/* Check for expected fb status */
	pass = CheckFramebufferStatus(expectedFbStatus) && pass;

	/* Clean up */
	glBindFramebuffer(GL_FRAMEBUFFER, piglit_winsys_fbo);
	glDeleteFramebuffers(1, &fbo);
	glDeleteTextures(2, texture);

	return pass;
}

void
piglit_init(int argc, char **argv)
{
	bool pass = true;

	pass = test_fbo_attachment_targets(GL_TEXTURE_2D, GL_TEXTURE_2D,
			GL_FRAMEBUFFER_COMPLETE)
			&& pass;

	pass = test_fbo_attachment_targets(GL_TEXTURE_2D,  GL_TEXTURE_3D,
			GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS)
			&& pass;

	pass = test_fbo_attachments_layered() && pass;

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result
piglit_display(void)
{
	/* Unreachable */
	return PIGLIT_FAIL;
}
