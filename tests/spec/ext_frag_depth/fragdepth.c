/*
 * Copyright 2017 Intel Corporation
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
 * Test GL_EXT_frag_depth support in GLSL ES 1.0
 * We draw overlapping red and green quads.  The red quad is at Z=0
 * while the green quad's fragment depths vary from left to right.
 * Should see intersecting quads.
 */


#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_es_version = 20;
	config.window_visual = (PIGLIT_GL_VISUAL_RGBA |
				PIGLIT_GL_VISUAL_DEPTH |
				PIGLIT_GL_VISUAL_DOUBLE);
PIGLIT_GL_TEST_CONFIG_END


static const char *vs_source =
	"#version 100 \n"
	"attribute vec4 piglit_vertex;\n"
	"varying float z; \n"
	"void main() { \n"
	"   gl_Position = piglit_vertex; \n"
	"   // Convert z from [-1, 1] to [0, 1] \n"
	"   z = piglit_vertex.x * 0.5 + 0.5; \n"
	"}\n";
static const char *fs_source =
	"#version 100 \n"
	"#extension GL_EXT_frag_depth : enable \n"
	"precision mediump float; \n"
	"varying float z; \n"
	"uniform vec4 color;\n"
	"void main() { \n"
	"   if (color.g == 1.0) \n"
	"         gl_FragDepthEXT = z; \n"
	"   else \n"
	"         gl_FragDepthEXT = 0.5; \n"
	"   gl_FragColor = color; \n"
	"}\n";

static GLuint program;


enum piglit_result
piglit_display(void)
{
	static const float red[4] = {1.0, 0.0, 0.0, 1.0};
	static const float green[4] = {0.0, 1.0, 0.0, 1.0};
	int x = piglit_width / 2;
	int y = piglit_height / 2;
	bool pass = true;
	int color_loc;

	glViewport(0, 0, piglit_width, piglit_height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(program);
	color_loc = glGetUniformLocation(program, "color");

	glUniform4fv(color_loc, 1, red);
	/* Draw a red rect at z = 0 (will be 0.5 in depth range [0,1]) */
	piglit_draw_rect(-0.5, -0.5, 1.0, 1.0);

	/* Draw green rect with variable z = piglit_vertex.x * 0.5 + 0.5 */
	glUniform4fv(color_loc, 1, green);
	piglit_draw_rect(-0.75, -0.25, 1.5, 0.5);

	pass = piglit_probe_pixel_rgba(x-10, y, green) && pass;
	pass = piglit_probe_pixel_rgba(x+10, y, red) && pass;

	piglit_present_results();
	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_EXT_frag_depth");
	program = piglit_build_simple_program(vs_source, fs_source);
	glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
	glEnable(GL_DEPTH_TEST);
}
