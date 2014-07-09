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
 * Tests FBO integer clearing with a value that is signed or unsigned
 * the reads back using the other type.
 * This checks that the signed->unsigned and unsigned->signed conversions
 * in the RP path are done correctly.
 */


#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static const char *TestName = "fbo-integer-readpixels-sint-uint";

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
   { "GL_RGBA32I_EXT",  GL_RGBA32I_EXT,  GL_RGBA_INTEGER_EXT, 32, GL_TRUE },
   { "GL_RGBA32UI_EXT", GL_RGBA32UI_EXT, GL_RGBA_INTEGER_EXT, 32, GL_FALSE },
};

#define NUM_FORMATS  (sizeof(Formats) / sizeof(Formats[0]))

/* For glDrawPixels */
static const char *PassthroughFragShaderText =
   "void main() \n"
   "{ \n"
   "   gl_FragColor = gl_Color; \n"
   "} \n";

static GLuint PassthroughProgram;


#if 0
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
#endif

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
static bool
test_fbo(const struct format_info *info)
{
   const int comps = num_components(info->BaseFormat);
   const GLenum type = get_datatype(info);
   GLint f;
   GLuint fbo, texObj;
   GLenum status;
   GLboolean intMode;
   GLint buf;
   bool pass = true;

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
      /* clear with an integer - exp unsigned int */
      static const GLint clr_i[4] = { 300000005, -7, 6, 5 };
      static const GLuint exp_ui[4] = { 300000005, 0, 6, 5 };
      /* clear with an unsigned integer - exp int */
      static const GLuint clr_ui[4] = { 300000005, 0x80000007, 6, 5 };
      static const GLint exp_i[4] = { 300000005, 0x7fffffff, 6, 5 };
      GLint pix[4], i;
      GLuint pix_ui[4];

      if (info->Signed)
          glClearColorIiEXT(clr_i[0], clr_i[1], clr_i[2], clr_i[3]);
      else
          glClearColorIuiEXT(clr_ui[0], clr_ui[1], clr_ui[2], clr_ui[3]);
      glClear(GL_COLOR_BUFFER_BIT);

      if (info->Signed)
          glReadPixels(5, 5, 1, 1, GL_RGBA_INTEGER_EXT, GL_UNSIGNED_INT, pix_ui);
      else
          glReadPixels(5, 5, 1, 1, GL_RGBA_INTEGER_EXT, GL_INT, pix);

      if (info->Signed) {
          for (i = 0; i < comps; i++) {
               if (pix_ui[i] != exp_ui[i]) {
                   fprintf(stderr, "%s: glClear failed\n", TestName);
                   fprintf(stderr, "  Texture format = %s\n", info->Name);
                   fprintf(stderr, "  Expected %u, %u, %u, %u\n",
                        exp_ui[0], exp_ui[1], exp_ui[2], exp_ui[3]);
                   fprintf(stderr, "  Found %u, %u, %u, %u\n",
                        pix_ui[0], pix_ui[1], pix_ui[2], pix_ui[3]);
		   pass = false;
		   break;
               }
          }
      } else {
          for (i = 0; i < comps; i++) {
               if (pix[i] != exp_i[i]) {
                   fprintf(stderr, "%s: glClear failed\n", TestName);
                   fprintf(stderr, "  Texture format = %s\n", info->Name);
                   fprintf(stderr, "  Expected %d, %d, %d, %d\n",
                        exp_i[0], exp_i[1], exp_i[2], exp_i[3]);
                   fprintf(stderr, "  Found %d, %d, %d, %d\n",
                        pix[0], pix[1], pix[2], pix[3]);
		   pass = false;
		   break;
               }
          }
      }
   }

   piglit_present_results();

   glDeleteTextures(1, &texObj);
   glDeleteFramebuffers(1, &fbo);

   return pass;
}


enum piglit_result
piglit_display(void)
{
   int f;
   for (f = 0; f < NUM_FORMATS; f++) {
      bool pass = test_fbo(&Formats[f]);
      if (!pass)
         return PIGLIT_FAIL;
   }
   return PIGLIT_PASS;
}


void
piglit_init(int argc, char **argv)
{
   piglit_require_extension("GL_EXT_texture_integer");
   piglit_require_GLSL_version(130);

   PassthroughProgram =
	   piglit_build_simple_program(NULL, PassthroughFragShaderText);

   (void) check_error(__FILE__, __LINE__);

   piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);
}
