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

/** @file targets.c
 *
 * Tests the following piece of the GL_ARB_copy_buffer spec:
 *
 *     "All or part of one buffer object's data store may be copied to
 *      the data store of another buffer object by calling
 *
 *      void CopyBufferSubData(enum readtarget, enum writetarget,
 *                             intptr readoffset, intptr writeoffset,
 *                             sizeiptr size);
 *
 *      with readtarget and writetarget each set to one of the targets
 *      ARRAY_BUFFER, COPY_READ_BUFFER, COPY_WRITE_BUFFER,
 *      ELEMENT_ARRAY_BUFFER, PIXEL_PACK_BUFFER, PIXEL_UNPACK_BUFFER,
 *      TEXTURE_BUFFER, TRANSFORM_FEEDBACK_BUFFER, or
 *      UNIFORM_BUFFER. While any of these targets may be used, the
 *      COPY_READ_BUFFER and COPY_WRITE_BUFFER targets are provided
 *      specifically for copies, so that they can be done without
 *      affecting other buffer binding targets that may be in use."
 *
 * Specifically, it walks over the available targets and makes sure
 * that copies work for them.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	/* uncreached */
	return PIGLIT_FAIL;
}

static bool
supported(GLenum target)
{
	switch (target) {
	case GL_ARRAY_BUFFER:
	case GL_ELEMENT_ARRAY_BUFFER:
		return piglit_is_extension_supported("GL_ARB_vertex_buffer_object");

	case GL_COPY_READ_BUFFER:
	case GL_COPY_WRITE_BUFFER:
		return true;

	case GL_PIXEL_PACK_BUFFER:
	case GL_PIXEL_UNPACK_BUFFER:
		return (piglit_is_extension_supported("GL_EXT_pixel_buffer_object") ||
			piglit_is_extension_supported("GL_ARB_pixel_buffer_object"));

	case GL_TEXTURE_BUFFER:
		return (piglit_is_extension_supported("GL_EXT_texture_buffer_object") ||
			piglit_is_extension_supported("GL_ARB_texture_buffer_object"));

	case GL_TRANSFORM_FEEDBACK_BUFFER:
		return piglit_is_extension_supported("GL_EXT_transform_feedback");

	case GL_UNIFORM_BUFFER:
		return (piglit_is_extension_supported("GL_EXT_bindable_uniform") ||
			piglit_is_extension_supported("GL_ARB_uniform_buffer_object"));
	}

	abort();
}

static void
test_copy(GLenum from, GLenum to)
{
	GLuint bufs[2];
	uint8_t data[8];
	uint8_t bad_data[8];
	void *ptr;
	int i;

	glGenBuffers(2, bufs);

	for (i = 0; i < ARRAY_SIZE(data); i++)
		data[i] = i;

	memset(bad_data, 0xd0, sizeof(bad_data));

	glBindBuffer(from, bufs[0]);
	glBufferData(from, sizeof(data), data, GL_DYNAMIC_DRAW);
	glBindBuffer(to, bufs[1]);
	glBufferData(to, sizeof(bad_data), bad_data, GL_DYNAMIC_DRAW);

	glCopyBufferSubData(from, to, 0, 0, sizeof(data));
	ptr = glMapBuffer(to, GL_READ_ONLY);
	if (memcmp(ptr, data, sizeof(data))) {
		fprintf(stderr, "data not copied\n");
		piglit_report_result(PIGLIT_FAIL);
	}
	glUnmapBuffer(to);

	glDeleteBuffers(2, bufs);
}


void
piglit_init(int argc, char **argv)
{
	int i, j;
	GLenum targets[] = {
		GL_ARRAY_BUFFER,
		GL_COPY_READ_BUFFER,
		GL_COPY_WRITE_BUFFER,
		GL_ELEMENT_ARRAY_BUFFER,
		GL_PIXEL_PACK_BUFFER,
		GL_PIXEL_UNPACK_BUFFER,
		GL_TEXTURE_BUFFER,
		GL_TRANSFORM_FEEDBACK_BUFFER,
		GL_UNIFORM_BUFFER,
	};

	piglit_require_extension("GL_ARB_copy_buffer");

	for (i = 0; i < ARRAY_SIZE(targets); i++) {
		GLenum from = targets[i];

		if (!supported(from))
			continue;

		for (j = 0; j < ARRAY_SIZE(targets); j++) {
			GLenum to = targets[j];

			if (from == to)
				continue;
			if (!supported(to))
				continue;

			test_copy(from, to);
		}
	}

	piglit_report_result(PIGLIT_PASS);
}
