/*
 * Copyright (c) 2014 VMware, Inc.
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
 * @file lineloop.c
 *
 * Test line loop with many vertices.
 * No additional lines should appear due to buffer splitting.
 */

#include "piglit-util-gl.h"

#define WSIZE 400

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
        config.window_width = WSIZE;
        config.window_height = WSIZE;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static const char *TestName = "lineloop";
static int vert_count = 10000;
static bool use_dlist = false;
static GLuint dlist;


static void
draw(GLuint numVerts, float radius)
{
   GLuint i;

   glColor3f(1,0,1);
   glBegin(GL_LINE_LOOP);
   for (i = 0; i < numVerts; i++) {
      float x = radius * sin(i*M_PI*2/numVerts);
      float y = radius * cos(i*M_PI*2/numVerts);
      glVertex3f(x, y, 0);
   }
   glEnd();
}

static void
test_prims(void)
{
   if (!piglit_automatic)
      printf("%s: %u vertices\n", TestName, vert_count);

   glClear(GL_COLOR_BUFFER_BIT);

   if (use_dlist) {
      glCallList(dlist);
   } else {
      draw(vert_count, 1.0);
   }
   piglit_present_results();
}


enum piglit_result
piglit_display(void)
{
   float expected[4] = {0.0f, 0.0f, 0.0f, 0.0f};
   int pass;
   int half_quad = (int)((float)(WSIZE/2) / sqrt(2.0f) - 1.0f);
   test_prims();
   pass = piglit_probe_rect_rgb(WSIZE / 2 - half_quad,
                                WSIZE / 2 - half_quad,
                                half_quad, half_quad, expected);
   return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


void
piglit_init(int argc, char**argv)
{
   int i;
   for (i = 1; i < argc; ++i) {
      if (i < argc) {
         if (strcmp(argv[i], "-count") == 0) {
            i++;
            if (i == argc) {
               printf("please specify vertex count\n");
               piglit_report_result(PIGLIT_FAIL);
            }
            vert_count = strtoul(argv[i], NULL, 0);
         }
         else if (strcmp(argv[i], "-dlist") == 0) {
            use_dlist = true;
         }
      }
   }
 
   glViewport(0,0, WSIZE, WSIZE);
   glOrtho(-1,1,-1,1,-1,1);

   if (use_dlist) {
      dlist = glGenLists(1);
      glNewList(dlist, GL_COMPILE);
      draw(vert_count, 1.0);
      glEndList();
   }
}
