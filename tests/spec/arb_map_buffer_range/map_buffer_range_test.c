/*
 * Copyright © 2011 Intel Corporation
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 * Authors:
 *    Ben Widawsky <ben@bwidawsk.net>
 *
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

uint8_t data[1 << 20];

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

static bool
verify_buffer(GLenum target, int offset, int length, void const *compare) {
	int ret;
	const void *ptr = glMapBufferRange(target, offset, length, GL_MAP_READ_BIT);
	ret = memcmp(ptr, compare, length);
	glUnmapBuffer(target);

	return ret == 0;
}

/* This test relies on simple patterns, so using offets which are multiples of
 * 0x100 is bad
 */
void
piglit_init(int argc, char *argv[])
{
	uint8_t *ptr;
	uint8_t temp_data[100];
	GLenum target = GL_ARRAY_BUFFER;
	GLenum verify = GL_COPY_WRITE_BUFFER;
	GLuint handles[2];
	int i;
	bool ret;

	piglit_require_gl_version(15);

	piglit_require_extension("GL_ARB_map_buffer_range");

	for (i = 0; i < sizeof(data); i++) {
		data[i] = i & 0xff;
	}

	for (i = 0; i < 100; i++) {
		temp_data[i] = i;
	}

	glGenBuffersARB(2, handles);
	glBindBufferARB(target, handles[0]);
	glBindBufferARB(verify, handles[1]);
	glBufferData(target, sizeof(data), data, GL_STATIC_DRAW);
	glBufferData(verify, 0x1000, NULL, GL_STREAM_READ);

	glGetError();

	/* Validate that reads work, this is required for remaining ops */
	ret = verify_buffer(target, 0x201, 100, &data[0x201]);
	if (!ret)
		piglit_report_result(PIGLIT_FAIL);

	/* Test 1: test the invalidate range */
	ptr = glMapBufferRange(target, 0x10004, 100, GL_MAP_WRITE_BIT |
						     GL_MAP_INVALIDATE_RANGE_BIT);
	memcpy(ptr, temp_data, 100);
	glUnmapBuffer(target);
	ret = verify_buffer(target, 0x10004, 100, temp_data);
	if (!ret)
		piglit_report_result(PIGLIT_FAIL);

	/* Test 2: test unsynchronized writes */
	ptr = glMapBufferRange(target, 0x50f, 100, GL_MAP_WRITE_BIT |
						   GL_MAP_UNSYNCHRONIZED_BIT);
	memcpy(ptr, temp_data, 100);
	glUnmapBuffer(target);
	ret = verify_buffer(target, 0x50f, 100, temp_data);
	if (!ret)
		piglit_report_result(PIGLIT_FAIL);

	/* Test 3: test explicitly flushed unsynchronized writes 
	 * 3a: Check if things are magically coherent (unmap doing more than it
	 * should */
	ptr = glMapBufferRange(target, 0xa000, 100, GL_MAP_WRITE_BIT |
						    GL_MAP_FLUSH_EXPLICIT_BIT |
						    GL_MAP_UNSYNCHRONIZED_BIT);
	memcpy(ptr, temp_data, 100);
	glUnmapBuffer(target);
	glCopyBufferSubData(target, verify, 0xa002, 0, 100);
	ret = verify_buffer(verify, 0, 100, temp_data);
	if (ret)
		fprintf(stderr, "Coherent without flush\n");

	/* 3b: test with flushed range */
	ptr = glMapBufferRange(target, 0xa002, 100, GL_MAP_WRITE_BIT |
						    GL_MAP_FLUSH_EXPLICIT_BIT |
						    GL_MAP_UNSYNCHRONIZED_BIT);
	memcpy(ptr, temp_data, 100);
	glFlushMappedBufferRange(target, 0x0, 100);
	glUnmapBuffer(target);
	glCopyBufferSubData(target, verify, 0xa002, 100, 100);
	ret = verify_buffer(verify, 100, 100, temp_data);
	if (!ret)
		piglit_report_result(PIGLIT_FAIL);

	piglit_report_result(PIGLIT_PASS);
}
