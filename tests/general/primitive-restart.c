/*
 * Copyright 2010 VMware, Inc.
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
 */

/**
 * Test GL_NV_primitive_restart and/or GL 3.1 primitive restart.
 * Note that these two extensions/features use different enum values
 * and Enable/Disable functions!
 *
 * Authors:
 *    Brian Paul
 */

#include "piglit-util.h"

int piglit_width = 400, piglit_height = 300;
int piglit_window_mode = GLUT_RGB | GLUT_DOUBLE;

static const char *TestName = "primitive-restart";

static const GLfloat red[4] = {1.0, 0.0, 0.0, 1.0};
static const GLfloat green[4] = {0.0, 1.0, 0.0, 0.0};
static const GLfloat black[4] = {0.0, 0.0, 0.0, 0.0};

static GLboolean Have_NV;
static GLboolean Have_31;
static GLboolean TestGL31;


static GLboolean
check_rendering(void)
{
   const GLfloat x0 = 0.0, x1 = piglit_width - 10.0, dx = 20.0;
   const GLint iy = piglit_height / 2;
   GLboolean draw = GL_TRUE;
   GLfloat x;

   for (x = x0 + 0.5 * dx; x < x1; x += dx) {
      GLboolean pass;
      const int ix = (int) x;

      if (draw) {
         /* there should be triangle drawing here */
         pass = piglit_probe_pixel_rgb(ix, iy, green);
      }
      else {
         /* there should not be triangle drawing here */
         pass = piglit_probe_pixel_rgb(ix, iy, black);
      }

      /* debug */
      if (0) {
         glWindowPos2i(ix, iy);
         glDrawPixels(1, 1, GL_RGBA, GL_FLOAT, red);
      }

      if (!pass) {
         return GL_FALSE;
      }

      draw = !draw;
   }

   return GL_TRUE;
}


/**
 * Test glBegin(GL_TRIANGLE/LINE_STRIP), glPrimitiveRestartNV(), glEnd().
 */
static GLboolean
test_begin_end(GLenum primMode)
{
   const GLfloat x0 = 0.0, x1 = piglit_width - 10.0, dx = 20.0;
   const GLfloat y0 = 0.5 * piglit_height - 10.0, y1 = y0 + 20.0, dy = 20.0;
   GLfloat x, y;
   GLint vert;
   GLboolean pass;

   piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

   glClear(GL_COLOR_BUFFER_BIT);

   glColor4fv(green);

   if (primMode == GL_TRIANGLE_STRIP) {
      /* Draw a tri-strip across the window, using restart to actually render
       * a series of quads/boxes.
       */
      glBegin(GL_TRIANGLE_STRIP);
      vert = 0;
      for (x = x0; x <= x1; x += dx) {
         for (y = y0; y <= y1; y += dy) {
            glVertex2f(x, y);
         }

         vert++;
         if (vert % 2 == 0)
            glPrimitiveRestartNV();
      }
      glEnd();
   }
   else {
      /* Draw a line strip across the window, using restart to actually render
       * a series of disconnected lines.
       */
      glLineWidth(5.0);
      glBegin(GL_LINE_STRIP);
      vert = 0;
      for (x = x0; x <= x1; x += dx) {
         y = 0.5 * piglit_height;

         glVertex2f(x, y);

         vert++;
         if (vert % 2 == 0)
            glPrimitiveRestartNV();
      }
      glEnd();
   }

   glFinish();

   pass = check_rendering();
   if (!pass) {
      fprintf(stderr, "%s: failure drawing with glBegin(%s) / glEnd()\n",
              TestName,
              (primMode == GL_TRIANGLE_STRIP
               ? "GL_TRIANGLE_STRIP" : "GL_LINE_STRIP"));
   }

   glutSwapBuffers();

   return pass;
}


static void
enable_restart(GLuint restart_index)
{
   if (TestGL31) {
#ifdef GL_VERSION_3_1
      glEnable(GL_PRIMITIVE_RESTART);
      glPrimitiveRestartIndex(restart_index);
#endif
   }
   else {
      glEnableClientState(GL_PRIMITIVE_RESTART_NV);
      glPrimitiveRestartIndexNV(restart_index);
   }
}


static void
disable_restart(void)
{
   if (TestGL31) {
#ifdef GL_VERSION_3_1
      glDisable(GL_PRIMITIVE_RESTART);
#endif
   }
   else {
      glDisableClientState(GL_PRIMITIVE_RESTART_NV);
   }
}


/**
 * Test glDrawElements() with glPrimitiveRestartIndexNV().
 */
