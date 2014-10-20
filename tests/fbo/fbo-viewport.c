/*
 * Copyright (c) 2012 VMware, Inc.
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
 */

/**
 * Test glViewport w/ FBOs.
 * In Mesa, on-screen windows and user-created FBOs are stored differently
 * (inverted).  Make sure viewports are handled properly.
 * Draw a test pattern (with many viewports) into the window, then draw the
 * same thing into an FBO.  Compare the images.  They should be the same.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 500;
	config.window_height = 500;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

/**
 * Draw some simple quads in a bunch of viewports which tile the window.
 * Note that viewports extend beyond the edges of the window too.
 */
static void
draw_test_image(void)
{
	int vx, vy, vw = 200, vh = 200;

	glClear(GL_COLOR_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(-1, 1, -1, 1, 3, 9.5);

	/* Draw some quads at an odd rotation.
	 * Note that we want near/far frustum clipping.
	 */
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef(0, 1, -6.20);
	glRotatef(-60, 1, 0, 0);
	glRotatef(30, 0, 0, 1);
	glScalef(3.5, 3.5, 3.5);

	/* loop over viewports */
	for (vy = -50; vy < piglit_height; vy += vh+10) {
		for (vx = -30; vx < piglit_width; vx += vw+10) {
			glViewport(vx, vy, vw, vh);

			glBegin(GL_QUADS);

			glColor3f(1, 0, 0);
			glVertex2f(-1, -1);
			glVertex2f( 0, -1);
			glVertex2f( 0,  0);
			glVertex2f(-1,  0);

			glColor3f(0, 1, 0);
			glVertex2f( 0, -1);
			glVertex2f( 1, -1);
			glVertex2f( 1,  0);
			glVertex2f( 0,  0);

			glColor3f(0, 0, 1);
			glVertex2f(-1,  0);
			glVertex2f( 0,  0);
			glVertex2f( 0,  1);
			glVertex2f(-1,  1);

			glColor3f(1, 1, 1);
			glVertex2f( 0,  0);
			glVertex2f( 1,  0);
			glVertex2f( 1,  1);
			glVertex2f( 0,  1);

			glEnd();
		}
	}

	glPopMatrix();
}


enum piglit_result
piglit_display(void)
{
	GLubyte *win_image, *fbo_image;
	GLuint fbo, rb;
	bool pass = true;

	win_image = (GLubyte *) malloc(piglit_width * piglit_height * 3);
	fbo_image = (GLubyte *) malloc(piglit_width * piglit_height * 3);

	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glGenRenderbuffers(1, &rb);
	glBindRenderbuffer(GL_RENDERBUFFER, rb);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA,
			      piglit_width, piglit_height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
				  GL_RENDERBUFFER, rb);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	assert(glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT) ==
			 GL_FRAMEBUFFER_COMPLETE_EXT);

	/* draw reference image in the window */
	glBindFramebuffer(GL_FRAMEBUFFER, piglit_winsys_fbo);
	draw_test_image();
	glReadPixels(0, 0, piglit_width, piglit_height,
		     GL_RGB, GL_UNSIGNED_BYTE, win_image);

	/* draw test image in fbo */
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	draw_test_image();
	glReadPixels(0, 0, piglit_width, piglit_height,
		     GL_RGB, GL_UNSIGNED_BYTE, fbo_image);

	/* compare images */
	if (memcmp(win_image, fbo_image, piglit_width * piglit_height * 3)) {
#if 0 /* helpful debug code */
		int i, k;
		for (i = k = 0; i < piglit_width * piglit_height * 3; i++) {
			if (win_image[i] != fbo_image[i] && k++ < 40)
				printf("%d: %d vs. %d\n",
				       i, win_image[i], fbo_image[i]);
		}
#endif
		printf("Image comparison failed!\n");
		pass = false;
	}
	else if (!piglit_automatic) {
		printf("Image comparison passed.\n");
	}

	glBindFramebuffer(GL_FRAMEBUFFER, piglit_winsys_fbo);

#if 0	/* for debug/compare (alternate diplaying Window vs. FBO image) */
	{
		int i;
		glWindowPos2i(0,0);
		for (i = 0; i < 10; i++) {
			GLubyte *image = (i & 1) ? fbo_image : win_image;
			printf("Showing %s image\n", (i & 1) ? "FBO" : "window");
			glDrawPixels(piglit_width, piglit_height,
				     GL_RGB, GL_UNSIGNED_BYTE, image);
			piglit_present_results();
			sleep(1);
		}
	}
#endif

	piglit_present_results();

	glDeleteRenderbuffers(1, &rb);
	glDeleteFramebuffers(1, &fbo);
	free(win_image);
	free(fbo_image);

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_ARB_framebuffer_object");
	glClearColor(0.2, 0.2, 0.2, 0.0);
}
