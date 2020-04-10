#ifndef INTEROPERABILITY_H
#define INTEROPERABILITY_H

#include <piglit-util-gl.h>
#include "vk.h"

struct gl_ext_semaphores {
	GLuint vk_frame_done;
	GLuint gl_frame_ready;
};

GLuint
gl_get_target(const struct vk_image_props *props);

bool
gl_create_mem_obj_from_vk_mem(struct vk_ctx *ctx,
			      struct vk_image_obj *vk_mem_obj,
			      GLuint *gl_mem_obj);

bool
gl_gen_tex_from_mem_obj(const struct vk_image_props *props,
			GLenum gl_format,
			GLuint mem_obj, uint32_t offset,
			GLuint *tex);

bool
gl_create_semaphores_from_vk(const struct vk_ctx *ctx,
			     const struct vk_semaphores *vk_smps,
			     struct gl_ext_semaphores *gl_smps);

GLenum
gl_get_layout_from_vk(const VkImageLayout vk_layout);

bool
gl_check_vk_compatibility(const struct vk_ctx *ctx);

#endif /* INTEROPERABILITY_H */
