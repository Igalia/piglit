/*
 * Copyright © 2010 VMware, Inc.
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *    Brian Paul
 */


/**
 * @file draw-elements-vs-inputs.c
 *
 * Test that state validation is properly done between calls to
 * glDrawRangeElements() / glDrawElements() when VS inputs change between
 * calls (with regard to per-vertex vs. per-primitive values).
 *
 * This is a regression test for a bug in Mesa/gallium/softpipe which
 * was fixed with commit 3cba779e16935f7c3a0bfd8af48bd5e015068e96.
 */


#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 300;
	config.window_height = 300;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

void
piglit_init(int argc, char **argv)
{
   GLfloat red[4] = {1, 0, 0, 1};

   piglit_require_gl_version(12);

   glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, red);

   glEnable(GL_DEPTH_TEST);
   glEnable(GL_LIGHTING);
   glEnable(GL_LIGHT0);

   glEnable(GL_NORMALIZE);

   glClearColor(0.5, 0.5, 0.5, 0.5);

   /*
   printf("GL_RENDERER = %s\n", (char *) glGetString(GL_RENDERER));
   printf("GL_VERSION = %s\n", (char *) glGetString(GL_VERSION));
   */
}


static void
draw_quad(GLboolean normals, GLboolean rangeElements)
{
   static GLfloat norms[4][3] = {
      { -0.1, -0.1, 1 },
      {  0.1, -0.1, 1 },
      {  0.1,  0.1, 1 },
      { -0.1,  0.1, 1 }
   };
   static GLfloat verts[4][3] = {
      { -25, -25, 0.0 },
      {  25, -25, 0.0 },
      {  25,  25, 0.0 },
      { -25,  25, 0.0 }
   };
   static GLuint indexes[4] = { 0, 1, 2, 3 };

   if (normals) {
      glNormalPointer(GL_FLOAT, 0, norms);
      glEnableClientState(GL_NORMAL_ARRAY);
   }
   else {
      glNormal3f(1, 1, 1);
   }
   glVertexPointer(3, GL_FLOAT, 0, verts);
   glEnableClientState(GL_VERTEX_ARRAY);
   if (rangeElements)
      glDrawRangeElements(GL_QUADS, 0, 3, 4, GL_UNSIGNED_INT, indexes);
   else
      glDrawElements(GL_QUADS, 4, GL_UNSIGNED_INT, indexes);

   glDisableClientState(GL_VERTEX_ARRAY);
   glDisableClientState(GL_NORMAL_ARRAY);
}


enum piglit_result
piglit_display(void)
{
   const GLfloat expected1[3] = { 1.0, 0.039, 0.039, };
   const GLfloat expected2[3] = { 0.615, 0.039, 0.039, };
   GLint row;
   GLboolean pass = GL_TRUE;

   glViewport(0, 0, piglit_width, piglit_height);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glOrtho(0, piglit_width, 0, piglit_height, -1.0, 1.0);

   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();

   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   /* first row = glDrawElements, second row = glDrawRangeElements */
   for (row = 0; row < 2; row++) {
      GLint x0 = piglit_width/4;
      GLint x1 = piglit_width/2;
      GLint x2 = piglit_width*3/4;
      GLint y = (piglit_height / 3) * (row + 1);
      GLboolean rangeElements = row == 1;

      /* quad with per-vertex normals */
      if (1) {
         glPushMatrix();
         glTranslatef(x0, y, 0);
         draw_quad(GL_TRUE, rangeElements);
         glPopMatrix();
         glFlush();
      }

      /* quad with one normal */
      if (1) {
         glPushMatrix();
         glTranslatef(x1, y, 0);
         glNormal3f(1, 0.5, 0.25);
         draw_quad(GL_FALSE, rangeElements);
         glPopMatrix();
         glFlush();
      }

      /* another quad with per-vertex normals */
      if (1) {
         glPushMatrix();
         glTranslatef(x2, y, 0);
         draw_quad(GL_TRUE, rangeElements);
         glPopMatrix();
      }

      /* left quad */
      pass = piglit_probe_pixel_rgb(x0, y, expected1) && pass;

      /* middle quad */
      pass = piglit_probe_pixel_rgb(x1, y, expected2) && pass;

      /* right quad */
      pass = piglit_probe_pixel_rgb(x2, y, expected1) && pass;
   }

   piglit_present_results();

   return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

