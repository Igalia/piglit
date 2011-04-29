/*
 * Test case from fdo bug #23489
 * http://bugs.freedesktop.org/show_bug.cgi?id=23489
 */

#include "piglit-util.h"

static int Automatic = 0;

static void display(void)
{
	int i;

	for (i = 0; i < 100; i++) {
		glBegin(GL_LINES);
		glEnd();
	}
	glFlush();

	if (Automatic) {
		piglit_report_result(PIGLIT_PASS);
	}
}

static void init(void)
{
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	if (argc == 2 && !strcmp(argv[1], "-auto"))
		Automatic = 1;
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowSize(250, 250);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("fdo23489");
	glutKeyboardFunc(piglit_escape_exit_key);
	init();
	glutDisplayFunc(display);
	glutMainLoop();
	return 0;
}

