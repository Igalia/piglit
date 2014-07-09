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

/** @file framebuffertexture.c
 *
 * Test that all textures can be attached with glFramebufferTexture().
 *
 * Section 4.4.2(Framebuffer Objects) From GL spec 3.2 core:
 *
 * "To render directly into a texture image, a specified level of a texture
 * object can be attached as one of the logical buffers of the currently
 * bound framebuffer object by calling:
 *
 *   void FramebufferTexture( enum target, enum attachment,
 *			      uint texture, int level );
 *
 * If texture is the name of a three-dimensional texture, cube map texture,
 * one-or two-dimensional array texture, or two-dimensional multisample array
 * texture, the texture level attached to the framebuffer attachment point
 * is an array of images, and the framebuffer attachment is considered layered."
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 32;
	config.supports_gl_core_version = 32;

PIGLIT_GL_TEST_CONFIG_END

GLenum textureType[] = {
	GL_TEXTURE_1D,
	GL_TEXTURE_1D_ARRAY,
	GL_TEXTURE_2D,
	GL_TEXTURE_2D_ARRAY,
	GL_TEXTURE_2D_MULTISAMPLE,
	GL_TEXTURE_2D_MULTISAMPLE_ARRAY,
	GL_TEXTURE_3D,
	GL_TEXTURE_CUBE_MAP,
	GL_TEXTURE_RECTANGLE
};

const char *vs_source = {
	"#version 150\n"
	"in vec4 piglit_vertex;\n"
	"void main() {\n"
	"	gl_Position = piglit_vertex;\n"
	"}\n"
};

const char *fs_source = {
	"#version 150\n"
	"void main() {\n"
	"	gl_FragColor = vec4(0, 1, 0, 1);\n"
	"}\n"
};


bool check_framebuffer_status(GLenum target, GLenum expected) {
	GLenum observed = glCheckFramebufferStatus(target);
	if(expected != observed) {
		printf("Unexpected framebuffer status!\n"
		       "  Observed: %s\n  Expected: %s\n",
		       piglit_get_gl_enum_name(observed),
		       piglit_get_gl_enum_name(expected));
		return false;
	}
	return true;
}

GLuint
create_bind_texture(GLenum textureType) {
	int i;
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(textureType, texture);

	switch(textureType) {
	case GL_TEXTURE_1D:
		glTexParameteri(textureType, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(textureType, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(textureType, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(textureType, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexImage1D(textureType, 0, GL_RGBA, 6, 0, GL_RGBA,
			     GL_UNSIGNED_INT, NULL);
		break;
	case GL_TEXTURE_RECTANGLE:
		glTexImage2D(textureType, 0, GL_RGBA, 6, 6,
			     0, GL_RGBA, GL_UNSIGNED_INT, NULL);
		break;
	case GL_TEXTURE_2D:
	case GL_TEXTURE_1D_ARRAY:
		glTexParameteri(textureType, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(textureType, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(textureType, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(textureType, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexImage2D(textureType, 0, GL_RGBA, 6, 6,
			     0, GL_RGBA, GL_UNSIGNED_INT, NULL);
		break;
	case GL_TEXTURE_3D:
	case GL_TEXTURE_2D_ARRAY:
		glTexParameteri(textureType, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(textureType, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(textureType, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(textureType, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(textureType, GL_TEXTURE_WRAP_R, GL_REPEAT);
		glTexImage3D(textureType, 0, GL_RGBA, 6, 6, 6,
			     0, GL_RGBA, GL_UNSIGNED_INT, NULL);
		break;
	case GL_TEXTURE_CUBE_MAP:
		for(i = 0; i < 6; i++) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,
				     GL_RGBA, 6, 6, 0,
				     GL_RGBA, GL_UNSIGNED_INT,
				     NULL);
		}
		break;
	case GL_TEXTURE_2D_MULTISAMPLE:
		glTexImage2DMultisample(textureType, 0, GL_RGB, 6, 6, GL_FALSE);
		break;
	case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
		glTexImage3DMultisample(textureType, 0, GL_RGB,
					6, 6, 6, GL_FALSE);
		break;
	}

	return texture;
}

/* Take a framebuffer object, that has a GL_TEXTURE_2D_MULTISAMPLE
 * or a layer of a GL_TEXTURE_2D_MULTISAMPLE_ARRAY attached to
 * color attachment 0. Then blit that framebuffer object to
 * a new fbo that has a GL_TEXTURE_2D attached. Finally
 * attach the new GL_TEXTURE_2D to the original fbo.
 */
