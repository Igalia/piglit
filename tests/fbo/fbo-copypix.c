/*
 * Copyright © 2010 Intel Corporation
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
 * Authors:
 *    Eric Anholt <eric@anholt.net>
 *    Brian Paul
 */

/** @file fbo-readdrawpix.c
 *
 * Test glCopyPixels between FBO and window.
 * See also fbo-blit.c and fbo-readdrawpix.c
 */

#include "piglit-util.h"

int piglit_width = 150;
int piglit_height = 150;
int piglit_window_mode = GLUT_RGB | GLUT_DOUBLE;
#define PAD 10
#define SIZE 20

/* size of texture/renderbuffer (power of two) */
#define FBO_SIZE 64


static GLuint
make_fbo(int w, int h)
{
	GLuint tex;
	GLuint fb;
 	GLenum status;

	glGenFramebuffersEXT(1, &fb);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
		     w, h, 0,
		     GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
				  GL_COLOR_ATTACHMENT0_EXT,
				  GL_TEXTURE_2D,
				  tex,
				  0);
	assert(glGetError() == 0);

	status = glCheckFramebufferStatusEXT (GL_FRAMEBUFFER_EXT);
	if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
		fprintf(stderr, "fbo incomplete (status = 0x%04x)\n", status);
		piglit_report_result(PIGLIT_SKIP);
	}

	return fb;
}

static void
draw_color_rect(int x, int y, int w, int h)
{
	int x1 = x;
	int x2 = x + w / 2;
	int y1 = y;
	int y2 = y + h / 2;

	glColor4f(1.0, 0.0, 0.0, 0.0);
	piglit_draw_rect(x1, y1, w / 2, h / 2);
	glColor4f(0.0, 1.0, 0.0, 0.0);
	piglit_draw_rect(x2, y1, w / 2, h / 2);
	glColor4f(0.0, 0.0, 1.0, 0.0);
	piglit_draw_rect(x1, y2, w / 2, h / 2);
	glColor4f(1.0, 1.0, 1.0, 0.0);
	piglit_draw_rect(x2, y2, w / 2, h / 2);
}

static GLboolean
verify_color_rect(int start_x, int start_y, int w, int h)
{
	float red[] =   {1, 0, 0, 0};
	float green[] = {0, 1, 0, 0};
	float blue[] =  {0, 0, 1, 0};
	float white[] = {1, 1, 1, 0};
	int x, y;

	for (y = 0; y < h; y++) {
		for (x = 0; x < w; x++) {
			float *expected;

			if ((y < h / 2) && (x < w / 2))
				expected = red;
			else if (y < h / 2)
				expected = green;
			else if (x < w / 2)
				expected = blue;
			else
				expected = white;

			if (!piglit_probe_pixel_rgb(start_x + x, start_y + y,
						    expected))
				return GL_FALSE;
		}
	}

	return GL_TRUE;
}


static void
copy(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1,
     GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1)
{
	GLsizei srcW = srcX1 - srcX0, srcH = srcY1 - srcY0;
	GLsizei dstW = dstX1 - dstX0, dstH = dstY1 - dstY0;
	glPixelZoom((float) dstW / (float) srcW,
		    (float) dstH / (float) srcH);
	glWindowPos2i(dstX0, dstY0);
	glCopyPixels(srcX0, srcY0, srcW, srcH, GL_COLOR);
}


static GLboolean
run_test(void)
{
	GLboolean pass = GL_TRUE;
	GLuint fbo;
	int fbo_width = FBO_SIZE;
	int fbo_height = FBO_SIZE;
	int x0 = PAD;
	int y0 = PAD;
	int y1 = PAD * 2 + SIZE;
	int y2 = PAD * 3 + SIZE * 2;

	glViewport(0, 0, piglit_width, piglit_height);
	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	glClearColor(0.5, 0.5, 0.5, 0.5);
	glClear(GL_COLOR_BUFFER_BIT);

	/* Draw the color rect in the window system window */
	draw_color_rect(x0, y0, SIZE, SIZE);

	fbo = make_fbo(fbo_width, fbo_height);

	glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, fbo);
	glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, 0);
	glViewport(0, 0, fbo_width, fbo_height);
	piglit_ortho_projection(fbo_width, fbo_height, GL_FALSE);
	glClearColor(1.0, 0.0, 1.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);

	/* Draw the color rect in the FBO */
	draw_color_rect(x0, y0, SIZE, SIZE);

	/* Now that we have correct samples, blit things around.
	 * FBO(bottom) -> WIN(middle)
	 */
	glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, 0);
	glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, fbo);
 	copy(x0, y0, x0 + SIZE, y0 + SIZE,
 	     x0, y1, x0 + SIZE, y1 + SIZE);

	/* WIN(bottom) -> FBO(middle) */
	glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, fbo);
	glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, 0);
 	copy(x0, y0, x0 + SIZE, y0 + SIZE,
 	     x0, y1, x0 + SIZE, y1 + SIZE);

	/* FBO(middle) -> WIN(top) back to verify WIN -> FBO */
	glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, 0);
	glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, fbo);
 	copy(x0, y1, x0 + SIZE, y1 + SIZE,
 	     x0, y2, x0 + SIZE, y2 + SIZE);

	glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, 0);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

	pass = verify_color_rect(PAD, y0, SIZE, SIZE) && pass;
	pass = verify_color_rect(PAD, y1, SIZE, SIZE) && pass;
	pass = verify_color_rect(PAD, y2, SIZE, SIZE) && pass;

	glutSwapBuffers();

	return pass;
}


enum piglit_result
piglit_display(void)
{
	GLboolean pass = run_test();

	return pass ? PIGLIT_SUCCESS : PIGLIT_FAILURE;
}


void
piglit_init(int argc, char **argv)
{
	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	piglit_require_extension("GL_EXT_framebuffer_object");
}
