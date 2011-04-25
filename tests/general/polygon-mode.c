/*
 * Copyright (c) 2011 VMware, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NON-INFRINGEMENT.  IN NO EVENT SHALL VMWARE AND/OR THEIR SUPPLIERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/**
 * Tests glPolygonMode.
 * Brian Paul
 * April 2011
 */

#include "piglit-util.h"


int piglit_width = 500, piglit_height = 100;
int piglit_window_mode = GLUT_RGB | GLUT_ALPHA | GLUT_DOUBLE;

static const char *TestName = "polygon-mode";

#define VERTS 16

static const GLfloat Positions[VERTS][2] = {
   /* clockwise */
   { 0, -1 },
   { 1, -1 },
   { 1,  1 },
   { 0,  1 },

   /* counter-clockwise */
   { 2, -1 },
   { 2,  1 },
   { 3,  1 },
   { 3, -1 },

   /* clockwise */
   { 4, -1 },
   { 5, -1 },
   { 5,  1 },
   { 4,  1 },

   /* counter-clockwise */
   { 6, -1 },
   { 6,  1 },
   { 7,  1 },
   { 7, -1 }
};

static const GLfloat Colors[VERTS][4] = {
   {1, 0, 0, 1},
   {1, 0, 0, 1},
   {1, 0, 0, 1},
   {1, 0, 0, 1},

   {0, 1, 0, 1},
   {0, 1, 0, 1},
   {0, 1, 0, 1},
   {0, 1, 0, 1},

   {0, 0, 1, 1},
   {0, 0, 1, 1},
   {0, 0, 1, 1},
   {0, 0, 1, 1},

   {1, 1, 1, 1},
   {1, 1, 1, 1},
   {1, 1, 1, 1},
   {1, 1, 1, 1}
};


static const char *
get_mode_str(GLenum mode)
{
   switch (mode) {
   case GL_POINT:
      return "GL_POINT";
   case GL_LINE:
      return "GL_LINE";
   case GL_FILL:
      return "GL_FILL";
   default:
      return NULL;
   }
}


static GLenum
get_prim_mode(GLenum mode)
{
   switch (mode) {
   case GL_POINT:
      return GL_POINTS;
   case GL_LINE:
      return GL_LINE_LOOP;
   case GL_FILL:
      return GL_QUADS;
   default:
      return 0;
   }
}


static GLboolean
test_combo(GLenum frontMode, GLenum backMode)
{
   GLenum frontPrim = get_prim_mode(frontMode);
   GLenum backPrim = get_prim_mode(backMode);
   GLubyte *ref, *test;
   GLboolean pass = GL_TRUE;

   ref = malloc(piglit_width * piglit_height * 4);
   test = malloc(piglit_width * piglit_height * 4);

   /* Draw reference image */
   glClear(GL_COLOR_BUFFER_BIT);
   glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
   glDrawArrays(frontPrim, 0, 4);
   glDrawArrays(backPrim, 4, 4);
   glDrawArrays(frontPrim, 8, 4);
   glDrawArrays(backPrim, 12, 4);
   glReadPixels(0, 0, piglit_width, piglit_height,
                GL_RGBA, GL_UNSIGNED_BYTE, ref);

   /* Draw test image */
   glClear(GL_COLOR_BUFFER_BIT);
   glPolygonMode(GL_FRONT, frontMode);
   glPolygonMode(GL_BACK, backMode);
   glDrawArrays(GL_QUADS, 0, 16);
   glReadPixels(0, 0, piglit_width, piglit_height,
                GL_RGBA, GL_UNSIGNED_BYTE, test);

   /*
    * This assumes that there are generally no rasterization differences
    * between the normal rendering paths vs. when glPolygonMode is in effect.
    * If that's not true, we'll need a different comparision method.
    */
   if (memcmp(ref, test, piglit_width * piglit_height * 4)) {
      fprintf(stderr, "%s: glPolygonMode(front=%s, back=%s) failed\n",
              TestName, get_mode_str(frontMode), get_mode_str(backMode));
      pass = GL_FALSE;
   }

   glutSwapBuffers();

   free(ref);
   free(test);

   return pass;
}


static GLboolean
test_polygonmode(void)
{
   GLenum pass = GL_TRUE;

   glVertexPointer(2, GL_FLOAT, 0, Positions);
   glColorPointer(4, GL_FLOAT, 0, Colors);

   glEnableClientState(GL_VERTEX_ARRAY);
   glEnableClientState(GL_COLOR_ARRAY);

   if (!test_combo(GL_FILL, GL_LINE))
      pass = GL_FALSE;

   if (!test_combo(GL_FILL, GL_POINT))
      pass = GL_FALSE;

   if (!test_combo(GL_POINT, GL_LINE))
      pass = GL_FALSE;

   if (!test_combo(GL_POINT, GL_FILL))
      pass = GL_FALSE;

   if (!test_combo(GL_LINE, GL_FILL))
      pass = GL_FALSE;

   if (!test_combo(GL_LINE, GL_POINT))
      pass = GL_FALSE;

   if (!test_combo(GL_LINE, GL_LINE))
      pass = GL_FALSE;

   if (!test_combo(GL_POINT, GL_POINT))
      pass = GL_FALSE;

   return pass;
}


enum piglit_result
piglit_display(void)
{
   if (!test_polygonmode())
      return PIGLIT_FAILURE;

   return PIGLIT_SUCCESS;
}


void
piglit_init(int argc, char **argv)
{
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glOrtho(-1, 8, -2, 2, -1, 1);

   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
}
