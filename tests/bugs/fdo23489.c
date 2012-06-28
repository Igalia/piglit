/*
 * Test case from fdo bug #23489
 * http://bugs.freedesktop.org/show_bug.cgi?id=23489
 */

#include "piglit-util-gl-common.h"

PIGLIT_GL_TEST_MAIN(
    250 /*window_width*/,
    250 /*window_height*/,
    GLUT_SINGLE | GLUT_RGB)

enum piglit_result
piglit_display(void)
{
	int i;

	for (i = 0; i < 100; i++) {
		glBegin(GL_LINES);
		glEnd();
	}
	glFlush();

	return PIGLIT_PASS;
}

void
piglit_init(int argc, char **argv)
{
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);
}
