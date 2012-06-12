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
 *    Marek Olšák <maraeo@gmail.com>
 */

/** @file fbo-blit-d24s8.c
 *
 * Tests EXT_framebuffer_blit with various combinations of window system and
 * FBO objects.  Because FBOs are generally stored upside down relative to
 * window system frambuffers, this could catch flipping failures in blit paths.
 * The FBOs in this test are of the D24S8 format.
 *
 * See also fbo-blit.c
 */

#include "piglit-util.h"

PIGLIT_GL_TEST_MAIN(
    150 /*window_width*/,
    150 /*window_height*/,
    GLUT_RGBA | GLUT_DOUBLE | GLUT_STENCIL | GLUT_DEPTH)

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
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8_EXT,
		     w, h, 0,
		     GL_DEPTH_STENCIL_EXT, GL_UNSIGNED_INT_24_8_EXT, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
				  GL_DEPTH_ATTACHMENT_EXT,
				  GL_TEXTURE_2D,
				  tex,
				  0);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
				  GL_STENCIL_ATTACHMENT_EXT,
				  GL_TEXTURE_2D,
				  tex,
				  0);
				  
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	assert(glGetError() == 0);

	status = glCheckFramebufferStatusEXT (GL_FRAMEBUFFER_EXT);
	if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
		fprintf(stderr, "fbo incomplete (status = 0x%04x)\n", status);
		piglit_report_result(PIGLIT_SKIP);
	}

	return fb;
}

static void
draw_depth_rect(int x, int y, int w, int h)
{
	int x1 = x;
	int x2 = x + w / 2;
	int y1 = y;
	int y2 = y + h / 2;
	
	glDepthRange(0, 0);
	piglit_draw_rect(x1, y1, w / 2, h / 2);
	glDepthRange(0.3, 0.3);
	piglit_draw_rect(x2, y1, w / 2, h / 2);
	glDepthRange(0.6, 0.6);
	piglit_draw_rect(x1, y2, w / 2, h / 2);
	glDepthRange(1, 1);
	piglit_draw_rect(x2, y2, w / 2, h / 2);
	glDepthRange(0, 1);
}

static GLboolean
verify_depth_rect(int start_x, int start_y, int w, int h)
{
	float zero  = 0;
	float darkgrey = 0.3;
	float grey  = 0.6;
	float one = 1;

	if (!piglit_probe_rect_depth(start_x, start_y, w / 2, h / 2, zero))
		return GL_FALSE;
	if (!piglit_probe_rect_depth(start_x + w/2, start_y, w/2, h/2, darkgrey))
		return GL_FALSE;
	if (!piglit_probe_rect_depth(start_x, start_y + h/2, w/2, h/2, grey))
		return GL_FALSE;
	if (!piglit_probe_rect_depth(start_x + w/2, start_y + h/2, w/2, h/2, one))
		return GL_FALSE;

	return GL_TRUE;
}


static void
copy(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1,
     GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1,
     GLenum mask)
{
	glBlitFramebufferEXT(srcX0, srcY0, srcX1, srcY1,
			     dstX0, dstY0, dstX1, dstY1,
			     mask, GL_NEAREST);
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
	GLenum err;
	GLint win_depth_bits, fbo_depth_bits, win_stencil_bits, fbo_stencil_bits;

	glViewport(0, 0, piglit_width, piglit_height);
	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_ALWAYS);
	glDepthMask(GL_TRUE);

	glClearColor(0.5, 0.5, 0.5, 0.5);
	glClearDepth(0.12345);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	/* Draw the color rect in the window system window */
	draw_depth_rect(x0, y0, SIZE, SIZE);

	fbo = make_fbo(fbo_width, fbo_height);

	/* query depth/stencil sizes */
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	glGetIntegerv(GL_DEPTH_BITS, &win_depth_bits);
	glGetIntegerv(GL_STENCIL_BITS, &win_stencil_bits);

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
	glGetIntegerv(GL_DEPTH_BITS, &fbo_depth_bits);
	glGetIntegerv(GL_STENCIL_BITS, &fbo_stencil_bits);

	if (win_depth_bits != fbo_depth_bits ||
	    win_stencil_bits != fbo_stencil_bits) {
		/* The spec doesn't allow blitting between depth/blitting surfaces
		 * of different formats.
		 */
		piglit_report_result(PIGLIT_SKIP);
	}

	glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, fbo);
	glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, 0);
	glViewport(0, 0, fbo_width, fbo_height);
	piglit_ortho_projection(fbo_width, fbo_height, GL_FALSE);
	glClearDepth(0.54321);
	glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	/* Draw the color rect in the FBO */
	draw_depth_rect(x0, y0, SIZE, SIZE);

	/* Now that we have correct samples, blit things around.
	 * FBO(bottom) -> WIN(middle)
	 *
	 * Also blit with stencil to exercise this path.
	 * Not that we need it for this test.
	 */
	glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, 0);
	glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, fbo);
	copy(x0, y0, x0 + SIZE, y0 + SIZE,
	     x0, y1, x0 + SIZE, y1 + SIZE,
	     GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	err = glGetError();
	if (err != GL_NO_ERROR) {
		printf("Unexpected GL error state 0x%04x\n", err);
		piglit_report_result(PIGLIT_FAIL);
	}

	/* WIN(bottom) -> FBO(middle) */
	glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, fbo);
	glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, 0);
	copy(x0, y0, x0 + SIZE, y0 + SIZE,
	     x0, y1, x0 + SIZE, y1 + SIZE,
	     GL_DEPTH_BUFFER_BIT);

	/* FBO(middle) -> WIN(top) back to verify WIN -> FBO */
	glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, 0);
	glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, fbo);
	copy(x0, y1, x0 + SIZE, y1 + SIZE,
	     x0, y2, x0 + SIZE, y2 + SIZE,
	     GL_DEPTH_BUFFER_BIT);

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	assert(glGetError() == 0);

	printf("Verify 1\n");
	pass = verify_depth_rect(PAD, y0, SIZE, SIZE) && pass;
	printf("Verify 2\n");
	pass = verify_depth_rect(PAD, y1, SIZE, SIZE) && pass;
	printf("Verify 3\n");
	pass = verify_depth_rect(PAD, y2, SIZE, SIZE) && pass;
	printf("Verify 4 (FBO)\n");
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
	pass = verify_depth_rect(PAD, y0, SIZE, SIZE) && pass;
	printf("Verify 5 (FBO)\n");
	pass = verify_depth_rect(PAD, y1, SIZE, SIZE) && pass;
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	assert(glGetError() == 0);

	glutSwapBuffers();

	return pass;
}


enum piglit_result
piglit_display(void)
{
	GLboolean pass = run_test();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


void
piglit_init(int argc, char **argv)
{
	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	piglit_require_extension("GL_EXT_framebuffer_object");
	piglit_require_extension("GL_EXT_framebuffer_blit");
	piglit_require_extension("GL_EXT_packed_depth_stencil");
}
