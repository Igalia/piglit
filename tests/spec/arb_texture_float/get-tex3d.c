/*
 * Copyright (c) 2016 VMware, Inc.
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
 * Tests glGetTexImage() with float format.
 * This hits a rare pixel transfer patch in Mesa.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;
PIGLIT_GL_TEST_CONFIG_END


enum piglit_result
piglit_display(void)
{
	/* should not called */
	return PIGLIT_FAIL;
}


void
piglit_init(int argc, char **argv)
{
   GLuint texture;
   const unsigned w = 16, h = 8, d = 4;
   unsigned i;
   float *texData;
   bool pass = true;

   piglit_require_extension("GL_ARB_texture_float");

   glGenTextures(1, &texture);
   glBindTexture(GL_TEXTURE_3D, texture);
   glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

   /* Define 3D texture with float values */
   texData = malloc(4 * w * h * d * sizeof(float));
   for (i = 0; i < w * h * d * 4; i++) {
	   texData[i] = (rand() & 0xff) / 255.0f;
   }
   glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA32F, w, h, d, 0,
		GL_RGBA, GL_FLOAT, texData);

   /* Get 3D texture as GLfloat (no transfer ops) */
   {
      GLfloat *getData = malloc(4 * w * h * d * sizeof(GLfloat));
      glGetTexImage(GL_TEXTURE_3D, 0, GL_RGBA, GL_FLOAT, getData);
      for (i = 0; i < w * h * d * 4; i++) {
	      GLfloat expected = texData[i];
	      if (getData[i] != expected) {
		      printf("Expected float value %f, found %f at %u\n",
			     expected, getData[i], i);
		      pass = false;
		      break;
	      }
      }

      free(getData);
   }

   /* Get 3D texture as GLubyte */
   {
      GLubyte *getData = malloc(4 * w * h * d * sizeof(GLubyte));
      glGetTexImage(GL_TEXTURE_3D, 0, GL_RGBA, GL_UNSIGNED_BYTE, getData);
      for (i = 0; i < w * h * d * 4; i++) {
	      GLubyte expected = (GLubyte) (texData[i] * 255);
	      if (getData[i] != expected) {
		      printf("Expected ubyte value %u, found %u at %u\n",
			     expected, getData[i], i);
		      pass = false;
		      break;
	      }
      }

      free(getData);
   }

   free(texData);

   glDeleteTextures(1, &texture);

   piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
