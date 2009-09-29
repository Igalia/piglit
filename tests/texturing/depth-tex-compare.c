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
 * This test draws depth textures as LUMINANCE, INTENSITY, and ALPHA. These
 * textures are compared to the r component of the texture coordinate and
 * compared using all eight texture compare functions. The result of the
 * comparison is modulated with the vertex color (pink) and blended with the
 * clear color (green) using the alpha value.
 */

#include "piglit-util.h"

static GLboolean Automatic = GL_FALSE;
static GLuint tex[3];

static void
Init(void)
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
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(0.0, 1.0, 0.0, 1.0);

}

static void
loadTex(void)
{
	#define height 2
	#define width 2
	int i, j;

	GLfloat texDepthData[width][height];
	for (i=0; i < width; ++i) {
		for (j=0; j < height; ++j) {
			if ((i+j) & 1) {
				texDepthData[i][j] = 1.0;
			}
			else {
				texDepthData[i][j] = 0.5;
			}
		}
	}

	//depth texture 0 using LUMINANCE
	glGenTextures(3, tex);
	glBindTexture(GL_TEXTURE_2D, tex[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_LUMINANCE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE,
			GL_COMPARE_R_TO_TEXTURE);
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
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE,
			GL_COMPARE_R_TO_TEXTURE);
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
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE,
			GL_COMPARE_R_TO_TEXTURE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0,
			GL_DEPTH_COMPONENT, GL_FLOAT, texDepthData);

	#undef height
	#undef width
}