void
ConvertMultiSample2DToTexture2D(GLuint fboRead) {
	GLuint fboDraw, texture;

	glGenFramebuffers(1, &fboDraw);
	glGenTextures(1, &texture);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, fboRead);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboDraw);

	texture = create_bind_texture(GL_TEXTURE_2D);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			       GL_TEXTURE_2D, texture, 0);

	if(!check_framebuffer_status(GL_DRAW_FRAMEBUFFER,
				     GL_FRAMEBUFFER_COMPLETE) ||
	   !check_framebuffer_status(GL_READ_FRAMEBUFFER,
				     GL_FRAMEBUFFER_COMPLETE)) {

		piglit_report_result(PIGLIT_FAIL);
	}

	glBlitFramebuffer(0, 0, 6, 6, 0, 0, 6, 6,
			  GL_COLOR_BUFFER_BIT, GL_NEAREST);

	if(!piglit_check_gl_error(GL_NO_ERROR)) {
		glDeleteTextures(1, &texture);
		piglit_report_result(PIGLIT_FAIL);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, fboRead);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			       GL_TEXTURE_2D, texture, 0);

	glDeleteFramebuffers(1, &fboDraw);

	if(!piglit_check_gl_error(GL_NO_ERROR)) {
		glDeleteTextures(1, &texture);
		piglit_report_result(PIGLIT_FAIL);
	}
}

bool
test_framebuffertexture(GLenum textureType)
{
	bool pass = true;
	GLuint fbo, texture;

	float expected[] = { 0, 1, 0 };

	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	texture = create_bind_texture(textureType);

	/* Attach the texture to the framebuffer object */
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			     texture, 0);

	if(!piglit_check_gl_error(GL_NO_ERROR) ||
	   !check_framebuffer_status(GL_FRAMEBUFFER, GL_FRAMEBUFFER_COMPLETE)) {
		glDeleteFramebuffers(1, &fbo);
		glDeleteTextures(1, &texture);
		printf("Texture Type: %s. Error during setup.\n",
		       piglit_get_gl_enum_name(textureType));
		return false;
	}

	piglit_draw_rect(-1, -1, 2, 2);

	/* If the texture is a multisample texture,
	 * convert it to a 2D texture */
	if(textureType == GL_TEXTURE_2D_MULTISAMPLE ||
	   textureType == GL_TEXTURE_2D_MULTISAMPLE_ARRAY) {
		ConvertMultiSample2DToTexture2D(fbo);
	}

	/* Probe for the expected color value */
	if(textureType == GL_TEXTURE_1D ||
	   textureType == GL_TEXTURE_1D_ARRAY) {
		if(!piglit_probe_rect_rgb(0, 0, 6, 1, expected)) {
			pass = false;
		}
	} else {
		if(!piglit_probe_rect_rgb(0, 0, 6, 6, expected)) {
			pass = false;
		}
	}

	/* Clean up */
	glDeleteFramebuffers(1, &fbo);
	glDeleteTextures(1, &texture);

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	return pass;
}

void
piglit_init(int argc, char **argv)
{
	int i;
	bool pass = true;
	GLuint program = piglit_build_simple_program(vs_source, fs_source);
	glUseProgram(program);

	for(i = 0; i < ARRAY_SIZE(textureType); i++) {
		if(!test_framebuffertexture(textureType[i])) {
			printf("Texture Type: %s. FramebufferTexture()"
			       " Test Failed.\n",
			       piglit_get_gl_enum_name(textureType[i]));
			pass = false;
		}
	}

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result
piglit_display(void)
{
	/* YOU SHALL NOT PASS */
	return PIGLIT_FAIL;
}
