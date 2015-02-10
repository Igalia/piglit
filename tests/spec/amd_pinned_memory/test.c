/*
 * Copyright © 2015 Advanced Micro Devices, Inc.
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
 */

/**
 * This tests GL_AMD_pinned_memory. The test does upload, draw, upload, draw...
 * Vertices are uploaded using the user pointer directly or using
 * glMapBufferRange. Only fences are used for synchronization.
 *
 * \author  Marek Olšák <maraeo@gmail.com>
 */

#include "piglit-util-gl.h"
#include <unistd.h>

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 600;
	config.window_height = 480;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

enum {
	TEST_OFFSET_0_FENCE_WAIT,
	TEST_OFFSET_INCR_NO_WAIT,
	TEST_OFFSET_DECR_NO_WAIT,
};

static int test_offset = TEST_OFFSET_0_FENCE_WAIT;
static bool map_buffer;

#define TRI_SIZE (6*4)

void piglit_init(int argc, char **argv)
{
	unsigned i;

	for (i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "offset=0"))
			test_offset = TEST_OFFSET_0_FENCE_WAIT;
		else if (!strcmp(argv[i], "increment-offset"))
			test_offset = TEST_OFFSET_INCR_NO_WAIT;
		else if (!strcmp(argv[i], "decrement-offset"))
			test_offset = TEST_OFFSET_DECR_NO_WAIT;
		else if (!strcmp(argv[i], "map-buffer")) {
			map_buffer = true;
			puts("Using glMapBufferRange.");
		} else {
			printf("Unknown parameter %s\n", argv[i]);
			piglit_report_result(PIGLIT_FAIL);
		}
	}

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);
	piglit_require_gl_version(15);
	piglit_require_extension("GL_AMD_pinned_memory");
	piglit_require_extension("GL_ARB_map_buffer_range");
	piglit_require_extension("GL_ARB_sync");

	switch (test_offset) {
	case TEST_OFFSET_0_FENCE_WAIT:
		puts("Offset = 0, fence wait between uploads.");
		break;
	case TEST_OFFSET_INCR_NO_WAIT:
		puts("Offset is incremented, no wait.");
		break;
	case TEST_OFFSET_DECR_NO_WAIT:
		puts("Offset is decremented, no wait.");
		break;
	default:
		assert(0);
	}

	glShadeModel(GL_FLAT);
	glClearColor(0.2, 0.2, 0.2, 1.0);
}

static void upload(GLuint buffer, float *mem, unsigned slot,
		   float x1, float y1, float x2, float y2)
{
	unsigned offset = slot * TRI_SIZE;

	if (map_buffer) {
		glBindBuffer(GL_ARRAY_BUFFER, buffer);
		mem = glMapBufferRange(GL_ARRAY_BUFFER, offset, TRI_SIZE,
				       GL_MAP_WRITE_BIT |
				       GL_MAP_UNSYNCHRONIZED_BIT);
		if (!mem) {
			printf("glMapBufferRange returned NULL.\n");
			piglit_report_result(PIGLIT_FAIL);
		}
	}
	else {
		mem += offset / sizeof(float);
	}

	*mem++ = x1;
	*mem++ = y1;
	*mem++ = x1;
	*mem++ = y2;
	*mem++ = x2;
	*mem++ = y1;

	if (map_buffer) {
		glUnmapBuffer(GL_ARRAY_BUFFER);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
}

#define NUM_PRIMS 700

enum piglit_result piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	float white[] = {1, 1, 1, 1};
	unsigned i, vbo, size, page_size;
	GLsync fence = 0;
	float x, y, *mem;

	page_size = sysconf(_SC_PAGESIZE);
	size = ALIGN(NUM_PRIMS * TRI_SIZE, page_size);
	mem = aligned_alloc(page_size, size);

	glClear(GL_COLOR_BUFFER_BIT);
	glEnableClientState(GL_VERTEX_ARRAY);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_EXTERNAL_VIRTUAL_MEMORY_BUFFER_AMD, vbo);
	glBufferData(GL_EXTERNAL_VIRTUAL_MEMORY_BUFFER_AMD, size, mem,
		     GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexPointer(2, GL_FLOAT, 0, 0);

	x = 0, y = 0;
	for (i = 0; i < NUM_PRIMS; i++) {
		unsigned slot = 0;

		switch (test_offset) {
		case TEST_OFFSET_0_FENCE_WAIT:
			if (fence)
				glClientWaitSync(fence,
						 GL_SYNC_FLUSH_COMMANDS_BIT,
						 GL_TIMEOUT_IGNORED);
			break;
		case TEST_OFFSET_INCR_NO_WAIT:
			slot = i;
			break;
		case TEST_OFFSET_DECR_NO_WAIT:
			slot = NUM_PRIMS - 1 - i;
			break;
		default:
			assert(0);
		}

		upload(vbo, mem, slot, x, y, x+20, y+20);
		glDrawArrays(GL_TRIANGLES, slot*3, 3);

		x += 20;
		if (x >= piglit_width) {
			x = 0;
			y += 20;
		}

		if (test_offset == TEST_OFFSET_0_FENCE_WAIT) {
			if (fence)
				glDeleteSync(fence);
			fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
		}
	}

	x = 0, y = 0;
	for (i = 0; i < NUM_PRIMS; i++) {
		GLboolean result = piglit_probe_pixel_rgb(x+5, y+5, white);
		if (!result)
			printf("  ... FAIL with primitive %i:\n", i+1);
		pass = result && pass;

		x += 20;
		if (x >= piglit_width) {
			x = 0;
			y += 20;
		}
	}

	if (fence)
		glDeleteSync(fence);
	glDeleteBuffers(1, &vbo);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);
	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
