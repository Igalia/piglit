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

#define BOX_SIZE 25

static GLboolean Automatic = GL_FALSE;
static GLuint tex;

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

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
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
	#undef height
	#undef width
}


static void
display(void)
{
	GLboolean pass = GL_TRUE;

	static const struct {
		GLenum compare;
		float r0;
		float r1;
	} tests[] = {
		{ GL_LESS,     2.0, 0.0 },
		{ GL_LEQUAL,   2.0, 0.0 },
		{ GL_GREATER,  2.0, 0.0 },
		{ GL_GEQUAL,   2.0, 0.0 },
		{ GL_ALWAYS,   2.0, 0.0 },
		{ GL_NEVER,    2.0, 0.0 },
		{ GL_NOTEQUAL, 0.5, 0.5 },
		{ GL_EQUAL,    0.5, 0.5 },
	};
	static const GLenum modes[3] = {
		GL_ALPHA, GL_LUMINANCE, GL_INTENSITY
	};
	GLfloat pink[3] = {1.0, 0.0, 1.0};
	GLfloat white[3] = {1.0, 1.0, 1.0};
	GLfloat black[3] = {0.0, 0.0, 0.0};
	GLfloat green[3] = {0.0, 1.0, 0.0};
	unsigned row;
	unsigned col;

	glClear(GL_COLOR_BUFFER_BIT);

	glColor3f(1.0, 0.0, 1.0);

	glBindTexture(GL_TEXTURE_2D, tex);
	for (row = 0; row < ARRAY_SIZE(tests); row++) {
		const int y = 275 - (35 * row);

		for (col = 0; col < 3; col++) {
			const int x = 150 + (col * 50);
			glTexParameteri(GL_TEXTURE_2D,
					GL_TEXTURE_COMPARE_FUNC,
					tests[row].compare);
			glTexParameteri(GL_TEXTURE_2D,
					GL_DEPTH_TEXTURE_MODE,
					modes[col]);

			glBegin(GL_TRIANGLE_STRIP);
			glTexCoord3f(1.0, 0.0, tests[row].r0);
			glVertex2f(x + BOX_SIZE, y);
			glTexCoord3f(1.0, 1.0, tests[row].r0);
			glVertex2f(x + BOX_SIZE, y + BOX_SIZE);
			glTexCoord3f(0.0, 0.0, tests[row].r1);
			glVertex2f(x,            y);
			glTexCoord3f(0.0, 1.0, tests[row].r1);
			glVertex2f(x,            y + BOX_SIZE);
			glEnd();
		}
	}


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
