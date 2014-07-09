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
 * Test GetInteger64i_v()
 *
 * GL 3.2 core spec added GetInteger64i_v() in section 6.1.1(Simple Queries)
 *
 * GetInteger64i_v() queries an int64 value corresponding to the size or
 * offset of the target buffer
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

static const struct {
	GLenum target, start, size;
} test_vectors[] = {
	{
		GL_UNIFORM_BUFFER,
		GL_UNIFORM_BUFFER_START,
		GL_UNIFORM_BUFFER_SIZE
	},
	{
		GL_TRANSFORM_FEEDBACK_BUFFER,
		GL_TRANSFORM_FEEDBACK_BUFFER_START,
		GL_TRANSFORM_FEEDBACK_BUFFER_SIZE
	}
};

GLint64 data = -2;

int size = 3 * sizeof(int);
int offset = 0;

int stuff[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
int sz = sizeof(stuff);

GLuint idx = 0;
GLuint buff = 0;

bool
DoTest(GLuint buf, GLenum target, GLenum targStart, GLenum targSize)
{
	bool pass = true;

	glBindBuffer(target, buf);
	glBufferData(target, sz, stuff, GL_STATIC_READ);
	glBindBufferRange(target, idx, buf, offset, size);

	glGetInteger64i_v(targStart, idx, &data);
	if(data != offset) {
		printf("%s was expected to be %d, but %d "
			"was returned.\n",
			piglit_get_gl_enum_name(target),
			offset, (int)data);
		pass = false;
	}
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	glGetInteger64i_v(targSize, idx, &data);
	if(data != size) {
		printf("%s was expected to be %d, but %d "
			"was returned.\n",
			piglit_get_gl_enum_name(target),
			size, (int)data);
		pass = false;
	}
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	return pass;
}

void
piglit_init(int argc, char **argv)
{
	bool pass = true;
	int i = 0;

	glGenBuffers(1, &buff);

	for (i = 0; i < ARRAY_SIZE(test_vectors); i++) {
		pass = DoTest(buff,
				test_vectors[i].target,
				test_vectors[i].start,
				test_vectors[i].size)
			&& pass;
	}

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
