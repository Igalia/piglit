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

/*
 * Test some unusual vertex array strides.
 * Brian Paul
 * June 14, 2011
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static const char *TestName = "array-stride";


#define ROWS 10
#define COLS 10
#define NUM_VERTS (ROWS * COLS * 4)

static GLfloat Verts[ROWS][COLS][4][2];


static void
gen_vertices(void)
{
   float dx = 9.0, dy = 9.0;
   int i, j;
   for (i = 0; i < ROWS; i++) {
      float y = i * 10.0;
      for (j = 0; j < COLS; j++) {
         float x = j * 10.0;
         Verts[i][j][0][0] = x;
         Verts[i][j][0][1] = y;
         Verts[i][j][1][0] = x + dx;
         Verts[i][j][1][1] = y;
         Verts[i][j][2][0] = x + dx;
         Verts[i][j][2][1] = y + dy;
         Verts[i][j][3][0] = x;
         Verts[i][j][3][1] = y + dy;
      }
   }
}


static void
draw_simple_stride(void)
{
   static GLushort elements[NUM_VERTS];
   int i;

   for (i = 0; i < ARRAY_SIZE(elements); i++)
      elements[i] = i;

   glVertexPointer(2, GL_FLOAT, 2 * sizeof(GLfloat), Verts);
   glEnableClientState(GL_VERTEX_ARRAY);
   glDrawElements(GL_QUADS, NUM_VERTS, GL_UNSIGNED_SHORT, elements);
   glDisableClientState(GL_VERTEX_ARRAY);
}


/**
 * Use a stride that's less than the vertex element size.
 * Use scaled indices in the element array.
 * Should render the same as the simple stride case above.
 */
static void
draw_unusual_stride(int stride)
{
   static GLushort elements[NUM_VERTS];
   int i;

   for (i = 0; i < ARRAY_SIZE(elements); i++)
      elements[i] = i * 8 / stride;

   glVertexPointer(2, GL_FLOAT, stride, Verts);
   glEnableClientState(GL_VERTEX_ARRAY);
   glDrawElements(GL_QUADS, NUM_VERTS, GL_UNSIGNED_SHORT, elements);
   glDisableClientState(GL_VERTEX_ARRAY);
}


enum piglit_result
piglit_display(void)
{
   GLubyte *buf0, *buf1;
   enum piglit_result result = PIGLIT_PASS;
   int stride;

   gen_vertices();

   buf0 = (GLubyte *) malloc(piglit_width * piglit_height * 4);
   buf1 = (GLubyte *) malloc(piglit_width * piglit_height * 4);

   /* draw reference image */
   glClear(GL_COLOR_BUFFER_BIT);
   draw_simple_stride();
   glReadPixels(0, 0, piglit_width, piglit_height,
                GL_RGBA, GL_UNSIGNED_BYTE, buf0);

   /* draw w/ unusual strides */
   for (stride = 1; stride <= 8; stride *= 2) {
      glClear(GL_COLOR_BUFFER_BIT);
      draw_unusual_stride(stride);
      glReadPixels(0, 0, piglit_width, piglit_height,
                   GL_RGBA, GL_UNSIGNED_BYTE, buf1);

      piglit_present_results();

      /* compare bufs */
      if (memcmp(buf0, buf1, piglit_width * piglit_height * 4) != 0) {
         printf("%s: image comparison failed at stride = %d\n",
                TestName, stride);
         result = PIGLIT_FAIL;
      }
   }

   free(buf0);
   free(buf1);

   return result;
}


void
piglit_init(int argc, char**argv)
{
   glOrtho(-1.0, 101.0, -1.0, 101.0, -1.0, 1.0);
}
