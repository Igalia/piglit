/* Copyright © 2014 Igalia S.L.
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
 * \file polygon-line-aa.c
 * Test case for a special case of line antialiasing
 * https://bugs.freedesktop.org/show_bug.cgi?id=78679
 *
 * This test renders a polygon using GL_LINE mode (with antialised lines)
 * for one face of the polygon, and GL_FILL for the other face. For gen < 6
 * intel hardware this setup requires special handling that if not done
 * correctly produces incorrect rendering of the GL_FILL face. This caused
 * regressions in the past.
 *
 * Author: Iago Toral <itoral@igalia.com>
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

   config.supports_gl_compat_version = 10;
   config.window_visual = PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

void
piglit_init(int argc, char **argv)
{
   /* This enables the case we want to test for */
   glEnable(GL_LINE_SMOOTH);
   glShadeModel(GL_SMOOTH);
   glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
   glLineWidth(1.5);
   glEnable(GL_BLEND);
   glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   glPolygonMode(GL_BACK, GL_LINE);

   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glOrtho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);

   glClearColor(1, 1, 1, 1);
}

enum piglit_result
piglit_display(void)
{
   GLfloat expected[4] = { 0.0, 0.0, 1.0, 1.0 };

   glClear(GL_COLOR_BUFFER_BIT);

   glViewport(0, 0, piglit_width, piglit_height);

   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();

   glColor4f(0.0f, 0.0f, 1.0f, 1.0);
   glBegin(GL_QUADS);
      glVertex2f(-1.0f, -1.0f);
      glVertex2f( 1.0f, -1.0f);
      glVertex2f( 1.0f,  1.0f);
      glVertex2f(-1.0f,  1.0f);
   glEnd();

   glFlush();

   /* The test checks that the fill face of the quad is all blue without
    * noise artifacts
    */
   if (piglit_probe_rect_rgba(0, 0, piglit_width, piglit_height, expected))
      return PIGLIT_PASS;
   else
      return PIGLIT_FAIL;
}
