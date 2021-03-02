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
 */

#include <piglit-util-gl.h>
#include "interop.h"
#include "params.h"
#include "helpers.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

config.supports_gl_compat_version = 30;
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

static const char vs_overwrite[] =
	"#version 130\n"
	"in vec4 piglit_vertex;\n"
	"in vec2 piglit_texcoord;\n"
	"out vec2 tex_coords;\n"
	"void main()\n"
	"{\n"
	"    gl_Position = piglit_vertex;\n"
	"    tex_coords = piglit_texcoord;\n"
	"}\n";

static const char fs_overwrite[] =
	"#version 130\n"
	"in vec2 tex_coords;\n"
	"uniform sampler2D tex; \n"
	"out vec4 color;\n"
	"const vec4 colors[] = vec4[] (\n"
	"	vec4(1.0, 0.0, 0.0, 1.0),\n"
	"	vec4(0.0, 1.0, 0.0, 1.0),\n"
	"	vec4(0.0, 0.0, 1.0, 1.0),\n"
	"	vec4(0.5, 0.5, 0.5, 1.0),\n"
	"	vec4(1.0, 0.0, 1.0, 1.0),\n"
	"	vec4(0.0, 1.0, 1.0, 1.0));\n"
	"void main()\n"
	"{\n"
	"	int band = int(gl_FragCoord.x * 6.0 / 160.0);\n"
	"	color =  colors[band];\n"
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

static GLenum gl_target = GL_TEXTURE_2D;
static GLenum gl_tex_storage_format = GL_RGBA32F;
static GLuint gl_tex;
static GLint gl_prog;
static GLint gl_prog_overwrite;
static GLuint gl_mem_obj;

static GLuint gl_fbo;
static GLuint gl_rbo;
static GLuint gl_disp_tex;

static struct gl_ext_semaphores gl_sem;
static struct vk_semaphores vk_sem;

static float vk_fb_color[4] = { 1.0, 1.0, 1.0, 1.0 };
static void *pixels;

void piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_ARB_texture_storage");
	piglit_require_extension("GL_EXT_memory_object");
	piglit_require_extension("GL_EXT_memory_object_fd");
	piglit_require_extension("GL_EXT_semaphore");
	piglit_require_extension("GL_EXT_semaphore_fd");

	atexit(cleanup);

	w = piglit_width;
	h = piglit_height;

	if (!vk_init(w, h, d, num_samples, num_levels, num_layers,
				color_format, depth_format,
				color_tiling, depth_tiling,
				color_in_layout, depth_in_layout,
				color_end_layout, depth_end_layout)) {
		fprintf(stderr, "Failed to initialize Vulkan, skipping the test.\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	if (!gl_create_mem_obj_from_vk_mem(&vk_core, &vk_color_att.obj.mobj,
				&gl_mem_obj)) {
		fprintf(stderr, "Failed to create GL memory object from Vulkan memory.\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	if (!gl_gen_tex_from_mem_obj(&vk_color_att.props,
				gl_tex_storage_format,
				gl_mem_obj, 0, &gl_tex)) {
		fprintf(stderr, "Failed to create texture from GL memory object.\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	if (!gl_create_semaphores_from_vk(&vk_core, &vk_sem, &gl_sem)) {
		fprintf(stderr, "Failed to import semaphores from Vulkan.\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	if (!gl_init()) {
		fprintf(stderr, "Failed to initialize structs for GL rendering.\n");
		piglit_report_result(PIGLIT_FAIL);
	}
}

enum piglit_result
piglit_display(void)
{
	enum piglit_result res = PIGLIT_PASS;
	int i;
	bool vk_sem_has_wait = true;
	bool vk_sem_has_signal = true;
	float colors[6][4] = {
		{1.0, 0.0, 0.0, 1.0},
		{0.0, 1.0, 0.0, 1.0},
		{0.0, 0.0, 1.0, 1.0},
		{0.5, 0.5, 0.5, 1.0},
		{1.0, 0.0, 1.0, 1.0},
		{0.0, 1.0, 1.0, 1.0}
	};
	GLuint layout = gl_get_layout_from_vk(color_in_layout);

	if (vk_sem_has_wait) {
		glSignalSemaphoreEXT(gl_sem.gl_frame_ready, 0, 0, 1,
				     &gl_tex, &layout);
		glFlush();
	}

	struct vk_image_att images[] = { vk_color_att, vk_depth_att };

	vk_draw(&vk_core, 0, &vk_rnd, vk_fb_color, 4, &vk_sem,
		vk_sem_has_wait, vk_sem_has_signal, images, ARRAY_SIZE(images),
		0, 0, w, h);

	layout = gl_get_layout_from_vk(color_end_layout);
	if (vk_sem_has_signal) {
		glWaitSemaphoreEXT(gl_sem.vk_frame_done, 0, 0, 1,
				   &gl_tex, &layout);
	}

	/* OpenGL overwrites the image */
	glBindTexture(gl_target, gl_tex);
	glBindFramebuffer(GL_FRAMEBUFFER, gl_fbo);
	glUseProgram(gl_prog_overwrite);
	piglit_draw_rect(-1, -1, 2, 2);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glFinish();

	vk_copy_image_to_buffer(&vk_core, &vk_color_att, &vk_bo, w, h);
	if (vkMapMemory(vk_core.dev, vk_bo.mobj.mem, 0,
					vk_bo.mobj.mem_sz, 0, &pixels) != VK_SUCCESS) {
		fprintf(stderr, "Failed to map Vulkan image memory.\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	/* Because we can't render using Vulkan on piglit, we use the
	 * pixels we've just read from Vulkan memory as texture data
	 * in a new OpenGL texture */
	glBindTexture(gl_target, gl_disp_tex);
	glTexSubImage2D(gl_target, 0, 0, 0, w, h, GL_RGBA, GL_FLOAT, pixels);
	glFinish();

	vkUnmapMemory(vk_core.dev, vk_bo.mobj.mem);

	/* OpenGL renders the Vulkan image pixels we've just read
	* from memory
	*/
	glUseProgram(gl_prog);
	glBindTexture(gl_target, gl_disp_tex);
	piglit_draw_rect_tex(-1, -1,
			     2,
			     2,
			     0, 0, 1, 1);

	const float y = (float)piglit_height / 2.0;
	for (i = 0; i < 6; i++) {
		float x = i * (float)piglit_width / 6.0 + (float)piglit_width / 12.0;

		if (!piglit_probe_pixel_rgba(x, y, colors[i])) {
			res = PIGLIT_FAIL;
			break;
		}
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

	/* creating external images */
	/* color image */
	if (!vk_fill_ext_image_props(&vk_core,
				     w, h, d,
				     num_samples,
				     num_levels,
				     num_layers,
				     color_format,
				     color_tiling,
				     color_in_layout,
				     color_end_layout,
				     true,
				     &vk_color_att.props)) {
		fprintf(stderr, "Unsupported color image properties.\n");
		return false;
	}
	if (!vk_create_ext_image(&vk_core, &vk_color_att.props, &vk_color_att.obj)) {
		fprintf(stderr, "Failed to create color image.\n");
		return false;
	}

	/* depth image */
	if (!vk_fill_ext_image_props(&vk_core,
				     w, h, d,
				     num_samples,
				     num_levels,
				     num_layers,
				     depth_format,
				     depth_tiling,
				     depth_in_layout,
				     depth_end_layout,
				     false,
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

	if (!vk_create_semaphores(&vk_core, &vk_sem)) {
		fprintf(stderr, "Failed to create semaphores.\n");
		goto fail;
	}

	if (!vk_create_buffer(&vk_core, false, w * h * 4 * sizeof(float), VK_BUFFER_USAGE_TRANSFER_DST_BIT, 0, &vk_bo)) {
		fprintf(stderr, "Failed to create buffer.\n");
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

static void
vk_cleanup(void)
{
	vk_destroy_ext_image(&vk_core, &vk_color_att.obj);
	vk_destroy_ext_image(&vk_core, &vk_depth_att.obj);

	vk_destroy_renderer(&vk_core, &vk_rnd);
	vk_destroy_semaphores(&vk_core, &vk_sem);

	vk_destroy_buffer(&vk_core, &vk_bo);

	vk_cleanup_ctx(&vk_core);
}

static void
cleanup(void)
{
	gl_cleanup();
	vk_cleanup();
}

static bool
gl_init()
{
	gl_prog = piglit_build_simple_program(vs, fs);
	gl_prog_overwrite = piglit_build_simple_program(vs_overwrite,
						       fs_overwrite);

	glGenFramebuffers(1, &gl_fbo);
	glGenRenderbuffers(1, &gl_rbo);

	glBindTexture(gl_target, gl_tex);
	glBindFramebuffer(GL_FRAMEBUFFER, gl_fbo);
	glBindRenderbuffer(GL_RENDERBUFFER, gl_rbo);

	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8,
			      w, h);

	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER,
				  GL_DEPTH_STENCIL_ATTACHMENT,
				  GL_RENDERBUFFER, gl_rbo);

	glFramebufferTexture2D(GL_FRAMEBUFFER,
			       GL_COLOR_ATTACHMENT0,
			       gl_target, gl_tex, 0);

	if (!check_bound_fbo_status())
		return false;

	glClearColor(1.0, 1.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(gl_target, 0);
	glGenTextures(1, &gl_disp_tex);
	glBindTexture(gl_target, gl_disp_tex);
	glTexParameteri(gl_target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(gl_target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(gl_target, 0, gl_tex_storage_format, w, h, 0, GL_RGBA, GL_FLOAT, 0);
	glBindTexture(gl_target, 0);

	glClearColor(0.1, 0.1, 0.1, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	return glGetError() == GL_NO_ERROR;
}

static void
gl_cleanup(void)
{
	glBindTexture(gl_target, 0);

	glDeleteTextures(1, &gl_tex);
	glDeleteProgram(gl_prog);

	glDeleteSemaphoresEXT(1, &gl_sem.gl_frame_ready);
	glDeleteSemaphoresEXT(1, &gl_sem.vk_frame_done);

	glDeleteFramebuffers(1, &gl_fbo);
	glDeleteRenderbuffers(1, &gl_rbo);

	glDeleteMemoryObjectsEXT(1, &gl_mem_obj);
}
