/*
 * Copyright (c) 2010 Marek Olšák <maraeo@gmail.com>
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
 * Tests 3D textures.
 */

#include "piglit-util.h"

int piglit_width = 128, piglit_height = 128;
int piglit_window_mode = GLUT_RGB | GLUT_ALPHA | GLUT_DOUBLE;

enum piglit_result
piglit_display(void)
{
	GLuint tex;
	GLint maxsize;
	char *data;
	int i, j;
	GLboolean pass = GL_TRUE;
	float c1[4] = {0.25, 0.25, 0.25, 1.0};
	float c2[4] = {0.75, 0.75, 0.75, 1.0};

	glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &maxsize);

	/* Create the texture. */
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_3D, tex);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	if (glGetError())
		return PIGLIT_FAILURE;

	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8, maxsize, maxsize, maxsize, 0,
		     GL_RGBA, GL_UNSIGNED_BYTE, 0);
	if (glGetError() == GL_OUT_OF_MEMORY) {
		glDeleteTextures(1, &tex);
		printf("Got GL_OUT_OF_MEMORY.\n");
		return PIGLIT_SUCCESS;
	}

	/* Set its pixels, slice by slice. */
	data = malloc(maxsize*maxsize*4);
	for (j = 0; j < maxsize; j++)
		for (i = 0; i < maxsize; i++) {
			int a = (j*maxsize+i)*4;
			data[a+0] = (i*255)/(maxsize-1);
			data[a+1] = (i*255)/(maxsize-1);
			data[a+2] = (i*255)/(maxsize-1);
			data[a+3] = (i*255)/(maxsize-1);
		}

	for (i = 0; i < maxsize; i++) {
		glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, i, maxsize, maxsize, 1,
				GL_RGBA, GL_UNSIGNED_BYTE, data);
	}
	free(data);

	/* Now try basic rendering. */
	glEnable(GL_TEXTURE_3D);
	glBegin(GL_QUADS);
	glTexCoord3f(0, 0, 1);
	glVertex2f(0, 0);
	glTexCoord3f(0, 1, 1);
	glVertex2f(0, piglit_height);
	glTexCoord3f(1, 1, 1);
	glVertex2f(piglit_width, piglit_height);
	glTexCoord3f(1, 0, 1);
	glVertex2f(piglit_width, 0);
	glEnd();
	glDeleteTextures(1, &tex);

	pass = piglit_probe_pixel_rgb(piglit_width * 1 / 4,
				      piglit_height * 1 / 4, c1) && pass;
	pass = piglit_probe_pixel_rgb(piglit_width * 3 / 4,
				      piglit_height * 1 / 4, c2) && pass;
	pass = piglit_probe_pixel_rgb(piglit_width * 1 / 4,
				      piglit_height * 3 / 4, c1) && pass;
	pass = piglit_probe_pixel_rgb(piglit_width * 3 / 4,
				      piglit_height * 3 / 4, c2) && pass;
	glutSwapBuffers();

	return pass ? PIGLIT_SUCCESS : PIGLIT_FAILURE;
}


static void Reshape(int width, int height)
{
	glViewport(0, 0, piglit_width, piglit_height);
	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);
}


void piglit_init(int argc, char **argv)
{
	if (!GLEW_VERSION_1_2) {
		printf("Requires OpenGL 1.2\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	glutReshapeFunc(Reshape);
	glDisable(GL_DITHER);
	Reshape(piglit_width, piglit_height);
}

