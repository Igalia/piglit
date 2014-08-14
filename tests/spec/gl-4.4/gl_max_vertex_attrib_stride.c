/*
 * Copyright (c) 2014 Timothy Arceri <t_arceri@yahoo.com.au>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NON-INFRINGEMENT.  IN NO EVENT SHALL AUTHORS AND/OR THEIR SUPPLIERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "piglit-util-gl.h"
#include "minmax-test.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_core_version = 44;

PIGLIT_GL_TEST_CONFIG_END

static bool check_stride(char *function, bool check_valid)
{
	bool pass = true;

	if (check_valid) {
		if (!piglit_check_gl_error(GL_NO_ERROR)) {
			fprintf(stderr, "error when testing valid "
				"MAX_VERTEX_ATTRIB_STRIDE with %s\n",
			        function);
			pass = false;
		}
	} else {
		if (!piglit_check_gl_error(GL_INVALID_VALUE)) {
			fprintf(stderr, "GL_INVALID_VALUE should be generated when setting"
				" %s stride too value large than MAX_VERTEX_ATTRIB_STRIDE\n",
				function);
			pass = false;
		}
	}

	return pass;
}

static bool test_stride_vertex_attribl(GLint stride,
				       bool check_valid)
{
	GLdouble vertices[4][4];

	glVertexAttribLPointer(0, 4, GL_DOUBLE, stride, vertices);

	return check_stride("glVertexAttribLPointer", check_valid);
}

static bool test_stride_vertex_attribi(GLint stride,
				       bool check_valid)
{
	GLuint vertices[4][4];

	glVertexAttribIPointer(0, 4, GL_UNSIGNED_INT,
			       stride, vertices);

	return check_stride("glVertexAttribIPointer", check_valid);
}

static bool test_stride_vertex_attrib(GLint stride,
				      bool check_valid)
{
	GLfloat vertices[4][4];

	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE,
			      stride, vertices);

	return check_stride("glVertexAttribPointer", check_valid);
}

static bool test_stride_bind_buffers(GLint stride,
				     bool check_valid)
{
	GLint strides[2];
	GLuint buf[2];
	GLintptr offsets[2] = { 1024, 1024 };

	glGetIntegerv(GL_MAX_VERTEX_ATTRIB_STRIDE, &strides[0]);
	strides[1] = stride;

	/* Create buffer objects */
	glGenBuffers(2, buf);
	glBindBuffer(GL_ARRAY_BUFFER, buf[0]);
	glBindBuffer(GL_ARRAY_BUFFER, buf[1]);

	glBindVertexBuffers(0, 2, buf, offsets, strides);

	return check_stride("glBindVertexBuffers", check_valid);
}

static bool test_stride_bind_buffer(GLint stride,
				    bool check_valid)
{
	GLuint vbo;

	/* Create a buffer object */
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	glBindVertexBuffer(0, vbo, 1024, stride);

	return check_stride("glBindVertexBuffer", check_valid);
}

void piglit_init(int argc, char **argv)
{
	bool pass = true;
	GLint stride_max, stride_max_plus_one;
	GLuint vao;

	/* Create and bind a vertex array object, this is needed
	   for glBindBuffer* tests */
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGetIntegerv(GL_MAX_VERTEX_ATTRIB_STRIDE, &stride_max);
	stride_max_plus_one = stride_max + 1;

	piglit_test_min_int(GL_MAX_VERTEX_ATTRIB_STRIDE, 2048);
	pass = piglit_minmax_pass;

	/* Try passing the max stride value */
	pass = test_stride_bind_buffer(stride_max, true) && pass;
	pass = test_stride_bind_buffers(stride_max, true) && pass;
	pass = test_stride_vertex_attrib(stride_max, true) && pass;
	pass = test_stride_vertex_attribi(stride_max, true) && pass;
	pass = test_stride_vertex_attribl(stride_max, true) && pass;

	/* Try passing a stride value that is to large */
	pass = test_stride_bind_buffer(stride_max_plus_one, false) && pass;
	pass = test_stride_bind_buffers(stride_max_plus_one, false) && pass;
	pass = test_stride_vertex_attrib(stride_max_plus_one, false) && pass;
	pass = test_stride_vertex_attribi(stride_max_plus_one, false) && pass;
	pass = test_stride_vertex_attribl(stride_max_plus_one, false) && pass;

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result
piglit_display(void)
{
	return PIGLIT_PASS;
}
