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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/**
 * \file draw-vertices.c
 *
 * This tests that drawing works with the ARB_vertex_type_10f_11f_11f_rev extension.
 *
 */

#include "piglit-util-gl.h"
#include "r11g11b10f.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 20;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;
	config.window_width = 128;
	config.window_height = 128;
PIGLIT_GL_TEST_CONFIG_END


static float unpacked_pos[][2] = {
	{ -1, -1 },
	{ -0.5f, -1 },
	{ -0.5f, 1 },
	{ -1, 1 },

	{ -0.5f, -1 },
	{ 0, -1 },
	{ 0, 1 },
	{ -0.5f, 1 },

	{ 0, -1 },
	{ 0.5f, -1 },
	{ 0.5f, 1 },
	{ 0, 1 },

	{ 0.5f, -1 },
	{ 1, -1 },
	{ 1, 1 },
	{ 0.5f, 1 },
};

static float unpacked_colors[][4] = {
	{ 0.5, 0, 1, 1 },
	{ 0.5, 0, 0, 1 },
	{ 0, 0.5, 0, 1 },
	{ 1, 0.5, 0, 1 },
};

enum piglit_result
piglit_display()
{
	bool pass = true;
	glDrawArrays(GL_QUADS, 0, 16);

	pass = piglit_probe_pixel_rgba(8, 64, unpacked_colors[0]) && pass;
	pass = piglit_probe_pixel_rgba(40, 64, unpacked_colors[1]) && pass;
	pass = piglit_probe_pixel_rgba(72, 64, unpacked_colors[2]) && pass;
	pass = piglit_probe_pixel_rgba(104, 64, unpacked_colors[3]) && pass;
	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	GLuint bo_pos, bo_color, prog;
	unsigned int colors[16];
	int n;

	for (n = 0; n < 16; n++)
		colors[n] = float3_to_r11g11b10f(unpacked_colors[n/4]);

	piglit_require_extension("GL_ARB_vertex_type_10f_11f_11f_rev");

	glGenBuffers(1, &bo_pos);
	glBindBuffer(GL_ARRAY_BUFFER, bo_pos);
	glBufferData(GL_ARRAY_BUFFER, sizeof(unpacked_pos), unpacked_pos, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid const *)0);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &bo_color);
	glBindBuffer(GL_ARRAY_BUFFER, bo_color);
	glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);

	glVertexAttribPointer(1, 3, GL_UNSIGNED_INT_10F_11F_11F_REV, GL_FALSE, 0, (GLvoid const *)0);
	glEnableVertexAttribArray(1);

	prog = piglit_build_simple_program_unlinked(
		"attribute vec2 p;\n"
		"attribute vec3 c;\n"
		"varying vec3 color;\n"
		"void main() { gl_Position = vec4(p, 0, 1); color = c; }\n",

		"varying vec3 color;\n"
		"void main() { gl_FragColor = vec4(color,1); }\n"
		);
	if (!prog)
		piglit_report_result(PIGLIT_FAIL);

	glBindAttribLocation(prog, 0, "p");
	glBindAttribLocation(prog, 1, "c");
	glLinkProgram(prog);
	if (!piglit_link_check_status(prog))
		piglit_report_result(PIGLIT_FAIL);

	glUseProgram(prog);
}

