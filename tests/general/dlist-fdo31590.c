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
 * Test related to fd.o bug 31590 involving glEvalCoord inside a display list
 * when running the glut molehill test.
 * The mesa-demos redbook/bezcurve.c test (when hacked to use a display list)
 * also demonstrated the problem.  This program is based on the later program.
 *
 * We test for two things:
 *  1. an unexpected GL_INVALID_OPERATION error
 *  2. a segfault/crash during display list compilation
 */


#include "piglit-util-gl.h"

static const char *TestName = "dlist-fdo31590";

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 500;
	config.window_height = 500;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static const GLfloat ctrlpoints[4][3] = {
   { -4.0, -4.0, 0.0}, { -2.0, 4.0, 0.0}, 
   {2.0, -4.0, 0.0}, {4.0, 4.0, 0.0}
};


static void
test1(void)
{
   int i;

   glNewList(5, GL_COMPILE);
      glBegin(GL_LINE_STRIP);
      glColor3f(1,1,0);
         for (i = 0; i <= 30; i++) {
            glEvalCoord1f((GLfloat) i/30.0);
         }
      glEnd();
   glEndList();

   glClear(GL_COLOR_BUFFER_BIT);
   glColor3f(1.0, 1.0, 1.0);
   glCallList(5);
}


/* As above, but with a glColor() call in the display list.
 * This caused a segfault in Mesa.
 */
static void
test2(void)
{
   int i;

   glNewList(5, GL_COMPILE);
      glBegin(GL_LINE_STRIP);
      glColor3f(1,1,0);             /* <<<<< this is the only difference */
         for (i = 0; i <= 30; i++) {
            glEvalCoord1f((GLfloat) i/30.0);
         }
      glEnd();
   glEndList();

   glClear(GL_COLOR_BUFFER_BIT);
   glColor3f(1.0, 1.0, 1.0);
   glCallList(5);
}


enum piglit_result
piglit_display(void)
{
   glMap1f(GL_MAP1_VERTEX_3, 0.0, 1.0, 3, 4, &ctrlpoints[0][0]);
   glEnable(GL_MAP1_VERTEX_3);

#if 0
   /* The following code displays the control points as dots. */
   glPointSize(5.0);
   glColor3f(1.0, 1.0, 0.0);
   glBegin(GL_POINTS);
      for (i = 0; i < 4; i++) 
         glVertex3fv(&ctrlpoints[i][0]);
   glEnd();
#endif

   test1();

   if (glGetError() != GL_NO_ERROR) {
      printf("%s: test1 generated an unexpected error\n", TestName);
      return PIGLIT_FAIL;
   }

   test2();

   if (glGetError() != GL_NO_ERROR) {
      printf("%s: test2 generated an unexpected error\n", TestName);
      return PIGLIT_FAIL;
   }

   piglit_present_results();

   return PIGLIT_PASS;
}


void
piglit_init(int argc, char **argv)
{
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glOrtho(-5.0, 5.0, -5.0, 5.0, -5.0, 5.0);
   glMatrixMode(GL_MODELVIEW);
}
