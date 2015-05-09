/*
 * Copyright (c) 2012 VMware, Inc.
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
 * Test glColorMaterial with glMaterial calls in a display list.
 * Used to test/fix a Mesa bug.
 *
 * Example: if glColorMaterial(GL_FRONT, GL_AMBIENT) is called and we
 * set the ambient material with glColor3f(green), then a call to
 * glMaterialfv(GL_FRONT, GL_AMBIENT, red) (in a display list) should
 * be a no-op.
 */


#include <assert.h>
#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

/**
 * Test glMaterial handling in a display list for one of GL_AMBIENT,
 * GL_DIFFUSE or GL_SPECULAR.
 */
bool
test_material_coef(GLenum coef)
{
   static const GLfloat black[4] = {0, 0, 0, 0};
   static const GLfloat white[4] = {1, 1, 1, 1};
   static const GLfloat red[4] = {1, 0, 0, 1};
   static const GLfloat green[4] = {0, 1, 0, 1};
   bool result;

   assert(coef == GL_AMBIENT ||
          coef == GL_DIFFUSE ||
          coef == GL_SPECULAR);

   glDisable(GL_COLOR_MATERIAL);

   /* set all mat coefs to black */
   glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, black);
   glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, black);
   glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, black);

   /* set all light coefs to black */
   glLightfv(GL_LIGHT0, GL_AMBIENT, black);
   glLightfv(GL_LIGHT0, GL_DIFFUSE, black);
   glLightfv(GL_LIGHT0, GL_SPECULAR, black);

   /* Now test the coefficient of interest */
   glLightfv(GL_LIGHT0, coef, white);  /* white light */
   glEnable(GL_COLOR_MATERIAL);
   glColorMaterial(GL_FRONT_AND_BACK, coef);

   /* Set the material coef via glColor - this is what we want to see */
   glColor4fv(green);

   /* This glMaterial setting should be ignored since glColorMaterial says
    * that glColor overrides the latched material.
    */
   glNewList(1, GL_COMPILE);
   glMaterialfv(GL_FRONT_AND_BACK, coef, red);
   glEndList();
   glCallList(1);

   /* draw tri (should be green, not red) */
   glClear(GL_COLOR_BUFFER_BIT);
   glBegin(GL_TRIANGLES);
   glNormal3f(0, 0, 1);
   glVertex2f(-1, -1);
   glVertex2f( 1, -1);
   glVertex2f( 0,  1);
   glEnd();

   result = piglit_probe_pixel_rgb(piglit_width / 2, piglit_height / 2, green);

   /* also query the material coef and check it */
   {
      GLfloat mat[4];
      glGetMaterialfv(GL_FRONT, coef, mat);
      if (mat[0] != green[0] ||
          mat[1] != green[1] ||
          mat[2] != green[2]) {
         printf("glGetMaterial failed."
                "  Expected (%g, %g, %g, %g)  Found (%g, %g, %g, %g)\n",
                green[0], green[1], green[2], green[3],
                mat[0], mat[1], mat[2], mat[3]);
         result = false;
      }
   }

   piglit_present_results();

   return result;
}


enum piglit_result
piglit_display(void)
{
   if (!test_material_coef(GL_AMBIENT))
      return PIGLIT_FAIL;
   if (!test_material_coef(GL_DIFFUSE))
      return PIGLIT_FAIL;
   if (!test_material_coef(GL_SPECULAR))
      return PIGLIT_FAIL;

   return PIGLIT_PASS;
}


void
piglit_init(int argc, char **argv)
{
   glEnable(GL_LIGHTING);
   glEnable(GL_LIGHT0);
}