static GLboolean
test_draw_elements(GLenum primMode, GLenum indexType)
{
#define NUM_VERTS 48
#define NUM_ELEMS (NUM_VERTS * 5 / 4)
   GLfloat verts[NUM_VERTS+2][2];
   GLuint elements[NUM_ELEMS];
   GLfloat x, dx;
   GLuint restart_index;
   GLuint num_elems;
   GLboolean pass;
   const char *typeStr = NULL, *primStr = NULL;

   switch (indexType) {
   case GL_UNSIGNED_BYTE:
      restart_index = 255;
      typeStr = "GL_UNSIGNED_BYTE";
      break;
   case GL_UNSIGNED_SHORT:
      restart_index = 1000;
      typeStr = "GL_UNSIGNED_SHORT";
      break;
   case GL_UNSIGNED_INT:
      restart_index = 1000 * 1000;
      typeStr = "GL_UNSIGNED_INT";
      break;
   default:
      assert(0);
   }

   x = 0.0;
   dx = 20.0;

   if (primMode == GL_TRIANGLE_STRIP) {
      GLuint i, j;
      const GLfloat y = 0.5 * piglit_height - 10.0, dy = 20.0;
      for (i = 0; i < NUM_VERTS / 2; i++) {
         verts[i*2+0][0] = x;
         verts[i*2+0][1] = y;
         verts[i*2+1][0] = x;
         verts[i*2+1][1] = y + dy;
         x += dx;
      }

      /* setup elements to draw series of squares w/ tri strip */
      for (i = j = 0; i < NUM_VERTS; i++) {
         elements[j++] = i;
         if (i > 0 && i % 4 == 3)
            elements[j++] = restart_index;
      }

      num_elems = j;
      primStr = "GL_TRIANGLE_STRIP";
   }
   else {
      GLuint i, j;
      const GLfloat y = 0.5 * piglit_height;

      assert(primMode == GL_LINE_STRIP);

      glLineWidth(5.0);

      for (i = 0; i < NUM_VERTS; i++) {
         verts[i][0] = x;
         verts[i][1] = y;
         x += dx;
      }

      /* setup elements to draw series of disjoint lines w/ line strip */
      for (i = j = 0; i < NUM_VERTS / 2; i++) {
         elements[j++] = i;
         if (i > 0 && i % 2 == 1)
            elements[j++] = restart_index;
      }

      num_elems = j;
      primStr = "GL_LINE_STRIP";
   }

   assert(num_elems <= NUM_ELEMS);

   /* debug */
   if (0) {
      GLint i;
      for (i = 0; i < num_elems; i++)
         printf("%2d: %d\n", i, elements[i]);
   }

   piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

   glClear(GL_COLOR_BUFFER_BIT);

   glColor4fv(green);

   glVertexPointer(2, GL_FLOAT, 2*sizeof(GLfloat), verts);
   glEnableClientState(GL_VERTEX_ARRAY);

   assert(glGetError()==0);
   enable_restart(restart_index);

   /* Draw */
   switch (indexType) {
   case GL_UNSIGNED_BYTE:
      {
         GLubyte ub_elements[NUM_ELEMS];
         int i;
         for (i = 0; i < num_elems; i++)
            ub_elements[i] = (GLubyte) elements[i];
         glDrawElements(primMode, num_elems, GL_UNSIGNED_BYTE, ub_elements);
      }
      break;
   case GL_UNSIGNED_SHORT:
      {
         GLushort us_elements[NUM_ELEMS];
         int i;
         for (i = 0; i < num_elems; i++)
            us_elements[i] = (GLushort) elements[i];
         glDrawElements(primMode, num_elems, GL_UNSIGNED_SHORT, us_elements);
      }
      break;
   case GL_UNSIGNED_INT:
      glDrawElements(primMode, num_elems, GL_UNSIGNED_INT, elements);
      break;
   default:
      assert(0);
   }

   disable_restart();
   glDisableClientState(GL_VERTEX_ARRAY);

   pass = check_rendering();
   if (!pass) {
      fprintf(stderr, "%s: failure drawing with glDrawElements(%s, %s)\n",
              TestName, primStr, typeStr);      
   }

   glutSwapBuffers();

   return pass;
#undef NUM_VERTS
}


/**
 * Test glDrawArrayss() with glPrimitiveRestartIndexNV().
 * We only test a line strip.
 */
