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
 *    Ben Holmes <shranzel@hotmail.com>
 */

/*
 * this test draws quads with RGBA and BGRA using glVertexAttribArray.
 * two quads are drawn without blending and two are drawn with alpha blending.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <GL/glew.h>
#if defined(__APPLE__)
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include "piglit-util.h"


static GLboolean Automatic = GL_FALSE;
static GLint prog;
static GLint fs;
static GLint vs;


static GLfloat verts[12] = {225.0, 175.0, 0.0,
				225.0, 225.0, 0.0,
				175.0, 175.0, 0.0,
				175.0, 225.0, 0.0};


static GLubyte colors[16] = {255, 0, 0, 127,
				255, 0, 0, 127,
				255, 0, 0, 127,
				255, 0, 0, 127};


static const char *vertShaderText =
	"attribute vec2 textureCoords;\n"
	"attribute vec4 vColor;\n"
	"varying vec4 vertColor;\n"
	"void main()\n"
	"{ \n"
	"	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
	"	vertColor = vColor;\n"
	"} \n";

static const char *fragShaderText =
	"varying vec4 vertColor;\n"
	"void main()\n"
	"{ \n"
	"	gl_FragColor = vertColor;\n"
	"} \n";


static void
compileLinkProg()
{
	GLint stat;

	vs = glCreateShader(GL_VERTEX_SHADER);
	fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(vs, 1, (const GLchar **) &vertShaderText, NULL);
	glShaderSource(fs, 1, (const GLchar **) &fragShaderText, NULL);
	glCompileShader(vs);
	glGetShaderiv(vs, GL_COMPILE_STATUS, &stat);
	if (!stat) {
		printf("error compiling vertex shader!\n");
		exit(1);
	}
	glCompileShader(fs);
	glGetShaderiv(fs, GL_COMPILE_STATUS, &stat);
	if (!stat) {
		printf("error compiling fragment shader!\n");
		exit(1);
	}

	prog = glCreateProgram();
	glAttachShader(prog, vs);
	glAttachShader(prog, fs);
	glBindAttribLocation(prog, 1, "vColor");
	glLinkProgram(prog);
	glUseProgram(prog);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat),
			      verts);

	glEnableVertexAttribArray(0);
}


static void
Init()
{

	glewInit();

	if (!GLEW_VERSION_2_0) {
		printf("Requires OpenGL 2.0\n");
		piglit_report_result(PIGLIT_SKIP);
	}
	piglit_require_extension("GL_EXT_vertex_array_bgra");

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, 400, 0, 300, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glClearColor(0.6, 0.6, 0.6, 1.0);

	compileLinkProg();

}

static void
display()
{
	glClear(GL_COLOR_BUFFER_BIT);

	glDisableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE,
				4*sizeof(GLubyte), colors);
	glEnableVertexAttribArray(1);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glPushMatrix();
	glTranslatef(75.0, 0.0, 0.0);

	glDisableVertexAttribArray(1);
	glVertexAttribPointer(1, GL_BGRA, GL_UNSIGNED_BYTE, GL_TRUE,
			      4*sizeof(GLubyte), colors);
	glEnableVertexAttribArray(1);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glPopMatrix();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	glPushMatrix();
	glTranslatef(0.0, -75.0, 0.0);

	glDisableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE,
			      4*sizeof(GLubyte), colors);
	glEnableVertexAttribArray(1);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glPushMatrix();
	glTranslatef(75.0, 0.0, 0.0);

	glDisableVertexAttribArray(1);
	glVertexAttribPointer(1, GL_BGRA, GL_UNSIGNED_BYTE, GL_TRUE,
			      4*sizeof(GLubyte), colors);
	glEnableVertexAttribArray(1);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glPopMatrix();
	glPopMatrix();

	GLboolean pass = GL_TRUE;
	GLfloat red[3]={1.0, 0.0, 0.0};
	GLfloat blue[3]={0.0, 0.0, 1.0};
	GLfloat greyRed[3]={1.0, 0.6, 0.6};
	GLfloat greyBlue[3]={0.6, 0.6, 1.0};

	pass = pass && piglit_probe_pixel_rgb(200, 200, red);
	pass = pass && piglit_probe_pixel_rgb(275, 200, blue);
	pass = pass && piglit_probe_pixel_rgb(200, 125, greyRed);
	pass = pass && piglit_probe_pixel_rgb(275, 125, greyBlue);

 	if(Automatic) {
		piglit_report_result(pass ? PIGLIT_SUCCESS : PIGLIT_FAILURE);
		exit(pass ? 0 : 1);
	}

	glFinish();
 	glutSwapBuffers();

	glDisable(GL_BLEND);
}


int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	if(argc==2 && !strncmp(argv[1], "-auto", 5))
		Automatic=GL_TRUE;
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(400, 300);
	glutCreateWindow("bgra-vert-attrib-pointer");
	glutDisplayFunc(display);
	glutKeyboardFunc(piglit_escape_exit_key);

	Init();

	glutMainLoop();

	return 0;
}

