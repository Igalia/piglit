#ifndef INTEROPERABILITY_H
#define INTEROPERABILITY_H

#include <piglit-util-gl.h>
#include "vk.h"

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

#endif /* INTEROPERABILITY_H */
