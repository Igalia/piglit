/*
 * Copyright © 2010 Marek Olšák <maraeo@gmail.com>
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 */

/** @file fbo-maxsize.c
 *
 * Tests that rendering to a texture of maximum size works.
 */

#include "piglit-util.h"

int piglit_width = 256;
int piglit_height = 256;
int piglit_window_mode = GLUT_DOUBLE | GLUT_RGB;

static int create_fbo(void)
{
	GLuint tex, fb;
	GLint maxsize;
	GLenum status;

	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxsize);

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, maxsize, maxsize,
		     0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	glGenFramebuffersEXT(1, &fb);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
				  GL_COLOR_ATTACHMENT0_EXT,
				  GL_TEXTURE_2D,
				  tex, 0);
	assert(glGetError() == 0);

	status = glCheckFramebufferStatusEXT (GL_FRAMEBUFFER_EXT);
	if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
		fprintf(stderr, "FBO incomplete\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	glViewport(0, 0, maxsize, maxsize);
	piglit_ortho_projection(maxsize, maxsize, GL_FALSE);

	glBegin(GL_QUADS);
	glColor3f(0, 0, 0);
	glVertex2f(0, 0);
	glColor3f(0, 1, 1);
	glVertex2f(0, maxsize);
	glColor3f(1, 1, 1);
	glVertex2f(maxsize, maxsize);
	glColor3f(1, 0, 0);
	glVertex2f(maxsize, 0);
	glEnd();

	glDeleteFramebuffersEXT(1, &fb);

	return tex;
}

enum piglit_result
piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	GLuint tex;
	int x1 = piglit_width / 4;
	int x2 = (piglit_width / 4) * 3;
	int y1 = piglit_height / 4;
	int y2 = (piglit_height / 4) * 3;
	float c1[3] = {0.25, 0.25, 0.25};
	float c2[3] = {0.75, 0.25, 0.25};
	float c3[3] = {0.25, 0.75, 0.75};
	float c4[3] = {0.75, 0.75, 0.75};

	glClearColor(0.5, 0.5, 0.5, 0.5);
	glClear(GL_COLOR_BUFFER_BIT);

	tex = create_fbo();
	glViewport(0, 0, piglit_width, piglit_height);
	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

	glEnable(GL_TEXTURE_2D);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	piglit_draw_rect_tex(0, 0, piglit_width, piglit_height,
			     0, 0, 1, 1);

	pass &= piglit_probe_pixel_rgb(x1, y1, c1);
	pass &= piglit_probe_pixel_rgb(x2, y1, c2);
	pass &= piglit_probe_pixel_rgb(x1, y2, c3);
	pass &= piglit_probe_pixel_rgb(x2, y2, c4);

	glDeleteTextures(1, &tex);
	glDisable(GL_TEXTURE_2D);

	glutSwapBuffers();

	return pass ? PIGLIT_SUCCESS : PIGLIT_FAILURE;
}

void piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_EXT_framebuffer_object");
}

