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
 */

/**
 * \file glsl-dlist-getattriblocation.c
 *
 * Validate the behavior of \c glGetAttribLocation while compiling a display
 * list.  See also bugzilla #15202.
 *
 * \author Ian Romanick <ian.d.romanick@intel.com>
 */

#include "piglit-util.h"
#include "piglit-framework.h"

int piglit_WindowMode = GLUT_RGB;
int piglit_Width = 100;
int piglit_Height = 100;

static const GLchar *vertShaderText =
	"attribute vec4 attrib;\n"
	"void main()\n"
	"{\n"
	"	gl_Position = gl_ModelViewProjectionMatrix * attrib;\n"
	"} \n";


int
piglit_Display(void)
{
	GLboolean pass = GL_TRUE;
	GLint vs;
	GLint prog;
	GLint stat;
	GLint attrib_loc;
	GLint attrib_loc_in_dlist;


	vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &vertShaderText, NULL);

	glCompileShader(vs);
	glGetShaderiv(vs, GL_COMPILE_STATUS, &stat);
	if (!stat) {
                printf("error compiling vertex shader1!\n");
                exit(1);
        }

	prog = glCreateProgram();
	glAttachShader(prog, vs);
	glBindAttribLocation(prog, 1, "attrib");
	glLinkProgram(prog);

	attrib_loc = glGetAttribLocation(prog, "attrib");
	if (!piglit_Automatic)
		printf("attrib_loc = %d\n", attrib_loc);

	glNewList(1, GL_COMPILE);

	/* Notice the trickery here!  glBindAttribLocation does not take effect
	 * until glLinkProgram is called!
	 */
	glBindAttribLocation(prog, 2, "attrib");
	attrib_loc_in_dlist = glGetAttribLocation(prog, "attrib");

	if (!piglit_Automatic)
		printf("attrib_loc_in_dlist = %d\n", attrib_loc_in_dlist);
	glEndList();

	pass = (attrib_loc == 1) && (attrib_loc == attrib_loc_in_dlist);
	return pass ? PIGLIT_SUCCESS : PIGLIT_FAILURE;
}


void
piglit_Init(int argc, char **argv)
{
	if (!GLEW_VERSION_2_0) {
		printf("Requires OpenGL 2.0\n");
		piglit_report_result(PIGLIT_SKIP);
		exit(1);
	}

	piglit_ortho_projection(piglit_Width, piglit_Height, GL_FALSE);
}
