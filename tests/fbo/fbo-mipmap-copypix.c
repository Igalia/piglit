/*
 * Copyright (c) 2011 VMware, Inc.
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

/*
 * Test copying images between texture mipmap levels using FBOs.
 * Test glCopyPixels and glReadPixels+glDrawPixels.
 * Test with and without pixel transfer.
 * Test with same/different src/dest texture formats.
 *
 * Brian Paul
 * May 25, 2011
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

static const GLuint NumLevels = 10;

static const GLfloat colors[10][4] = {
   { 1, 0, 0, 1 },
   { 0, 1, 0, 1 },
   { 0, 0, 1, 1 },
   { 0, 1, 1, 1 },
   { 1, 0, 1, 1 },
   { 1, 1, 0, 1 },
   { 1, 1, 1, 1 },
   { .5, .5, .5, 1 },
   { 0, 1, .5, 1 },
   { .5, 0, 1, 1 }
};


static GLuint
create_texture(GLboolean fillInColors, GLenum intFormat)
{
   GLuint tex, level, size, i;
   GLfloat *image;

   glGenTextures(1, &tex);

   glBindTexture(GL_TEXTURE_2D, tex);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

   /* level 0 size */
   size = 1 << (NumLevels - 1);

   image = malloc(size * size * 4 * sizeof(GLfloat));

   if (!fillInColors) {
      /* Set image to black */
      memset(image, 0, size * size * 4 * sizeof(GLfloat));
   }

   for (level = 0; level < NumLevels; level++) {
      assert(size >= 1);

      if (fillInColors) {
         /* fill in src image with specific color */
         for (i = 0; i < size * size; i++) {
            image[i*4+0] = colors[level][0];
            image[i*4+1] = colors[level][1];
            image[i*4+2] = colors[level][2];
            image[i*4+3] = colors[level][3];
         }
      }

      glTexImage2D(GL_TEXTURE_2D, level, intFormat,
                   size, size, 0, GL_RGBA, GL_FLOAT, image);

      size /= 2;
   }

   free(image);

   return tex;
}


static GLboolean
test_mipmap_copypixels(GLenum srcIntFormat, GLenum dstIntFormat,
                       GLboolean doPixelTransfer, GLboolean useReadDrawPix)
{
   GLuint srcTex, dstTex;
   GLuint fboSrc, fboDst;
   GLuint level, size;
   GLboolean pass = GL_TRUE;
   GLenum status;

   if (doPixelTransfer) {
      glPixelTransferf(GL_ALPHA_SCALE, 0.0000001);
      glPixelTransferf(GL_ALPHA_BIAS, 1.0);
   }

   srcTex = create_texture(GL_TRUE, srcIntFormat);
   dstTex = create_texture(GL_FALSE, dstIntFormat);

   glGenFramebuffers(1, &fboSrc);
   glGenFramebuffers(1, &fboDst);

   size = 1 << (NumLevels - 1);

   for (level = 0; level < NumLevels; level++) {
      /* setup src */
      glBindFramebuffer(GL_READ_FRAMEBUFFER, fboSrc);
      glFramebufferTexture2D(GL_READ_FRAMEBUFFER,
                             GL_COLOR_ATTACHMENT0,
                             GL_TEXTURE_2D,
                             srcTex,
                             level);

      status = glCheckFramebufferStatus(GL_READ_FRAMEBUFFER);
      if (status != GL_FRAMEBUFFER_COMPLETE) {
         fprintf(stderr, "Source FBO incomplete for level %u (0x%x)\n",
                 level, status);
         pass = GL_FALSE;
         break;
      }

      /* setup dest */
      glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboDst);
      glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER,
                             GL_COLOR_ATTACHMENT0,
                             GL_TEXTURE_2D,
                             dstTex,
                             level);

      status = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
      if (status != GL_FRAMEBUFFER_COMPLETE) {
         fprintf(stderr, "Dest FBO incomplete for level %u (0x%x)\n",
                 level, status);
         pass = GL_FALSE;
         break;
      }

      glReadBuffer(GL_COLOR_ATTACHMENT0);
      glDrawBuffer(GL_COLOR_ATTACHMENT0);

      /* copy from src mipmap level to dest */
      if (useReadDrawPix) {
         GLubyte *tmp = (GLubyte *) malloc(size * size * 4 * sizeof(GLubyte));
         glReadPixels(0, 0, size, size, GL_RGBA, GL_UNSIGNED_BYTE, tmp);
         if (0)
            printf("copy color %u %u %u %u\n", tmp[0], tmp[1], tmp[2], tmp[3]);
         glDrawPixels(size, size, GL_RGBA, GL_UNSIGNED_BYTE, tmp);
         free(tmp);
      }
      else {
         glCopyPixels(0, 0, size, size, GL_COLOR);
      }

      size /= 2;
   }

   /* restore */
   glPixelTransferf(GL_ALPHA_SCALE, 1.0);
   glPixelTransferf(GL_ALPHA_BIAS, 0.0);

   glDeleteFramebuffers(1, &fboSrc);
   glDeleteFramebuffers(1, &fboDst);

   glBindFramebuffer(GL_FRAMEBUFFER, piglit_winsys_fbo);

   if (!pass)
      return GL_FALSE;

   /* Draw with dest texture and test the color */
   glBindTexture(GL_TEXTURE_2D, dstTex);
   glEnable(GL_TEXTURE_2D);
   glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

   size = 1 << (NumLevels - 1);

   for (level = 0; level < NumLevels; level++) {
      GLboolean p;

      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_LOD, level);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, level);

      glClear(GL_COLOR_BUFFER_BIT);

      piglit_draw_rect_tex(0, 0, piglit_width, piglit_height,
                           0.0, 0.0, 1.0, 1.0);

      p = piglit_probe_pixel_rgba(piglit_width/2, piglit_height/2, colors[level]);
      if (!p) {
         printf("  Mipmap level %d\n", level);
         if (useReadDrawPix)
            printf("  Using glRead/DrawPixels()\n");
         else
            printf("  Using glCopyPixels()\n");
         if (srcIntFormat == dstIntFormat)
            printf("  Matching src/dest texture formats\n");
         else
            printf("  Different src/dest texture formats\n");
         if (doPixelTransfer)
            printf("  With pixel transfer enabled\n");
         pass = GL_FALSE;
      }

      size /= 2;

      piglit_present_results();
   }

   glDisable(GL_TEXTURE_2D);

   glDeleteTextures(1, &srcTex);
   glDeleteTextures(1, &dstTex);

   return pass;
}


enum piglit_result
piglit_display(void)
{
   GLboolean pass = GL_TRUE;
   GLint formats, pixelTransfer, readDrawPix;

   for (formats = 0; formats < 2; formats++) {
      for (pixelTransfer = 0; pixelTransfer < 2; pixelTransfer++) {
         for (readDrawPix = 0; readDrawPix < 2; readDrawPix++) {
            GLenum srcFmt, dstFmt;

            if (formats == 0) {
               srcFmt = dstFmt = GL_RGBA;
            }
            else {
               srcFmt = GL_RGBA8;
               dstFmt = GL_RGB10_A2;
            }

            pass = test_mipmap_copypixels(srcFmt, dstFmt,
                                          pixelTransfer, readDrawPix) && pass;
         }
      }
   }

   return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


void
piglit_init(int argc, char **argv)
{
   piglit_require_extension("GL_ARB_framebuffer_object");
   piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);
   glClearColor(0.5, 0.5, 0.5, 0.5);
}
