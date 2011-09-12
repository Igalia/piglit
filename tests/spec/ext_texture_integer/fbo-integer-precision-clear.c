/*
 * Copyright (c) 2010 VMware, Inc.
 * Copyright (c) 2011 Dave Airlie
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
 * Tests FBO integer clearing with a value that is outside a float precision,
 * if any part of the stack does an int->float conversion this test will fail
 * also tests read/draw pixels paths.
 */


#include "piglit-util.h"

int piglit_width = 100, piglit_height = 100;
int piglit_window_mode = GLUT_RGB | GLUT_ALPHA | GLUT_DOUBLE;

static const char *TestName = "fbo-integer-precision-clear";

static GLint TexWidth = 256, TexHeight = 256;

struct format_info
{
   const char *Name;
   GLenum IntFormat, BaseFormat;
   GLuint BitsPerChannel;
   GLboolean Signed;
};

/* Only test 32-bit formats - since you won't see precision problems on lower sizes */
static const struct format_info Formats[] = {
   { "GL_RGBA32I_EXT",  GL_RGBA32I_EXT,  GL_RGBA_INTEGER_EXT, 32, GL_TRUE  },
   { "GL_RGBA32UI_EXT", GL_RGBA32UI_EXT, GL_RGBA_INTEGER_EXT, 32, GL_FALSE },

   { "GL_RGB32I_EXT",  GL_RGB32I_EXT,  GL_RGB_INTEGER_EXT, 32, GL_TRUE  },
   { "GL_RGB32UI_EXT", GL_RGB32UI_EXT, GL_RGB_INTEGER_EXT, 32, GL_FALSE },
};

#define NUM_FORMATS  (sizeof(Formats) / sizeof(Formats[0]))

/* For glDrawPixels */
static const char *PassthroughFragShaderText =
   "void main() \n"
   "{ \n"
   "   gl_FragColor = gl_Color; \n"
   "} \n";

static GLuint PassthroughFragShader, PassthroughProgram;



static int
get_max_val(const struct format_info *info)
{
   int max;

   switch (info->BitsPerChannel) {
   case 32:
      if (info->Signed)
         max = 300000000; /* don't use 0x8fffffff to avoid overflow issues */
      else
         max = 200000000;
      break;
   default:
      assert(0);
      max = 0;
   }

   return max;
}


static int
num_components(GLenum format)
{
   switch (format) {
   case GL_RGBA:
   case GL_RGBA_INTEGER_EXT:
      return 4;
   case GL_RGB_INTEGER_EXT:
      return 3;
   case GL_ALPHA_INTEGER_EXT:
      return 1;
   case GL_LUMINANCE_INTEGER_EXT:
      return 1;
   case GL_LUMINANCE_ALPHA_INTEGER_EXT:
      return 2;
   case GL_RED_INTEGER_EXT:
      return 1;
   default:
      assert(0);
      return 0;
   }
}


static GLenum
get_datatype(const struct format_info *info)
{
   switch (info->BitsPerChannel) {
   case 8:
      return info->Signed ? GL_BYTE : GL_UNSIGNED_BYTE;
   case 16:
      return info->Signed ? GL_SHORT : GL_UNSIGNED_SHORT;
   case 32:
      return info->Signed ? GL_INT : GL_UNSIGNED_INT;
   default:
      assert(0);
      return 0;
   }
}


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


