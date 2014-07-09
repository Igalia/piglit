/*
 * Copyright © 2011 Intel Corporation
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

/** @file elements-negative-offset.c
 *
 * Tests for a bug in the i965 driver.  When moving all VBO pointers
 * down by the same offset in the same batchbuffer, it would be unable
 * to access the new vertex data.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static GLuint vbo;

void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_ARB_vertex_buffer_object");
}

enum piglit_result
piglit_display(void)
{
	bool pass = true;
	float green[] = {0.0, 1.0, 0.0, 0.0};
	float blue[] = {0.0, 0.0, 1.0, 0.0};
	float vertex_data[] = {
		/* quad 1 position */
		-1.0, -1.0, 0.0, 1.0,
		 0.0, -1.0, 0.0, 1.0,
		 0.0,  1.0, 0.0, 1.0,
		-1.0,  1.0, 0.0, 1.0,

		/* quad 1 color */
		0.0, 1.0, 0.0, 0.0,
		0.0, 1.0, 0.0, 0.0,
		0.0, 1.0, 0.0, 0.0,
		0.0, 1.0, 0.0, 0.0,

		/* quad 0 position */
		 0.0, -1.0, 0.0, 1.0,
		 1.0, -1.0, 0.0, 1.0,
		 1.0,  1.0, 0.0, 1.0,
		 0.0,  1.0, 0.0, 1.0,

		/* quad 0 color */
		0.0, 0.0, 1.0, 0.0,
		0.0, 0.0, 1.0, 0.0,
		0.0, 0.0, 1.0, 0.0,
		0.0, 0.0, 1.0, 0.0,
	};
	intptr_t quad0_pos_offset = 8 * 4 * sizeof(float);
	intptr_t quad0_color_offset = 12 * 4 * sizeof(float);
	intptr_t quad1_pos_offset = 0;
	intptr_t quad1_color_offset = 4 * 4 * sizeof(float);

	glClearColor(1.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glGenBuffersARB(1, &vbo);
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo);
	glBufferDataARB(GL_ARRAY_BUFFER_ARB, sizeof(vertex_data),
			vertex_data, GL_DYNAMIC_DRAW);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	glVertexPointer(4, GL_FLOAT, 0, (void *)quad0_pos_offset);
	glColorPointer(4, GL_FLOAT, 0, (void *)quad0_color_offset);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	glVertexPointer(4, GL_FLOAT, 0, (void *)quad1_pos_offset);
	glColorPointer(4, GL_FLOAT, 0, (void *)quad1_color_offset);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	pass = piglit_probe_rect_rgba(0, 0,
				      piglit_width / 2, piglit_height / 2,
				      green) && pass;
	pass = piglit_probe_rect_rgba(piglit_width / 2, 0,
				      piglit_width / 2, piglit_height / 2,
				      blue) && pass;

	piglit_present_results();

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDeleteBuffersARB(1, &vbo);

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
