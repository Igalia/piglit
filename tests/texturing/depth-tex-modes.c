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

// author: Ben Holmes

/*
 * Draws depth textures as LUMINANCE, INTENSITY, and ALPHA using both 2d
 * textures and texture rectangles.
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
static GLuint tex[6];

static void
Init()
{

        glewInit();
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0, 400, 0, 300, -1, 1);

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        glEnable(GL_TEXTURE_2D);
	glClearColor(0.2, 0.2, 0.2, 1.0);
}

static void
loadTex()
{
	int height = 2;
        int width = 2;
        int i, j;

	GLfloat texDepthData[width][height];
	for (i=0; i < width; ++i) {
		for (j=0; j < height; ++j) {
			if ((i+j) & 1) {
				texDepthData[i][j] = 1.0;
			}
			else {
				texDepthData[i][j] = 0.0;
			}
		}
	}

	//depth texture 0 using LUMINANCE
	glGenTextures(6, tex);
	glBindTexture(GL_TEXTURE_2D, tex[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_LUMINANCE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0,
			GL_DEPTH_COMPONENT, GL_FLOAT, texDepthData);


	//depth texture 1 using INTENSITY
	glBindTexture(GL_TEXTURE_2D, tex[1]);
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0,
			GL_DEPTH_COMPONENT, GL_FLOAT, texDepthData);


	//depth texture 2 using ALPHA
	glBindTexture(GL_TEXTURE_2D, tex[2]);
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_ALPHA);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0,
			GL_DEPTH_COMPONENT, GL_FLOAT, texDepthData);


	//depth texture 3 rectangle using LUMINANCE
	glBindTexture(GL_TEXTURE_RECTANGLE_ARB, tex[3]);
	glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_GENERATE_MIPMAP, GL_FALSE);
	glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER,
			GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER,
			GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_DEPTH_TEXTURE_MODE,
			GL_LUMINANCE);
	glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_DEPTH_COMPONENT, width,
			height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, texDepthData);


	//depth texture 4 rectangle using INTENSITY
	glBindTexture(GL_TEXTURE_RECTANGLE_ARB, tex[4]);
	glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_GENERATE_MIPMAP, GL_FALSE);
	glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER,
			GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER,
			GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_DEPTH_TEXTURE_MODE,
			GL_INTENSITY);
	glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_DEPTH_COMPONENT, width,
			height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, texDepthData);


	//depth texture 5 rectangle using ALPHA
	glBindTexture(GL_TEXTURE_RECTANGLE_ARB, tex[5]);
	glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_GENERATE_MIPMAP, GL_FALSE);
	glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER,
			GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER,
			GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_DEPTH_TEXTURE_MODE,
			GL_ALPHA);
	glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_DEPTH_COMPONENT, width,
			height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, texDepthData);


	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_TEXTURE);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_TEXTURE);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_CONSTANT);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, GL_CONSTANT);

	const GLfloat color[4] = {1.0, 0.0, 1.0, 1.0};
	glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, color);


}


static void
display()
{

	glClear(GL_COLOR_BUFFER_BIT);

	const GLfloat color2[4] = {0.0, 1.0, 0.0, 1.0};
	const GLfloat color1[4] = {1.0, 0.0, 1.0, 1.0};
	glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, color1);

	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, GL_SRC_ALPHA);

	glDisable(GL_TEXTURE_RECTANGLE_ARB);
	glEnable(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, tex[0]);
	glBegin(GL_TRIANGLE_STRIP);
		glTexCoord2f(1.0, 0.0);
		glVertex3f(225, 175, 0);
		glTexCoord2f(1.0, 1.0);
		glVertex3f(225, 225, 0);
		glTexCoord2f(0.0, 0.0);
		glVertex3f(175, 175, 0);
		glTexCoord2f(0.0, 1.0);
		glVertex3f(175, 225, 0);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, tex[1]);
	glBegin(GL_TRIANGLE_STRIP);
		glTexCoord2f(1.0, 0.0);
		glVertex3f(300, 175, 0);
		glTexCoord2f(1.0, 1.0);
		glVertex3f(300, 225, 0);
		glTexCoord2f(0.0, 0.0);
		glVertex3f(250, 175, 0);
		glTexCoord2f(0.0, 1.0);
		glVertex3f(250, 225, 0);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, tex[2]);
	glBegin(GL_TRIANGLE_STRIP);
		glTexCoord2f(1.0, 0.0);
		glVertex3f(150, 175, 0);
		glTexCoord2f(1.0, 1.0);
		glVertex3f(150, 225, 0);
		glTexCoord2f(0.0, 0.0);
		glVertex3f(100, 175, 0);
		glTexCoord2f(0.0, 1.0);
		glVertex3f(100, 225, 0);
	glEnd();


	glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, color2);

	glDisable(GL_TEXTURE_2D);
	glEnable(GL_TEXTURE_RECTANGLE_ARB);


	glBindTexture(GL_TEXTURE_RECTANGLE_ARB, tex[3]);
	glBegin(GL_TRIANGLE_STRIP);
		glTexCoord2f(2.0, 0.0);
		glVertex3f(225, 25, 0);
		glTexCoord2f(2.0, 2.0);
		glVertex3f(225, 75, 0);
		glTexCoord2f(0.0, 0.0);
		glVertex3f(175, 25, 0);
		glTexCoord2f(0.0, 2.0);
		glVertex3f(175, 75, 0);
	glEnd();

	glBindTexture(GL_TEXTURE_RECTANGLE_ARB, tex[4]);
	glBegin(GL_TRIANGLE_STRIP);
		glTexCoord2f(2.0, 0.0);
		glVertex3f(300, 25, 0);
		glTexCoord2f(2.0, 2.0);
		glVertex3f(300, 75, 0);
		glTexCoord2f(0.0, 0.0);
		glVertex3f(250, 25, 0);
		glTexCoord2f(0.0, 2.0);
		glVertex3f(250, 75, 0);
	glEnd();

	glBindTexture(GL_TEXTURE_RECTANGLE_ARB, tex[5]);
	glBegin(GL_TRIANGLE_STRIP);
		glTexCoord2f(2.0, 0.0);
		glVertex3f(150, 25, 0);
		glTexCoord2f(2.0, 2.0);
		glVertex3f(150, 75, 0);
		glTexCoord2f(0.0, 0.0);
		glVertex3f(100, 25, 0);
		glTexCoord2f(0.0, 2.0);
		glVertex3f(100, 75, 0);
	glEnd();


	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_ALPHA);

	glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, color1);

	glDisable(GL_TEXTURE_RECTANGLE_ARB);
	glEnable(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, tex[0]);
	glBegin(GL_TRIANGLE_STRIP);
		glTexCoord2f(1.0, 0.0);
		glVertex3f(225, 250, 0);
		glTexCoord2f(1.0, 1.0);
		glVertex3f(225, 300, 0);
		glTexCoord2f(0.0, 0.0);
		glVertex3f(175, 250, 0);
		glTexCoord2f(0.0, 1.0);
		glVertex3f(175, 300, 0);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, tex[1]);
	glBegin(GL_TRIANGLE_STRIP);
		glTexCoord2f(1.0, 0.0);
		glVertex3f(300, 250, 0);
		glTexCoord2f(1.0, 1.0);
		glVertex3f(300, 300, 0);
		glTexCoord2f(0.0, 0.0);
		glVertex3f(250, 250, 0);
		glTexCoord2f(0.0, 1.0);
		glVertex3f(250, 300, 0);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, tex[2]);
	glBegin(GL_TRIANGLE_STRIP);
		glTexCoord2f(1.0, 0.0);
		glVertex3f(150, 250, 0);
		glTexCoord2f(1.0, 1.0);
		glVertex3f(150, 300, 0);
		glTexCoord2f(0.0, 0.0);
		glVertex3f(100, 250, 0);
		glTexCoord2f(0.0, 1.0);
		glVertex3f(100, 300, 0);
	glEnd();


	glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, color2);

	glDisable(GL_TEXTURE_2D);
	glEnable(GL_TEXTURE_RECTANGLE_ARB);


	glBindTexture(GL_TEXTURE_RECTANGLE_ARB, tex[3]);
	glBegin(GL_TRIANGLE_STRIP);
		glTexCoord2f(2.0, 0.0);
		glVertex3f(225, 100, 0);
		glTexCoord2f(2.0, 2.0);
		glVertex3f(225, 150, 0);
		glTexCoord2f(0.0, 0.0);
		glVertex3f(175, 100, 0);
		glTexCoord2f(0.0, 2.0);
		glVertex3f(175, 150, 0);
	glEnd();

	glBindTexture(GL_TEXTURE_RECTANGLE_ARB, tex[4]);
	glBegin(GL_TRIANGLE_STRIP);
		glTexCoord2f(2.0, 0.0);
		glVertex3f(300, 100, 0);
		glTexCoord2f(2.0, 2.0);
		glVertex3f(300, 150, 0);
		glTexCoord2f(0.0, 0.0);
		glVertex3f(250, 100, 0);
		glTexCoord2f(0.0, 2.0);
		glVertex3f(250, 150, 0);
	glEnd();

	glBindTexture(GL_TEXTURE_RECTANGLE_ARB, tex[5]);
	glBegin(GL_TRIANGLE_STRIP);
		glTexCoord2f(2.0, 0.0);
		glVertex3f(150, 100, 0);
		glTexCoord2f(2.0, 2.0);
		glVertex3f(150, 150, 0);
		glTexCoord2f(0.0, 0.0);
		glVertex3f(100, 100, 0);
		glTexCoord2f(0.0, 2.0);
		glVertex3f(100, 150, 0);
	glEnd();


	GLboolean pass = GL_TRUE;

	GLfloat pink[3] = {1.0, 0.0, 1.0};
	GLfloat green[3] = {0.0, 1.0, 0.0};
	GLfloat black[3] = {0.0, 0.0, 0.0};

	pass = piglit_probe_pixel_rgb(110, 180, black);
	pass = pass && piglit_probe_pixel_rgb(140, 180, black);
	pass = pass && piglit_probe_pixel_rgb(185, 180, black);
	pass = pass && piglit_probe_pixel_rgb(215, 180, pink);
	pass = pass && piglit_probe_pixel_rgb(260, 180, black);
	pass = pass && piglit_probe_pixel_rgb(290, 180, pink);

	pass = pass && piglit_probe_pixel_rgb(110, 255, black);
	pass = pass && piglit_probe_pixel_rgb(140, 255, pink);
	pass = pass && piglit_probe_pixel_rgb(185, 255, pink);
	pass = pass && piglit_probe_pixel_rgb(215, 255, pink);
	pass = pass && piglit_probe_pixel_rgb(260, 255, black);
	pass = pass && piglit_probe_pixel_rgb(290, 255, pink);

	pass = pass && piglit_probe_pixel_rgb(110, 35, black);
	pass = pass && piglit_probe_pixel_rgb(140, 35, black);
	pass = pass && piglit_probe_pixel_rgb(185, 35, black);
	pass = pass && piglit_probe_pixel_rgb(215, 35, green);
	pass = pass && piglit_probe_pixel_rgb(260, 35, black);
	pass = pass && piglit_probe_pixel_rgb(290, 35, green);

	pass = pass && piglit_probe_pixel_rgb(110, 110, black);
	pass = pass && piglit_probe_pixel_rgb(140, 110, green);
	pass = pass && piglit_probe_pixel_rgb(185, 110, green);
	pass = pass && piglit_probe_pixel_rgb(215, 110, green);
	pass = pass && piglit_probe_pixel_rgb(260, 110, black);
	pass = pass && piglit_probe_pixel_rgb(290, 110, green);

	if(Automatic) {
		piglit_report_result(pass ? PIGLIT_SUCCESS : PIGLIT_FAILURE);
		exit(pass ? 0 : 1);
	}

	glFinish();
	glutSwapBuffers();

	printf(" Left to Right: ALPHA, LUMINANCE, INTENSITY\n Lower row: Combined with color\n Upper row: combined with alpha\n pink: TEXTURE_2D green: TEXTURE_RECTANGLE\n");

}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	if(argc==2 && !strncmp(argv[1], "-auto", 5))
		Automatic=GL_TRUE;
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(400, 300);
	glutCreateWindow("depth-tex-compare");
	glutDisplayFunc(display);
	glutKeyboardFunc(piglit_escape_exit_key);

	Init();

	loadTex();

	glutMainLoop();

	return 0;
}
//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE,
			//GL_COMPARE_R_TO_TEXTURE);

