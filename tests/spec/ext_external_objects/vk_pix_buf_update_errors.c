/*
 * Copyright © 2020 Igalia S.L.
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
 */

#include <piglit-util.h>
#include "interop.h"
#include "helpers.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

config.supports_gl_compat_version = 46;
config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;
config.khr_no_error_support = PIGLIT_HAS_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

static const char vs[] =
	"#version 130\n"
	"in vec4 piglit_vertex;\n"
	"in vec2 piglit_texcoord;\n"
	"out vec2 tex_coords;\n"
	"void main()\n"
	"{\n"
	"    gl_Position = piglit_vertex;\n"
	"    tex_coords = piglit_texcoord;\n"
	"}\n";

static const char fs[] =
	"#version 130\n"
	"in vec2 tex_coords;\n"
	"uniform sampler2D tex; \n"
	"out vec4 color;\n"
	"void main() \n"
	"{\n"
	"    color = texture(tex, tex_coords);\n"
	"}\n";

static bool
vk_init(uint32_t w,
	uint32_t h,
	uint32_t d,
	uint32_t num_samples,
	uint32_t num_levels,
	uint32_t num_layers,
	VkFormat color_format,
	VkFormat depth_format,
	VkImageUsageFlagBits color_usage,
	VkImageUsageFlagBits depth_usage,
	VkImageTiling color_tiling,
	VkImageTiling depth_tiling,
	VkImageLayout color_in_layout,
	VkImageLayout depth_in_layout,
	VkImageLayout color_end_layout,
	VkImageLayout depth_end_layout);

static void
cleanup(void);

static void
vk_cleanup(void);

static bool
gl_init();

static void
gl_cleanup(void);

static struct vk_ctx vk_core;
static struct vk_image_att vk_color_att;
static struct vk_image_att vk_depth_att;
static struct vk_renderer vk_rnd;
static struct vk_buf vk_bo;
static VkBufferUsageFlagBits vk_bo_usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT |
					   VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
					   VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;

static GLuint gl_prog;
static GLuint gl_memobj;
static GLuint gl_bo;
static GLuint gl_tex;

static float vk_fb_color[4] = { 1.0, 1.0, 1.0, 1.0 };

static uint32_t w, h;
static uint32_t d = 1;
static uint32_t num_samples = 1;
static uint32_t num_levels = 1;
static uint32_t num_layers = 1;
static VkFormat color_format = VK_FORMAT_R32G32B32A32_SFLOAT;
static VkFormat depth_format = VK_FORMAT_D32_SFLOAT;
static VkImageUsageFlagBits color_usage = VK_IMAGE_USAGE_SAMPLED_BIT |
					  VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
					  VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
static VkImageUsageFlagBits depth_usage = VK_IMAGE_USAGE_SAMPLED_BIT |
					  VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
