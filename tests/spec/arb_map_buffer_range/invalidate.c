/*
 * Copyright © 2010 Marek Olšák <maraeo@gmail.com>
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

/* This tests whether the invalidate map flags work as expected with rendering
 * between map calls.
 *
 * An alternative approach to invalidating a buffer range with
 * CopyBufferSubData while the destination buffer is bound as an array buffer
 * is also tested.
 *
 * The alignment of returned pointers is also checked
 * if ARB_map_buffer_alignment is supported.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 600;
	config.window_height = 480;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

enum {
	TEST_MAP_INVALIDATE_RANGE_BIT,
	TEST_MAP_INVALIDATE_BUFFER_BIT,
	TEST_COPY_BUFFER_SUBDATA
} test_flag;

enum {
	TEST_OFFSET_0,
	TEST_OFFSET_INCR,
	TEST_OFFSET_DECR,
} test_offset;

#define TRI_SIZE (6*4)

int alignment = 1;

void piglit_init(int argc, char **argv)
{
	unsigned i;

	for (i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "MAP_INVALIDATE_BUFFER_BIT")) {
			test_flag = TEST_MAP_INVALIDATE_BUFFER_BIT;
		} else if (!strcmp(argv[i], "MAP_INVALIDATE_RANGE_BIT")) {
			test_flag = TEST_MAP_INVALIDATE_RANGE_BIT;
		} else if (!strcmp(argv[i], "CopyBufferSubData")) {
			test_flag = TEST_COPY_BUFFER_SUBDATA;
			piglit_require_extension("GL_ARB_copy_buffer");
		} else if (!strcmp(argv[i], "offset=0")) {
			test_offset = TEST_OFFSET_0;
		} else if (!strcmp(argv[i], "increment-offset")) {
			test_offset = TEST_OFFSET_INCR;
		} else if (!strcmp(argv[i], "decrement-offset")) {
			test_offset = TEST_OFFSET_DECR;
		} else {
			printf("Unknown parameter %s\n", argv[i]);
			piglit_report_result(PIGLIT_FAIL);
		}
	}

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);
	piglit_require_gl_version(15);
	piglit_require_extension("GL_ARB_map_buffer_range");

	if (piglit_is_extension_supported("GL_ARB_map_buffer_alignment")) {
		glGetIntegerv(GL_MIN_MAP_BUFFER_ALIGNMENT, &alignment);
	}

	switch (test_flag) {
	case TEST_MAP_INVALIDATE_RANGE_BIT:
		puts("Testing GL_MAP_INVALIDATE_RANGE_BIT.");
		break;
	case TEST_MAP_INVALIDATE_BUFFER_BIT:
		puts("Testing GL_MAP_INVALIDATE_BUFFER_BIT.");
		break;
	case TEST_COPY_BUFFER_SUBDATA:
		puts("Testing glCopyBufferSubData");
		break;
	default:
		assert(0);
	}

	switch (test_offset) {
	case TEST_OFFSET_0:
		puts("Offset = 0.");
		break;
	case TEST_OFFSET_INCR:
		puts("Offset is incremented.");
		break;
	case TEST_OFFSET_DECR:
		puts("Offset is decremented.");
		break;
	default:
		assert(0);
	}

	glShadeModel(GL_FLAT);
	glClearColor(0.2, 0.2, 0.2, 1.0);
}

static void upload(GLuint buffer, unsigned slot, float x1, float y1, float x2, float y2)
{
	unsigned offset = slot * TRI_SIZE;
	float *v;
	GLuint temp_buf;

	if (test_flag == TEST_COPY_BUFFER_SUBDATA) {
		glGenBuffers(1, &temp_buf);
		glBindBuffer(GL_ARRAY_BUFFER, temp_buf);
		glBufferData(GL_ARRAY_BUFFER, TRI_SIZE, NULL, GL_STATIC_DRAW);
		v = glMapBufferRange(GL_ARRAY_BUFFER, 0, TRI_SIZE, GL_MAP_WRITE_BIT);
	} else {
		glBindBuffer(GL_ARRAY_BUFFER, buffer);
		v = glMapBufferRange(GL_ARRAY_BUFFER, offset, TRI_SIZE,
				     GL_MAP_WRITE_BIT |
				     (test_flag == TEST_MAP_INVALIDATE_BUFFER_BIT ? GL_MAP_INVALIDATE_BUFFER_BIT : 0) |
				     (test_flag == TEST_MAP_INVALIDATE_RANGE_BIT ? GL_MAP_INVALIDATE_RANGE_BIT : 0));
		if (!v) {
			printf("glMapBufferRange returned NULL.\n");
			piglit_report_result(PIGLIT_FAIL);
		}
		if (((uintptr_t)v - offset) % alignment != 0) {
			printf("glMapBufferRange returned a pointer not aligned to GL_MIN_MAP_BUFFER_ALIGNMENT.\n");
			piglit_report_result(PIGLIT_FAIL);
		}
	}

	*v++ = x1;
	*v++ = y1;
	*v++ = x1;
	*v++ = y2;
	*v++ = x2;
	*v++ = y1;

	glUnmapBuffer(GL_ARRAY_BUFFER);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	if (test_flag == TEST_COPY_BUFFER_SUBDATA) {
		glBindBuffer(GL_COPY_READ_BUFFER, temp_buf);
		glBindBuffer(GL_COPY_WRITE_BUFFER, buffer);
		glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER,
				    0, offset, TRI_SIZE);
		glBindBuffer(GL_COPY_READ_BUFFER, 0);
		glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
		glDeleteBuffers(1, &temp_buf);
	}
}

#define NUM_PRIMS 700

enum piglit_result piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	float white[] = {1, 1, 1, 1};
	unsigned i, vbo;
	float x, y;

	glClear(GL_COLOR_BUFFER_BIT);
	glEnableClientState(GL_VERTEX_ARRAY);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, NUM_PRIMS * TRI_SIZE, NULL, GL_STATIC_DRAW);
	glVertexPointer(2, GL_FLOAT, 0, 0);

	/* just make the GPU busy, render a degenerated triangle */
	upload(vbo, 0, 0, 0, 0, 0);
	glDrawArrays(GL_TRIANGLES, 0, 3);

	x = 0, y = 0;
	for (i = 0; i < NUM_PRIMS; i++) {
		unsigned slot = 0;

		switch (test_offset) {
		case TEST_OFFSET_0:
			break;
		case TEST_OFFSET_INCR:
			slot = i;
			break;
		case TEST_OFFSET_DECR:
			slot = NUM_PRIMS - 1 - i;
			break;
		default:
			assert(0);
		}

		upload(vbo, slot, x, y, x+20, y+20);
		glDrawArrays(GL_TRIANGLES, slot*3, 3);

		x += 20;
		if (x >= piglit_width) {
			x = 0;
			y += 20;
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

	glDeleteBuffers(1, &vbo);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);
	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
