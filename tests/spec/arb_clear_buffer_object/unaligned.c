/*
 * Copyright © 2014 Intel Corporation
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

/**
 * \file unaligned.c
 *
 * Test R8 copying with non-dword alignment.
 */

#include "piglit-util-gl.h"
#include "common.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 15;
	config.supports_gl_core_version = 31;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

#define SIZE (1 << 20)

static bool debug;

static void
clear_buffer(int i, unsigned offset, unsigned size, unsigned char value, char *cpu)
{
	glClearBufferSubData(GL_ARRAY_BUFFER, GL_R8, offset, size,
			     GL_RED, GL_UNSIGNED_BYTE, &value);
	memset(cpu+offset, value, size);

	if (debug && !check_array_buffer_data(SIZE, cpu)) {
		printf("Clear %i failed: offset=%i (%%4 = %i), size=%i (%%4 = %i)\n",
		       i, offset, offset % 4, size, size % 4);
		piglit_report_result(PIGLIT_FAIL);
	}
}

void
piglit_init(int argc, char **argv)
{
	bool pass = true;
	GLuint buffer;
	int i;
	unsigned offset, size;
	char *cpu = malloc(SIZE);

	if (!cpu)
		piglit_report_result(PIGLIT_FAIL);

	for (i = 1; i < argc; i++)
		if (!strcmp(argv[i], "-debug"))
			debug = true;

	piglit_require_extension("GL_ARB_clear_buffer_object");
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, SIZE, NULL, GL_STREAM_READ);

	clear_buffer(0, 0, SIZE, 0, cpu);

	srand(6487216);
	for (i = 1; i <= 200; i++) {
		offset = rand() % SIZE;
		size = 1 + (rand() % (SIZE - offset));

		clear_buffer(i, offset, size, rand(), cpu);
	}

	/* and some small unaligned copies within one dword */
	for (; i < 230; i++) {
		offset = (rand() % (SIZE/4)) * 4 + 1 + (rand() % 2);
		size = 1;

		clear_buffer(i, offset, size, rand(), cpu);
	}

	pass = piglit_check_gl_error(GL_NO_ERROR) &&
	       check_array_buffer_data(SIZE, cpu);
	glDeleteBuffers(1, &buffer);
	free(cpu);
	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}


enum piglit_result
piglit_display(void)
{
	return PIGLIT_PASS;
}
