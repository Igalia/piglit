/*
 * Copyright (c) 2010 VMware, Inc.
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
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORES OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/**
 * @file
 * Test that points and lines are not effected by polygon culling,
 * polygon stippling or "unfilled" mode.
 *
 * @author Brian Paul
 */


#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

/** Test if the pixels at (x,y) and (x,y+1) are black.
 * We test two pixels to be sure we hit the primitive we drew.  We could
 * be off by one and miss the line if it's only one pixel wide otherwise.
 */
static GLboolean
black_pixel(float x, float y)
{
   GLfloat pixel[2][3];

   glReadPixels((int) x, (int) (y-0.5), 1, 2, GL_RGB, GL_FLOAT, pixel);

   if (pixel[0][0] == 0.0 &&
       pixel[0][1] == 0.0 &&
       pixel[0][2] == 0.0 &&
       pixel[1][0] == 0.0 &&
       pixel[1][1] == 0.0 &&
       pixel[1][2] == 0.0)
      return GL_TRUE;
   else
      return GL_FALSE;
}


/** test that lines aren't effected by polygon culling */
static GLboolean
test_lines_no_culling(void)
{
   const GLfloat x0 = 5.0, x1 = 40.0, xmid = 0.5 * (x0 + x1);
   const GLfloat x2 = 45.0, x3 = 85.0, xmid_aa = 0.5 * (x2 + x3);
   const GLfloat y0 = 5.0, y1 = 15.0, y2 = 25.0;
   GLboolean pass = GL_TRUE;

   glLineWidth(3.0);
   glEnable(GL_CULL_FACE);

   /* Non-AA */

   glCullFace(GL_FRONT);
   glBegin(GL_LINES);
   glVertex2f(x0, y0);
   glVertex2f(x1, y0);
   glEnd();
   if (black_pixel(xmid, y0)) {
      fprintf(stderr, "Error: Line culled by GL_CULL_FACE = GL_FRONT\n");
      pass = GL_FALSE;
   }

   glCullFace(GL_BACK);
   glBegin(GL_LINES);
   glVertex2f(x0, y1);
   glVertex2f(x1, y1);
   glEnd();
   if (black_pixel(xmid, y1)) {
      fprintf(stderr, "Error: Line culled by GL_CULL_FACE = GL_BACK\n");
      pass = GL_FALSE;
   }

   glCullFace(GL_FRONT_AND_BACK);
   glBegin(GL_LINES);
   glVertex2f(x0, y2);
   glVertex2f(x1, y2);
   glEnd();
   if (black_pixel(xmid, y2)) {
      fprintf(stderr, "Error: Line culled by GL_CULL_FACE = GL_FRONT_AND_BACK\n");
      pass = GL_FALSE;
   }


   /* AA */
   glEnable(GL_LINE_SMOOTH);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   glEnable(GL_BLEND);

   glCullFace(GL_FRONT);
   glBegin(GL_LINES);
   glVertex2f(x2, y0);
   glVertex2f(x3, y0);
   glEnd();
   if (black_pixel(xmid_aa, y0)) {
      fprintf(stderr, "Error: AA Line culled by GL_CULL_FACE = GL_FRONT\n");
      pass = GL_FALSE;
   }

   glCullFace(GL_BACK);
   glBegin(GL_LINES);
   glVertex2f(x2, y1);
   glVertex2f(x3, y1);
   glEnd();
   if (black_pixel(xmid_aa, y1)) {
      fprintf(stderr, "Error: AA Line culled by GL_CULL_FACE = GL_BACK\n");
      pass = GL_FALSE;
   }

   glCullFace(GL_FRONT_AND_BACK);
   glBegin(GL_LINES);
   glVertex2f(x2, y2);
   glVertex2f(x3, y2);
   glEnd();
   if (black_pixel(xmid_aa, y2)) {
      fprintf(stderr, "Error: AA Line culled by GL_CULL_FACE = GL_FRONT_AND_BACK\n");
      pass = GL_FALSE;
   }

   glDisable(GL_BLEND);
   glDisable(GL_LINE_SMOOTH);
   glDisable(GL_CULL_FACE);
   glLineWidth(1.0);

   return pass;
}


