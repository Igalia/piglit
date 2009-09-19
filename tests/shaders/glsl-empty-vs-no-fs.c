/*
 * Copyright 2009 VMware, Inc.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL TUNGSTEN GRAPHICS AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *    Alex Corscadden
 *    Vinson Lee
 */

/*
 * Test an empty GLSL vertex shader without a fragment shader. The program
 * may not link, but if it does, should not trigger a driver crash.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <GL/glew.h>
#if defined(__APPLE__)
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include "piglit-util.h"

static GLboolean Automatic;
static const char *vs_source = "void main() {}";

static void
display(void)
{
	GLint vs;
	GLint prog;
	GLint linked;
	int i;

	for (i = 0; i < 32; i++) {
		vs = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vs, 1, &vs_source, NULL);
		glCompileShader(vs);

		prog = glCreateProgram();
		glAttachShader(prog, vs);
		glLinkProgram(prog);
		glGetProgramiv(prog, GL_LINK_STATUS, &linked);
		if (linked) {
			glUseProgram(prog);
		}

		glFlush();

		glDeleteProgram(prog);
		glDeleteShader(vs);
	}

	if (Automatic) {
		piglit_report_result(PIGLIT_SUCCESS);
	}
}

int
main(int argc, char** argv)
{
	glutInit(&argc, argv);
	if (argc == 2 && !strcmp(argv[1], "-auto"))
		Automatic = 1;
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowSize(250, 250);
	glutCreateWindow("glsl-empty-vs-no-fs");
	glutKeyboardFunc(piglit_escape_exit_key);
	glutDisplayFunc(display);
	glewInit();

	if (!GLEW_VERSION_2_0) {
		printf("Requires OpenGL 2.0\n");
		piglit_report_result(PIGLIT_SKIP);
		exit(1);
	}

	glutMainLoop();

	return 0;
}
