/*
 * Copyright © 2012 Intel Corporation
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
 * @file fs-discard-exit-2.c
 *
 * Tests that the discard keyword stops all further execution on that channel.
 *
 * From the GLSL 1.30 spec revision 9:
 *
 *     "Control flow exits the shader, and subsequent implicit or
 *      explicit derivatives are undefined when this control flow is
 *      non-uniform (meaning different fragments within the primitive
 *      take different control paths)."
 *
 * Here's the testing plan: Divide the 64x64 window into an 8x8 grid.
 * For each grid entry, choose a unique pixel to discard, and then run
 * a loop that would infinite loop on that pixel.  This should get at
 * the intent of the spec and a bug in my first implementation of the
 * fix on i965, while also improving our coverage of pixel discard
 * (which previously tended to discard big regions that were at least
 * 2x2 subspan aligned).
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 64;
	config.window_height = 64;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static int coord1_location, coord2_location;

static const char *vs_source =
	"#version 130\n"
	"in vec4 vertex;\n"
	""
	"void main()\n"
	"{\n"
	"	gl_Position = gl_Vertex;\n"
	"}\n";

/* This shader will discard one pixel coordinate, and do an infinite
 * loop on another pixel.  We set the two coordinates to the same, to
 * test whether discard on a channel avoids execution on that channel.
 */
static const char *fs_source =
	"#version 130\n"
	"uniform ivec2 coord1, coord2;\n"
	"void main()\n"
	"{\n"
	"	ivec2 fc = ivec2(gl_FragCoord);\n"
	"	int inc = abs(fc.x - coord2.x) + abs(fc.y - coord2.y);\n"
	"\n"
	"	if (fc == coord1)\n"
	"		discard;\n"
	"\n"
	"	gl_FragColor = vec4(0);\n"
	"       for (int i = 0; i < 10; i += inc)\n"
	"		gl_FragColor.b += 0.1;\n"
	"}\n";

enum piglit_result
piglit_display(void)
{
	int x, y;
	bool pass = true;
	float expected[64 * 64 * 4];

	glClearColor(0.0, 1.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);

	for (x = 0; x < 8; x++) {
		for (y = 0; y < 8; y++) {
			glUniform2i(coord1_location, x * 9, y * 9);
			glUniform2i(coord2_location, x * 9, y * 9);
			piglit_draw_rect(-1.0 + 0.25 * x,
					 -1.0 + 0.25 * y,
					 0.25,
					 0.25);
		}
	}

	for (x = 0; x < 64; x++) {
		for (y = 0; y < 64; y++) {
			int sx = x % 8;
			int sy = y % 8;
			int dx = fabs(sx - x / 8);
			int dy = fabs(sy - y / 8);
			float pixel[4];

			if (dx == 0 && dy == 0) {
				pixel[0] = 0.0;
				pixel[1] = 1.0;
				pixel[2] = 0.0;
				pixel[3] = 0.0;
			} else {
				int i;

				pixel[0] = 0.0;
				pixel[1] = 0.0;
				pixel[2] = 0.0;
				pixel[3] = 0.0;

				for (i = 0; i < 10; i += (dx + dy))
					pixel[2] += 0.1;
			}

			memcpy(expected + (y * 64 + x) * 4, pixel, 4 * 4);
		}
	}

	pass = piglit_probe_image_rgba(0, 0, 64, 64, expected);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	int prog;

	piglit_require_GLSL_version(130);

	prog = piglit_build_simple_program(vs_source, fs_source);

	coord1_location = glGetUniformLocation(prog, "coord1");
	coord2_location = glGetUniformLocation(prog, "coord2");

	glUseProgram(prog);
}