/** test that points aren't effected by polygon culling */
static GLboolean
test_points_no_culling(void)
{
   const GLfloat x0 = 100.0, x1 = 110.0;
   const GLfloat y0 = 5.0, y1 = 15.0, y2 = 25.0;
   GLboolean pass = GL_TRUE;

   glPointSize(5.0);
   glEnable(GL_CULL_FACE);

   /* Non-AA */

   glCullFace(GL_FRONT);
   glBegin(GL_POINTS);
   glVertex2f(x0, y0);
   glEnd();
   if (black_pixel(x0, y0)) {
      fprintf(stderr, "Error: Point culled by GL_CULL_FACE = GL_FRONT\n");
      pass = GL_FALSE;
   }

   glCullFace(GL_BACK);
   glBegin(GL_POINTS);
   glVertex2f(x0, y1);
   glEnd();
   if (black_pixel(x0, y1)) {
      fprintf(stderr, "Error: Point culled by GL_CULL_FACE = GL_BACK\n");
      pass = GL_FALSE;
   }

   glCullFace(GL_FRONT_AND_BACK);
   glBegin(GL_POINTS);
   glVertex2f(x0, y2);
   glEnd();
   if (black_pixel(x0, y2)) {
      fprintf(stderr, "Error: Point culled by GL_CULL_FACE = GL_FRONT_AND_BACK\n");
      pass = GL_FALSE;
   }

   /* AA */
   glEnable(GL_POINT_SMOOTH);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   glEnable(GL_BLEND);

   glCullFace(GL_FRONT);
   glBegin(GL_POINTS);
   glVertex2f(x1, y0);
   glEnd();
   if (black_pixel(x1, y0)) {
      fprintf(stderr, "Error: AA Point culled by GL_CULL_FACE = GL_FRONT\n");
      pass = GL_FALSE;
   }

   glCullFace(GL_BACK);
   glBegin(GL_POINTS);
   glVertex2f(x1, y1);
   glEnd();
   if (black_pixel(x1, y1)) {
      fprintf(stderr, "Error: AA Point culled by GL_CULL_FACE = GL_BACK\n");
      pass = GL_FALSE;
   }

   glCullFace(GL_FRONT_AND_BACK);
   glBegin(GL_POINTS);
   glVertex2f(x1, y2);
   glEnd();
   if (black_pixel(x1, y2)) {
      fprintf(stderr, "Error: AA Point culled by GL_CULL_FACE = GL_FRONT_AND_BACK\n");
      pass = GL_FALSE;
   }

   glDisable(GL_BLEND);
   glDisable(GL_POINT_SMOOTH);
   glDisable(GL_CULL_FACE);
   glPointSize(1.0);

   return pass;
}


/** test that lines aren't effected by polygon stipple */
static GLboolean
test_lines_no_stippling(void)
{
   const GLfloat x0 = 5.0, x1 = 40.0, xmid = 0.5 * (x0 + x1);
   const GLfloat x2 = 45.0, x3 = 85.0, xmid_aa = 0.5 * (x2 + x3);
   const GLfloat y0 = 50.0;
   GLubyte stipple[4 * 32];
   GLboolean pass = GL_TRUE;

   memset(stipple, 0, sizeof(stipple));
   glPolygonStipple(stipple);
   glEnable(GL_POLYGON_STIPPLE);

   glLineWidth(3.0);

   /* Non-AA */
   glBegin(GL_LINES);
   glVertex2f(x0, y0);
   glVertex2f(x1, y0);
   glEnd();
   if (black_pixel(xmid, y0)) {
      fprintf(stderr, "Error: Line not drawn because of polygon stipple.\n");
      pass = GL_FALSE;
   }

   /* AA */
   glEnable(GL_LINE_SMOOTH);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   glEnable(GL_BLEND);

   glBegin(GL_LINES);
   glVertex2f(x2, y0);
   glVertex2f(x3, y0);
   glEnd();
   if (black_pixel(xmid_aa, y0)) {
      fprintf(stderr, "Error: AA Line not drawn because of polygon stipple.\n");
      pass = GL_FALSE;
   }

   glDisable(GL_BLEND);
   glDisable(GL_LINE_SMOOTH);
   glDisable(GL_POLYGON_STIPPLE);
   glLineWidth(1.0);

   return pass;
}


