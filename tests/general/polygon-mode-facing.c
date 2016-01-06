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
 * Tests glPolygonMode wrt facing.
 * Roland Scheidegger
 * December 2015
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 20;

	config.window_width = 400;
	config.window_height = 100;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static const char *TestName = "polygon-mode-facing";

static const char *vstext =
        "#version 130\n"
        "\n"
        "void main()\n"
        "{\n"
        "  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
        "}\n";

static const char *fstext =
        "#version 130\n"
        "\n"
        "void main()\n"
        "{\n"
        "  vec4 color = gl_FrontFacing ? vec4(1.0, 0.0, 0.0, 1.0)\n"
        "                              : vec4(0.0, 1.0, 0.0, 1.0);\n"
        "  gl_FragColor = color;\n"
        "}\n";


static const GLfloat Colors[2][4] = {
   /* back color */
   {0, 1, 0, 1},
   /* front color */
   {1, 0, 0, 1},
};

static const GLfloat Positions[4][4][2] = {
   {{10, 10},
    {90, 10},
    {90, 90},
    {10, 90}},

   {{190, 10},
    {110, 10},
    {110, 90},
    {190, 90}},

   {{290, 10},
    {210, 10},
    {210, 90},
    {290, 90}},
  
   {{310, 10},
    {390, 10},
    {390, 90},
    {310, 90}},
};

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


/**
 * Probe a 3x3 pixel region to see if any of the pixels matches the
 * expected color.
 */
static GLboolean
probe_region(float px, float py, const GLfloat expectedColor[4])
{
   GLfloat img[3][3][4];
   int i, j;

   glReadPixels(px-1, py-1, 3, 3, GL_RGBA, GL_FLOAT, img);

   /* see if any of the pixels matches the expected color */
   for (i = 0; i < 3; i++) {
      for (j = 0; j < 3; j++) {
         if (img[i][j][0] == expectedColor[0] &&
             img[i][j][1] == expectedColor[1] &&
             img[i][j][2] == expectedColor[2]) {
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
   float lx = MIN2(positions[0][0], positions[2][0]);
   float ly = cy;
   /* right edge */
   float rx = MAX2(positions[0][0], positions[2][0]);
   float ry = cy;
   /* bottom edge */
   float bx = cx;
   float by = positions[0][1];
   /* top edge */
   float tx = cx;
   float ty = positions[1][1];

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
   int expectFrontColorRef[4] = {1,1,1,1};
   
   glClear(GL_COLOR_BUFFER_BIT);

   /*
    * Drawing 4 quads. The first and 3rd should always be red (front facing).
    * The 2nd and 4th are green with FILL backMode (lines/points are always
    * front facing).
    */
   if (backMode == GL_FILL) {
      expectFrontColorRef[1] = expectFrontColorRef[3] = 0;
   }

   /* Draw reference image */
   glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
   glFrontFace(GL_CCW);
   glDrawArrays(frontPrim, 0, 4);
   glDrawArrays(backPrim, 4, 4);
   glFrontFace(GL_CW);
   glDrawArrays(frontPrim, 8, 4);
   glDrawArrays(backPrim, 12, 4);

   /* determine what kind of primitives were drawn */
   for (i = 0; i < 4; i++) {
      bool err = false;
      expectedPrims[i] = identify_primitive(Positions[i], Colors[expectFrontColorRef[i]]);
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
                 TestName, piglit_get_gl_enum_name(frontMode),
                           piglit_get_gl_enum_name(backMode));
         return GL_FALSE;
      }
   }

   /* Draw test image */
   glClear(GL_COLOR_BUFFER_BIT);
   glPolygonMode(GL_FRONT, frontMode);
   glPolygonMode(GL_BACK, backMode);
   glFrontFace(GL_CCW);
   glDrawArrays(GL_QUADS, 0, 8);
   glFrontFace(GL_CW);
   glDrawArrays(GL_QUADS, 8, 8);

   /* check that these prims match expectations */
   /*
    * The first and 3rd should always be red (front-facing), the 2nd and 4th
    * green (back-facing).
    */
   for (i = 0; i < 4; i++) {
      GLenum prim = identify_primitive(Positions[i], Colors[!(i % 2)]);
      if (prim != expectedPrims[i]) {
         fprintf(stderr, "%s: glPolygonMode(front=%s, back=%s) failed\n",
                 TestName, piglit_get_gl_enum_name(frontMode),
                           piglit_get_gl_enum_name(backMode));
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
   glEnableClientState(GL_VERTEX_ARRAY);

   /*
    * First test with same front/back mode.
    * Those are probably more important to get right...
    */
   if (!test_combo(GL_FILL, GL_FILL))
      pass = GL_FALSE;

   if (!test_combo(GL_POINT, GL_POINT))
      pass = GL_FALSE;

   /*
    * Be extra mean to mesa draw stage interactions turning lines back
    * to tris...
    */
   glEnable(GL_LINE_SMOOTH);

   if (!test_combo(GL_LINE, GL_LINE))
      pass = GL_FALSE;

   glDisable(GL_LINE_SMOOTH);

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


   /*
    * Be really mean to mesa draw stage interactions turning lines back
    * to tris...
    */
   glEnable(GL_LINE_SMOOTH);

   if (!test_combo(GL_FILL, GL_LINE))
      pass = GL_FALSE;

   if (!test_combo(GL_POINT, GL_LINE))
      pass = GL_FALSE;

   if (!test_combo(GL_LINE, GL_FILL))
      pass = GL_FALSE;

   if (!test_combo(GL_LINE, GL_POINT))
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
   GLuint prog;
   piglit_require_GLSL_version(130);
   piglit_ortho_projection(piglit_width, piglit_height, false);
   prog = piglit_build_simple_program(vstext, fstext);
   glUseProgram(prog);
}
