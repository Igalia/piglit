/*
 * Copyright © 2013 Intel Corporation
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

/** @file subdata-sync.c
 *
 * Tests that glBufferSubData() synchronizes correctly with
 * glCopyBufferSubData().
 *
 * We make sure that a subdata over the read buffer after the copy has
 * no effect, while a subdata over the write buffer after the copy
 * does have an effect.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 15;
	config.supports_gl_core_version = 31;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

void
piglit_init(int argc, char *argv[])
{
	bool pass = true;
	uint32_t dummy_data_1[4], dummy_data_2[4];
	uint32_t good_data[4] = {0, 1, 2, 3};
	uint32_t result_data[4];
	bool subtest_pass;
	size_t size = sizeof(good_data);
	GLuint buffer_handles[2];

	memset(dummy_data_1, 0xaa, size);
	memset(dummy_data_2, 0xbb, size);

	piglit_require_extension("GL_ARB_copy_buffer");

	glGenBuffers(2, buffer_handles);
	glBindBuffer(GL_COPY_READ_BUFFER, buffer_handles[0]);
	glBindBuffer(GL_COPY_WRITE_BUFFER, buffer_handles[1]);

	glBufferData(GL_COPY_READ_BUFFER, 4096, NULL, GL_STREAM_COPY);
	glBufferData(GL_COPY_WRITE_BUFFER, 4096, NULL, GL_STREAM_COPY);
	glBufferSubData(GL_COPY_READ_BUFFER, 0, size, good_data);
	glBufferSubData(GL_COPY_WRITE_BUFFER, 0, size, dummy_data_1);

	glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER,
			    0, 0, size);
	glBufferSubData(GL_COPY_READ_BUFFER, 0, size, dummy_data_2);
	memset(result_data, 0xd0, size);
	glGetBufferSubData(GL_COPY_WRITE_BUFFER, 0, size, result_data);
	subtest_pass = memcmp(good_data, result_data, size) == 0;
	if (!subtest_pass) {
		fprintf(stderr, "found 0x%08x 0x%08x 0x%08x 0x%08x\n",
			result_data[0], result_data[1],
			result_data[2], result_data[3]);
		pass = false;
	}
	piglit_report_subtest_result(subtest_pass ? PIGLIT_PASS : PIGLIT_FAIL,
				     "overwrite source data");


	glBufferData(GL_COPY_READ_BUFFER, 4096, NULL, GL_STREAM_COPY);
	glBufferData(GL_COPY_WRITE_BUFFER, 4096, NULL, GL_STREAM_COPY);
	glBufferSubData(GL_COPY_READ_BUFFER, 0, size, dummy_data_1);
	glBufferSubData(GL_COPY_WRITE_BUFFER, 0, size, dummy_data_2);
	glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER,
			    0, 0, size);
	glBufferSubData(GL_COPY_WRITE_BUFFER, 0, size, good_data);
	memset(result_data, 0xd0, size);
	glGetBufferSubData(GL_COPY_WRITE_BUFFER, 0, size, result_data);
	subtest_pass = memcmp(good_data, result_data, size) == 0;
	if (!subtest_pass) {
		fprintf(stderr, "found 0x%08x 0x%08x 0x%08x 0x%08x\n",
			result_data[0], result_data[1],
			result_data[2], result_data[3]);
		pass = false;
	}
	piglit_report_subtest_result(subtest_pass ? PIGLIT_PASS : PIGLIT_FAIL,
				     "overwrite destination data");

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result
piglit_display(void)
{
	/* UNREACHED */
	return PIGLIT_FAIL;
}
