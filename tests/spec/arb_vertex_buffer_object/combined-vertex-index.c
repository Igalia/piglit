/*
 * Copyright 2017 VMware, Inc.
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
 * Test rendering with vertex data and index data in the same VBO.
 * Brian Paul
 * Feb 6, 2017
 */


#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 13;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;
PIGLIT_GL_TEST_CONFIG_END


// four verts of (vertex3f, color3f), all colors green
static const float vertex_data[4][2][3] = {
	{ { -1, -1, 0 }, { 0, 1, 0 } },  // v0
	{ {  1,  1, 0 }, { 0, 1, 0 } },  // v2
	{ {  1, -1, 0 }, { 0, 1, 0 } },  // v1
	{ { -1,  1, 0 }, { 0, 1, 0 } }   // v3
};

static const GLushort index_data[4] = { 3, 1, 2, 0 };

static const int buffer_size = sizeof(vertex_data) + sizeof(index_data);
static const int vstride = sizeof(vertex_data[0]);
static const void *vpos_offset = (void *) 0;
static const void *color_offset = (void *) (sizeof(vertex_data[0][0]));
static const size_t index_offset = sizeof(vertex_data);

static GLuint vbo;


enum piglit_result
piglit_display(void)
{
	bool pass;

	glViewport(0, 0, piglit_width, piglit_height);

	glClear(GL_COLOR_BUFFER_BIT);

	glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_SHORT,
		       (void *) index_offset);

	pass = piglit_probe_rect_rgb(0, 0, piglit_width, piglit_height,
				     vertex_data[0][1]);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


void
piglit_init(int argc, char *argv[])
{
	piglit_require_extension("GL_ARB_vertex_buffer_object");

	// create the VBO
	glGenBuffersARB(1, &vbo);
	glBindBufferARB(GL_ARRAY_BUFFER, vbo);
	glBufferDataARB(GL_ARRAY_BUFFER, buffer_size, NULL, GL_STATIC_DRAW);

	// upload vertex data at offset=0
	glBufferSubData(GL_ARRAY_BUFFER, 0,
			sizeof(vertex_data), vertex_data);

	// upload index data at index_offset
	glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, vbo);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, index_offset,
			sizeof(index_data), index_data);

	// setup vertex array pointers
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, vstride, vpos_offset);
	glEnableClientState(GL_COLOR_ARRAY);
	glColorPointer(3, GL_FLOAT, vstride, color_offset);

	piglit_check_gl_error(GL_NO_ERROR);
}
