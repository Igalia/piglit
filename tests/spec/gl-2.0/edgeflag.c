/* Copyright © 2012 Intel Corporation
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

/** @file edgeflag.c
 *
 * Test for glEdgeFlagPointerv() API working with a GLSL program enabled.
 *
 * The i965 driver had a bug where edge flags didn't work under GLSL.
 * We also use both gl_Vertex and a user attribute so that edge flags
 * sit in the middle of the list of vertex attributes and results
 * in Mesa.
 */

#include "piglit-util-gl.h"
#include "minmax-test.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

static GLuint color_index;

enum piglit_result
piglit_display(void)
{
	bool pass = true;
	float green[] = {0, 1, 0, 0};
	float clear[] = {0.5, 0.5, 0.5, 0.5};
	struct {
		GLfloat x, y;
		GLfloat r, g, b, a;
		GLboolean edgeflag;
	} verts[] = {
		{ 1.5, 1.5, 0.0, 1.0, 0.0, 0.0, GL_TRUE },
		{ 5.5, 1.5, 0.0, 1.0, 0.0, 0.0, GL_FALSE },
		{ 5.5, 5.5, 0.0, 1.0, 0.0, 0.0, GL_TRUE },
		{ 1.5, 5.5, 0.0, 1.0, 0.0, 0.0, GL_FALSE },
	};

	piglit_ortho_projection(piglit_width, piglit_height, false);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glClearColor(0.5, 0.5, 0.5, 0.5);
	glClear(GL_COLOR_BUFFER_BIT);

	glColor4f(0, 1, 0, 0);

	glVertexPointer(2, GL_FLOAT, sizeof(verts[0]), &verts[0].x);
	glVertexAttribPointer(color_index, 4, GL_FLOAT, GL_FALSE,
			      sizeof(verts[0]), &verts[0].r);
	glEdgeFlagPointer(sizeof(verts[0]), &verts[0].edgeflag);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableVertexAttribArray(color_index);
	glEnableClientState(GL_EDGE_FLAG_ARRAY);

	glDrawArrays(GL_POLYGON, 0, 4);

	pass = piglit_probe_pixel_rgba(3, 1, green) && pass;
	pass = piglit_probe_pixel_rgba(3, 5, green) && pass;
	pass = piglit_probe_pixel_rgba(1, 3, clear) && pass;
	pass = piglit_probe_pixel_rgba(5, 3, clear) && pass;

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

const char *vs_source =
	"attribute vec4 in_color;\n"
	"varying vec4 color;\n"
	"\n"
	"void main()\n"
	"{\n"
	"	gl_Position = ftransform();\n"
	"	color = in_color;\n"
	"}\n";

const char *fs_source =
	"varying vec4 color;\n"
	"\n"
	"void main()\n"
	"{\n"
	"	gl_FragColor = color;\n"
	"}\n";

void
piglit_init(int argc, char **argv)
{
	GLuint prog;

	prog = piglit_build_simple_program(vs_source, fs_source);
	glUseProgram(prog);
	color_index = glGetAttribLocation(prog, "in_color");
}
