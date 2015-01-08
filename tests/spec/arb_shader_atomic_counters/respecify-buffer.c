/*
 * Copyright © 2014 Intel Corporation
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

/** @file respecify-buffer.c
 *
 * Tests that the required state is reemitted when the buffer backing
 * an atomic counter is respecified; taking care not to dirty too much
 * other state which would mask flagging problems.
 *
 * This demonstrates a mesa bug.
 */

#include "common.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

        config.supports_gl_core_version = 31;

        config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

int data[] = { 0, 1, 2, 3 };

float red[] = { 1, 0, 0 };
float green[] = { 0, 1, 0 };
float blue[] = { 0, 0, 1 };
float white[] = { 1, 1, 1 };

enum piglit_result
piglit_display(void)
{
	bool pass;
	int i;

	glViewport(0, 0, piglit_width, piglit_height);

	glClearColor(0.2, 0.2, 0.2, 0.2);
	glClear(GL_COLOR_BUFFER_BIT);

	for (i = 0; i < 4; i++) {
		/* respecify the buffer */
		glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(*data),
			     &data[i], GL_STATIC_DRAW);

		piglit_draw_rect(i % 2 - 1, i / 2 - 1, 1, 1);
	}

	pass = piglit_probe_pixel_rgb(piglit_width / 4,
				      piglit_height / 4, red);
	pass = piglit_probe_pixel_rgb(3 * piglit_width / 4,
				      piglit_height / 4, green) && pass;
	pass = piglit_probe_pixel_rgb(piglit_width / 4,
				      3 * piglit_height / 4, blue) && pass;
	pass = piglit_probe_pixel_rgb(3 * piglit_width / 4,
				      3 * piglit_height / 4, white) && pass;

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	GLuint prog;
	GLuint abo;

	piglit_require_extension("GL_ARB_shader_atomic_counters");

	prog = piglit_build_simple_program(
		"#version 140\n"
		"in vec4 piglit_vertex;\n"
		"void main() {\n"
		"	gl_Position = piglit_vertex;\n"
		"}\n",

		"#version 140\n"
		"#extension GL_ARB_shader_atomic_counters: require\n"
		"layout(binding=0) uniform atomic_uint x;\n"
		"void main() {\n"
		"	uint n = atomicCounter(x);\n"
		"	if (n == 0u) gl_FragColor = vec4(1,0,0,0);\n"
		"	else if (n == 1u) gl_FragColor = vec4(0,1,0,0);\n"
		"	else if (n == 2u) gl_FragColor = vec4(0,0,1,0);\n"
		"	else gl_FragColor = vec4(1,1,1,0);\n"
		"}\n");

	glUseProgram(prog);

	glGenBuffers(1, &abo);
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, abo);
	glBufferData(GL_ATOMIC_COUNTER_BUFFER, 4, NULL, GL_STATIC_DRAW);
	glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, abo);
}
