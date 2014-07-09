/*
 * Copyright (c) 2011 VMware, Inc.
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
 * Test that the VBO's buffers are unmapped before drawing.
 * In particular, test the VMware svga Gallium driver.
 *
 * Brian Paul
 * 18 Feb 2011
 */


#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
   GLubyte image[8][8][4];
   GLuint tex;

   memset(image, 0x80, sizeof(image));

   glGenTextures(1, &tex);
   glBindTexture(GL_TEXTURE_2D, tex);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 8, 8, 0,
                GL_RGBA, GL_UNSIGNED_BYTE, image);

   glClear(GL_COLOR_BUFFER_BIT);

   glColor3f(0, 1, 0);

   glBegin(GL_POINTS);
   glVertex2f(-1, 0);
   glEnd();
   glBegin(GL_POINTS);
   glVertex2f(0, 0);
   glEnd();
   glBegin(GL_POINTS);
   glVertex2f(1, 0);
   glEnd();

   glFlush();

   /* This call may cause a the internal VBO to be mapped and the
    * glTexImage2D call which may do a DMA transfer may hit a path
    * where we expect all VBOs to be unmapped.
    */
   glColor3f(1, 0, 0);

   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 8, 8, 0,
                GL_RGBA, GL_UNSIGNED_BYTE, image);

   piglit_present_results();

   return PIGLIT_PASS;
}


void
piglit_init(int argc, char **argv)
{
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glOrtho(-2, 2, -2, 2, -1, 1);
}
