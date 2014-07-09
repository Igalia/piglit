/**
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

/**
 * Test GetBufferParameteri64v()
 *
 * GL 3.2 core spec added GetBufferParameteri64v() in section 6.1.8
 *
 * GetBufferParameteri64v() returns an int64 value corresponding to the size, map
 * offset, or map length of the target buffer
 *
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_core_version = 32;
	config.supports_gl_compat_version = 32;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	/* UNREACHED */
	return PIGLIT_FAIL;
}

static const GLenum buffers[] = { GL_ARRAY_BUFFER, GL_COPY_READ_BUFFER,
			GL_COPY_WRITE_BUFFER, GL_ELEMENT_ARRAY_BUFFER,
			GL_PIXEL_PACK_BUFFER, GL_PIXEL_UNPACK_BUFFER,
			GL_TEXTURE_BUFFER, GL_TRANSFORM_FEEDBACK_BUFFER,
			GL_UNIFORM_BUFFER };

void
piglit_init(int argc, char **argv)
{
	bool pass = true;
	GLint64 data = -2;
	int i = 0;

	int stuff[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
	int size = sizeof(stuff);
	int offset = 1;
	int range = 5;

	GLuint buff = 0;
	glGenBuffers(1, &buff);

	for (i = 0; i < ARRAY_SIZE(buffers); i++) {
		glBindBuffer(buffers[i], buff);
		glBufferData(buffers[i], size, stuff, GL_STATIC_READ);
		glMapBufferRange(buffers[i], offset, range, GL_MAP_READ_BIT);

		glGetBufferParameteri64v(buffers[i], GL_BUFFER_SIZE, &data);
		if(data != size) {
			printf("GL_BUFFER_SIZE for %s expected %d, but %d "
				"was returned.\n",
				piglit_get_gl_enum_name(buffers[i]),
				size, (int)data);
			pass = false;
		}
		pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

		glGetBufferParameteri64v(buffers[i], GL_BUFFER_MAP_OFFSET, &data);
		if(data != offset) {
			printf("GL_BUFFER_MAP_OFFSET for %s expected %d, but "
				"%d was returned.\n",
				piglit_get_gl_enum_name(buffers[i]),
				offset, (int)data);
			pass = false;
		}
		pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

		glGetBufferParameteri64v(buffers[i], GL_BUFFER_MAP_LENGTH, &data);
		if(data != range) {
			printf("GL_BUFFER_MAP_LENGTH for %s expected %d, but "
				"%d was returned.\n",
				piglit_get_gl_enum_name(buffers[i]),
				range, (int)data);
			pass = false;
		}
		glUnmapBuffer(buffers[i]);
		pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
	}

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
