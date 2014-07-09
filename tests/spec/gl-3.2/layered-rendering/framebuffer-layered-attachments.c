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

/** @file framebuffer-layered-attachments.c
 *
 * Section 6.1.11(Querying GL State) From GL spec 3.2 core:
 *
 * void GetFramebufferAttachmentParameteriv( enum target, enum attachment,
 *					     enum pname, int *params );
 *
 * If pname is FRAMEBUFFER_ATTACHMENT_LAYERED, then params will contain
 * TRUE if an entire level of a three-dimesional texture, cube map texture,
 * or one-or two-dimensional array texture is attached. Otherwise, params will
 * contain FALSE.
 *
 *
 * Section 4.4.2(Framebuffer Objects) From GL spec 3.2 core:
 *
 * void FramebufferTexture( enum target, enum attachment, uint texture,
 *                          int level );
 *
 * If texture is the name of a three-dimensional texture, cube map texture,
 * one-or two-dimensional array texture, or two-dimensional multisample array
 * texture, the texture level attached to the framebuffer attachment point is
 * an array of images, and the framebuffer attachment is considered layered.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 32;
	config.supports_gl_core_version   = 32;

PIGLIT_GL_TEST_CONFIG_END

GLenum textureType[] = {
	GL_TEXTURE_3D,
	GL_TEXTURE_CUBE_MAP,
	GL_TEXTURE_1D_ARRAY,
	GL_TEXTURE_2D_ARRAY,
	GL_TEXTURE_2D_MULTISAMPLE_ARRAY
};

GLuint
create_bind_texture(GLenum textureType) {
	int i;
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(textureType, texture);

	switch(textureType) {
	case GL_TEXTURE_2D:
	case GL_TEXTURE_1D_ARRAY:
		glTexParameteri(textureType, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(textureType, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(textureType, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(textureType, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexImage2D(textureType, 0, GL_RGB, 10, 10,
			     0, GL_RGB, GL_FLOAT, NULL);
		break;
	case GL_TEXTURE_3D:
	case GL_TEXTURE_2D_ARRAY:
		glTexParameteri(textureType, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(textureType, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(textureType, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(textureType, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(textureType, GL_TEXTURE_WRAP_R, GL_REPEAT);
		glTexImage3D(textureType, 0, GL_RGB, 10, 10, 6, 0, GL_RGB,
			     GL_FLOAT, NULL);
		break;
	case GL_TEXTURE_CUBE_MAP:
		for(i = 0; i < 6; i++) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,
				     GL_RGB, 10, 10, 0, GL_RGB, GL_FLOAT, NULL);
		}
		break;
	case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
		glTexImage3DMultisample(GL_TEXTURE_2D_MULTISAMPLE_ARRAY, 4,
					GL_RGB, 10, 10, 2, GL_FALSE);
		break;
	}

	return texture;
}

void
piglit_init(int argc, char **argv)
{
	int i;
	bool pass = true;
	GLenum fbStatus;
	GLuint fbo, texture;
	GLint attachmentLayeredStatus;

	for(i = 0; i < ARRAY_SIZE(textureType); i++) {
		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);

		texture = create_bind_texture(textureType[i]);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
				     texture, 0);

		if(!piglit_check_gl_error(GL_NO_ERROR)) {
			printf("Error creating texture and framebuffer setup\n"
			       "texture type: %s\n",
			       piglit_get_gl_enum_name(textureType[i]));
			glDeleteFramebuffers(1, &fbo);
			glDeleteTextures(1, &texture);
			piglit_report_result(PIGLIT_FAIL);
		}

		fbStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if(fbStatus != GL_FRAMEBUFFER_COMPLETE) {
			printf("Framebuffer Status: %s\n",
				piglit_get_gl_enum_name(fbStatus));
			glDeleteFramebuffers(1, &fbo);
			glDeleteTextures(1, &texture);
			piglit_report_result(PIGLIT_FAIL);
		}

		/* Check if the attachment is layered */
		glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,
						      GL_COLOR_ATTACHMENT0,
						      GL_FRAMEBUFFER_ATTACHMENT_LAYERED,
						      &attachmentLayeredStatus);

		pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

		if(attachmentLayeredStatus != GL_TRUE) {
			pass = false;
		}

		glDeleteFramebuffers(1, &fbo);
		glDeleteTextures(1, &texture);
	}

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result
piglit_display(void)
{
	/* UNREACHABLE */
	return PIGLIT_FAIL;
}