/** \return GL_TRUE for pass, GL_FALSE for fail */
static GLboolean
test_fbo(const struct format_info *info)
{
   const int max = get_max_val(info);
   const int comps = num_components(info->BaseFormat);
   const GLenum type = get_datatype(info);
   GLint f;
   GLuint fbo, texObj;
   GLenum status;
   GLboolean intMode;
   GLint buf;

   if (0)
      fprintf(stderr, "============ Testing format = %s ========\n", info->Name);

   /* Create texture */
   glGenTextures(1, &texObj);
   glBindTexture(GL_TEXTURE_2D, texObj);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

   glTexImage2D(GL_TEXTURE_2D, 0, info->IntFormat, TexWidth, TexHeight, 0,
                info->BaseFormat, type, NULL);

   if (check_error(__FILE__, __LINE__))
      return GL_FALSE;

   glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &f);
   assert(f == info->IntFormat);


   /* Create FBO to render to texture */
   glGenFramebuffers(1, &fbo);
   glBindFramebuffer(GL_FRAMEBUFFER, fbo);
   glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
                          GL_TEXTURE_2D, texObj, 0);

   if (check_error(__FILE__, __LINE__))
      return GL_FALSE;

   status = glCheckFramebufferStatus(GL_FRAMEBUFFER_EXT);
   if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
      fprintf(stderr, "%s: failure: framebuffer incomplete.\n", TestName);
      return GL_FALSE;
   }


   glGetBooleanv(GL_RGBA_INTEGER_MODE_EXT, &intMode);
   if (check_error(__FILE__, __LINE__))
      return GL_FALSE;
   if (!intMode) {
      fprintf(stderr, "%s: GL_RGBA_INTEGER_MODE_EXT return GL_FALSE\n",
              TestName);
      return GL_FALSE;
   }

   glGetIntegerv(GL_READ_BUFFER, &buf);
   assert(buf == GL_COLOR_ATTACHMENT0_EXT);
   glGetIntegerv(GL_DRAW_BUFFER, &buf);
   assert(buf == GL_COLOR_ATTACHMENT0_EXT);


   /* test clearing */
   if (1) {
      static const GLint clr[4] = { 300000005, 7, 6, 5 };
      GLint pix[4], i;

      glClearColorIiEXT(clr[0], clr[1], clr[2], clr[3]);
      glClear(GL_COLOR_BUFFER_BIT);

      glReadPixels(5, 5, 1, 1, GL_RGBA_INTEGER_EXT, GL_INT, pix);

      for (i = 0; i < comps; i++) {
         if (pix[i] != clr[i]) {
            fprintf(stderr, "%s: glClear failed\n", TestName);
            fprintf(stderr, "  Texture format = %s\n", info->Name);
            fprintf(stderr, "  Expected %d, %d, %d, %d\n",
                    clr[0], clr[1], clr[2], clr[3]);
            fprintf(stderr, "  Found %d, %d, %d, %d\n",
                    pix[0], pix[1], pix[2], pix[3]);
         }
      }
   }


   /* Do glDraw/ReadPixels test */
   if (1) {
#define W 15
#define H 10
      GLint image[H * W * 4], readback[H * W * 4];
      GLint i;

      if (info->Signed) {
         for (i = 0; i < W * H * 4; i++) {
	    image[i] = ((i - 10) % max) + max;
         }
      }
      else {
         for (i = 0; i < W * H * 4; i++) {
	    image[i] = ((i + 3) % max) + max;
         }
      }

      glUseProgram(PassthroughProgram);

      glWindowPos2i(1, 1);
      glDrawPixels(W, H, GL_RGBA_INTEGER_EXT, GL_INT, image);

      if (check_error(__FILE__, __LINE__))
         return GL_FALSE;

      glReadPixels(1, 1, W, H, GL_RGBA_INTEGER_EXT, GL_INT, readback);

      if (check_error(__FILE__, __LINE__))
         return GL_FALSE;

      for (i = 0; i < W * H * 4; i++) {
         if (readback[i] != image[i]) {
            if (comps == 3 && i % 4 == 3 && readback[i] == 1)
               continue; /* alpha = 1 if base format == RGB */

            fprintf(stderr,
                 "%s: glDraw/ReadPixels failed at %d.  Expected %d, found %d\n",
                    TestName, i, image[i], readback[i]);
            fprintf(stderr, "Texture format = %s\n", info->Name);
            return GL_FALSE;
         }
      }
#undef W
#undef H
   }

   glutSwapBuffers();

   glDeleteTextures(1, &texObj);
   glDeleteFramebuffers(1, &fbo);

   return GL_TRUE;
}


enum piglit_result
piglit_display(void)
{
   int f;
   for (f = 0; f < NUM_FORMATS; f++) {
      GLboolean pass = test_fbo(&Formats[f]);
      if (!pass)
         return PIGLIT_FAIL;
   }
   return PIGLIT_PASS;
}


void
piglit_init(int argc, char **argv)
{
   bool es;
   int glslMajor, glslMinor;

   piglit_require_extension("GL_EXT_texture_integer");

   piglit_get_glsl_version(&es, &glslMajor, &glslMinor);
   if (glslMajor * 100 + glslMinor < 130) {
      printf("%s requires GLSL 1.30 or later\n", TestName);
      piglit_report_result(PIGLIT_SKIP);
      return;
   }

   PassthroughFragShader = piglit_compile_shader_text(GL_FRAGMENT_SHADER,
                                                      PassthroughFragShaderText);
   assert(PassthroughFragShader);
   PassthroughProgram = piglit_link_simple_program(0, PassthroughFragShader);

   (void) check_error(__FILE__, __LINE__);

   piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);
}
