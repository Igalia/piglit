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
 * NON-INFRINGEMENT.  IN NO EVENT SHALL VMWARE AND/OR THEIR SUPPLIERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/**
 * @file
 * Tests FBO rendering with GL_EXT_texture_integer and GL_EXT_gpu_shader4.
 */


#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static const char *TestName = "texture-integer";

static GLint TexWidth = 256, TexHeight = 256;

struct format_info
{
   const char *Name;
   GLenum IntFormat, BaseFormat;
   GLuint BitsPerChannel;
   GLboolean Signed;
};


static const struct format_info Formats[] = {
   /*   { "GL_RGBA", GL_RGBA, GL_RGBA, 8, GL_FALSE },*/
   { "GL_RGBA8I_EXT",   GL_RGBA8I_EXT,   GL_RGBA_INTEGER_EXT, 8,  GL_TRUE  },
   { "GL_RGBA8UI_EXT",  GL_RGBA8UI_EXT , GL_RGBA_INTEGER_EXT, 8,  GL_FALSE },
   { "GL_RGBA16I_EXT",  GL_RGBA16I_EXT,  GL_RGBA_INTEGER_EXT, 16, GL_TRUE  },
   { "GL_RGBA16UI_EXT", GL_RGBA16UI_EXT, GL_RGBA_INTEGER_EXT, 16, GL_FALSE },
   { "GL_RGBA32I_EXT",  GL_RGBA32I_EXT,  GL_RGBA_INTEGER_EXT, 32, GL_TRUE  },
   { "GL_RGBA32UI_EXT", GL_RGBA32UI_EXT, GL_RGBA_INTEGER_EXT, 32, GL_FALSE },

   { "GL_RGB8I_EXT",   GL_RGB8I_EXT,   GL_RGB_INTEGER_EXT, 8,  GL_TRUE  },
   { "GL_RGB8UI_EXT",  GL_RGB8UI_EXT , GL_RGB_INTEGER_EXT, 8,  GL_FALSE },
   { "GL_RGB16I_EXT",  GL_RGB16I_EXT,  GL_RGB_INTEGER_EXT, 16, GL_TRUE  },
   { "GL_RGB16UI_EXT", GL_RGB16UI_EXT, GL_RGB_INTEGER_EXT, 16, GL_FALSE },
   { "GL_RGB32I_EXT",  GL_RGB32I_EXT,  GL_RGB_INTEGER_EXT, 32, GL_TRUE  },
   { "GL_RGB32UI_EXT", GL_RGB32UI_EXT, GL_RGB_INTEGER_EXT, 32, GL_FALSE },
};

#define NUM_FORMATS  (sizeof(Formats) / sizeof(Formats[0]))

/* Need to declare an ivec4-valued output variable for rendering to
 * an integer-valued color buffer.
 */
static const char *SimpleFragShaderText =
   "#version 130 \n"
   "#extension GL_EXT_gpu_shader4: enable \n"
   "uniform ivec4 value; \n"
   "out ivec4 out_color; \n"
   "void main() \n"
   "{ \n"
   "   out_color = value; \n"
   "} \n";

static GLuint SimpleFragShader, SimpleProgram;


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
   case 8:
      if (info->Signed)
         max = 127;
      else
         max = 255;
      break;
   case 16:
      if (info->Signed)
         max = 32767;
      else
         max = 65535;
      break;
   case 32:
      if (info->Signed)
         max = 10*1000; /* don't use 0x8fffffff to avoid overflow issues */
      else
         max = 20*1000;
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
      static const GLint clr[4] = { 8, 7, 6, 5 };
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
            return GL_FALSE;
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
            image[i] = (i - 10) % max;
            assert(image[i] < max);
         }
      }
      else {
         for (i = 0; i < W * H * 4; i++) {
            image[i] = (i + 3) % max;
            assert(image[i] < max);
         }
      }

      glUseProgram(PassthroughProgram);
      if(0)glUseProgram(SimpleProgram);

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
            assert(0);
            return GL_FALSE;
         }
      }
#undef W
#undef H
   }

   /* Do rendering test */
   if (1) {
      GLint value[4], result[4], loc, w = piglit_width, h = piglit_height;
      GLint error = 1; /* XXX fix */

      /* choose random value/color for polygon */
      value[0] = rand() % 100;
      value[1] = rand() % 100;
      value[2] = rand() % 100;
      value[3] = rand() % 100;

      glUseProgram(SimpleProgram);
      check_error(__FILE__, __LINE__);

      loc = glGetUniformLocation(SimpleProgram, "value");
      assert(loc >= 0);
      glUniform4iv(loc, 1, value);
      check_error(__FILE__, __LINE__);

#if 0 /* allow testing on mesa until this is implemented */
      loc = glGetFragDataLocationEXT(SimpleProgram, "out_color");
      assert(loc >= 0);
#endif

      glBegin(GL_POLYGON);
      glVertex2f(0, 0);
      glVertex2f(w, 0);
      glVertex2f(w, h);
      glVertex2f(0, h);
      glEnd();
      check_error(__FILE__, __LINE__);

      glReadPixels(w/2, h/2, 1, 1, GL_RGBA_INTEGER, GL_INT, result);
      check_error(__FILE__, __LINE__);

      if (info->BaseFormat == GL_RGB_INTEGER_EXT) {
         value[3] = 1;
      }

      if (abs(result[0] - value[0]) > error ||
          abs(result[1] - value[1]) > error ||
          abs(result[2] - value[2]) > error ||
          abs(result[3] - value[3]) > error) {
         fprintf(stderr, "%s: failure with format %s:\n", TestName, info->Name);
         fprintf(stderr, "  input value = %d, %d, %d, %d\n",
                 value[0], value[1], value[2], value[3]);
         fprintf(stderr, "  result color = %d, %d, %d, %d\n",
                 result[0], result[1], result[2], result[3]);
         return GL_FALSE;
      }
   }

   piglit_present_results();

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
   piglit_require_extension("GL_ARB_framebuffer_object");
   piglit_require_extension("GL_EXT_texture_integer");
   piglit_require_extension("GL_EXT_gpu_shader4");

   piglit_require_GLSL_version(130);

   PassthroughFragShader = piglit_compile_shader_text(GL_FRAGMENT_SHADER,
                                                      PassthroughFragShaderText);
   assert(PassthroughFragShader);
   PassthroughProgram = piglit_link_simple_program(0, PassthroughFragShader);


   SimpleFragShader = piglit_compile_shader_text(GL_FRAGMENT_SHADER,
                                                 SimpleFragShaderText);
   assert(SimpleFragShader);
   SimpleProgram = piglit_link_simple_program(0, SimpleFragShader);


   (void) check_error(__FILE__, __LINE__);

   piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);
}