static GLboolean
test_draw_arrays(void)
{
#define NUM_VERTS 12
   GLfloat verts[NUM_VERTS+2][2];
   GLfloat x, dx;
   GLuint restart_index;
   GLboolean pass = GL_TRUE;
   const char *primStr = "GL_LINE_STRIP";
   GLuint test;
   const GLenum primMode = GL_LINE_STRIP;

   x = 0.0;
   dx = 20.0;

   /* setup vertices */
   {
      GLuint i;
      const GLfloat y = 0.5 * piglit_height;

      glLineWidth(5.0);

      for (i = 0; i < NUM_VERTS; i++) {
         verts[i][0] = x;
         verts[i][1] = y;
         x += dx;
      }
   }

   piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

   glColor4fv(green);

   glVertexPointer(2, GL_FLOAT, 2*sizeof(GLfloat), verts);
   glEnableClientState(GL_VERTEX_ARRAY);

   assert(glGetError()==0);


   /*
    * Render and do checks.
    * Try three different restart indexes at start, end, middle.
    */
   for (test = 0; test < 3 && pass; test++) {
      /* choose the restart index */
      if (test == 0)
         restart_index = 0;
      else if (test == 1)
         restart_index = NUM_VERTS - 1;
      else
         restart_index = NUM_VERTS / 2;

      /* draw */
      glClear(GL_COLOR_BUFFER_BIT);
      enable_restart(restart_index);
      glDrawArrays(primMode, 0, NUM_VERTS);
      disable_restart();
      glutSwapBuffers();

      /* check */
      {
         const GLfloat x0 = 0.0, dx = 20.0;
         const GLint iy = piglit_height / 2;
         GLint i;

         /* probe at midpoint of each line segment */
         for (i = 0; i < NUM_VERTS - 1 && pass; i++) {
            const float fx = x0 + 0.5 * dx + i * dx;
            const int ix = (int) fx;

            /* read pixel */
            if (restart_index == i || restart_index == i + 1) {
               /* pixel should NOT be drawn here */
               if (!piglit_probe_pixel_rgb(ix, iy, black)) {
                  if (0)
                     fprintf(stderr, "bad pixel drawn\n");
                  pass = GL_FALSE;
               }
            }
            else {
               /* pixel should be drawn here */
               if (!piglit_probe_pixel_rgb(ix, iy, green)) {
                  if (0)
                     fprintf(stderr, "bad pixel drawn\n");
                  pass = GL_FALSE;
               }
            }
         }
      }
   }

   if (!pass) {
      fprintf(stderr, "%s: failure drawing with glDrawArrays(%s), "
              "restart index = %u\n",
              TestName, primStr, restart_index);
   }

   return pass;
}


enum piglit_result
piglit_display(void)
{
   GLboolean pass = GL_TRUE;

   if (Have_NV) {
      TestGL31 = GL_FALSE;
      pass = pass && test_begin_end(GL_TRIANGLE_STRIP);
      pass = pass && test_begin_end(GL_LINE_STRIP);
      pass = pass && test_draw_elements(GL_TRIANGLE_STRIP, GL_UNSIGNED_BYTE);
      pass = pass && test_draw_elements(GL_TRIANGLE_STRIP, GL_UNSIGNED_SHORT);
      pass = pass && test_draw_elements(GL_TRIANGLE_STRIP, GL_UNSIGNED_INT);
      pass = pass && test_draw_elements(GL_LINE_STRIP, GL_UNSIGNED_BYTE);
      pass = pass && test_draw_elements(GL_LINE_STRIP, GL_UNSIGNED_SHORT);
      pass = pass && test_draw_elements(GL_LINE_STRIP, GL_UNSIGNED_INT);
      pass = pass && test_draw_arrays();
   }

   if (Have_31) {
      TestGL31 = GL_TRUE;
      pass = pass && test_draw_elements(GL_TRIANGLE_STRIP, GL_UNSIGNED_BYTE);
      pass = pass && test_draw_elements(GL_TRIANGLE_STRIP, GL_UNSIGNED_SHORT);
      pass = pass && test_draw_elements(GL_TRIANGLE_STRIP, GL_UNSIGNED_INT);
      pass = pass && test_draw_elements(GL_LINE_STRIP, GL_UNSIGNED_BYTE);
      pass = pass && test_draw_elements(GL_LINE_STRIP, GL_UNSIGNED_SHORT);
      pass = pass && test_draw_elements(GL_LINE_STRIP, GL_UNSIGNED_INT);
      pass = pass && test_draw_arrays();
   }

   return pass ? PIGLIT_SUCCESS : PIGLIT_FAILURE;
}


void
piglit_init(int argc, char **argv)
{
   Have_NV = glewIsSupported("GL_NV_primitive_restart");
#ifdef GL_VERSION_3_1
   Have_31 = glewIsSupported("GL_VERSION_3_1");
#else
   Have_31 = GL_FALSE;
#endif

   /* Debug */
   /* NOTE!  glew 1.5.2's OpenGL 3.1 detection is broken.  You'll need
    * to upgrade to a newer version if you want to test the GL 3.1
    * primitive restart feature!
    */
   if (0) {
      printf("Have NV: %d\n", Have_NV);
      printf("Have 31: %d\n", Have_31);
   }

   if (!Have_NV && !Have_31) {
      piglit_report_result(PIGLIT_SKIP);
      exit(1);
   }

   glClearColor(0, 0, 0, 0);
}
