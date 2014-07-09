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

/**
 * \file overrun.c
 * Verify that queries don't over-run the size of the supplied buffer.
 */

#include "piglit-util-gl.h"
#include <stdlib.h>

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

/**
 * Fill the real buffer and the scratch buffer with the same set of
 * garbage data.
 */
static void
fill_with_garbage(void *buffer, void *buffer_scratch,
		  size_t buffer_size_in_bytes)
{
	unsigned i;
	uint32_t *scratch = (uint32_t *) buffer;

	assert(buffer_size_in_bytes % sizeof(uint32_t) == 0);
	for (i = 0; i < (buffer_size_in_bytes / sizeof(uint32_t)); i++)
		do {
			scratch[i] = 0xDEADBEEF;
		} while (scratch[i] == 0);

	memcpy(buffer_scratch, buffer, buffer_size_in_bytes);
}

static bool
verify_no_overrun(const void *buffer, const void *buffer_scratch,
      size_t buffer_size_in_bytes, size_t data_size_in_bytes,
      const char *getter_name, const char *enum_name)
{
	if (memcmp((uint8_t *)buffer + data_size_in_bytes,
		   (uint8_t *)buffer_scratch + data_size_in_bytes,
		   buffer_size_in_bytes - data_size_in_bytes) == 0)
		return true;

	fprintf(stderr, "%s(%s) over-ran the buffer\n",
		getter_name, enum_name);
	return false;
}

/**
 * \param num_formats_enum  GL enum used to query the number of binary formats
 *                          (either shader or program) supported by the
 *                          implementation.
 *
 * \param formats_enum      GL enum used to query the binary formats (either
 *                          shader or program) supported by the
 *                          implementation.
 */
static bool
test_queries(GLenum num_formats_enum, GLenum formats_enum)
{
	bool pass = true;
	GLint count = 0;
	char *buffer;
	char *buffer_scratch;
	unsigned buffer_size_in_elements;
	size_t buffer_size_in_bytes;


	glGetIntegerv(num_formats_enum, &count);

	if (!piglit_check_gl_error(0))
		return false;

	if (count < 0) {
		fprintf(stderr, "%s returned %d\n",
			piglit_get_gl_enum_name(num_formats_enum),
			count);
		return false;
	}

	/* Both of the queries can return zero, so make sure that at least one
	 * element is allocated.  We need to check that when the first query
	 * returns zero, the second query doesn't write any data.
	 */
	buffer_size_in_elements = (count + 1) * 2;
	buffer_size_in_bytes = buffer_size_in_elements * sizeof(GLint64);
	buffer = malloc(buffer_size_in_bytes);
	buffer_scratch = malloc(buffer_size_in_bytes);

#define TEST_CASE(fn, type)						\
	do {								\
		fill_with_garbage(buffer, buffer_scratch,		\
				  buffer_size_in_bytes);		\
		fn (formats_enum, (type *) buffer);			\
		pass = piglit_check_gl_error(0) && pass;		\
		pass = verify_no_overrun(buffer, buffer_scratch,	\
					 buffer_size_in_bytes,		\
					 sizeof(type) * count,		\
					 # fn,				\
					 piglit_get_gl_enum_name(formats_enum))	\
			&& pass;					\
	} while (0)

	TEST_CASE(glGetBooleanv, GLboolean);
	TEST_CASE(glGetIntegerv, GLint);
	TEST_CASE(glGetInteger64v, GLint64);
	TEST_CASE(glGetFloatv, GLfloat);
	TEST_CASE(glGetDoublev, GLdouble);

	free(buffer);
	free(buffer_scratch);
	return pass;
}

void
piglit_init(int argc, char **argv)
{
	bool pass = true;

	if (argc == 1) {
		fprintf(stderr, "Usage: %s [shader|program]\n", argv[0]);
		piglit_report_result(PIGLIT_FAIL);
	}

	if (strcmp(argv[1], "shader") == 0) {
		piglit_require_extension("GL_ARB_ES2_compatibility");
		pass = test_queries(GL_NUM_SHADER_BINARY_FORMATS,
				    GL_SHADER_BINARY_FORMATS);
	} else {
		piglit_require_extension("GL_ARB_get_program_binary");
		pass = test_queries(GL_NUM_PROGRAM_BINARY_FORMATS,
				    GL_PROGRAM_BINARY_FORMATS);
	}

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}
