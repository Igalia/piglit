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

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 500;
	config.window_height = 100;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static float ortho_left = -1, ortho_right = 8, ortho_bottom = -2, ortho_top = 2;

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


static void
obj_pos_to_win_pos(float x, float y, int *wx, int *wy)
{
   float ortho_width = ortho_right - ortho_left;
   float ortho_height = ortho_top - ortho_bottom;
   *wx = (int) ((x - ortho_left) / ortho_width * piglit_width);
   *wy = (int) ((y - ortho_bottom) / ortho_height * piglit_height);
}


/**
 * Probe a 3x3 pixel region to see if any of the pixels matches the
 * expected color.
 */
static GLboolean
probe_region(float px, float py, const GLfloat expectedColor[4])
{
   GLfloat img[3][3][4];
   int i, j;
   int wx, wy;

   obj_pos_to_win_pos(px, py, &wx, &wy);

   glReadPixels(wx-1, wy-1, 3, 3, GL_RGBA, GL_FLOAT, img);

   /* see if any of the pixels matches the expected color */
   for (i = 0; i < 3; i++) {
      for (j = 0; j < 3; j++) {
         if (img[i][j][0] == expectedColor[0] &&
             img[i][j][1] == expectedColor[1] &&
             img[i][j][2] == expectedColor[2] &&
             img[i][j][3] == expectedColor[3]) {
            return GL_TRUE;
         }
      }
   }

   return GL_FALSE;
}


/**
 * Examine the pixels drawn by a rect using the four vertex positions
 * and determine if it was drawn filled, outlined, or as four points.
 * \return GL_FILL, GL_LINE, GL_POINT or GL_NONE
 */
static GLenum
identify_primitive(const GLfloat positions[4][2],
                   const GLfloat expectedColor[4])
{
   /* center */
   float cx = (positions[0][0] + positions[2][0]) / 2.0;
   float cy = (positions[0][1] + positions[2][1]) / 2.0;
   /* left edge */
   float lx = positions[0][0];
   float ly = cy;
   /* right edge */
   float rx = positions[2][0];
   float ry = cy;
   /* bottom edge */
   float bx = cx;
   float by = positions[0][1];
   /* bottom edge */
   float tx = cx;
   float ty = positions[2][1];

   /* probe center */
   if (probe_region(cx, cy, expectedColor))
      return GL_FILL;

   /* probe left edge */
   if (probe_region(lx, ly, expectedColor)) {
      /* and bottom edge */
      if (probe_region(bx, by, expectedColor)) {
         /* and right edge */
         if (probe_region(rx, ry, expectedColor)) {
            /* and top edge */
            if (probe_region(tx, ty, expectedColor)) {
               return GL_LINE;
            }
         }
      }
   }

   /* probe lower-left corner */
   if (probe_region(lx, by, expectedColor)) {
      /* probe lower-right corner */
      if (probe_region(rx, by, expectedColor)) {
         /* probe top-left corner */
         if (probe_region(lx, ty, expectedColor)) {
            /* probe top-right corner */
            if (probe_region(rx, ty, expectedColor)) {
               return GL_POINT;
            }
         }
      }
   }

   return GL_NONE;
}



static GLboolean
test_combo(GLenum frontMode, GLenum backMode)
{
   GLenum frontPrim = get_prim_mode(frontMode);
   GLenum backPrim = get_prim_mode(backMode);
   GLboolean pass = GL_TRUE;
   GLenum expectedPrims[4];
   int i;

   /* Draw reference image */
   glClear(GL_COLOR_BUFFER_BIT);
   glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
   glDrawArrays(frontPrim, 0, 4);
   glDrawArrays(backPrim, 4, 4);
   glDrawArrays(frontPrim, 8, 4);
   glDrawArrays(backPrim, 12, 4);

   /* determine what kind of primitives were drawn */
   for (i = 0; i < 4; i++) {
      bool err = false;
      expectedPrims[i] = identify_primitive(&Positions[4 * i], Colors[4 * i]);
      if (i & 1) {
         if (expectedPrims[i] != backMode) {
            err = true;
         }
      }
      else {
         if (expectedPrims[i] != frontMode) {
            err = true;
         }
      }
      if (err) {
         /* we didn't get the expected reference primitive */
         fprintf(stderr,
                 "%s: reference drawing failed for frontPrim=%s, backPrim=%s\n",
                 TestName, get_mode_str(frontMode), get_mode_str(backMode));
         return GL_FALSE;
      }
   }

   /* Draw test image */
   glClear(GL_COLOR_BUFFER_BIT);
   glPolygonMode(GL_FRONT, frontMode);
   glPolygonMode(GL_BACK, backMode);
   glDrawArrays(GL_QUADS, 0, 16);

   /* check that these prims match the reference prims */
   for (i = 0; i < 4; i++) {
      GLenum prim = identify_primitive(&Positions[4 * i], Colors[4 * i]);
      if (prim != expectedPrims[i]) {
         fprintf(stderr, "%s: glPolygonMode(front=%s, back=%s) failed\n",
                 TestName, get_mode_str(frontMode), get_mode_str(backMode));
         pass = GL_FALSE;
      }
   }

   piglit_present_results();

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
      return PIGLIT_FAIL;

   return PIGLIT_PASS;
}


void
piglit_init(int argc, char **argv)
{
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glOrtho(ortho_left, ortho_right, ortho_bottom, ortho_top, -1, 1);

   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
}
