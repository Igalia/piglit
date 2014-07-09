/*
 * Copyright (c) 2011 Red Hat Inc.
 * derived from texture-rg - Copyright (c) 2010 VMware, Inc. 
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
 * @file
 * Tests for a regression on r200 AL upload
 * https://bugs.freedesktop.org/show_bug.cgi?id=34280
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static const char *TestName = "texture-al";

static GLint TexWidth = 128, TexHeight = 128;

struct format_info
{
   const char *Name;
   GLenum IntFormat, BaseFormat;
  float expected0;
};


static const struct format_info IntFormats[] = {
   { "GL_ALPHA", GL_ALPHA, GL_ALPHA, 1.0 },
   { "GL_ALPHA_2", GL_ALPHA, GL_LUMINANCE_ALPHA, 1.0 },
   { "GL_LUMINANCE_ALPHA", GL_LUMINANCE_ALPHA, GL_LUMINANCE_ALPHA, 1.0 },
   { "GL_LUMINANCE_ALPHA_2", GL_LUMINANCE_ALPHA, GL_ALPHA, 0.0 },
};

#define NUM_INT_FORMATS  (sizeof(IntFormats) / sizeof(IntFormats[0]))


static GLboolean
check_error(const char *file, int line)
{
   GLenum err = glGetError();
   if (err) {
      fprintf(stderr, "%s: error 0x%x at %s:%d\n", TestName, err, file, line);
      return GL_TRUE;
   }
   return GL_FALSE;
}


static void
fill_texture_image(GLint w, GLint h, GLint comps, GLubyte *buf)
{
   GLint i, j, k;
   for (i = 0; i < h; i++) {
      for (j = 0; j < w; j++) {
         for (k = 0; k < comps; k++) {
            GLfloat val;
            if (k == 0) {
               /* left/right = red gradient */
               val = (int) (255 * j / (float) (w - 1));
            }
            else {
               /* up/down = green gradient */
               val = (int) (255 * i / (float) (h - 1));
            }
            *buf++ = val;
         }
      }
   }
}


static GLboolean
test_teximage_formats(void)
{
   GLint i;
   GLubyte *image;

   GLuint tex;

   glGenTextures(1, &tex);
   glBindTexture(GL_TEXTURE_2D, tex);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
   glEnable(GL_TEXTURE_2D);

   image = (GLubyte *) malloc(TexWidth * TexHeight * 2 * sizeof(GLubyte));

   for (i = 0; i < NUM_INT_FORMATS; i++) {
      const struct format_info *info = &IntFormats[i];
      const GLuint comps = (info->BaseFormat == GL_ALPHA) ? 1 : 2;
      GLfloat expected[4], result[4];
      GLfloat error = 2.0 / 255.0; /* XXX fix */

      fill_texture_image(TexWidth, TexHeight, comps, image);

      glTexImage2D(GL_TEXTURE_2D, 0, info->IntFormat,
                   TexWidth, TexHeight, 0,
                   info->BaseFormat, GL_UNSIGNED_BYTE, image);

      if (check_error(__FILE__, __LINE__)) {
         fprintf(stderr, "%s: Error in glTexImage2D for "
                 "internalFormat = %s\n", TestName, info->Name);
         return GL_FALSE;
      }

      if (0) {
         GLint f;
         glGetTexLevelParameteriv(GL_TEXTURE_2D, 0,
                                  GL_TEXTURE_INTERNAL_FORMAT, &f);
         assert(f == info->IntFormat);
      }

      glClear(GL_COLOR_BUFFER_BIT);
      glBegin(GL_POLYGON);
      glTexCoord2f(0, 0);   glVertex2f(0, 0);
      glTexCoord2f(1, 0);   glVertex2f(TexWidth, 0);
      glTexCoord2f(1, 1);   glVertex2f(TexWidth, TexHeight);
      glTexCoord2f(0, 1);   glVertex2f(0, TexHeight);
      glEnd();

      /* setup expected polygon color */
      expected[0] = info->expected0;
      expected[1] = 0.5;
      expected[2] = 0.0;
      expected[3] = 1.0;

      /* test center pixel */
      result[0] = result[1] = result[2] = 0.0;
      result[3] = 1.0;
      glReadPixels(TexWidth/2, TexHeight/2, 1, 1, GL_LUMINANCE_ALPHA, GL_FLOAT, result);

      if (check_error(__FILE__, __LINE__)) {
         fprintf(stderr, "%s: Error in glReadPixels(format = GL_LA)\n",
                 TestName);
         return GL_FALSE;
      }

      if (fabsf(result[0] - expected[0]) > error ||
          fabsf(result[1] - expected[1]) > error ||
          fabsf(result[2] - expected[2]) > error ||
          fabsf(result[3] - expected[3]) > error) {
         fprintf(stderr, "%s: failure with format %s:\n", TestName,
                 info->Name);
         fprintf(stderr, "  expected color = %g, %g, %g, %g\n",
                 expected[0], expected[1], expected[2], expected[3]);
         fprintf(stderr, "  result color = %g, %g, %g, %g\n",
                 result[0], result[1], result[2], result[3]);
         return GL_FALSE;
      }

      piglit_present_results();
   }

   free(image);

   glDisable(GL_TEXTURE_2D);

   return GL_TRUE;
}


static GLboolean
test_drawpixels_formats(void)
{
   GLubyte *image;
   GLfloat result[4], expected[4];
   GLfloat error = 2.0 / 255.0; /* XXX fix */

   image = (GLubyte *) malloc(TexWidth * TexHeight * 2 * sizeof(GLubyte));

   fill_texture_image(TexWidth, TexHeight, 2, image);

   glWindowPos2iARB(0, 0);
   glDrawPixels(TexWidth, TexHeight, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, image);

   if (check_error(__FILE__, __LINE__)) {
      fprintf(stderr, "%s: Error in glDrawPixels(format = GL_LA)\n",
              TestName);
      return GL_FALSE;
   }

   /* test center pixel */
   result[0] = result[1] = result[2] = 0.0;
   result[3] = 1.0;
   glReadPixels(TexWidth/2, TexHeight/2, 1, 1, GL_LUMINANCE_ALPHA, GL_FLOAT, result);
   
   expected[0] = 1.0;
   expected[1] = 0.5;
   expected[2] = 0.0;
   expected[3] = 1.0;

   if (fabsf(result[0] - expected[0]) > error ||
       fabsf(result[1] - expected[1]) > error ||
       fabsf(result[2] - expected[2]) > error ||
       fabsf(result[3] - expected[3]) > error) {
      fprintf(stderr, "%s: glDrawPixels failure with format GL_LA:\n", TestName);
      fprintf(stderr, "  expected color = %g, %g, %g, %g\n",
              expected[0], expected[1], expected[2], expected[3]);
      fprintf(stderr, "  result color = %g, %g, %g, %g\n",
              result[0], result[1], result[2], result[3]);
      return GL_FALSE;
   }


   free(image);

   return GL_TRUE;
}


enum piglit_result
piglit_display(void)
{
   if (test_teximage_formats() && test_drawpixels_formats())
      return PIGLIT_PASS;
   else
      return PIGLIT_FAIL;
}


void
piglit_init(int argc, char **argv)
{
   piglit_require_extension("GL_ARB_window_pos");
   piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);
}
