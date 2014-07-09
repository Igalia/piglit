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

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 30;
	config.window_visual = PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

/* These are all the formats that are required to be color-renderable by the
 * OpenGL 3.0 spec.
 */
static const GLenum valid_formats[] = {
	GL_RGBA32F,
	GL_RGBA32I,
	GL_RGBA32UI,
	GL_RGBA16,
	GL_RGBA16F,
	GL_RGBA16I,
	GL_RGBA16UI,
	GL_RGBA8,
	GL_RGBA8I,
	GL_RGBA8UI,
	GL_SRGB8_ALPHA8,
	GL_R11F_G11F_B10F,
	GL_RG32F,
	GL_RG32I,
	GL_RG32UI,
	GL_RG16,
	GL_RG16F,
	GL_RG16I,
	GL_RG16UI,
	GL_RG8,
	GL_RG8I,
	GL_RG8UI,
	GL_R32F,
	GL_R32I,
	GL_R32UI,
	GL_R16,
	GL_R16F,
	GL_R16I,
	GL_R16UI,
	GL_R8,
	GL_R8I,
	GL_R8UI,
	GL_ALPHA8,
};

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

static bool
try(GLenum format)
{
	bool pass = true;
	GLint num_sample_counts;
	GLint *buffer;
	GLint *buffer_copy;
	size_t buffer_size_in_elements;
	size_t buffer_size_in_bytes;
	unsigned i;

	glGetInternalformativ(GL_RENDERBUFFER,
			      format,
			      GL_NUM_SAMPLE_COUNTS,
			      1, &num_sample_counts);
	pass = piglit_check_gl_error(0)
		&& pass;

	buffer_size_in_elements = num_sample_counts + 2;
	buffer_size_in_bytes = buffer_size_in_elements * sizeof(GLint);
	buffer = malloc(buffer_size_in_bytes);
	buffer_copy = malloc(buffer_size_in_bytes);

	/* Try GL_NUM_SAMPLE_COUNTS.
	 *
	 * It seems very unlikely that an implementation will support
	 * 0xDEADBEEF number sample counts.
	 */
	buffer[0] = 0xDEADBEEF;
	glGetInternalformativ(GL_RENDERBUFFER,
			      format,
			      GL_NUM_SAMPLE_COUNTS,
			      0, buffer);
	pass = piglit_check_gl_error(0)
		&& pass;

	if (buffer[0] != 0xDEADBEEF) {
		pass = false;
		fprintf(stderr,
			"pname = GL_NUM_SAMPLE_COUNTS, bufSize = 0 "
			"over-ran the buffer.\n");
	}

	/* Try GL_SAMPLES.
	 *
	 * Call it once with the full size buffer.  Smash the data in the
	 * buffer.  Call it again with a buffer size of 1.  Verify that all of
	 * the data after the first element is still the smashed data.
	 */
	memset(buffer, 0x7e, buffer_size_in_bytes);

	glGetInternalformativ(GL_RENDERBUFFER,
			      format,
			      GL_SAMPLES,
			      buffer_size_in_elements,
			      buffer);
	pass = piglit_check_gl_error(0)
		&& pass;

	for (i = 0; i < buffer_size_in_elements; i++) {
		buffer[i] = ~buffer[i];
		buffer_copy[i] = buffer[i];
	}

	glGetInternalformativ(GL_RENDERBUFFER,
			      format,
			      GL_SAMPLES,
			      1,
			      buffer);
	pass = piglit_check_gl_error(0)
		&& pass;

	for (i = 1; i < buffer_size_in_elements; i++) {
		if (buffer[i] != buffer_copy[i]) {
			fprintf(stderr,
				"pname = GL_SAMPLES, bufSize = 1 "
				"over-ran the buffer at element %u.\n",
				i);
			pass = false;
		}
	}

	free(buffer);
	free(buffer_copy);
	return pass;
}

void
piglit_init(int argc, char **argv)
{
	bool pass = true;
	unsigned i;

	piglit_require_extension("GL_ARB_framebuffer_object");
	piglit_require_extension("GL_ARB_internalformat_query");

	for (i = 0; i < ARRAY_SIZE(valid_formats); i++) {
		pass = try(valid_formats[i]) && pass;
	}

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
