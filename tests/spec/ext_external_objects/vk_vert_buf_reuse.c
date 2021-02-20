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
#include "helpers.h"
#include "interop.h"
#include "params.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

config.supports_gl_compat_version = 30;
config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;
config.khr_no_error_support = PIGLIT_HAS_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

static struct vk_ctx vk_core;
static struct vk_buf vk_vb;
static struct vk_buf vk_tmp_buf;

static VkBufferUsageFlagBits vk_vb_usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT |
					   VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
					   VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
static struct vk_vertex_info vert_info;

struct Vec2 {
	float x;
	float y;
};

static struct vk_image_att vk_color_att;
static struct vk_image_att vk_depth_att;
static struct vk_renderer vk_rnd;

static GLuint gl_disp_tex;
static GLuint gl_disp_vk_prog;

static void cleanup();
static void vk_cleanup();
static void gl_cleanup();
static bool vk_init();
static bool gl_init();

static void gen_checkerboard_quads(struct Vec2 *vptr);
static bool vk_init_vulkan_drawing();
static void vk_draw_checkerboard();

#define WHITE_QUADS 32
#define WHITE_TRIANGLES (WHITE_QUADS * 2)
#define WHITE_VERTS (WHITE_TRIANGLES * 3)

static GLuint gl_prog;
static GLuint gl_memobj;
static GLuint gl_vk_vb;

static const char vs[] =
	"#version 130\n"
	"in vec2 vertex;\n"
	"void main()\n"
	"{\n"
	"    gl_Position = vec4(vertex, 0.0, 1.0);\n"
	"}\n";

static const char fs[] =
	"#version 130\n"
	"out vec4 color;\n"
	"void main() \n"
	"{\n"
	"    color = vec4(0.0, 0.0, 1.0, 1.0);\n"
	"}\n";

static const char vs_disp[] =
	"#version 130\n"
	"in vec4 piglit_vertex;\n"
	"in vec2 piglit_texcoord;\n"
	"out vec2 tex_coords;\n"
	"void main()\n"
	"{\n"
	"    gl_Position = piglit_vertex;\n"
	"    tex_coords = piglit_texcoord;\n"
	"}\n";

static const char fs_disp[] =
	"#version 130\n"
	"in vec2 tex_coords;\n"
	"uniform sampler2D tex; \n"
	"out vec4 color;\n"
	"void main() \n"
	"{\n"
	"    color = texture(tex, tex_coords);\n"
	"}\n";

