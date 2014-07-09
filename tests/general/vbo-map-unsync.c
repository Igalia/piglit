/*
 * Copyright © 2012 VMware, Inc.
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
 * Test mapping VBOs with GL_MAP_UNSYNCHRONIZED_BIT.
 * This could cause a driver crash if there's a bug in the driver.
 *
 * Based on a test program by Keith Whitwell, modified by Thomas Hellstrom.
 */


#include "piglit-util-gl.h"


PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;
PIGLIT_GL_TEST_CONFIG_END

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

static const struct {
	GLfloat pos[3];
	GLubyte color[4];
} verts[] =
{
	{
		{  1.0, -1.0, 0.0 },
		{ 0x00, 0x00, 0xff, 0x00 }
	},

	{
		{  1.0,  1.0, 0.0 },
		{ 0x00, 0xff, 0x00, 0x00 }
	},

	{
		{ -1.0,  1.0, 0.0 },
		{ 0xff, 0x00, 0x00, 0x00 }
	},

	{
		{ -1.0, -1.0, 0.0 },
		{ 0xff, 0xff, 0xff, 0x00 }
	},
};

static const GLuint indices[] = { 0, 1, 2, 3 };
static const GLuint indices2[] = { 0, 2, 3 };

static GLuint arrayObj, elementObj;

static const float red[4] = {1, 0, 0, 0};
static const float green[4] = {0, 1, 0, 0};
static const float blue[4] = {0, 0, 1, 0};
static const float white[4] = {1, 1, 1, 0};


void
piglit_init(int arg, char *argv[])
{
	piglit_require_extension("GL_ARB_map_buffer_range");
	glGenBuffersARB(1, &arrayObj);
	glGenBuffersARB(1, &elementObj);
}


enum piglit_result
piglit_display(void)
{
	int vstride = sizeof(verts[0]);
	int voffset1 = 0;
	int voffset2 = sizeof(verts);
	int coffset1 = 3 * sizeof(float);
	int coffset2 = voffset2 + coffset1;
	bool pass = true;
	void *verts_map;
	void *elems_map;

	glViewport(0, 0, piglit_width, piglit_height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-1.0, 1.0, -1.0, 1.0, -0.1, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glClear(GL_COLOR_BUFFER_BIT);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	/* Create empty vertex, index buffers.  The vertex buffer is large
	 * enough to store two of the vertex arrays defined above.
	 */
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, arrayObj);
	glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, elementObj);
	glBufferDataARB(GL_ARRAY_BUFFER_ARB, 2 * sizeof(verts),
			NULL, GL_STATIC_DRAW_ARB);
	glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, sizeof(indices),
			NULL, GL_STATIC_DRAW_ARB);

	/* Map first half of vertex buffer */
	verts_map = glMapBufferRange(GL_ARRAY_BUFFER_ARB,
				     0, sizeof(verts),
				     GL_MAP_WRITE_BIT |
				     GL_MAP_INVALIDATE_BUFFER_BIT);
	elems_map = glMapBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,
				   GL_WRITE_ONLY_ARB);

	memcpy(verts_map, verts, sizeof(verts));
	memcpy(elems_map, indices, sizeof(indices));
	glUnmapBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB);
	glUnmapBufferARB(GL_ARRAY_BUFFER_ARB);

	/* Draw first triangle: upper-right half of window */
	glVertexPointer(3, GL_FLOAT, vstride, BUFFER_OFFSET(voffset1));
	glColorPointer(4, GL_UNSIGNED_BYTE, vstride, BUFFER_OFFSET(coffset1));

	glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, NULL);


	/* Map second half of vertex buffer */
	verts_map = glMapBufferRange(GL_ARRAY_BUFFER_ARB,
				     sizeof(verts), sizeof(verts),
				     GL_MAP_WRITE_BIT |
				     GL_MAP_INVALIDATE_RANGE_BIT |
				     GL_MAP_UNSYNCHRONIZED_BIT);
	elems_map = glMapBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,
				   GL_WRITE_ONLY_ARB);
	memcpy(verts_map, verts, sizeof(verts));
	memcpy(elems_map, indices2, sizeof(indices2));
	glUnmapBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB);
	glUnmapBufferARB(GL_ARRAY_BUFFER_ARB);

	/* Draw second triangle: lower-left half of window */
	glVertexPointer(3, GL_FLOAT, vstride, BUFFER_OFFSET(voffset2));
	glColorPointer(4, GL_UNSIGNED_BYTE, vstride, BUFFER_OFFSET(coffset2));

	glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, NULL);

	/* check corner colors */
	pass = piglit_probe_pixel_rgb(0, 0, white) && pass;
	pass = piglit_probe_pixel_rgb(piglit_width-1, 0, blue) && pass;
	pass = piglit_probe_pixel_rgb(piglit_width-1, piglit_height-1,
				      green) && pass;
	pass = piglit_probe_pixel_rgb(0, piglit_height-1, red) && pass;

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
