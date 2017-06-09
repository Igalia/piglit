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
 * Test cases which exercice the texture handle API with
 * glGetTextureHandleARB(), glMakeTextureHandleResidentARB(), etc.
 */

#include "common.h"

static struct piglit_gl_test_config *piglit_config;

PIGLIT_GL_TEST_CONFIG_BEGIN

	piglit_config = &config;
	config.supports_gl_compat_version = 33;
	config.supports_gl_core_version = 33;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

static enum piglit_result
check_GetTextureHandle_zero_handle(void *data)
{
	bool pass = true;
	GLuint64 handle;

	if (piglit_khr_no_error)
		return PIGLIT_SKIP;

	/* The ARB_bindless_texture spec says:
	 *
	 * "If an error occurs, a handle of zero is returned."
	 */
	handle = glGetTextureHandleARB(42);
	if (!piglit_check_gl_error(GL_INVALID_VALUE))
		return PIGLIT_FAIL;

	pass &= handle == 0;

	handle = glGetTextureSamplerHandleARB(42, 42);
	if (!piglit_check_gl_error(GL_INVALID_VALUE))
		return PIGLIT_FAIL;

	pass &= handle == 0;

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

static enum piglit_result
check_GetTextureHandle_reserved_zero_handle(void *data)
{
	GLuint texture, sampler;
	bool pass = true;
	GLuint64 handle;

	texture = piglit_rgbw_texture(GL_RGBA32F, 16, 16, GL_FALSE, GL_FALSE,
				      GL_UNSIGNED_NORMALIZED);
	sampler = new_sampler();
	glBindTexture(GL_TEXTURE_2D, 0);

	/* The ARB_bindless_texture spec says:
	 *
	 * "The handle zero is reserved and will never be assigned to a valid
	 *  texture handle."
	 */
	handle = glGetTextureHandleARB(texture);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		return PIGLIT_FAIL;

	pass &= handle > 0;

	handle = glGetTextureSamplerHandleARB(texture, sampler);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		return PIGLIT_FAIL;

	pass &= handle > 0;

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

static enum piglit_result
check_GetTextureHandle_uniqueness(void *data)
{
	GLuint sampler, texture;
	GLuint64 handles[4];
	bool pass = true;

	texture = piglit_rgbw_texture(GL_RGBA32F, 16, 16, GL_FALSE, GL_FALSE,
				      GL_UNSIGNED_NORMALIZED);
	sampler = new_sampler();
	glBindTexture(GL_TEXTURE_2D, 0);

	/* The ARB_bindless_texture spec says:
	 *
	 * "The handle for each texture or texture/sampler pair is unique; the
	 *  same handle will be returned if GetTextureHandleARB is called
	 *  multiple times for the same texture or if GetTextureSamplerHandleARB
	 *  is called multiple times for the same texture/sampler pair."
	 */
	handles[0] = glGetTextureHandleARB(texture);
	handles[1] = glGetTextureHandleARB(texture);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		return PIGLIT_FAIL;

	pass &= handles[0] == handles[1];

	handles[2] = glGetTextureSamplerHandleARB(texture, sampler);
	handles[3] = glGetTextureSamplerHandleARB(texture, sampler);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		return PIGLIT_FAIL;

	pass &= handles[2] == handles[3];
	pass &= handles[0] != handles[2];

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

static enum piglit_result
check_IsTextureHandleResident_valid(void *data)
{
	bool pass = true;
	GLuint64 handle;
	GLboolean ret;
	GLuint tex;

	tex = piglit_rgbw_texture(GL_RGBA32F, 16, 16, GL_FALSE, GL_FALSE,
				  GL_UNSIGNED_NORMALIZED);
	glBindTexture(GL_TEXTURE_2D, 0);

	/* Resident */
	handle = glGetTextureHandleARB(tex);
	glMakeTextureHandleResidentARB(handle);

	ret = glIsTextureHandleResidentARB(handle);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		return PIGLIT_FAIL;

	pass &= ret;

	/* Non resident */
	glMakeTextureHandleNonResidentARB(handle);

	ret = glIsTextureHandleResidentARB(handle);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		return PIGLIT_FAIL;

	pass &= !ret;

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

static enum piglit_result
delete_texture_sampler_while_handle_is_allocated(void *data)
{
	GLuint texture, sampler;
	GLuint64 handle;

	if (piglit_khr_no_error)
		return PIGLIT_SKIP;

	/* The ARB_bindless_texture spec says:
	 *
	 * "(5) Is there a way to release a texture or image handle after it
	 *  is created?"
	 *
	 * "RESOLVED:  No API is provided to release or delete handles once
	 *  they are created.  Texture and image handles are automatically
	 *  reclaimed when the underlying texture or sampler objects are finally
	 *  deleted.  This deletion will happen only when no handle using the
	 *  texture or sampler object is resident on any context."
	 */

	/* Test #1: Create a texture handle and remove it. */
	texture = piglit_rgbw_texture(GL_RGBA32F, 16, 16, GL_FALSE, GL_FALSE,
				      GL_UNSIGNED_NORMALIZED);
	glBindTexture(GL_TEXTURE_2D, 0);

	handle = glGetTextureHandleARB(texture);
	glDeleteTextures(1, &texture);

	/* Texture handle should have been removed. */
	glMakeTextureHandleResidentARB(handle);
	if (!piglit_check_gl_error(GL_INVALID_OPERATION))
		return PIGLIT_FAIL;

	/* Test #2: Create a texture/sampler handle and remove the sampler. */
	texture = piglit_rgbw_texture(GL_RGBA32F, 16, 16, GL_FALSE, GL_FALSE,
				      GL_UNSIGNED_NORMALIZED);
	sampler = new_sampler();
	glBindTexture(GL_TEXTURE_2D, 0);

	handle = glGetTextureSamplerHandleARB(texture, sampler);
	glDeleteSamplers(1, &sampler);

	/* Texture handle should have been removed. */
	glMakeTextureHandleResidentARB(handle);
	if (!piglit_check_gl_error(GL_INVALID_OPERATION))
		return PIGLIT_FAIL;

	return PIGLIT_PASS;
}

static enum piglit_result
delete_texture_sampler_while_handle_is_resident(void *data)
{
	GLuint texture, sampler;
	GLuint64 handle;
	GLboolean ret;

	if (piglit_khr_no_error)
		return PIGLIT_SKIP;

	/* The ARB_bindless_texture_spec says:
	 *
	 * "(7) What happens if you try to delete a texture or sampler object
	 *  with a handle that is resident in another context?"
	 *
	 * "RESOLVED:  Deleting the texture will remove the texture from the
	 *  name space and make all handles using the texture non-resident in
	 *  the current context.  However, texture or image handles for a
	 *  deleted texture are not deleted until the underlying texture or
	 *  sampler object itself is deleted.  That deletion won't happen
	 *  until the object is not bound anywhere and there are no handles
	 *  using the object that are resident in any context."
	 */

	/* Test #1: Create a texture handle, make it resident and remove the
	 * texture. */
	texture = piglit_rgbw_texture(GL_RGBA, 16, 16, GL_FALSE, GL_FALSE,
				      GL_UNSIGNED_NORMALIZED);
	glBindTexture(GL_TEXTURE_2D, 0);

	handle = glGetTextureHandleARB(texture);
	glMakeTextureHandleResidentARB(handle);
	glDeleteTextures(1, &texture);

	/* Texture handle should have been removed. */
	glIsTextureHandleResidentARB(handle);
	if (!piglit_check_gl_error(GL_INVALID_OPERATION))
		return PIGLIT_FAIL;

	/* Test #2: Create a texture/sampler handle, make it resident and
	 * remove the sampler. */
	texture = piglit_rgbw_texture(GL_RGBA32F, 16, 16, GL_FALSE, GL_FALSE,
				      GL_UNSIGNED_NORMALIZED);
	sampler = new_sampler();
	glBindTexture(GL_TEXTURE_2D, 0);

	handle = glGetTextureSamplerHandleARB(texture, sampler);
	glMakeTextureHandleResidentARB(handle);
	glDeleteSamplers(1, &sampler);

	/* Texture handle should still be resident. */
	ret = glIsTextureHandleResidentARB(handle);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		return PIGLIT_FAIL;

	if (!ret)
		return PIGLIT_FAIL;

	glDeleteTextures(1, &texture);

	/* Texture handle should have been removed. */
	glIsTextureHandleResidentARB(handle);
	if (!piglit_check_gl_error(GL_INVALID_OPERATION))
		return PIGLIT_FAIL;

	return PIGLIT_PASS;
}

static enum piglit_result
check_GetImageHandle_zero_handle(void *data)
{
	GLuint64 handle;

	if (!piglit_is_extension_supported("GL_ARB_shader_image_load_store"))
		return PIGLIT_SKIP;

	if (piglit_khr_no_error)
		return PIGLIT_SKIP;

	/* The ARB_bindless_texture spec says:
	 *
	 * "A 64-bit unsigned integer handle is returned if the command
	 *  succeeds; otherwise, zero is returned."
	 */
	handle = glGetImageHandleARB(42, 0, GL_FALSE, 0, GL_RGBA32F);
	if (!piglit_check_gl_error(GL_INVALID_VALUE))
		return PIGLIT_FAIL;

	return handle == 0 ? PIGLIT_PASS : PIGLIT_FAIL;
}

static enum piglit_result
check_GetImageHandle_reserved_zero_handle(void *data)
{
	GLuint64 handle;
	GLuint tex;

	if (!piglit_is_extension_supported("GL_ARB_shader_image_load_store"))
		return PIGLIT_SKIP;

	tex = piglit_rgbw_texture(GL_RGBA32F, 16, 16, GL_FALSE, GL_FALSE,
				  GL_UNSIGNED_NORMALIZED);
	glBindTexture(GL_TEXTURE_2D, 0);

	/* The ARB_bindless_texture spec says:
	 *
	 * "The handle zero is reserved and will never be assigned to a valid
	 *  image handle."
	 */
	handle = glGetImageHandleARB(tex, 0, GL_FALSE, 0, GL_RGBA32F);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		return PIGLIT_FAIL;

	return handle > 0 ? PIGLIT_PASS : PIGLIT_FAIL;
}

static enum piglit_result
check_GetImageHandle_uniqueness(void *data)
{
	GLuint64 handles[4];
	bool pass = true;
	GLuint tex;

	if (!piglit_is_extension_supported("GL_ARB_shader_image_load_store"))
		return PIGLIT_SKIP;

	tex = piglit_rgbw_texture(GL_RGBA32F, 16, 16, GL_TRUE, GL_FALSE,
				  GL_UNSIGNED_NORMALIZED);
	glBindTexture(GL_TEXTURE_2D, 0);

	/* The ARB_bindless_texture spec says:
	 *
	 * "The handle returned for each combination of <texture>, <level>,
	 *  <layered>, <layer>, and <format> is unique; the same handle will
	 *  be returned if GetImageHandleARB is called multiple times with
	 *  the same parameters."
	 */
	handles[0] = glGetImageHandleARB(tex, 0, GL_FALSE, 0, GL_RGBA32F);
	handles[1] = glGetImageHandleARB(tex, 0, GL_FALSE, 0, GL_RGBA32F);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		return PIGLIT_FAIL;

	pass &= handles[0] == handles[1];

	/* Use a different format. */
	handles[2] = glGetImageHandleARB(tex, 0, GL_FALSE, 0, GL_RGBA32I);
	handles[3] = glGetImageHandleARB(tex, 0, GL_FALSE, 0, GL_RGBA32I);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		return PIGLIT_FAIL;

	pass &= handles[2] == handles[3];
	pass &= handles[0] != handles[2];

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

static enum piglit_result
check_IsImageHandleResident_valid(void *data)
{
	bool pass = true;
	GLuint64 handle;
	GLboolean ret;
	GLuint tex;

	if (!piglit_is_extension_supported("GL_ARB_shader_image_load_store"))
		return PIGLIT_SKIP;

	tex = piglit_rgbw_texture(GL_RGBA32F, 16, 16, GL_TRUE, GL_FALSE,
				  GL_UNSIGNED_NORMALIZED);
	glBindTexture(GL_TEXTURE_2D, 0);

	/* Resident */
	handle = glGetImageHandleARB(tex, 0, GL_FALSE, 0, GL_RGBA32F);
	glMakeImageHandleResidentARB(handle, GL_READ_WRITE);

	ret = glIsImageHandleResidentARB(handle);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		return PIGLIT_FAIL;

	pass &= ret;

	/* Non resident */
	glMakeImageHandleNonResidentARB(handle);

	ret = glIsImageHandleResidentARB(handle);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		return PIGLIT_FAIL;

	pass &= !ret;

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

static enum piglit_result
delete_texture_while_image_handle_resident(void *data)
{
	GLuint64 handle;
	GLuint tex;

	if (!piglit_is_extension_supported("GL_ARB_shader_image_load_store"))
		return PIGLIT_SKIP;

	if (piglit_khr_no_error)
		return PIGLIT_SKIP;

	tex = piglit_rgbw_texture(GL_RGBA32F, 16, 16, GL_TRUE, GL_FALSE,
				  GL_UNSIGNED_NORMALIZED);
	glBindTexture(GL_TEXTURE_2D, 0);

	handle = glGetImageHandleARB(tex, 0, GL_FALSE, 0, GL_RGBA32F);
	glMakeImageHandleResidentARB(handle, GL_READ_WRITE);
	glDeleteTextures(1, &tex);

	/* Image handle should have been removed. */
	glMakeImageHandleResidentARB(handle, GL_READ_WRITE);
	if (!piglit_check_gl_error(GL_INVALID_OPERATION))
		return PIGLIT_FAIL;

	return PIGLIT_PASS;
}

static const struct piglit_subtest subtests[] = {
	{
		"Check glGetTexture*HandleARB() zero handle",
		"check_GetTextureHandle_zero_handle",
		check_GetTextureHandle_zero_handle,
		NULL
	},
	{
		"Check glGetTexture*HandleARB() reserved zero handle",
		"check_GetTextureHandle_reserved_zero_handle",
		check_GetTextureHandle_reserved_zero_handle,
		NULL
	},
	{
		"Check glGetTexture*HandleARB() uniqueness",
		"check_GetTextureHandle_uniqueness",
		check_GetTextureHandle_uniqueness,
		NULL
	},
	{
		"Check glIsTextureHandleResidentARB() valid",
		"check_IsTextureHandleResident_valid",
		check_IsTextureHandleResident_valid,
		NULL
	},
	{
		"Delete the texture/sampler while a handle is allocated",
		"delete_texture_sampler_while_handle_is_allocated",
		delete_texture_sampler_while_handle_is_allocated,
		NULL
	},
	{
		"Delete the texture/sampler while the handle is resident",
		"delete_texture_sampler_while_handle_is_resident",
		delete_texture_sampler_while_handle_is_resident,
	},
	{
		"Check glGetImageHandleARB() zero handle",
		"check_GetImageHandle_zero_handle",
		check_GetImageHandle_zero_handle,
		NULL
	},
	{
		"Check glGetImageHandleARB() reserved zero handle",
		"check_GetImageHandle_reserved_zero_handle",
		check_GetImageHandle_reserved_zero_handle,
		NULL
	},
	{
		"Check glGetImageHandleARB() uniqueness",
		"check_GetImageHandle_uniqueness",
		check_GetImageHandle_uniqueness,
		NULL
	},
	{
		"Check glIsImageHandleResidentARB() valid",
		"check_IsImageHandleResident_valid",
		check_IsImageHandleResident_valid,
		NULL
	},
	{
		"Delete the texture while the image handle is resident",
		"delete_texture_while_image_handle_resident",
		delete_texture_while_image_handle_resident,
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
