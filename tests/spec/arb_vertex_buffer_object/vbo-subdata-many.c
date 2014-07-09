/*
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

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 200;
	config.window_height = 200;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static enum {
	DRAWARRAYS,
	DRAWELEMENTS,
	DRAWRANGEELEMENTS,
} mode = DRAWARRAYS;

enum piglit_result
piglit_display(void)
{
	uint32_t buffer_size = 4096;
	bool pass = true;
	GLuint vbo;
	float green[] = {0, 1, 0, 0};
	uint32_t count = piglit_width * piglit_height;
	int i;

	glClearColor(1, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	piglit_ortho_projection(piglit_width, piglit_height, false);
	glColor4fv(green);

	glGenBuffersARB(1, &vbo);
	glBindBufferARB(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, buffer_size, NULL, GL_STREAM_DRAW);
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2, GL_FLOAT, 0, NULL);

	for (i = 0; i < count; i++) {
		int x = i % piglit_width;
		int y = (i / piglit_height) % piglit_height;
		float vert[] = {
			x,     y,
			x + 1, y,
			x + 1, y + 1,
			x,     y + 1,
		};
		uint32_t offset = i % (buffer_size / sizeof(vert));
		uint32_t indices[4] = {
			offset * 4,
			offset * 4 + 1,
			offset * 4 + 2,
			offset * 4 + 3
		};

		glBufferSubData(GL_ARRAY_BUFFER, offset * sizeof(vert),
				sizeof(vert), vert);

		switch (mode) {
		case DRAWARRAYS:
			glDrawArrays(GL_TRIANGLE_FAN, offset * 4, 4);
			break;
		case DRAWELEMENTS:
			glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_INT,
				       indices);
			break;
		case DRAWRANGEELEMENTS:
			glDrawRangeElements(GL_TRIANGLE_FAN,
					    indices[0], indices[3],
					    4, GL_UNSIGNED_INT,
					    indices);
			break;
		}
	}

	pass = piglit_probe_rect_rgba(0, 0, piglit_width, piglit_height, green);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

static bool
string_matches(const char *a, const char *b)
{
	return strcmp(a, b) == 0;
}

void
piglit_init(int argc, char *argv[])
{
	int i;

	piglit_require_extension("GL_ARB_vertex_buffer_object");

	for (i = 1; i < argc; i++) {
		if (string_matches(argv[i], "drawarrays"))
			mode = DRAWARRAYS;
		else if (string_matches(argv[i], "drawelements"))
			mode = DRAWELEMENTS;
		else if (string_matches(argv[i], "drawrangeelements"))
			mode = DRAWRANGEELEMENTS;
		else
			piglit_report_result(PIGLIT_FAIL);
	}
}
