/*
 * Copyright © 2020 Intel Corporation
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
 *
 * Author:
 *    Eleni Maria Stea <estea@igalia.com>
 *    Juan A. Suarez Romero <jasuarez@igalia.com>
 */

#include <piglit-util-gl.h>
#include <sized-internalformats.h>
#include "interop.h"

GLuint
gl_get_target(const struct vk_image_props *props)
{
	if (props->h == 1)
		return GL_TEXTURE_1D;

	if (props->depth > 1)
		return GL_TEXTURE_3D;

	return GL_TEXTURE_2D;
}

bool
gl_create_mem_obj_from_vk_mem(struct vk_ctx *ctx,
			      struct vk_image_obj *vk_mem_obj,
			      GLuint *gl_mem_obj)
{
	VkMemoryGetFdInfoKHR fd_info;
	int fd;

	PFN_vkGetMemoryFdKHR _vkGetMemoryFdKHR =
		(PFN_vkGetMemoryFdKHR)vkGetDeviceProcAddr(ctx->dev,
				"vkGetMemoryFdKHR");

	if (!_vkGetMemoryFdKHR) {
		fprintf(stderr, "vkGetMemoryFdKHR not found\n");
		return false;
	}

	memset(&fd_info, 0, sizeof fd_info);
	fd_info.sType = VK_STRUCTURE_TYPE_MEMORY_GET_FD_INFO_KHR;
	fd_info.memory = vk_mem_obj->mem;
	fd_info.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;

	if (_vkGetMemoryFdKHR(ctx->dev, &fd_info, &fd) != VK_SUCCESS) {
		fprintf(stderr, "Failed to get the Vulkan memory FD");
		return false;
	}

	glCreateMemoryObjectsEXT(1, gl_mem_obj);
	glImportMemoryFdEXT(*gl_mem_obj, vk_mem_obj->mem_sz, GL_HANDLE_TYPE_OPAQUE_FD_EXT, fd);

	if (!glIsMemoryObjectEXT(*gl_mem_obj))
		return false;

	return glGetError() == GL_NO_ERROR;
}

bool
gl_gen_tex_from_mem_obj(const struct vk_image_props *props,
			GLenum tex_storage_format,
			GLuint mem_obj, uint32_t offset,
			GLuint *tex)
{
	GLint filter;
	GLuint target = gl_get_target(props);
	const struct sized_internalformat *format = get_sized_internalformat(tex_storage_format);

	glGenTextures(1, tex);
	glBindTexture(target, *tex);

	switch (target) {
	case GL_TEXTURE_1D:
		assert(props->depth == 1);
		glTexStorageMem1DEXT(target, props->num_levels,
				     tex_storage_format,
				     props->w,
				     mem_obj, offset);
		break;
	case GL_TEXTURE_2D:
		assert(props->depth == 1);
		glTexStorageMem2DEXT(target, props->num_levels,
				     tex_storage_format,
				     props->w, props->h,
				     mem_obj, offset);
		break;
	case GL_TEXTURE_3D:
		glTexStorageMem3DEXT(target, props->num_levels,
				     tex_storage_format,
				     props->w, props->h, props->depth,
				     mem_obj, offset);
		break;
	default:
		fprintf(stderr, "Invalid GL texture target\n");
		return false;
	}

	switch (get_channel_type(format, 1)) {
	case GL_INT:
	case GL_UNSIGNED_INT:
		filter = GL_NEAREST;
		break;
	default:
		filter = GL_LINEAR;
		break;
	}

	glTexParameteri(target, GL_TEXTURE_MIN_FILTER, filter);
	glTexParameteri(target, GL_TEXTURE_MAG_FILTER, filter);

	return glGetError() == GL_NO_ERROR;
}

bool
gl_create_semaphores_from_vk(const struct vk_ctx *ctx,
			     const struct vk_semaphores *vk_smps,
			     struct gl_ext_semaphores *gl_smps)
{
	VkSemaphoreGetFdInfoKHR sem_fd_info;
	int fd_gl_ready;
	int fd_vk_done;
	PFN_vkGetSemaphoreFdKHR _vkGetSemaphoreFdKHR;

	glGenSemaphoresEXT(1, &gl_smps->vk_frame_done);
	glGenSemaphoresEXT(1, &gl_smps->gl_frame_ready);