/** test that points aren't effected by polygon stipple */
static GLboolean
test_points_no_stippling(void)
{
   const GLfloat x0 = 100.0, x1 = 110.0;
   const GLfloat y0 = 50.0;
   GLubyte stipple[4 * 32];
   GLboolean pass = GL_TRUE;

   memset(stipple, 0, sizeof(stipple));
   glPolygonStipple(stipple);
   glEnable(GL_POLYGON_STIPPLE);

   glPointSize(5.0);

   /* Non-AA */
   glBegin(GL_POINTS);
   glVertex2f(x0, y0);
   glEnd();
   if (black_pixel(x0, y0)) {
      fprintf(stderr, "Error: Point not drawn because of polygon stipple.\n");
      pass = GL_FALSE;
   }

   /* AA */
   glEnable(GL_POINT_SMOOTH);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   glEnable(GL_BLEND);

   glBegin(GL_POINTS);
   glVertex2f(x1, y0);
   glEnd();
   if (black_pixel(x1, y0)) {
      fprintf(stderr, "Error: AA Point not drawn because of polygon stipple.\n");
      pass = GL_FALSE;
   }

   glDisable(GL_BLEND);
   glDisable(GL_POINT_SMOOTH);
   glDisable(GL_POLYGON_STIPPLE);
   glPointSize(1.0);

   return pass;
}


/** test that lines aren't effected by glPolygonMode */
static GLboolean
test_lines_no_pgonmode(void)
{
   const GLfloat x0 = 5.0, x1 = 40.0, xmid = 0.5 * (x0 + x1);
   const GLfloat x2 = 45.0, x3 = 85.0, xmid_aa = 0.5 * (x2 + x3);
   const GLfloat y0 = 80.0;
   GLboolean pass = GL_TRUE;

   glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
   glLineWidth(3.0);

   /* Non-AA */
   glBegin(GL_LINES);
   glVertex2f(x0, y0);
   glVertex2f(x1, y0);
   glEnd();
   if (black_pixel(xmid, y0)) {
      fprintf(stderr, "Error: Line not drawn because of polygon mode.\n");
      pass = GL_FALSE;
   }

   /* AA */
   glEnable(GL_LINE_SMOOTH);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   glEnable(GL_BLEND);

   glBegin(GL_LINES);
   glVertex2f(x2, y0);
   glVertex2f(x3, y0);
   glEnd();
   if (black_pixel(xmid_aa, y0)) {
      fprintf(stderr, "Error: Line not drawn because of polygon mode.\n");
      pass = GL_FALSE;
   }

   glDisable(GL_BLEND);
   glDisable(GL_LINE_SMOOTH);
   glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
   glLineWidth(1.0);

   return pass;
}


/** test that points aren't effected by glPolygonMode */
static GLboolean
test_points_no_pgonmode(void)
{
   const GLfloat x0 = 100.0, x1 = 110.0;
   const GLfloat y0 = 80.0;
   GLboolean pass = GL_TRUE;

   glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
   glPointSize(5.0);

   /* Non-AA */
   glBegin(GL_POINTS);
   glVertex2f(x0, y0);
   glEnd();
   if (black_pixel(x0, y0)) {
      fprintf(stderr, "Error: Line not drawn because of polygon mode.\n");
      pass = GL_FALSE;
   }

   /* AA */
   glEnable(GL_POINT_SMOOTH);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   glEnable(GL_BLEND);

   glBegin(GL_POINTS);
   glVertex2f(x1, y0);
   glEnd();
   if (black_pixel(x1, y0)) {
      fprintf(stderr, "Error: AA Line not drawn because of polygon mode.\n");
      pass = GL_FALSE;
   }

   glDisable(GL_BLEND);
   glDisable(GL_POINT_SMOOTH);
   glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
   glPointSize(1.0);

   return pass;
}


enum piglit_result
piglit_display(void)
{
   GLboolean pass = GL_TRUE;

   piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

   glClear(GL_COLOR_BUFFER_BIT);

   pass = test_lines_no_culling() && pass;
   pass = test_points_no_culling() && pass;
   pass = test_lines_no_stippling() && pass;
   pass = test_points_no_stippling() && pass;
   pass = test_lines_no_pgonmode() && pass;
   pass = test_points_no_pgonmode() && pass;

   piglit_present_results();

   return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
}