static void
display(void)
{
	GLboolean pass = GL_TRUE;

	GLfloat pink[3] = {1.0, 0.0, 1.0};
	GLfloat white[3] = {1.0, 1.0, 1.0};
	GLfloat black[3] = {0.0, 0.0, 0.0};
	GLfloat green[3] = {0.0, 1.0, 0.0};

	glClear(GL_COLOR_BUFFER_BIT);

	glColor3f(1.0, 0.0, 1.0);
	glBindTexture(GL_TEXTURE_2D, tex[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LESS);
	glBegin(GL_TRIANGLE_STRIP);
		glTexCoord3f(1.0, 0.0, 2.0);
		glVertex3f(225, 275, 0);
		glTexCoord3f(1.0, 1.0, 2.0);
		glVertex3f(225, 300, 0);
		glTexCoord3f(0.0, 0.0, 0.0);
		glVertex3f(200, 275, 0);
		glTexCoord3f(0.0, 1.0, 0.0);
		glVertex3f(200, 300, 0);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, tex[1]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LESS);
	glBegin(GL_TRIANGLE_STRIP);
		glTexCoord3f(1.0, 0.0, 2.0);
		glVertex3f(275, 275, 0);
		glTexCoord3f(1.0, 1.0, 2.0);
		glVertex3f(275, 300, 0);
		glTexCoord3f(0.0, 0.0, 0.0);
		glVertex3f(250, 275, 0);
		glTexCoord3f(0.0, 1.0, 0.0);
		glVertex3f(250, 300, 0);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, tex[2]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LESS);
	glBegin(GL_TRIANGLE_STRIP);
		glTexCoord3f(1.0, 0.0, 2.0);
		glVertex3f(175, 275, 0);
		glTexCoord3f(1.0, 1.0, 2.0);
		glVertex3f(175, 300, 0);
		glTexCoord3f(0.0, 0.0, 0.0);
		glVertex3f(150, 275, 0);
		glTexCoord3f(0.0, 1.0, 0.0);
		glVertex3f(150, 300, 0);
	glEnd();


	glBindTexture(GL_TEXTURE_2D, tex[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glBegin(GL_TRIANGLE_STRIP);
		glTexCoord3f(1.0, 0.0, 2.0);
		glVertex3f(225, 240, 0);
		glTexCoord3f(1.0, 1.0, 2.0);
		glVertex3f(225, 265, 0);
		glTexCoord3f(0.0, 0.0, 0.0);
		glVertex3f(200, 240, 0);
		glTexCoord3f(0.0, 1.0, 0.0);
		glVertex3f(200, 265, 0);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, tex[1]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glBegin(GL_TRIANGLE_STRIP);
		glTexCoord3f(1.0, 0.0, 2.0);
		glVertex3f(275, 240, 0);
		glTexCoord3f(1.0, 1.0, 2.0);
		glVertex3f(275, 265, 0);
		glTexCoord3f(0.0, 0.0, 0.0);
		glVertex3f(250, 240, 0);
		glTexCoord3f(0.0, 1.0, 0.0);
		glVertex3f(250, 265, 0);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, tex[2]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glBegin(GL_TRIANGLE_STRIP);
		glTexCoord3f(1.0, 0.0, 2.0);
		glVertex3f(175, 240, 0);
		glTexCoord3f(1.0, 1.0, 2.0);
		glVertex3f(175, 265, 0);
		glTexCoord3f(0.0, 0.0, 0.0);
		glVertex3f(150, 240, 0);
		glTexCoord3f(0.0, 1.0, 0.0);
		glVertex3f(150, 265, 0);
	glEnd();

		glBindTexture(GL_TEXTURE_2D, tex[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_GREATER);
	glBegin(GL_TRIANGLE_STRIP);
		glTexCoord3f(1.0, 0.0, 2.0);
		glVertex3f(225, 205, 0);
		glTexCoord3f(1.0, 1.0, 2.0);
		glVertex3f(225, 230, 0);
		glTexCoord3f(0.0, 0.0, 0.0);
		glVertex3f(200, 205, 0);
		glTexCoord3f(0.0, 1.0, 0.0);
		glVertex3f(200, 230, 0);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, tex[1]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_GREATER);
	glBegin(GL_TRIANGLE_STRIP);
		glTexCoord3f(1.0, 0.0, 2.0);
		glVertex3f(275, 205, 0);
		glTexCoord3f(1.0, 1.0, 2.0);
		glVertex3f(275, 230, 0);
		glTexCoord3f(0.0, 0.0, 0.0);
		glVertex3f(250, 205, 0);
		glTexCoord3f(0.0, 1.0, 0.0);
		glVertex3f(250, 230, 0);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, tex[2]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_GREATER);
	glBegin(GL_TRIANGLE_STRIP);
		glTexCoord3f(1.0, 0.0, 2.0);
		glVertex3f(175, 205, 0);
		glTexCoord3f(1.0, 1.0, 2.0);
		glVertex3f(175, 230, 0);
		glTexCoord3f(0.0, 0.0, 0.0);
		glVertex3f(150, 205, 0);
		glTexCoord3f(0.0, 1.0, 0.0);
		glVertex3f(150, 230, 0);
	glEnd();


	glBindTexture(GL_TEXTURE_2D, tex[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_GEQUAL);
	glBegin(GL_TRIANGLE_STRIP);
		glTexCoord3f(1.0, 0.0, 2.0);
		glVertex3f(225, 170, 0);
		glTexCoord3f(1.0, 1.0, 2.0);
		glVertex3f(225, 195, 0);
		glTexCoord3f(0.0, 0.0, 0.0);
		glVertex3f(200, 170, 0);
		glTexCoord3f(0.0, 1.0, 0.0);
		glVertex3f(200, 195, 0);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, tex[1]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_GEQUAL);
	glBegin(GL_TRIANGLE_STRIP);
		glTexCoord3f(1.0, 0.0, 2.0);
		glVertex3f(275, 170, 0);
		glTexCoord3f(1.0, 1.0, 2.0);
		glVertex3f(275, 195, 0);
		glTexCoord3f(0.0, 0.0, 0.0);
		glVertex3f(250, 170, 0);
		glTexCoord3f(0.0, 1.0, 0.0);
		glVertex3f(250, 195, 0);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, tex[2]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_GEQUAL);
	glBegin(GL_TRIANGLE_STRIP);
		glTexCoord3f(1.0, 0.0, 2.0);
		glVertex3f(175, 170, 0);
		glTexCoord3f(1.0, 1.0, 2.0);
		glVertex3f(175, 195, 0);
		glTexCoord3f(0.0, 0.0, 0.0);
		glVertex3f(150, 170, 0);
		glTexCoord3f(0.0, 1.0, 0.0);
		glVertex3f(150, 195, 0);
	glEnd();


	glBindTexture(GL_TEXTURE_2D, tex[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_ALWAYS);
	glBegin(GL_TRIANGLE_STRIP);
		glTexCoord3f(1.0, 0.0, 2.0);
		glVertex3f(225, 135, 0);
		glTexCoord3f(1.0, 1.0, 2.0);
		glVertex3f(225, 160, 0);
		glTexCoord3f(0.0, 0.0, 0.0);
		glVertex3f(200, 135, 0);
		glTexCoord3f(0.0, 1.0, 0.0);
		glVertex3f(200, 160, 0);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, tex[1]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_ALWAYS);
	glBegin(GL_TRIANGLE_STRIP);
		glTexCoord3f(1.0, 0.0, 2.0);
		glVertex3f(275, 135, 0);
		glTexCoord3f(1.0, 1.0, 2.0);
		glVertex3f(275, 160, 0);
		glTexCoord3f(0.0, 0.0, 0.0);
		glVertex3f(250, 135, 0);
		glTexCoord3f(0.0, 1.0, 0.0);
		glVertex3f(250, 160, 0);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, tex[2]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_ALWAYS);
	glBegin(GL_TRIANGLE_STRIP);
		glTexCoord3f(1.0, 0.0, 2.0);
		glVertex3f(175, 135, 0);
		glTexCoord3f(1.0, 1.0, 2.0);
		glVertex3f(175, 160, 0);
		glTexCoord3f(0.0, 0.0, 0.0);
		glVertex3f(150, 135, 0);
		glTexCoord3f(0.0, 1.0, 0.0);
		glVertex3f(150, 160, 0);
	glEnd();


	glBindTexture(GL_TEXTURE_2D, tex[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_NEVER);
	glBegin(GL_TRIANGLE_STRIP);
		glTexCoord3f(1.0, 0.0, 2.0);
		glVertex3f(225, 100, 0);
		glTexCoord3f(1.0, 1.0, 2.0);
		glVertex3f(225, 125, 0);
		glTexCoord3f(0.0, 0.0, 0.0);
		glVertex3f(200, 100, 0);
		glTexCoord3f(0.0, 1.0, 0.0);
		glVertex3f(200, 125, 0);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, tex[1]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_NEVER);
	glBegin(GL_TRIANGLE_STRIP);
		glTexCoord3f(1.0, 0.0, 2.0);
		glVertex3f(275, 100, 0);
		glTexCoord3f(1.0, 1.0, 2.0);
		glVertex3f(275, 125, 0);
		glTexCoord3f(0.0, 0.0, 0.0);
		glVertex3f(250, 100, 0);
		glTexCoord3f(0.0, 1.0, 0.0);
		glVertex3f(250, 125, 0);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, tex[2]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_NEVER);
	glBegin(GL_TRIANGLE_STRIP);
		glTexCoord3f(1.0, 0.0, 2.0);
		glVertex3f(175, 100, 0);
		glTexCoord3f(1.0, 1.0, 2.0);
		glVertex3f(175, 125, 0);
		glTexCoord3f(0.0, 0.0, 0.0);
		glVertex3f(150, 100, 0);
		glTexCoord3f(0.0, 1.0, 0.0);
		glVertex3f(150, 125, 0);
	glEnd();


	glBindTexture(GL_TEXTURE_2D, tex[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_NOTEQUAL);
	glBegin(GL_TRIANGLE_STRIP);
		glTexCoord3f(1.0, 0.0, 0.5);
		glVertex3f(225, 65, 0);
		glTexCoord3f(1.0, 1.0, 0.5);
		glVertex3f(225, 90, 0);
		glTexCoord3f(0.0, 0.0, 0.5);
		glVertex3f(200, 65, 0);
		glTexCoord3f(0.0, 1.0, 0.5);
		glVertex3f(200, 90, 0);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, tex[1]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_NOTEQUAL);
	glBegin(GL_TRIANGLE_STRIP);
		glTexCoord3f(1.0, 0.0, 0.5);
		glVertex3f(275, 65, 0);
		glTexCoord3f(1.0, 1.0, 0.5);
		glVertex3f(275, 90, 0);
		glTexCoord3f(0.0, 0.0, 0.5);
		glVertex3f(250, 65, 0);
		glTexCoord3f(0.0, 1.0, 0.5);
		glVertex3f(250, 90, 0);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, tex[2]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_NOTEQUAL);
	glBegin(GL_TRIANGLE_STRIP);
		glTexCoord3f(1.0, 0.0, 0.5);
		glVertex3f(175, 65, 0);
		glTexCoord3f(1.0, 1.0, 0.5);
		glVertex3f(175, 90, 0);
		glTexCoord3f(0.0, 0.0, 0.5);
		glVertex3f(150, 65, 0);
		glTexCoord3f(0.0, 1.0, 0.5);
		glVertex3f(150, 90, 0);
	glEnd();


	glBindTexture(GL_TEXTURE_2D, tex[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_EQUAL);
	glBegin(GL_TRIANGLE_STRIP);
		glTexCoord3f(1.0, 0.0, 0.5);
		glVertex3f(225, 30, 0);
		glTexCoord3f(1.0, 1.0, 0.5);
		glVertex3f(225, 55, 0);
		glTexCoord3f(0.0, 0.0, 0.5);
		glVertex3f(200, 30, 0);
		glTexCoord3f(0.0, 1.0, 0.5);
		glVertex3f(200, 55, 0);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, tex[1]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_EQUAL);
	glBegin(GL_TRIANGLE_STRIP);
		glTexCoord3f(1.0, 0.0, 0.5);
		glVertex3f(275, 30, 0);
		glTexCoord3f(1.0, 1.0, 0.5);
		glVertex3f(275, 55, 0);
		glTexCoord3f(0.0, 0.0, 0.5);
		glVertex3f(250, 30, 0);
		glTexCoord3f(0.0, 1.0, 0.5);
		glVertex3f(250, 55, 0);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, tex[2]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_EQUAL);
	glBegin(GL_TRIANGLE_STRIP);
		glTexCoord3f(1.0, 0.0, 0.5);
		glVertex3f(175, 30, 0);
		glTexCoord3f(1.0, 1.0, 0.5);
		glVertex3f(175, 55, 0);
		glTexCoord3f(0.0, 0.0, 0.5);
		glVertex3f(150, 30, 0);
		glTexCoord3f(0.0, 1.0, 0.5);
		glVertex3f(150, 55, 0);
	glEnd();


	//less
	pass = pass && piglit_probe_pixel_rgb(155, 285, pink);
	pass = pass && piglit_probe_pixel_rgb(205, 285, pink);
	pass = pass && piglit_probe_pixel_rgb(255, 285, pink);
	pass = pass && piglit_probe_pixel_rgb(160, 285, white);
	pass = pass && piglit_probe_pixel_rgb(210, 285, black);
	pass = pass && piglit_probe_pixel_rgb(260, 285, green);
	pass = pass && piglit_probe_pixel_rgb(165, 285, white);
	pass = pass && piglit_probe_pixel_rgb(215, 285, black);
	pass = pass && piglit_probe_pixel_rgb(265, 285, green);

	//lequal
	pass = pass && piglit_probe_pixel_rgb(155, 250, pink);
	pass = pass && piglit_probe_pixel_rgb(205, 250, pink);
	pass = pass && piglit_probe_pixel_rgb(255, 250, pink);
	pass = pass && piglit_probe_pixel_rgb(160, 250, white);
	pass = pass && piglit_probe_pixel_rgb(210, 250, black);
	pass = pass && piglit_probe_pixel_rgb(260, 250, green);
	pass = pass && piglit_probe_pixel_rgb(165, 250, white);
	pass = pass && piglit_probe_pixel_rgb(215, 250, black);
	pass = pass && piglit_probe_pixel_rgb(265, 250, green);

	//greater
	pass = pass && piglit_probe_pixel_rgb(155, 215, white);
	pass = pass && piglit_probe_pixel_rgb(205, 215, black);
	pass = pass && piglit_probe_pixel_rgb(255, 215, green);
	pass = pass && piglit_probe_pixel_rgb(160, 215, pink);
	pass = pass && piglit_probe_pixel_rgb(210, 215, pink);
	pass = pass && piglit_probe_pixel_rgb(260, 215, pink);
	pass = pass && piglit_probe_pixel_rgb(165, 215, pink);
	pass = pass && piglit_probe_pixel_rgb(215, 215, pink);
	pass = pass && piglit_probe_pixel_rgb(265, 215, pink);

	//gequal
	pass = pass && piglit_probe_pixel_rgb(155, 180, white);
	pass = pass && piglit_probe_pixel_rgb(205, 180, black);
	pass = pass && piglit_probe_pixel_rgb(255, 180, green);
	pass = pass && piglit_probe_pixel_rgb(160, 180, pink);
	pass = pass && piglit_probe_pixel_rgb(210, 180, pink);
	pass = pass && piglit_probe_pixel_rgb(260, 180, pink);
	pass = pass && piglit_probe_pixel_rgb(165, 180, pink);
	pass = pass && piglit_probe_pixel_rgb(215, 180, pink);
	pass = pass && piglit_probe_pixel_rgb(265, 180, pink);

	//always
	pass = pass && piglit_probe_pixel_rgb(155, 145, pink);
	pass = pass && piglit_probe_pixel_rgb(205, 145, pink);
	pass = pass && piglit_probe_pixel_rgb(255, 145, pink);
	pass = pass && piglit_probe_pixel_rgb(165, 145, pink);
	pass = pass && piglit_probe_pixel_rgb(215, 145, pink);
	pass = pass && piglit_probe_pixel_rgb(265, 145, pink);

	//never
	pass = pass && piglit_probe_pixel_rgb(155, 110, white);
	pass = pass && piglit_probe_pixel_rgb(205, 110, black);
	pass = pass && piglit_probe_pixel_rgb(255, 110, green);
	pass = pass && piglit_probe_pixel_rgb(165, 110, white);
	pass = pass && piglit_probe_pixel_rgb(215, 110, black);
	pass = pass && piglit_probe_pixel_rgb(265, 110, green);

	//notequal
	pass = pass && piglit_probe_pixel_rgb(155, 75, white);
	pass = pass && piglit_probe_pixel_rgb(205, 75, black);
	pass = pass && piglit_probe_pixel_rgb(255, 75, green);
	pass = pass && piglit_probe_pixel_rgb(165, 75, pink);
	pass = pass && piglit_probe_pixel_rgb(215, 75, pink);
	pass = pass && piglit_probe_pixel_rgb(265, 75, pink);

	//equal
	pass = pass && piglit_probe_pixel_rgb(155, 40, pink);
	pass = pass && piglit_probe_pixel_rgb(205, 40, pink);
	pass = pass && piglit_probe_pixel_rgb(255, 40, pink);
	pass = pass && piglit_probe_pixel_rgb(165, 40, white);
	pass = pass && piglit_probe_pixel_rgb(215, 40, black);
	pass = pass && piglit_probe_pixel_rgb(265, 40, green);


	if(Automatic) {
		piglit_report_result(pass ? PIGLIT_SUCCESS : PIGLIT_FAILURE);
		exit(pass ? 0 : 1);
	}

	glFinish();
	glutSwapBuffers();

	printf(" Left to Right: ALPHA, LUMINANCE, INTENSITY\n Top to Bottom: LESS, LEQUAL, GREATER, GEQUAL, ALWAYS, NEVER, NOTEQUAL, EQUAL\n");

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