static VkImageTiling color_tiling = VK_IMAGE_TILING_OPTIMAL;
static VkImageTiling depth_tiling = VK_IMAGE_TILING_OPTIMAL;
static VkImageLayout color_in_layout = VK_IMAGE_LAYOUT_UNDEFINED;
static VkImageLayout color_end_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
static VkImageLayout depth_in_layout = VK_IMAGE_LAYOUT_UNDEFINED;
static VkImageLayout depth_end_layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_EXT_memory_object");
	piglit_require_extension("GL_EXT_memory_object_fd");
	piglit_require_extension("GL_ARB_texture_storage");
	piglit_require_extension("GL_ARB_pixel_buffer_object");

	atexit(cleanup);

	w = piglit_width;
	h = piglit_height;

	if (!vk_init(w, h, d, num_samples, num_levels, num_layers,
		     color_format, depth_format,
		     color_usage, depth_usage,
		     color_tiling, depth_tiling,
		     color_in_layout, depth_in_layout,
		     color_end_layout, depth_end_layout)) {
		fprintf(stderr, "Failed to initialize Vulkan, skipping the test.\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	if (!gl_create_mem_obj_from_vk_mem(&vk_core, &vk_bo.mobj, &gl_memobj)) {
		fprintf(stderr, "Failed to create GL memory object from Vulkan memory.\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	if (!gl_gen_buf_from_mem_obj(gl_memobj, GL_PIXEL_UNPACK_BUFFER, vk_bo.mobj.mem_sz, 0, &gl_bo)) {
		fprintf(stderr, "Failed to create GL buffer from memory object.\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	vk_draw(&vk_core, 0, &vk_rnd, vk_fb_color, 4, 0,
		NULL, 0, false, false, 0, 0, w, h);

	vk_copy_image_to_buffer(&vk_core, &vk_color_att, &vk_bo, w, h);

	if (!gl_init()) {
		fprintf(stderr, "Failed to initialize OpenGL.\n");
		piglit_report_result(PIGLIT_FAIL);
	}
}

enum piglit_result
piglit_display(void)
{
	int i;
	float colors[6][4] = {
		{1.0, 0.0, 0.0, 1.0},
		{0.0, 1.0, 0.0, 1.0},
		{0.0, 0.0, 1.0, 1.0},
		{1.0, 1.0, 0.0, 1.0},
		{1.0, 0.0, 1.0, 1.0},
		{0.0, 1.0, 1.0, 1.0}
	};
	unsigned char *data;
	enum piglit_result res = PIGLIT_PASS;

	data = malloc(w * h);
	for (i = 0; i < w * h; i++)
		data[i] = 127;

	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, gl_bo);
	glBufferSubData(GL_PIXEL_UNPACK_BUFFER, 0, vk_bo.mobj.mem_sz, data);

	if (glGetError() != GL_INVALID_OPERATION) {
		fprintf(stderr, "glBufferSubData should return GL_INVALID_OPERATION error!\n");
		res = PIGLIT_FAIL;
	}
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

	glUseProgram(gl_prog);
	glBindTexture(GL_TEXTURE_2D, gl_tex);

	piglit_draw_rect_tex(-1, -1, 2, 2, 0, 0, 1, 1);

	/* We make sure that the gl_bo buffer data are the initial ones */

	for (i = 0; i < 6; i++) {
		float x = i * (float)piglit_width / 6.0 + (float)piglit_width / 12.0;
		float y = (float)piglit_height / 2.0;

		if (!piglit_probe_pixel_rgba(x, y, colors[i]))
			res = PIGLIT_FAIL;
	}

	piglit_present_results();

	return res;
}

static bool
vk_init(uint32_t w,
	uint32_t h,
	uint32_t d,
	uint32_t num_samples,
	uint32_t num_levels,
	uint32_t num_layers,
	VkFormat color_format,
	VkFormat depth_format,
	VkImageUsageFlagBits color_usage,
	VkImageUsageFlagBits depth_usage,
	VkImageTiling color_tiling,
	VkImageTiling depth_tiling,
	VkImageLayout color_in_layout,
	VkImageLayout depth_in_layout,
	VkImageLayout color_end_layout,
	VkImageLayout depth_end_layout)
{
	char *vs_src = 0;
	char *fs_src = 0;
	unsigned int vs_sz;
	unsigned int fs_sz;

	if (!vk_init_ctx_for_rendering(&vk_core)) {
		fprintf(stderr, "Failed to create Vulkan context.\n");
		return false;
	}

	if (!vk_check_gl_compatibility(&vk_core)) {
		fprintf(stderr, "Mismatch in driver/device UUID\n");
		return false;
	}

	if (!vk_fill_ext_image_props(&vk_core,
				     w, h, d,
				     num_samples,
				     num_levels,
				     num_layers,
				     color_format,
				     color_tiling,
				     color_usage,
				     color_in_layout,
				     color_end_layout,
				     &vk_color_att.props)) {
		fprintf(stderr, "Unsupported color image properties.\n");
		return false;
	}
	if (!vk_create_ext_image(&vk_core, &vk_color_att.props, &vk_color_att.obj)) {
		fprintf(stderr, "Failed to create color image.\n");
		return false;
	}

	if (!vk_fill_ext_image_props(&vk_core,
				     w, h, d,
				     num_samples,
				     num_levels,
				     num_layers,
				     depth_format,
				     depth_tiling,
				     depth_usage,
				     depth_in_layout,
				     depth_end_layout,
				     &vk_depth_att.props)) {
		fprintf(stderr, "Unsupported depth image properties.\n");
		return false;
	}

	if (!vk_create_ext_image(&vk_core, &vk_depth_att.props, &vk_depth_att.obj)) {
		fprintf(stderr, "Failed to create depth image.\n");
		goto fail;
	}

	if (!(vs_src = load_shader(VK_BANDS_VERT, &vs_sz)))
		goto fail;

	if (!(fs_src = load_shader(VK_BANDS_FRAG, &fs_sz)))
		goto fail;

	if (!vk_create_renderer(&vk_core, vs_src, vs_sz, fs_src, fs_sz,
				false, false,
				&vk_color_att, &vk_depth_att, 0, &vk_rnd)) {
		fprintf(stderr, "Failed to create Vulkan renderer.\n");
		goto fail;
	}

	if (!vk_create_ext_buffer(&vk_core, w * h * 4 * sizeof(float), vk_bo_usage, &vk_bo)) {
		fprintf(stderr, "Failed to create Vulkan buffer.\n");
		goto fail;
	}

	free(vs_src);
	free(fs_src);

	return true;

fail:
	free(vs_src);
	free(fs_src);

	return false;
}

static bool
gl_init()
{
	gl_prog = piglit_build_simple_program(vs, fs);

	glClearColor(1.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	/* We use the gl_bo buffer as backing storage for the gl_tex texture */

	glGenTextures(1, &gl_tex);
	glBindTexture(GL_TEXTURE_2D, gl_tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, gl_bo);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, w, h, 0, GL_RGBA, GL_FLOAT, 0);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

	glBindTexture(GL_TEXTURE_2D, 0);
	return glGetError() == GL_NO_ERROR;
}

static void
cleanup(void)
{
	gl_cleanup();
	vk_cleanup();
}

static void
vk_cleanup(void)
{
	vk_destroy_ext_image(&vk_core, &vk_color_att.obj);
	vk_destroy_ext_image(&vk_core, &vk_depth_att.obj);
	vk_destroy_renderer(&vk_core, &vk_rnd);
	vk_destroy_buffer(&vk_core, &vk_bo);
	vk_cleanup_ctx(&vk_core);
}

static void
gl_cleanup(void)
{
	glDeleteProgram(gl_prog);
	glDeleteMemoryObjectsEXT(1, &gl_memobj);
	glDeleteBuffers(1, &gl_bo);
}
