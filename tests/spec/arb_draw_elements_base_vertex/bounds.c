/*
 * Copyright © 2009 Intel Corporation
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
 *
 * Authors:
 *    Eric Anholt <eric@anholt.net>
 *    Michal Suchanek <hramrach@centrum.cz>
 *    based on mesa demo added by Keith Whitwell <keith@tungstengraphics.com>
 */

/** @file draw-elements-base-vertex.c
 * Tests ARB_draw_elements_base_vertex functionality by drawing two
 * triangles using different base vertices, using the same vertex and
 * index buffers.
 * In Mesa Gallium 7.11 this causes a crash.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 300;
	config.window_height = 300;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static GLfloat vertices[][4] = {
	{  1, -1, 0, 1 }, /* bottom right */
	{  1,  1, 0, 1 }, /*    top right */
	{ -1,  1, 0, 1 }, /*    top left  */
	{ -1, -1, 0, 1 }, /* bottom left  */
};
static GLubyte colors[][4] = {
	{   0, 255,   0, 0 },
	{   0,   0, 255, 0 },
	{ 255, 255, 255, 0 },
	{ 255,   0,   0, 0 },
};

void
piglit_init(int argc, char **argv)
{
	static const char * program =
		"!!ARBvp1.0\n"
		"MOV result.color, vertex.color;\n"
		"MOV result.position, vertex.position;\n"
		"END\n";
	GLuint program_no;

	piglit_require_extension("GL_ARB_draw_elements_base_vertex");
	piglit_require_extension("GL_ARB_vertex_program");
	glGenProgramsARB(1, &program_no);
	glBindProgramARB(GL_VERTEX_PROGRAM_ARB, program_no);
	glProgramStringARB(GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB,
			strlen(program), program);
	assert(glIsProgramARB(program_no));

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, sizeof(vertices[0]), vertices);
	glEnableClientState(GL_COLOR_ARRAY);
	glColorPointer(4, GL_UNSIGNED_BYTE, 0, colors);
}

enum piglit_result
piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	GLuint indices[] = { 1, 2, 0,
			   3, 0, 1 };
	static GLfloat test_colors[][3] = {
		{ 1, 0, 0 },
		{ 0, 1, 0 },
		{ 0, 0, 1 },
		{ 1, 1, 1 },
	};

	glViewport(0, 0, piglit_width, piglit_height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-1, 1, -1, 1, -0.5, 1000);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glClearColor(0.3, 0.3, 0.3, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_VERTEX_PROGRAM_ARB);
        /* draw elements 3,4,5 ->verts 3,0,1 (lower-right tri) */
	glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, indices + 3);
        /* draw elements 0,1,2 ->verts (1+1),(2+1),(0+1) (upper-left tri) */
	glDrawElementsBaseVertex(GL_TRIANGLES, 3, GL_UNSIGNED_INT, indices, 1);
	glFlush();

	pass = piglit_probe_pixel_rgb(0, 0, test_colors[0]) && pass;
	pass = piglit_probe_pixel_rgb(piglit_width - 1, 0, test_colors[1]) && pass;
	pass = piglit_probe_pixel_rgb(piglit_width - 1, piglit_height - 1, test_colors[2]) && pass;
	pass = piglit_probe_pixel_rgb(0, piglit_height - 1, test_colors[3]) && pass;

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
