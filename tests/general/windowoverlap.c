/*
 * Copyright (c) The Piglit project 2008
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.  IN NO EVENT SHALL
 * VA LINUX SYSTEM, IBM AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/**
 * @file
 * Test whether rendering does not bleed into areas outside the window.
 * This is done by creating a subwindow and verifying that rendering in
 * the main window vs. in the sub window is clipped correctly.
 *
 * This test was prompted by http://bugs.freedesktop.org/show_bug.cgi?id=16123
 */

#include "piglit-util-gl.h"

static const int MainWidth = 128, MainHeight = 128;
static const int SubX = 32, SubY = 32;
static const int SubWidth = 64, SubHeight = 64;

static int Automatic = 0;
static int MainWindow;
static int SubWindow;


static int verify(float mainr, float maing, float mainb, float subr, float subg, float subb, const char* testname)
{
	float mainf[128][128][3];
	float sub[64][64][3];
	int x, y;

	glutSetWindow(MainWindow);
	glReadPixels(0, 0, 128, 128, GL_RGB, GL_FLOAT, mainf);

	glutSetWindow(SubWindow);
	glReadPixels(0, 0, 64, 64, GL_RGB, GL_FLOAT, sub);

	for(x = 0; x < 128; ++x) {
		for(y = 0; y < 128; ++y) {
			float delta;

			if (x >= 32 && x < 96 && y >= 32 && y < 96)
				continue;

			delta = fabs(mainr - mainf[y][x][0]) + fabs(maing - mainf[y][x][1]) + fabs(mainb - mainf[y][x][2]);
			if (delta > 0.01) {
				printf("Test %s: Fail at main window pixel %i,%i\n", testname, x, y);
				printf("  Expected: %5.3f %5.2f %5.3f\n", mainr, maing, mainb);
				printf("  Actual:   %5.3f %5.2f %5.3f\n", mainf[y][x][0], mainf[y][x][1], mainf[y][x][2]);
				return 0;
			}
		}
	}

	for(x = 0; x < 64; ++x) {
		for(y = 0; y < 64; ++y) {
			float delta;

			delta = fabs(subr - sub[y][x][0]) + fabs(subg - sub[y][x][1]) + fabs(subb - sub[y][x][2]);
			if (delta > 0.01) {
				printf("Test %s: Fail at sub window pixel %i,%i\n", testname, x, y);
				printf("  Expected: %5.3f %5.2f %5.3f\n", subr, subg, subb);
				printf("  Actual:   %5.3f %5.2f %5.3f\n", sub[y][x][0], sub[y][x][1], sub[y][x][2]);
				return 0;
			}
		}
	}

	return 1;
}

static void test(void)
{
	int success = 1;

	glutSetWindow(MainWindow);
	glClearColor(1, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);
	glFinish();

	glutSetWindow(SubWindow);
	glClearColor(0, 1, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);
	glFinish();

	success = success && verify(1, 0, 0, 0, 1, 0, "initial clear");

	glutSetWindow(MainWindow);
	glClearColor(0, 0, 1, 0);
	glClear(GL_COLOR_BUFFER_BIT);
	glFinish();

	success = success && verify(0, 0, 1, 0, 1, 0, "re-clear main window");

	glutSetWindow(SubWindow);
	glColor3f(1, 1, 0);
	glBegin(GL_QUADS);
	glVertex2f(-1, -1);
	glVertex2f( 2, -1);
	glVertex2f( 2,  2);
	glVertex2f(-1,  2);
	glEnd();
	glFinish();

	success = success && verify(0, 0, 1, 1, 1, 0, "render in sub window");

	glutSetWindow(MainWindow);
	glColor3f(0, 1, 1);
	glBegin(GL_QUADS);
	glVertex2f(-1, -1);
	glVertex2f( 2, -1);
	glVertex2f( 2,  2);
	glVertex2f(-1,  2);
	glEnd();
	glFinish();

	success = success && verify(0, 1, 1, 1, 1, 0, "render in main window");

	if (Automatic)
		piglit_report_result(success ? PIGLIT_PASS : PIGLIT_FAIL);
}

static void RedisplayMain(void)
{
	test();
}

static void RedisplaySub(void)
{
	test();
}


static void Reshape(int width, int height)
{
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}


static void Key(unsigned char key, int x, int y)
{
	(void) x;
	(void) y;
	switch (key) {
	case 27:
		exit(0);
		break;
	}
	glutPostRedisplay();
}

int main(int argc, char *argv[])
{
	glutInit(&argc, argv);
	if (argc == 2 && !strcmp(argv[1], "-auto"))
		Automatic = 1;
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(MainWidth, MainHeight);
	glutInitDisplayMode(PIGLIT_GL_VISUAL_RGB);
	glutCreateWindow(argv[0]);
	piglit_dispatch_default_init(PIGLIT_DISPATCH_GL);
	glutReshapeFunc(Reshape);
	glutDisplayFunc(RedisplayMain);
	if (!Automatic)
		glutKeyboardFunc(Key);

	MainWindow = glutGetWindow();
	SubWindow = glutCreateSubWindow(MainWindow, SubX, SubY, SubWidth, SubHeight);
	glutReshapeFunc(Reshape);
	glutDisplayFunc(RedisplaySub);
	glutMainLoop();
	return 0;
}

