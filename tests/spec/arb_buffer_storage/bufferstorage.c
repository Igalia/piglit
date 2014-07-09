/*
 * Copyright © 2014 Advanced Micro Devices, Inc.
 * All rights reserved.
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
 * Authors:
 *    Marek Olšák <marek.olsak@amd.com>
 */

/**
 * This tests GL_MAP_PERSISTENT_BIT and glBufferStorage
 * from ARB_buffer_storage.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

enum test_flag {
	NONE,
	READ,
	DRAW
};

static GLuint buffer;
static GLfloat *map;
static GLboolean coherent, client_storage;
static enum test_flag test = NONE;

#define BUF_SIZE (12 * 4 * sizeof(float))

void
piglit_init(int argc, char **argv)
{
	int i;

	for (i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "coherent")) {
			coherent = GL_TRUE;
			continue;
		}
		if (!strcmp(argv[i], "read")) {
			test = READ;
			continue;
		}
		if (!strcmp(argv[i], "draw")) {
			test = DRAW;
			continue;
		}
		if (!strcmp(argv[i], "client-storage")) {
			client_storage = GL_TRUE;
			continue;
		}

		printf("Unknown param: %s\n", argv[i]);
		piglit_report_result(PIGLIT_FAIL);
	}

	if (test == NONE) {
		puts("Wrong parameters.");
		piglit_report_result(PIGLIT_FAIL);
	}

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	piglit_require_gl_version(15);
	piglit_require_extension("GL_ARB_buffer_storage");
        piglit_require_extension("GL_ARB_map_buffer_range");
	if (test == READ) {
		piglit_require_extension("GL_ARB_copy_buffer");
		piglit_require_extension("GL_ARB_sync");
	}
	if (!coherent) { /* for MemoryBarrier */
		piglit_require_extension("GL_ARB_shader_image_load_store");
	}

	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferStorage(GL_ARRAY_BUFFER, BUF_SIZE, NULL,
			GL_MAP_WRITE_BIT |
			GL_MAP_PERSISTENT_BIT |
			(coherent ? GL_MAP_COHERENT_BIT : 0) |
			GL_DYNAMIC_STORAGE_BIT |
			(client_storage ? GL_CLIENT_STORAGE_BIT : 0));

	piglit_check_gl_error(GL_NO_ERROR);

	map = glMapBufferRange(GL_ARRAY_BUFFER, 0, BUF_SIZE,
			       GL_MAP_WRITE_BIT |
			       GL_MAP_PERSISTENT_BIT |
			       (coherent ? GL_MAP_COHERENT_BIT : 0));

	piglit_check_gl_error(GL_NO_ERROR);

	if (!map)
		piglit_report_result(PIGLIT_FAIL);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

enum piglit_result
piglit_display(void)
{
	float white[4] = {1.0, 1.0, 1.0, 0.0};
	GLboolean pass = GL_TRUE;
	int i;
	float array[] = {
		17, 13, 0,
		17, 18, 0,
		12, 13, 0,
		12, 18, 0,
		27, 13, 0,
		27, 18, 0,
		22, 13, 0,
		22, 18, 0,
		37, 13, 0,
		37, 18, 0,
		32, 13, 0,
		32, 18, 0,
		47, 13, 0,
		47, 18, 0,
		42, 13, 0,
		42, 18, 0
	};

	glClear(GL_COLOR_BUFFER_BIT);

	if (test == DRAW) {
		glEnableClientState(GL_VERTEX_ARRAY);
		glBindBuffer(GL_ARRAY_BUFFER, buffer);
		glVertexPointer(3, GL_FLOAT, 0, 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		memcpy(map, array, 12 * sizeof(float));
		if (!coherent)
			glMemoryBarrier(GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		memcpy(map+12, array+12, 12 * sizeof(float));
		if (!coherent)
			glMemoryBarrier(GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT);

		glDrawArrays(GL_TRIANGLE_STRIP, 4, 4);

		memcpy(map+12*2, array+12*2, 12 * sizeof(float));
		if (!coherent)
			glMemoryBarrier(GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT);

		glDrawArrays(GL_TRIANGLE_STRIP, 8, 4);

		memcpy(map+12*3, array+12*3, 12 * sizeof(float));
		if (!coherent)
			glMemoryBarrier(GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT);

		glDrawArrays(GL_TRIANGLE_STRIP, 12, 4);

		piglit_check_gl_error(0);

		pass = pass && piglit_probe_pixel_rgb(15, 15, white);
		pass = pass && piglit_probe_pixel_rgb(25, 15, white);
		pass = pass && piglit_probe_pixel_rgb(35, 15, white);
		pass = pass && piglit_probe_pixel_rgb(45, 15, white);

		glDisableClientState(GL_VERTEX_ARRAY);
	}
	else if (test == READ) {
		GLuint srcbuf;
		GLsync fence;

		glGenBuffers(1, &srcbuf);
		glBindBuffer(GL_COPY_READ_BUFFER, srcbuf);
		glBufferData(GL_COPY_READ_BUFFER, BUF_SIZE, array, GL_STATIC_DRAW);

		/* Copy some data to the mapped buffer and check if the CPU can see it. */
		glBindBuffer(GL_COPY_WRITE_BUFFER, buffer);
		glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER,
				    0, 0, BUF_SIZE);

		glBindBuffer(GL_COPY_READ_BUFFER, 0);
		glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
		glDeleteBuffers(1, &srcbuf);

		if (!coherent)
			glMemoryBarrier(GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT);

		fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
		glClientWaitSync(fence, GL_SYNC_FLUSH_COMMANDS_BIT, GL_TIMEOUT_IGNORED);

		for (i = 0; i < ARRAY_SIZE(array); i++) {
			if (map[i] != array[i]) {
				printf("Probe [%i] failed. Expected: %f  Observed: %f\n",
				       i, array[i], map[i]);
				pass = GL_FALSE;
			}
		}
	}
	else {
		assert(0);
	}

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