	_vkGetSemaphoreFdKHR =
		(PFN_vkGetSemaphoreFdKHR)vkGetDeviceProcAddr(ctx->dev,
							     "vkGetSemaphoreFdKHR");
	if (!_vkGetSemaphoreFdKHR) {
		fprintf(stderr, "vkGetSemaphoreFdKHR not found\n");
		return false;
	}

	memset(&sem_fd_info, 0, sizeof sem_fd_info);
	sem_fd_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_GET_FD_INFO_KHR;
	sem_fd_info.semaphore = vk_smps->vk_frame_ready;
	sem_fd_info.handleType = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_FD_BIT_KHR;

	if (_vkGetSemaphoreFdKHR(ctx->dev, &sem_fd_info, &fd_vk_done) != VK_SUCCESS) {
		fprintf(stderr, "Failed to get the Vulkan memory FD");
		return false;
	}

	sem_fd_info.semaphore = vk_smps->gl_frame_done;
	if (_vkGetSemaphoreFdKHR(ctx->dev, &sem_fd_info, &fd_gl_ready) != VK_SUCCESS) {
		fprintf(stderr, "Failed to get the Vulkan memory FD");
		return false;
	}

	glImportSemaphoreFdEXT(gl_smps->vk_frame_done,
			       GL_HANDLE_TYPE_OPAQUE_FD_EXT,
			       fd_vk_done);

	glImportSemaphoreFdEXT(gl_smps->gl_frame_ready,
			       GL_HANDLE_TYPE_OPAQUE_FD_EXT,
			       fd_gl_ready);

	if (!glIsSemaphoreEXT(gl_smps->vk_frame_done))
		return false;

	if (!glIsSemaphoreEXT(gl_smps->gl_frame_ready))
		return false;

	return glGetError() == GL_NO_ERROR;
}

GLenum
gl_get_layout_from_vk(const VkImageLayout vk_layout)
{
	switch (vk_layout) {
	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		return GL_LAYOUT_COLOR_ATTACHMENT_EXT;
	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		return GL_LAYOUT_DEPTH_STENCIL_ATTACHMENT_EXT;
	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
		return GL_LAYOUT_DEPTH_STENCIL_READ_ONLY_EXT;
	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		return GL_LAYOUT_SHADER_READ_ONLY_EXT;
	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		return GL_LAYOUT_TRANSFER_SRC_EXT;
	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		return GL_LAYOUT_TRANSFER_DST_EXT;
	case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL_KHR:
		return GL_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_EXT;
	case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL_KHR:
		return GL_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_EXT;
	case VK_IMAGE_LAYOUT_UNDEFINED:
	default:
		return GL_NONE;
	};
}

bool
gl_check_vk_compatibility(const struct vk_ctx *ctx)
{
	GLubyte deviceUUID[GL_UUID_SIZE_EXT];
	GLubyte driverUUID[GL_UUID_SIZE_EXT];

	/* FIXME: we select the first device so make sure you've
	 * exported VK_ICD_FILENAMES */
	glGetUnsignedBytei_vEXT(GL_DEVICE_UUID_EXT, 0, deviceUUID);
	glGetUnsignedBytevEXT(GL_DRIVER_UUID_EXT, driverUUID);

	if ((strncmp((const char *)deviceUUID,
		     (const char *)ctx->deviceUUID, GL_UUID_SIZE_EXT) != 0) ||
	    (strncmp((const char* )driverUUID,
		     (const char* )ctx->driverUUID, GL_UUID_SIZE_EXT) != 0)) {
		fprintf(stderr, "Mismatch in device/driver UUID\n");
		return false;
	}

	return glGetError() == GL_NO_ERROR;
}

bool
vk_check_gl_compatibility(struct vk_ctx *ctx)
{
	GLubyte deviceUUID[GL_UUID_SIZE_EXT];
	GLubyte driverUUID[GL_UUID_SIZE_EXT];

	/* FIXME: we select the first device so make sure you've
	 * exported VK_ICD_FILENAMES */
	glGetUnsignedBytei_vEXT(GL_DEVICE_UUID_EXT, 0, deviceUUID);
	glGetUnsignedBytevEXT(GL_DRIVER_UUID_EXT, driverUUID);

	if ((strncmp((const char *)deviceUUID,
		     (const char *)ctx->deviceUUID, GL_UUID_SIZE_EXT) != 0) ||
	    (strncmp((const char *)driverUUID,
		     (const char *)ctx->driverUUID, GL_UUID_SIZE_EXT) != 0))
		return false;

	return glGetError() == GL_NO_ERROR;
}
