/*
 * Copyright 2016 VMware, Inc.
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
 * Test glRasterPos, glBitmap and triangle rendering to be sure the
 * bitmap and triangle color are handled correctly.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;
PIGLIT_GL_TEST_CONFIG_END


enum piglit_result
piglit_display(void)
{
   static const GLfloat green[3] = { 0, 1, 0 };
   static const GLfloat blue[3] = { 0, 0, 1 };
   static GLubyte bitmap[8] = { 0xff, 0xff, 0xff, 0xff,
                                0xff, 0xff, 0xff, 0xff };
   bool pass = true;

   glClear(GL_COLOR_BUFFER_BIT);

   glViewport(0, 0, piglit_width, piglit_height);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glOrtho(0, piglit_width, 0, piglit_height, -1, 1);

   /* set raster color to green */
   glColor3fv(green);
   glRasterPos2i(8, 8);

   /* set triangle drawing color to blue */
   glColor3fv(blue);

   /* draw green bitmap */
   glBitmap(8, 8, 0, 0, 32, 0, bitmap);

   /* draw blue quad */
   glBegin(GL_QUADS);
   glVertex2f(24, 8);
   glVertex2f(32, 8);
   glVertex2f(32, 16);
   glVertex2f(24, 16);
   glEnd();

   /* draw green bitmap */
   glBitmap(8, 8, 0, 0, 0, 0, bitmap);

   if (!piglit_probe_pixel_rgb(12, 12, green)) {
      printf("first bitmap color should be green\n");
      pass = false;
   }

   if (!piglit_probe_pixel_rgb(12 + 16, 12, blue)) {
      printf("quad color should be blue\n");
      pass = false;
   }

   if (!piglit_probe_pixel_rgb(12 + 32, 12, green)) {
      printf("second bitmap color should be green\n");
      pass = false;
   }

   piglit_present_results();

   return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


void
piglit_init(int argc, char **argv)
{
   glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
}