void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_EXT_memory_object");
	piglit_require_extension("GL_EXT_memory_object_fd");
	piglit_require_extension("GL_ARB_texture_storage");
	piglit_require_extension("GL_ARB_pixel_buffer_object");

	atexit(cleanup);

	if (!vk_init()) {
		fprintf(stderr, "Failed to initialize Vulkan, skipping the test.\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	if (!gl_create_mem_obj_from_vk_mem(&vk_core, &vk_vb.mobj, &gl_memobj)) {
		fprintf(stderr, "Failed to create GL memory object from Vulkan memory.\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	if (!gl_gen_buf_from_mem_obj(gl_memobj, GL_ARRAY_BUFFER, vk_vb.mobj.mem_sz, 0, &gl_vk_vb)) {
		fprintf(stderr, "Failed to create GL buffer from memory object.\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	if (!gl_init()) {
		fprintf(stderr, "Failed to initialize OpenGL\n");
		piglit_report_result(PIGLIT_FAIL);
	}
}

static enum piglit_result
check_red_blue_chess_pattern(bool is_gl)
{
	int i, j;
	float expected_color[2][4] = {
		{0.0, 0.0, 1.0, 1.0},
		{1.0, 0.0, 0.0, 1.0},
	};

	for (i = 0; i < 8; i++) {
		int y = i * piglit_height / 8 + piglit_height / 16;
		for (j = 0; j < 8; j++) {
			int x = j * piglit_width / 8 + piglit_width / 16;
			int chess = (i & 1) ^ (j & 1);
			if (!piglit_probe_pixel_rgba(x, y, expected_color[chess])) {
				fprintf(stderr, "Wrong %s pattern.\n", is_gl ? "OpenGL" : "Vulkan");
				return PIGLIT_FAIL;
			}
		}
	}
	return PIGLIT_PASS;
}

enum piglit_result
piglit_display(void)
{
	int res = PIGLIT_PASS;
	void *pixels;

	glUseProgram(gl_prog);

	/* We are going to use the Vulkan allocated vertex buffer
	 * in an OpenGL shader that paints the pixels blue.
	 * As the vertices are set to render quads following a checkerboard
	 * pattern (quad, no geometry, quad, no geometry) we should
	 * see a checkerboard pattern where the color is blue when
	 * we have geometry and red (framebuffer color) where there's no
	 * geometry.
	 */

	glBindBuffer(GL_ARRAY_BUFFER, gl_vk_vb);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glDrawArrays(GL_TRIANGLES, 0, WHITE_VERTS);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	/* First we check that we've actually rendered a checkerboard
	 * pattern by probing the middle pixel of each quad of the chess
	 * image (should be red and blue)
	 */
	res = check_red_blue_chess_pattern(true);

	/* Round 2: We render the same checkerboard using Vulkan and we copy the render target
	 * to a buffer to map the memory and take a pointer to the data. Then we compare them with
	 * the GL image checkerboard pattern
	 */

	vk_draw_checkerboard();
	vk_copy_image_to_buffer(&vk_core, &vk_color_att, &vk_tmp_buf, piglit_width, piglit_height);

	if (vkMapMemory(vk_core.dev, vk_tmp_buf.mobj.mem, 0,
				vk_tmp_buf.mobj.mem_sz, 0, &pixels) != VK_SUCCESS) {
		fprintf(stderr, "Failed to map Vulkan image memory.\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	/* Because we can't render using Vulkan on piglit, we use the
	 * pixels we've just read from Vulkan memory as texture data
	 * in a new OpenGL texture.
	 */
	glBindTexture(GL_TEXTURE_2D, gl_disp_tex);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, piglit_width, piglit_height, GL_RGBA, GL_FLOAT, pixels);
	glFinish();
	vkUnmapMemory(vk_core.dev, vk_tmp_buf.mobj.mem);

	glUseProgram(gl_disp_vk_prog);

	piglit_draw_rect_tex(-1, -1, 2, 2, 0, 0, 1, 1);
	res = check_red_blue_chess_pattern(false);

	piglit_present_results();
	return res;
}

static void
vk_cleanup()
{
	vk_destroy_buffer(&vk_core, &vk_vb);
	vk_cleanup_ctx(&vk_core);
}

static void
gl_cleanup()
{
	glDeleteProgram(gl_prog);
	glDeleteMemoryObjectsEXT(1, &gl_memobj);
	glDeleteBuffers(1, &gl_vk_vb);
}

static void
cleanup()
{
	vk_cleanup();
	gl_cleanup();
}

static bool
vk_init()
{
	if (!vk_init_ctx_for_rendering(&vk_core)) {
		fprintf(stderr, "Failed to initialize Vulkan context.\n");
		return false;
	}

	if (!vk_check_gl_compatibility(&vk_core)) {
		fprintf(stderr, "Mismatch in driver/device UUID\n");
		return false;
	}

	if (!vk_create_ext_buffer(&vk_core, WHITE_VERTS * sizeof(struct Vec2), vk_vb_usage, &vk_vb)) {
		fprintf(stderr, "Failed to create external Vulkan vertex buffer.\n");
		return false;
	}

	/* Filling the Vulkan vertex buffer with vdata */
	struct Vec2 *pdata;
	if (vkMapMemory(vk_core.dev, vk_vb.mobj.mem, 0,
			vk_vb.mobj.mem_sz, 0, (void**)&pdata) != VK_SUCCESS) {
		fprintf(stderr, "Failed to map Vulkan buffer memory.\n");
		piglit_report_result(PIGLIT_FAIL);
	}
	gen_checkerboard_quads(pdata);
	vkUnmapMemory(vk_core.dev, vk_vb.mobj.mem);

	if (!vk_init_vulkan_drawing()) {
		fprintf(stderr, "Failed to initialize Vulkan drawing.\n");
		return false;
	}
	return true;
}

static bool
gl_init()
{
	glClearColor(1.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	gl_prog = piglit_build_simple_program(vs, fs);

	/* For Vulkan rendering */
	gl_disp_vk_prog = piglit_build_simple_program(vs_disp, fs_disp);

	glGenTextures(1, &gl_disp_tex);
	glBindTexture(GL_TEXTURE_2D, gl_disp_tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, piglit_width, piglit_height, 0, GL_RGBA, GL_FLOAT, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	return true;
}

#define QUAD_SIZE (2.0 / 8.0)
#define ADD_QUAD_VERT(a, b) do {vptr->x = a; vptr->y = b; vptr++;} while(0)
static void
gen_checkerboard_quads(struct Vec2 *vptr)
{
	int i, j;
	struct Vec2 pos;

	for (i = 0; i < 8; i++) {
		pos.x = -1 + (i % 2) * QUAD_SIZE;
		pos.y = -1 + i * QUAD_SIZE;

		for (j = 0; j < 4; j++) {
			ADD_QUAD_VERT(pos.x, pos.y);
			ADD_QUAD_VERT(pos.x + QUAD_SIZE, pos.y);
			ADD_QUAD_VERT(pos.x + QUAD_SIZE, pos.y + QUAD_SIZE);
			ADD_QUAD_VERT(pos.x, pos.y);
			ADD_QUAD_VERT(pos.x + QUAD_SIZE, pos.y + QUAD_SIZE);
			ADD_QUAD_VERT(pos.x, pos.y + QUAD_SIZE);

			pos.x += QUAD_SIZE * 2.0;
		}
	}
}

static bool
vk_init_vulkan_drawing()
{
	char *vs_src = 0;
	char *fs_src = 0;
	unsigned int vs_sz;
	unsigned int fs_sz;

	int w = piglit_width;
	int h = piglit_height;
	int d = 1;

	/* Creating external images */
	/* Color image */
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
				     false,
				     &vk_color_att.props)) {
		fprintf(stderr, "Unsupported color image properties.\n");
		return false;
	}
	if (!vk_create_ext_image(&vk_core, &vk_color_att.props, &vk_color_att.obj)) {
		fprintf(stderr, "Failed to create color image.\n");
		return false;
	}

	/* Depth image */
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
				     false,
				     &vk_depth_att.props)) {
		fprintf(stderr, "Unsupported depth image properties.\n");
		return false;
	}

	if (!vk_create_ext_image(&vk_core, &vk_depth_att.props, &vk_depth_att.obj)) {
		fprintf(stderr, "Failed to create depth image.\n");
		goto fail;
	}

	if (!(vs_src = load_shader(VK_BLUE_VERT, &vs_sz)))
		goto fail;

	if (!(fs_src = load_shader(VK_BLUE_FRAG, &fs_sz)))
		goto fail;

	vert_info.num_verts = WHITE_VERTS;
	vert_info.num_components = WHITE_TRIANGLES;
	vert_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	if (!vk_create_renderer(&vk_core, vs_src, vs_sz, fs_src, fs_sz,
				false, false,
				&vk_color_att, &vk_depth_att, &vert_info, &vk_rnd)) {
		fprintf(stderr, "Failed to create Vulkan renderer.\n");
		goto fail;
	}

	if (!vk_create_buffer(&vk_core, false, w * h * 4 * sizeof(float), VK_BUFFER_USAGE_TRANSFER_DST_BIT, 0,
			      &vk_tmp_buf)) {
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
vk_draw_checkerboard()
{
	float fb_color[4] = {1.0, 0.0, 0.0, 1.0};
	struct vk_image_att images[] = { vk_color_att, vk_depth_att };

	vk_draw(&vk_core, &vk_vb, &vk_rnd, fb_color, 4, 0, 0, 0, images, 2, 0, 0, piglit_width, piglit_height);
}
