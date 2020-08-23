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

PIGLIT_GL_TEST_CONFIG_BEGIN

config.supports_gl_compat_version = 46;
config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;
config.khr_no_error_support = PIGLIT_HAS_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

static void cleanup();
static void vk_cleanup();
static void gl_cleanup();

static bool vk_init();
static bool gl_init();

static struct vk_ctx vk_core;
static struct vk_buf vk_vb;
static VkBufferUsageFlagBits vk_vb_usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT |
					   VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
					   VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
struct Vec2 {
	float x;
	float y;
};

static void gen_checkerboard_quads(struct Vec2 *vptr);

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

void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_EXT_memory_object");
	piglit_require_extension("GL_EXT_memory_object_fd");
	piglit_require_extension("GL_ARB_texture_storage");
	piglit_require_extension("GL_ARB_pixel_buffer_object");

	atexit(cleanup);

	if (!vk_init()) {
		fprintf(stderr, "Failed to initialize Vulkan.\n");
		piglit_report_result(PIGLIT_FAIL);
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
check_chessboard_pattern()
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
			if (!piglit_probe_pixel_rgba(x, y, expected_color[chess]))
				return PIGLIT_FAIL;
		}
	}

	return PIGLIT_PASS;
}

enum piglit_result
piglit_display(void)
{
	enum piglit_result res = PIGLIT_PASS;
	static float uninteresting_data[WHITE_VERTS * 2];

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

	if ((res = check_chessboard_pattern()) == PIGLIT_FAIL) {
		fprintf(stderr, "Unexpected geometry inside the vertex buffer.\n");
		return res;
	}

	piglit_present_results();

	/* Checking that calling glBufferSubData with random vertices returns
	 * invalid operation error.
	 */

	glBindBuffer(GL_ARRAY_BUFFER, gl_vk_vb);

	glBufferSubData(GL_ARRAY_BUFFER, 0, vk_vb.mobj.mem_sz, uninteresting_data);
	if (glGetError() != GL_INVALID_OPERATION) {
		fprintf(stderr, "glBufferSubData should return GL_INVALID_OPERATION error!\n");
		return PIGLIT_FAIL;
	}

	/* Render again, and check that the checkerboard pattern hasn't
	 * been changed (array was not modified)
	 */

	glClear(GL_COLOR_BUFFER_BIT);
	glDrawArrays(GL_TRIANGLES, 0, WHITE_VERTS);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	if ((res = check_chessboard_pattern()) == PIGLIT_FAIL) {
		fprintf(stderr, "Vulkan buffer has been modified.\n");
		return res;
	}

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
	if (!vk_init_ctx(&vk_core)) {
		fprintf(stderr, "Failed to initialize Vulkan context.\n");
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

	return true;
}

static bool
gl_init()
{
	glClearColor(1.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	gl_prog = piglit_build_simple_program(vs, fs);
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
