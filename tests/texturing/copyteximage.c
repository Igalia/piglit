/*
 * Copyright 2011 VMware, Inc.
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
 * Basic test of glCopyTexImage() for various internal formats.
 */


#include "piglit-util.h"

#define TEX_SIZE 64

static const GLenum formats[] = { GL_RGBA, GL_LUMINANCE, GL_LUMINANCE_ALPHA, GL_INTENSITY };

#define NUM_FORMATS (sizeof(formats) / sizeof(formats[0])) 

int piglit_width = TEX_SIZE*(NUM_FORMATS+1), piglit_height = TEX_SIZE;
int piglit_window_mode = GLUT_DOUBLE | GLUT_RGB;


static GLboolean
test(void)
{
   static const GLfloat expected[NUM_FORMATS][2][4] = {
      /* GL_RGBA */
      {
         {0, 0, 0, 0},
         {1, 1, 1, 1}
      },
      /* GL_LUMINANCE */
      {
         {0, 0, 0, 0},
         {1, 1, 1, 1}
      },
      /* GL_LUMINANCE_ALPHA */
      {
         {0, 0, 0, 0},
         {1, 1, 1, 1}
      },
      /* GL_INTENSITY */
      {
         {0, 0, 0, 0},
         {1, 1, 1, 1}
      }
   };
   GLubyte buf[TEX_SIZE][TEX_SIZE][4];
   GLuint tex;
   GLboolean pass = GL_TRUE;
   int i, j;

   for (i = 0; i < TEX_SIZE; i++) {
      for (j = 0; j < TEX_SIZE; j++) {
         buf[i][j][0] = (i * 255) / (TEX_SIZE - 1);
         buf[i][j][1] = (j * 255) / (TEX_SIZE - 1);
         buf[i][j][2] = (buf[i][j][0] * buf[i][j][1]) / 255;
         buf[i][j][3] = 255;
      }
   }

   /* glDrawPixels the image on left */
   glWindowPos2iARB(0, 0);
   glDrawPixels(TEX_SIZE, TEX_SIZE, GL_RGBA, GL_UNSIGNED_BYTE, buf);

   glGenTextures(1, &tex);
   glBindTexture(GL_TEXTURE_2D, tex);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

   /* Do glCopyPixels and draw a texture quad for each format */
   for (i = 0; i < NUM_FORMATS; i++) {
      GLint x = TEX_SIZE * (i + 1);
      GLint y = 0;
      GLenum intFormat = formats[i];

      glCopyTexImage2D(GL_TEXTURE_2D, 0, intFormat, 0, 0, TEX_SIZE, TEX_SIZE, 0);
      if (glGetError()) {
         printf("Unexpected error for format 0x%x\n", intFormat);
         return GL_FALSE;
      }

      glViewport(x, y, TEX_SIZE, TEX_SIZE);

      glEnable(GL_TEXTURE_2D);
      glBegin(GL_POLYGON);
      glTexCoord2f(0, 0);
      glVertex2f(-1, -1);
      glTexCoord2f(1, 0);
      glVertex2f(1, -1);
      glTexCoord2f(1, 1);
      glVertex2f(1, 1);
      glTexCoord2f(0, 1);
      glVertex2f(-1, 1);
      glEnd();

      glDisable(GL_TEXTURE_2D);

      /* test lower-left pixel */
      if (!piglit_probe_pixel_rgb(x, y, expected[i][0]))
         pass = GL_FALSE;
      /* test upper-right pixel */
      if (!piglit_probe_pixel_rgb(x + TEX_SIZE - 1, y + TEX_SIZE - 1, expected[i][1]))
         pass = GL_FALSE;
   }

   glutSwapBuffers();

   return pass;
}


enum piglit_result
piglit_display(void)
{
   if (test())
      return PIGLIT_PASS;
   else
      return PIGLIT_FAIL;
}


void
piglit_init(int argc, char **argv)
{
   if ((piglit_get_gl_version() < 14) && !piglit_is_extension_supported("GL_ARB_window_pos")) {
	printf("Requires GL 1.4 or GL_ARB_window_pos");
	piglit_report_result(PIGLIT_SKIP);
   }
}
