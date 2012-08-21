/**
 * @file getteximage-simple.c
 *
 * Extremely basic test to check whether image data can be retrieved.
 * 
 * Note that the texture is used in a full frame of rendering before
 * the readback, to ensure that buffer manager related code for uploading
 * texture images is executed before the readback.
 *
 * This used to crash for R300+bufmgr.
 */

#include "piglit-util-gl-common.h"

PIGLIT_GL_TEST_MAIN(
    100 /*window_width*/,
    100 /*window_height*/,
    GLUT_RGBA | GLUT_DOUBLE)

static GLubyte data[4096]; /* 64*16*4 */

static int test_getteximage(void)
{
	GLubyte compare[4096];
	int i;

	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, compare);

	for(i = 0; i < 4096; ++i) {
		if (data[i] != compare[i]) {
			printf("GetTexImage() returns incorrect data in byte %i\n", i);
			printf("    corresponding to (%i,%i) channel %i\n", i / 64, (i / 4) % 16, i % 4);
			printf("    expected: %i\n", data[i]);
			printf("    got: %i\n", compare[i]);
			return 0;
		}
	}

	return 1;
}
	
enum piglit_result
piglit_display(void)
{
	int pass;
	
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0);
	glVertex2f(0, 0);
	glTexCoord2f(1, 0);
	glVertex2f(1, 0);
	glTexCoord2f(1, 1);
	glVertex2f(1, 1);
	glTexCoord2f(0, 1);
	glVertex2f(0, 1);
	glEnd();

	piglit_present_results();

	pass = test_getteximage();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

static void Reshape(int width, int height)
{
	piglit_width = width;
	piglit_height = height;
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, 1.0, 0.0, 1.0, -2.0, 6.0);
	glScalef(1.0, 1.0, -1.0); // flip z-axis
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void piglit_init(int argc, char **argv)
{
	int i;

	glutReshapeFunc(Reshape);

	for(i = 0; i < 4096; ++i)
		data[i] = rand() & 0xff;

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 64, 16, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

	Reshape(piglit_width, piglit_height);
}
