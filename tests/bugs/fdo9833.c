/**
 * Test case from fd.o bug #9833.
 * https://bugs.freedesktop.org/show_bug.cgi?id=9833
 */

#include <stdlib.h>
#include <stdio.h>
#if defined(__APPLE__)
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

static int Automatic = 0;

static void display(void)
{
	static int goterrors = 0;
	static int frame = 0;
	GLuint error;

	frame++;
	glClear(GL_COLOR_BUFFER_BIT);

	glPushAttrib(GL_TEXTURE_BIT);
	while ( (error = glGetError()) != GL_NO_ERROR) {
		fprintf(stderr, "OpenGL error 0x%0x occured after glPushAttrib!\n", error);
		goterrors++;
	}


	glPopAttrib();
	while ( (error = glGetError()) != GL_NO_ERROR) {
		fprintf(stderr, "OpenGL error 0x%0x occured after glPopAttrib!\n", error);
		goterrors++;
	}

	if (Automatic && frame > 2) {
		printf("PIGLIT: {'result': '%s' }\n", goterrors ? "fail" : "pass");
		exit(0);
	}

	glutPostRedisplay();
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	if (argc == 2 && !strcmp(argv[1], "-auto"))
		Automatic = 1;
	glutCreateWindow("fdo9833");
	glutDisplayFunc(display);
	glutMainLoop();
	return 0;
}
