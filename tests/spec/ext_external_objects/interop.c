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
	GLuint target = gl_get_target(props);

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

	return glGetError() == GL_NO_ERROR;
}
