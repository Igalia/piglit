/*
 * Copyright (c) 2010 VMware, Inc.
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
 * @file longprim.c
 *
 * Test primitive with many vertices.  Just don't crash.
 * Brian Paul
 */

#include "piglit-util.h"
#include "piglit-framework.h"

int piglit_width = 100;
int piglit_height = 100;
int piglit_window_mode = GLUT_RGB | GLUT_DOUBLE;

static const char *TestName = "longprim";


static const GLenum primTypes[] = {
   GL_POINTS,
   GL_LINES,
   GL_LINE_LOOP,
   GL_LINE_STRIP,
   GL_TRIANGLES,
   GL_TRIANGLE_STRIP,
   GL_TRIANGLE_FAN,
   GL_QUADS,
   GL_QUAD_STRIP,
   GL_POLYGON
};

static const char *primNames[] = {
   "GL_POINTS",
   "GL_LINES",
   "GL_LINE_LOOP",
   "GL_LINE_STRIP",
   "GL_TRIANGLES",
   "GL_TRIANGLE_STRIP",
   "GL_TRIANGLE_FAN",
   "GL_QUADS",
   "GL_QUAD_STRIP",
   "GL_POLYGON"
};


static void
draw(GLenum mode, GLuint numVerts)
{
   GLuint i;

   glBegin(mode);
   for (i = 0; i < numVerts; i++) {
      float x = 0.025 * (rand() % 2000 - 1000);
      float y = 0.025 * (rand() % 2000 - 1000);
      glVertex2f(x, y);
   }
   glEnd();
}

static void
test_prims(void)
{
   GLuint len, prim;

   for (len = 1000; len <= 1000 * 1000; len *= 10) {
      for (prim = 0; prim < ARRAY_SIZE(primTypes); prim++) {
         if (!piglit_automatic)
            printf("%s: %s %u vertices\n", TestName, primNames[prim], len);
         glClear(GL_COLOR_BUFFER_BIT);
         draw(primTypes[prim], len);
         glutSwapBuffers();
      }
   }
}


enum piglit_result
piglit_display(void)
{
   test_prims();
   return PIGLIT_PASS;
}


void
piglit_init(int argc, char**argv)
{
   glOrtho(-100.0, 100.0, -100.0, 100.0, -1.0, 1.0);
   glShadeModel(GL_FLAT);
}
