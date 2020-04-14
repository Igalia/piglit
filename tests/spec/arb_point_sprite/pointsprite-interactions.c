/*
 * Copyright (c) 2020 Advanced Micro Devices, Inc.
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
 * @file pointsprite-interactions.c
 *
 * This is a reproducer for https://gitlab.freedesktop.org/mesa/mesa/-/issues/2747
 * The root issue on radeonsi was a bad interaction between point sprite and VS
 * outputs optimization.
 * This test draws a rectangle with point sprite enabled using a constant
 * gl_TexCoord[0] as the color.
 * Then we verify that the screen was indeed painted to a solid color.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 20;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

const char* vs_source_tmpl =
"attribute vec4 piglit_vertex;\n"
"\n"
"void main()\n"
"{\n"
	"gl_TexCoord[0] = vec4(%f);\n"
	"gl_Position = piglit_vertex;\n"
"}\n";

const char* fs_source =
"void main()\n"
"{\n"
"	gl_FragColor = gl_TexCoord[0];\n"
"}\n";

static float color;

enum piglit_result
piglit_display(void)
{
	int result;
	float expected[3] = { color, color, color };

	piglit_draw_rect(-1, -1, 2, 2);
	result = piglit_probe_rect_rgb(0, 0, piglit_width, piglit_height, expected);
	piglit_present_results();

	return result ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char**argv)
{
	color = argc > 1 ? atof(argv[1]) : 1.0;

	char vs[1024];
	sprintf(vs, vs_source_tmpl, color);

	glUseProgram(piglit_build_simple_program(vs, fs_source));

	glDisable(GL_DEPTH_TEST);
	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);

	/* Enable point sprite, but it shouldn't have any effect since,
	 * we're drawing triangles. */
	glEnable(GL_POINT_SPRITE);
	glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, 1);
}
