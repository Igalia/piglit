/*
 * Copyright © 2014 Intel Corporation
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

/**
 * \file common.c
 *
 * Common routines to fill or check array buffer data.
 */

#include "piglit-util-gl.h"

/* Check that the range of ARRAY_BUFFER specified with <ofs> and <length>
 * is filled with chunks of data of length <expected_data_size>.
 */
bool
check_array_buffer_sub_data(const int ofs, const int length,
		const int expected_data_size, const void *const expected_data)
{
	bool pass = true;
	int i;
	int buffer_size;
	char *buffer_data = glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);
	glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &buffer_size);

	assert(length % expected_data_size == 0);

	assert(ofs >= 0);
	assert(length >= 0);
	assert(ofs + length <= buffer_size);

	for (i = ofs; i < ofs + length; i += expected_data_size) {
		pass = memcmp(buffer_data + i, expected_data,
				expected_data_size) == 0 && pass;
	}

	glUnmapBuffer(GL_ARRAY_BUFFER);

	return pass;
}


/* As above, but for entire buffer. */
bool
check_array_buffer_data(const int expected_data_size,
		const void *const expected_data)
{
	int buffer_size;
	glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &buffer_size);

	return check_array_buffer_sub_data(0, buffer_size, expected_data_size,
			expected_data);
}

/* Fill the entire ARRAY_BUFFER with chunks of data of <data_size> length.
 */
void
fill_array_buffer(const int data_size, const void *const data)
{
	int i;
	int buffer_size;
	char *buffer_data = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &buffer_size);

	assert (buffer_size % data_size == 0);

	for (i = 0; i < buffer_size; i += data_size) {
		memcpy(buffer_data + i, data, data_size);
	}

	glUnmapBuffer(GL_ARRAY_BUFFER);
}



