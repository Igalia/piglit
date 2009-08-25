/**
 * @file crash-texparameter-before-teximage.c
 *
 * Test case for "crash before first TexImage"; this happened in the R300
 * driver before Mesa commit c1fb448ce8dd98f8e5fd5a39707f96cc14535bd4
 *
 * This bug was originally triggered by Glest.
 */

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#if defined(__APPLE__)
#include <GLUT/glut.h>
#else
#include "GL/glut.h"
#endif

#include "piglit-util.h"


static int Width = 100, Height = 100;

static void Display(void)
{
	piglit_report_result(PIGLIT_SUCCESS);
}

static void init(void)
{
	glViewport(0, 0, Width, Height);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	if (glutExtensionSupported("GL_ARB_shadow"))
		glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE_ARB, GL_ALPHA);
	if (glutExtensionSupported("GL_ARB_shadow_ambient"))
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FAIL_VALUE_ARB, 0.1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_LOD, -4);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, 4);
}


int main(int argc, char**argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(Width, Height);
	glutCreateWindow(argv[0]);
	glutDisplayFunc(Display);
	init();
	glutMainLoop();
	return 0;
}
