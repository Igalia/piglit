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
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NON-INFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COYPRIGTH
 * HOLDERS AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/**
 * @file
 * Test drawing a textured triangle.  This is to test a Mesa/Gallium
 * LLVMpipe crash which only seems to happen when SSE4.1 is not used.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 400;
	config.window_height = 400;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static void
make_texture(void)
{
   GLubyte image[64][64][3];
   int i, j, size;

   for (i = 0; i < 64; i++) {
      for (j = 0; j < 64; j++) {
         image[i][j][0] = 255;
         image[i][j][1] = 0;
         image[i][j][2] = 0;
      }
   }

   glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

   i = 0;
   size = 64;
   while (size) {
      glTexImage2D(GL_TEXTURE_2D, i, GL_RGB, size, size, 0,
                   GL_RGB, GL_UNSIGNED_BYTE, image);

      size /= 2;
      i++;
   }

   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                   GL_NEAREST_MIPMAP_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                   GL_LINEAR);
   glEnable(GL_TEXTURE_2D);
}


enum piglit_result
piglit_display(void)
{
   /*
    * Simply draw a textured tri.  The texture is solid red.
    * The tri is clipped.
    */
   static const GLfloat v[3][4] = {
      { 10.0, 10.0, 0.0, 1.0 },
      { 10.0, 1.0, 0.0, 1.0 },
      { 1.0, 1.0, 0.0, 1.0 },
   };
   static const GLfloat t[3][2] = {
      { 0, 0 },
      { 1, 0 },
      { 1, 1 }
   };
   static const GLfloat red[4] = { 1, 0, 0, 1 };
   enum piglit_result result;

   make_texture();

   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   glBegin(GL_TRIANGLES);
   glTexCoord2fv(t[0]);   glVertex4fv(v[0]);
   glTexCoord2fv(t[1]);   glVertex4fv(v[1]);
   glTexCoord2fv(t[2]);   glVertex4fv(v[2]);
   glEnd();

   if (piglit_probe_pixel_rgb(piglit_width-5, piglit_height-20, red))
      result = PIGLIT_PASS;
   else
      result = PIGLIT_FAIL;

   piglit_present_results();

   return result;
}

void
piglit_init(int argc, char **argv)
{
   piglit_frustum_projection(GL_FALSE, -1.0, 1.0, -1.0, 1.0, 5.0, 50.0);
   glTranslatef(0.0, 0.0, -25.0);
}
