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

#ifndef INTEROPERABILITY_H
#define INTEROPERABILITY_H

#include <piglit-util-gl.h>
#include "vk.h"

struct gl_ext_semaphores {
	GLuint vk_frame_done;
	GLuint gl_frame_ready;
};

enum fragment_type {
	FLOAT_FS = 0,
	INT_FS,
	UINT_FS,
};

struct format_mapping {
	char *name;
	GLenum glformat;
	VkFormat vkformat;
	enum fragment_type fs_type;

	uint32_t rbits;
	uint32_t gbits;
	uint32_t bbits;
	uint32_t abits;

	VkImageTiling tiling;
	VkImageUsageFlagBits usage;
};

GLuint
gl_get_target(const struct vk_image_props *props);

bool
gl_create_mem_obj_from_vk_mem(struct vk_ctx *ctx,
			      struct vk_mem_obj *vk_mem_obj,
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
