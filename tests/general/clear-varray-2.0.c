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

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#define GL_GLEXT_PROTOTYPES
#include "GL/glew.h"
#if defined(__APPLE__)
#include <GLUT/glut.h>
#else
#include "GL/glut.h"
#endif

#include "piglit-util.h"

#define WIN_WIDTH 200
#define WIN_HEIGHT 100

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

static GLboolean Automatic = GL_FALSE;

static void
display(void)
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
	piglit_probe_pixel_rgb(30, 30, green);
	/* The first rectangle should have been cleared to green. */
	piglit_probe_pixel_rgb(15, 15, green);
	/* The second rectangle should have shown blue. */
	piglit_probe_pixel_rgb(35, 15, blue);

	glutSwapBuffers();

	if (Automatic) {
		printf("PIGLIT: {'result': '%s' }\n",
		       pass ? "pass" : "fail");
		exit(pass ? 0 : 1);
	}
}

static void reshape(int width, int height)
{
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, width, 0.0, height, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

static void
init(void)
{
	GLuint vert_prog, frag_prog;

	reshape(WIN_WIDTH, WIN_HEIGHT);

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

int main(int argc, char**argv)
{
	glutInit(&argc, argv);
	if (argc == 2 && !strcmp(argv[1], "-auto"))
		Automatic = 1;
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(WIN_WIDTH, WIN_HEIGHT);
	glutCreateWindow("clear-varray-2.0");
	glutReshapeFunc(reshape);
	glutDisplayFunc(display);
	glutKeyboardFunc(piglit_escape_exit_key);

	glewInit();

	init();

	if (!GLEW_VERSION_2_0) {
		printf("Requires OpenGL 2.0\n");
		piglit_report_result(PIGLIT_SKIP);
	}
	piglit_require_extension("GL_ARB_fragment_program");
	piglit_require_extension("GL_ARB_vertex_program");

	glutMainLoop();

	return 0;
}
