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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 * Authors:
 *    Eric Anholt <eric@anholt.net>
 *
 */

/** @file clear-varray-2.0.c
 *
 * Tests that enabling 2.0's vertex attributes doesn't interfere with glClear.
 *
 * fd.o bug #21638
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 200;
	config.window_height = 100;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_DEPTH;

PIGLIT_GL_TEST_CONFIG_END

/* apply MVP and set the color to blue. */
static const GLchar *const vp_code =
	"!!ARBvp1.0\n"
	"PARAM mvp[4] = { state.matrix.mvp };\n"
	"DP4 result.position.x, mvp[0], vertex.attrib[0];\n"
	"DP4 result.position.y, mvp[1], vertex.attrib[0];\n"
	"DP4 result.position.z, mvp[2], vertex.attrib[0];\n"
	"DP4 result.position.w, mvp[3], vertex.attrib[0];\n"
	"MOV result.color, {0, 0, 1, 0};\n"
	"END"
	;

static const GLchar *const fp_code =
	"!!ARBfp1.0\n"
	"MOV	result.color, fragment.color;\n"
	"END"
	;

enum piglit_result
piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	float vertices[4][4];
	int i;
	float green[4] = {0, 1, 0, 0};
	float blue[4] =  {0, 0, 1, 0};

	vertices[0][0] = 10;
	vertices[0][1] = 10;
	vertices[0][2] = 0;
	vertices[0][3] = 1;

	vertices[1][0] = 20;
	vertices[1][1] = 10;
	vertices[1][2] = 0;
	vertices[1][3] = 1;

	vertices[2][0] = 20;
	vertices[2][1] = 20;
	vertices[2][2] = 0;
	vertices[2][3] = 1;

	vertices[3][0] = 10;
	vertices[3][1] = 20;
	vertices[3][2] = 0;
	vertices[3][3] = 1;

	/* Clear red. */
	glClearColor(1.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/* Draw a blue rect at (10,10)-(20,20) */
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
			      vertices);
	glEnableVertexAttribArray(0);

	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	/* Clear everything to green. Note that we left the attr enabled*/
	glClearColor(0.0, 1.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/* Draw a blue rect at (30,10)-(40,20) */
	for (i = 0; i < 4; i++)
		vertices[i][0] += 20;
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	/* The second clear should have made everything green. */
	pass = pass && piglit_probe_pixel_rgb(30, 30, green);
	/* The first rectangle should have been cleared to green. */
	pass = pass && piglit_probe_pixel_rgb(15, 15, green);
	/* The second rectangle should have shown blue. */
	pass = pass && piglit_probe_pixel_rgb(35, 15, blue);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

static void reshape(int width, int height)
{
	glViewport(0, 0, width, height);
	piglit_ortho_projection(width, height, GL_FALSE);
}

void
piglit_init(int argc, char **argv)
{
	GLuint vert_prog, frag_prog;

	piglit_require_gl_version(20);

	reshape(piglit_width, piglit_height);

	piglit_require_extension("GL_ARB_fragment_program");
	piglit_require_extension("GL_ARB_vertex_program");

	glGenProgramsARB(1, &vert_prog);
	glBindProgramARB(GL_VERTEX_PROGRAM_ARB, vert_prog);
	glProgramStringARB(GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB,
			   strlen(vp_code), vp_code);

	glGenProgramsARB(1, &frag_prog);
	glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, frag_prog);
	glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB,
			   strlen(fp_code), fp_code);

	glEnable(GL_VERTEX_PROGRAM_ARB);
	glEnable(GL_FRAGMENT_PROGRAM_ARB);
}
