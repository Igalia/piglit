/* Copyright © 2011 Intel Corporation
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
 * \file drawbuffer-modes.c
 * Verify the functionality of glDrawBuffer() function with different color
 * buffer modes in default framebuffer
 *
 * This test works by calling glDrawBuffer function for each color buffer mode
 * and testing the buffer's color value against expected value. All the calls
 * should ensure no error.
 *
 * This test case also verifies the fix for Bug 44153:
 * https://bugs.freedesktop.org/show_bug.cgi?id=44153
 *
 * \Author Yi Sun <yi.sun@intel.com>, Anuj Phogat <anuj.phogat@gmail.com>
 */
#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;
	config.requires_displayed_window = true;

PIGLIT_GL_TEST_CONFIG_END

float color[7][4] = {
	{ 0.1, 0.2, 0.3, 1.0 },
	{ 0.2, 0.3, 0.4, 1.0 },
	{ 0.3, 0.4, 0.5, 1.0 },
	{ 0.4, 0.5, 0.6, 1.0 },
	{ 0.5, 0.6, 0.7, 1.0 },
	{ 0.6, 0.7, 0.8, 1.0 },
	{ 1.0, 1.0, 1.0, 1.0 } };

int bufferlist[] = {
	GL_FRONT_AND_BACK,
	GL_BACK, GL_FRONT,
	GL_LEFT,
	GL_BACK_LEFT,
	GL_FRONT_LEFT,
	GL_NONE };

enum piglit_result
piglit_display(void)
{
	bool pass = true;
	bool pixel_probe = true;
	int i;
	GLfloat probe[4];
	for ( i = 0; i < ARRAY_SIZE(bufferlist); i++) {
		glDrawBuffer(bufferlist[i]);
		glClear(GL_COLOR_BUFFER_BIT);
		glColor4fv(color[i]);
		piglit_draw_rect(20, 20, 50, 50);
		pass = piglit_check_gl_error(GL_NO_ERROR)
		       && pass;
		switch (bufferlist[i]) {
		case GL_FRONT_AND_BACK:
			glReadBuffer(GL_BACK_LEFT);
			pass = piglit_probe_rect_rgba(20, 20, 50, 50,
						      color[i])
			       && pass;
			glReadBuffer(GL_FRONT_LEFT);
			pass = pass &&
			       piglit_probe_rect_rgba(20, 20, 50, 50,
						      color[i]);
			break;
		case GL_LEFT:
			glReadBuffer(GL_BACK_LEFT);
			pass = piglit_probe_rect_rgba(20, 20, 50, 50,
						      color[i])
			       && pass;
			glReadBuffer(GL_FRONT_LEFT);
			pass = piglit_probe_rect_rgba(20, 20, 50, 50,
						      color[i])
			       && pass;
			break;
		case GL_BACK:
		case GL_BACK_LEFT:
			glReadBuffer(bufferlist[i]);
			pass = piglit_probe_rect_rgba(20, 20, 50, 50,
						      color[i])
			       && pass;
			/* This should not modify GL_FRONT_LEFT buffer */
			glReadBuffer(GL_FRONT_LEFT);
			pixel_probe = piglit_probe_pixel_rgba_silent(25, 25,
								     color[i],
								     probe);
			pass = !pixel_probe && pass;
			if(pixel_probe)
				printf("glDrawBuffer(GL_BACK) modifies"
				       " GL_FRONT_LEFT buffer\n");
			break;
		case GL_FRONT:
		case GL_FRONT_LEFT:
			glReadBuffer(bufferlist[i]);
			pass = piglit_probe_rect_rgba(20, 20, 50, 50,
						      color[i])
			       && pass;
			/* This should not modify GL_BACK_LEFT buffer */
			glReadBuffer(GL_BACK_LEFT);
			pixel_probe = piglit_probe_pixel_rgba_silent(25, 25,
								     color[i],
								     probe);
			pass = !pixel_probe && pass;
			if(pixel_probe)
				printf("glDrawBuffer(GL_FRONT) modifies"
				       " GL_BACK_LEFT buffer\n");
			break;
		case GL_NONE:
			/* Drawing to a buffer GL_NONE should not
			 * modify any existing buffers
			 */
			glReadBuffer(GL_BACK_LEFT);
			pixel_probe = piglit_probe_pixel_rgba_silent(25, 25,
								     color[i],
								     probe);
			pass = !pixel_probe && pass;
			if(pixel_probe)
				printf("glDrawBuffer(GL_NONE) modifies"
				       " GL_BACK_LEFT buffer\n");
			glReadBuffer(GL_FRONT_LEFT);
			pixel_probe = piglit_probe_pixel_rgba_silent(25, 25,
								     color[i],
								     probe);
			pass = !pixel_probe && pass;
			if (pixel_probe)
				printf("glDrawBuffer(GL_NONE) modifies"
				       " GL_FRONT_LEFT buffer\n");
			break;
		}
		pass = piglit_check_gl_error(GL_NO_ERROR)
		       && pass;
	}
	return (pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

void piglit_init(int argc, char **argv)
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, piglit_width, 0, piglit_height, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glClearColor(0.2, 0.2, 0.2, 1.0);
}
