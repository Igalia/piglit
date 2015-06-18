/*
 * Copyright 2015  VMware, Inc.
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/*
 * Test interleaved vertex arrays with unusual element offsets and strides.
 *
 * The game Flockers (from Steam) uses some unusual vertex arrays.
 * For example:  glVertexAttribPointerARB(index = 1, size = 3, type = GL_FLOAT,
 * normalized = GL_FALSE, stride = 87, pointer = 0x4b).  Note that the
 * offset to the float[3] attribute is 75 (0x4b) bytes and the stride between
 * vertices is 87 bytes.
 *
 * According to the OpenGL specification, OpenGL 1.5, page 33:
 * "Clients must align data elements consistent with the requirements of the
 *  client platform, with an additional base-level requirement that an offset
 *  within a buffer to a datum comprising N basic machine units be a multiple
 *  of N."
 *
 * However, the spec does not say what might happen if that requirement is
 * not met.  There is no language about raising a GL error or undefined
 * behavior.
 *
 * This test exercises float[3] attributes at unusual offsets/strides.
 * If a failure is detected we generate "warn" instead of "fail" since
 * according to the spec, the failure is allowed, but there are apps (such
 * as Flockers) that will hit this issue.
 *
 * If a failure/warning is reported, the OpenGL implementor will have to
 * decide if conformance or app support is more important.
 *
 * Brian Paul
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 15;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;
PIGLIT_GL_TEST_CONFIG_END


void
piglit_init(int argc, char **argv)
{
	/* nothing */
}


static bool
test_offset_stride(int color_offset, int stride)
{
	static const GLfloat vertex[4][2] = {
		{ -1, -1 },
		{  1, -1 },
		{  1,  1 },
		{ -1,  1 }
	};
	static const GLfloat color[4] = { 0.0, 1.0, 0.5, 1.0 };
	GLubyte buffer[1000];
	GLuint buf;
	int i, pos;
	bool p;

	assert(color_offset >= sizeof(vertex[0]));
	assert(stride >= color_offset + sizeof(color));

	pos = 0;
	for (i = 0; i < 4; i++) {
		/* copy vertex position into buffer */
		memcpy(buffer + pos, vertex[i], sizeof(vertex[i]));

		/* copy vertex color into buffer at unusual offset */
		memcpy(buffer + pos + color_offset, color, sizeof(color));

		pos += stride;
	}
	assert(pos <= sizeof(buffer));

	glGenBuffers(1, &buf);
	glBindBuffer(GL_ARRAY_BUFFER, buf);
	glBufferData(GL_ARRAY_BUFFER, sizeof(buffer),
					 buffer, GL_STATIC_DRAW);

	glVertexPointer(2, GL_FLOAT, stride, (void *) 0);
	glColorPointer(4, GL_FLOAT, stride, (void *) (size_t) color_offset);
	glEnable(GL_VERTEX_ARRAY);
	glEnable(GL_COLOR_ARRAY);

	glClear(GL_COLOR_BUFFER_BIT);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	p = piglit_probe_rect_rgba(0, 0, piglit_width, piglit_height, color);

	if (!p) {
		printf("Failure for color_offset %d, stride %d\n",
		       color_offset, stride);
	}

	piglit_present_results();

	glDeleteBuffers(1, &buf);

	return p;
}


enum piglit_result
piglit_display(void)
{
	bool pass = true;

	/* test nice values */
	pass = test_offset_stride(8, 24) && pass;
	pass = test_offset_stride(12, 28) && pass;

        /* NOTE: if any of the following tests fail on non-x86 systems it
         * may be due to unaligned loads of floats (typically a bus error).
         */

	/* test unusual offset */
	pass = test_offset_stride(9, 32) && pass;

	/* test unusual stride */
	pass = test_offset_stride(8, 27) && pass;

	/* test unusual offset, unusual stride */
	pass = test_offset_stride(9, 25) && pass;
	pass = test_offset_stride(10, 26) && pass;
	pass = test_offset_stride(11, 27) && pass;

	/* Report warn, not fail (see comments above) */
	return pass ? PIGLIT_PASS : PIGLIT_WARN;
}
