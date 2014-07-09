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


/** @file framebuffer-multi-attachments.c
 *
 * Section 4.4.2(Framebuffer Objects) From GL spec 3.2 core:
 *
 * The remaining comments in this section apply to all forms of Framebuffer-
 * Texture*.

 * If texture is zero, any image or array of images attached to the attachment
 * point named by attachment is detached. Any additional parameters (level,
 * textarget, and/or layer) are ignored when texture is zero. All state values
 * of the attachment point specified by attachment are set to their default
 * values listed in table 6.23.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 32;
	config.supports_gl_core_version = 32;

PIGLIT_GL_TEST_CONFIG_END

bool
check_texture_parameters(GLenum objType, int objName, int level, int layer, int layered) {
	int objectType = -1;
	int objectName = -1;
	int textureLevel = -1;
	int textureLayer = -1;
	int textureLayered = -1;
	bool pass = true;

	/* Object Type */
	glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
					      GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,
					      &objectType);
	if(objectType != objType) {
		printf("Object Type\nExpected: %s\nObserved: %s\n",
			piglit_get_gl_enum_name(objType),
			piglit_get_gl_enum_name(objectType));
		return false;
	}

	/* Object Name */
	glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
					      GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,
					      &objectName);
	if(objectName != objName) {
		printf("Object Name\nExpected: %i\nObserved: %i\n",
			objName, objectName);
		return false;
	}

	if(!piglit_check_gl_error(GL_NO_ERROR)) {
		printf("Error has occured in check_texture_parameters()\n");
		return false;
	}

	/*
	 * If the value of FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE is NONE, no
	 * framebuffer is bound to target. In this case querying pname
	 * FRAMEBUFFER_ATTACHMENT_OBJECT_NAME will return zero, and all
	 * other queries will generate an INVALID_OPERATION error.
	 */
	if( objectType != GL_NONE ) {
		/* Texture Level */
		glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
						      GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL,
						      &textureLevel);
		if(textureLevel != level) {
			printf("Texture Level\nExpected: %2i\nObserved: %2i\n",
				level, textureLevel);
			return false;
		}

		/* Texture Layer */
		glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
						      GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYER,
						      &textureLayer);
		if(textureLayer != layer) {
			printf("Texture Layer\nExpected: %2i\nObserved: %2i\n",
				layer, textureLayer);
			return false;
		}

		/* Texture Layered */
		glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
						      GL_FRAMEBUFFER_ATTACHMENT_LAYERED,
						      &textureLayered);
		if(textureLayered != layered) {
			printf("Texture Layered\nExpected: %2i\nObserved: %2i\n",
				layered, textureLayered);
			return false;
		}

		if(!piglit_check_gl_error(GL_NO_ERROR)) {
			printf("Error has occured in check_texture_parameters()\n");
			return false;
		}
	}

	return pass;
}

void
piglit_init(int argc, char **argv)
{
	bool pass = true;
	GLuint fbo, texture;

	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER,
			GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER,
			GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, 32, 32, 2, 0,
		     GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

	if(!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	/* Check Default Values */
	pass = check_texture_parameters(GL_NONE, 0, 0, 0, GL_FALSE) && pass;

	/* Attach Texture to Framebuffer's Color Attachment0 */
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture, 2);

	/* Check Values of Texture */
	pass = check_texture_parameters(GL_TEXTURE, texture, 2, 0, GL_TRUE) && pass;

	/* Attach Texture of Zero to Framebuffer's Color Attachment0 */
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 0, 0);

	/* Check Values reset to default values */
	pass = check_texture_parameters(GL_NONE, 0, 0, 0, GL_FALSE) && pass;

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}



enum piglit_result
piglit_display(void)
{
	/* UNREACHABLE */
	return PIGLIT_FAIL;
}
