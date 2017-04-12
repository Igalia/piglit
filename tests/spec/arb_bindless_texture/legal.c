/*
 * Copyright (C) 2017 Valve Corporation
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

/** \file
 *
 * Test cases which exercise legal operations when a texture/sampler object
 * has been referenced by one or more texture handles.
 */

#include "common.h"

static struct piglit_gl_test_config *piglit_config;

PIGLIT_GL_TEST_CONFIG_BEGIN

	piglit_config = &config;
	config.supports_gl_compat_version = 33;
	config.supports_gl_core_version = 33;

PIGLIT_GL_TEST_CONFIG_END

static enum piglit_result
call_TexSubImage_when_texture_is_referenced(void *data)
{
	int *img = malloc(16 * 16 * 4 * sizeof(unsigned));
	GLuint tex;

	tex = piglit_integer_texture(GL_RGBA32I, 16, 16, 0, 0);

	glGetTextureHandleARB(tex);
	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		free(img);
		return PIGLIT_FAIL;
	}

	/* The ARB_bindless_texture spec says:
	 *
	 * "The contents of the images in a texture object may still be
	 *  updated via commands such as TexSubImage*, CopyTexSubImage*, and
	 *  CompressedTexSubImage*, and by rendering to a framebuffer object,
	 *  even if the texture object is referenced by one or more texture
	 *  handles."
	 */
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 16, 16, GL_RGBA_INTEGER,
			GL_INT, img);
	free(img);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		return PIGLIT_FAIL;

	return PIGLIT_PASS;
}

static enum piglit_result
call_CopyTexSubImage_when_texture_is_referenced(void *data)
{
	GLuint tex;

	tex = piglit_rgbw_texture(GL_RGBA8, 16, 16, GL_FALSE, GL_FALSE,
				  GL_UNSIGNED_BYTE);

	glGetTextureHandleARB(tex);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		return PIGLIT_FAIL;

	/* The ARB_bindless_texture spec says:
	 *
	 * "The contents of the images in a texture object may still be
	 *  updated via commands such as TexSubImage*, CopyTexSubImage*, and
	 *  CompressedTexSubImage*, and by rendering to a framebuffer object,
	 *  even if the texture object is referenced by one or more texture
	 *  handles."
	 */
	glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, 16, 16);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		return PIGLIT_FAIL;

	return PIGLIT_PASS;
}

static enum piglit_result
call_CompressedTexSubImage_when_texture_is_referenced(void *data)
{
	void *compressed;
	GLuint tex;
	GLint size;

	tex = piglit_rgbw_texture(GL_COMPRESSED_RGBA_BPTC_UNORM, 16, 16,
				  GL_FALSE, GL_FALSE, GL_UNSIGNED_NORMALIZED);

	glGetTextureHandleARB(tex);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		return PIGLIT_FAIL;

	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0,
				 GL_TEXTURE_COMPRESSED_IMAGE_SIZE, &size);

	compressed = malloc(size);
	glGetCompressedTexImage(GL_TEXTURE_2D, 0, compressed);

	/* The ARB_bindless_texture spec says:
	 *
	 * "The contents of the images in a texture object may still be
	 *  updated via commands such as TexSubImage*, CopyTexSubImage*, and
	 *  CompressedTexSubImage*, and by rendering to a framebuffer object,
	 *  even if the texture object is referenced by one or more texture
	 *  handles."
	 */
	glCompressedTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 16, 16,
				  GL_COMPRESSED_RGBA_BPTC_UNORM, size,
				  compressed);
	free(compressed);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		return PIGLIT_FAIL;

	return PIGLIT_PASS;
}

static enum piglit_result
call_BufferSubData_when_texture_is_referenced(void *data)
{
	static const float red[4] = {1, 0, 0, 0};
	GLuint tex, tbo;

	glGenBuffers(1, &tbo);
	glBindBuffer(GL_TEXTURE_BUFFER, tbo);
	glBufferData(GL_TEXTURE_BUFFER, sizeof(red), red, GL_STATIC_DRAW);

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_BUFFER, tex);
	glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, tbo);

	glGetTextureHandleARB(tex);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		return PIGLIT_FAIL;

	/* The ARB_bindless_texture spec says:
	 *
	 * "The contents of the buffer object may still be updated via buffer
	 *  update commands such as BufferSubData and MapBuffer*, or via the
	 *  texture update commands, even if the buffer is bound to a texture
	 *  while that buffer texture object is referenced by one or more
	 *  texture handles."
	 */
	glBufferSubData(GL_TEXTURE_BUFFER, 0, sizeof(red), red);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		return PIGLIT_FAIL;

	return PIGLIT_PASS;
}

static enum piglit_result
call_MapBuffer_when_texture_is_referenced(void *data)
{
	static const float red[4] = {1, 0, 0, 0};
	GLuint tex, tbo;

	glGenBuffers(1, &tbo);
	glBindBuffer(GL_TEXTURE_BUFFER, tbo);
	glBufferData(GL_TEXTURE_BUFFER, sizeof(red), red, GL_STATIC_DRAW);

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_BUFFER, tex);
	glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, tbo);

	glGetTextureHandleARB(tex);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		return PIGLIT_FAIL;

	/* The ARB_bindless_texture spec says:
	 *
	 * "The contents of the buffer object may still be updated via buffer
	 *  update commands such as BufferSubData and MapBuffer*, or via the
	 *  texture update commands, even if the buffer is bound to a texture
	 *  while that buffer texture object is referenced by one or more
	 *  texture handles."
	 */
	glMapBuffer(GL_TEXTURE_BUFFER, GL_READ_ONLY);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		return PIGLIT_FAIL;
	glUnmapBuffer(GL_TEXTURE_BUFFER);

	return PIGLIT_PASS;
}

static const struct piglit_subtest subtests[] = {
	{
		"Call glTexSubImage* when a texture handle is referenced",
		"call_TexSubImage_when_texture_referenced",
		call_TexSubImage_when_texture_is_referenced,
		NULL
	},
	{
		"Call glCopyTexSubImage* when a texture handle is referenced",
		"call_CopyTexSubImage_when_texture_referenced",
		call_CopyTexSubImage_when_texture_is_referenced,
		NULL
	},
	{
		"Call glCompressedTexSubImage* when a texture handle is referenced",
		"call_CompressedTexSubImage_when_texture_referenced",
		call_CompressedTexSubImage_when_texture_is_referenced,
		NULL
	},
	{
		"Call glBufferSubData when a texture handle is referenced",
		"call_BufferSubData_when_texture_referenced",
		call_BufferSubData_when_texture_is_referenced,
		NULL
	},
	{
		"Call glMapBuffer when a texture handle is referenced",
		"call_MapBuffer_when_texture_referenced",
		call_MapBuffer_when_texture_is_referenced,
		NULL
	},
	{
		NULL,
		NULL,
		NULL,
		NULL
	}
};

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	enum piglit_result result;

	piglit_require_extension("GL_ARB_bindless_texture");
	result = piglit_run_selected_subtests(subtests,
					      piglit_config->selected_subtests,
					      piglit_config->num_selected_subtests,
					      PIGLIT_SKIP);
	piglit_report_result(result);
}
