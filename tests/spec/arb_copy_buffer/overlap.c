/*
 * Copyright © 2012 Intel Corporation
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
 */

/** @file overlap.c
 *
 * Tests the following piece of the GL_ARB_copy_buffer spec:
 *
 *     "An INVALID_VALUE error is generated if the same buffer object
 *      is bound to both readtarget and writetarget, and the ranges
 *      [readoffset, readoffset+size) and [writeoffset,
 *      writeoffset+size) overlap."
 *
 * And it also tests that copying works correctly if there isn't
 * overlap, but with one buffer being both src and dst.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static void
test_copy(GLenum usage, int data_size, int src, int dst, int size)
{
	uint8_t *data;
	uint8_t *expected;
	uint8_t *ptr;
	int i;

	data = (uint8_t *)malloc(data_size);
	expected = (uint8_t *)malloc(data_size);

	for (i = 0; i < data_size; i++) {
		data[i] = i;
	}

	glBufferData(GL_COPY_READ_BUFFER, data_size, data, usage);

	glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER,
			    src, dst, size);

	if (src > dst - size &&
	    src < dst + size) {
		if (!piglit_check_gl_error(GL_INVALID_VALUE)) {
			fprintf(stderr,
				"No error reported for overlapping "
				"glCopyBufferSubData() from %d to %d, "
				"size %d\n",
				src, dst, size);
			piglit_report_result(PIGLIT_FAIL);
		} else {
			free(expected);
			free(data);
			return;
		}
	} else {
		if (!piglit_check_gl_error(0)) {
			fprintf(stderr,
				"Error reported for non-overlapping "
				"glCopyBufferSubData() from %d to %d, "
				"size %d\n",
				src, dst, size);
			piglit_report_result(PIGLIT_FAIL);
		}
	}
	//piglit_reset_gl_error();

	/* Now compute what the result should be and check that it matches. */
	memcpy(expected, data, data_size);
	memcpy(expected + dst, expected + src, size);
	ptr = glMapBuffer(GL_COPY_READ_BUFFER, GL_READ_ONLY);
	if (memcmp(expected, ptr, data_size)) {
		fprintf(stderr,
			"Data not copied correctly for non-overlapping "
			"glCopyBufferSubData().\n"
			"from offset %d to offset %d, size %d\n",
			src, dst, size);

		fprintf(stderr, "original:  expected:  found:\n");
		for (i = 0; i < data_size; i++) {
			fprintf(stderr, "0x%02x       0x%02x       0x%02x\n",
				i, expected[i], ptr[i]);
		}
		piglit_report_result(PIGLIT_FAIL);
	}
	glUnmapBuffer(GL_COPY_READ_BUFFER);

	free(expected);
	free(data);
}

enum piglit_result
piglit_display(void)
{
	/* uncreached */
	return PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	GLuint buf;
	int src, dst, i;
	int size = 6;
	static const GLenum bo_modes[] = {
		GL_STREAM_DRAW,
		GL_STREAM_READ,
		GL_STREAM_COPY,
		GL_STATIC_DRAW,
		GL_STATIC_READ,
		GL_STATIC_COPY,
		GL_DYNAMIC_DRAW,
		GL_DYNAMIC_READ,
		GL_DYNAMIC_COPY,
	};

	piglit_require_extension("GL_ARB_copy_buffer");

	glGenBuffers(1, &buf);

	glBindBufferARB(GL_COPY_READ_BUFFER, buf);
	glBindBufferARB(GL_COPY_WRITE_BUFFER, buf);

	for (i = 0; i < ARRAY_SIZE(bo_modes); i++) {
		GLenum usage = bo_modes[i];

		for (src = 0; src < size; src++) {
			int max_src_size = size - src;
			for (dst = 0; dst < size; dst++) {
				int max_dst_size = size - dst;
				int max_size = ((max_src_size < max_dst_size) ?
						max_src_size :
						max_dst_size);
				int copy_size;

				for (copy_size = 1; copy_size <= max_size;
				     copy_size++) {
					test_copy(usage, size,
						  src, dst, copy_size);
				}
			}
		}
	}

	glDeleteBuffers(1, &buf);

	piglit_report_result(PIGLIT_PASS);
}
