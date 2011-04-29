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
 * Tests GL_EXT_texture_integer texture formats (and GL_EXT_gpu_shader4).
 */

#include "piglit-util.h"

#define ELEMENTS(ARRAY)  (sizeof(ARRAY) / sizeof(ARRAY[0]))


int piglit_width = 100, piglit_height = 100;
int piglit_window_mode = GLUT_RGB | GLUT_ALPHA | GLUT_DOUBLE;

static const char *TestName = "texture-integer";

static GLint TexWidth = 16, TexHeight = 16;
static GLuint Texture;

static GLint BiasUniform = -1, TexUniform = -1;

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

   { "GL_ALPHA8I_EXT",   GL_ALPHA8I_EXT,   GL_ALPHA_INTEGER_EXT, 8,  GL_TRUE  },
   { "GL_ALPHA8UI_EXT",  GL_ALPHA8UI_EXT , GL_ALPHA_INTEGER_EXT, 8,  GL_FALSE },
   { "GL_ALPHA16I_EXT",  GL_ALPHA16I_EXT,  GL_ALPHA_INTEGER_EXT, 16, GL_TRUE  },
   { "GL_ALPHA16UI_EXT", GL_ALPHA16UI_EXT, GL_ALPHA_INTEGER_EXT, 16, GL_FALSE },
   { "GL_ALPHA32I_EXT",  GL_ALPHA32I_EXT,  GL_ALPHA_INTEGER_EXT, 32, GL_TRUE  },
   { "GL_ALPHA32UI_EXT", GL_ALPHA32UI_EXT, GL_ALPHA_INTEGER_EXT, 32, GL_FALSE },

   { "GL_LUMINANCE8I_EXT",   GL_LUMINANCE8I_EXT,   GL_LUMINANCE_INTEGER_EXT, 8,  GL_TRUE  },
   { "GL_LUMINANCE8UI_EXT",  GL_LUMINANCE8UI_EXT , GL_LUMINANCE_INTEGER_EXT, 8,  GL_FALSE },
   { "GL_LUMINANCE16I_EXT",  GL_LUMINANCE16I_EXT,  GL_LUMINANCE_INTEGER_EXT, 16, GL_TRUE  },
   { "GL_LUMINANCE16UI_EXT", GL_LUMINANCE16UI_EXT, GL_LUMINANCE_INTEGER_EXT, 16, GL_FALSE },
   { "GL_LUMINANCE32I_EXT",  GL_LUMINANCE32I_EXT,  GL_LUMINANCE_INTEGER_EXT, 32, GL_TRUE  },
   { "GL_LUMINANCE32UI_EXT", GL_LUMINANCE32UI_EXT, GL_LUMINANCE_INTEGER_EXT, 32, GL_FALSE },

   { "GL_LUMINANCE_ALPHA8I_EXT",   GL_LUMINANCE_ALPHA8I_EXT,   GL_LUMINANCE_ALPHA_INTEGER_EXT, 8,  GL_TRUE  },
   { "GL_LUMINANCE_ALPHA8UI_EXT",  GL_LUMINANCE_ALPHA8UI_EXT , GL_LUMINANCE_ALPHA_INTEGER_EXT, 8,  GL_FALSE },
   { "GL_LUMINANCE_ALPHA16I_EXT",  GL_LUMINANCE_ALPHA16I_EXT,  GL_LUMINANCE_ALPHA_INTEGER_EXT, 16, GL_TRUE  },
   { "GL_LUMINANCE_ALPHA16UI_EXT", GL_LUMINANCE_ALPHA16UI_EXT, GL_LUMINANCE_ALPHA_INTEGER_EXT, 16, GL_FALSE },
   { "GL_LUMINANCE_ALPHA32I_EXT",  GL_LUMINANCE_ALPHA32I_EXT,  GL_LUMINANCE_ALPHA_INTEGER_EXT, 32, GL_TRUE  },
   { "GL_LUMINANCE_ALPHA32UI_EXT", GL_LUMINANCE_ALPHA32UI_EXT, GL_LUMINANCE_ALPHA_INTEGER_EXT, 32, GL_FALSE },

   { "GL_INTENSITY8I_EXT",   GL_INTENSITY8I_EXT,   GL_RED_INTEGER_EXT, 8,  GL_TRUE  },
   { "GL_INTENSITY8UI_EXT",  GL_INTENSITY8UI_EXT , GL_RED_INTEGER_EXT, 8,  GL_FALSE },
   { "GL_INTENSITY16I_EXT",  GL_INTENSITY16I_EXT,  GL_RED_INTEGER_EXT, 16, GL_TRUE  },
   { "GL_INTENSITY16UI_EXT", GL_INTENSITY16UI_EXT, GL_RED_INTEGER_EXT, 16, GL_FALSE },
   { "GL_INTENSITY32I_EXT",  GL_INTENSITY32I_EXT,  GL_RED_INTEGER_EXT, 32, GL_TRUE  },
   { "GL_INTENSITY32UI_EXT", GL_INTENSITY32UI_EXT, GL_RED_INTEGER_EXT, 32, GL_FALSE },

};

#define NUM_FORMATS  (sizeof(Formats) / sizeof(Formats[0]))


static const char *FragShaderText =
#if 1
   "#extension GL_EXT_gpu_shader4: enable \n"
   "uniform vec4 bias; \n"
   "#if GL_EXT_gpu_shader4 \n"
   "  uniform isampler2D tex; \n"
   "#else \n"
   "  uniform sampler2D tex; \n"
   "#endif \n"
   "void main() \n"
   "{ \n"
   "#if GL_EXT_gpu_shader4 \n"
   "   vec4 t = vec4(texture2D(tex, gl_TexCoord[0].xy)); \n"
   "#else \n"
   "   vec4 t = texture2D(tex, gl_TexCoord[0].xy); \n"
   "#endif \n"
   "   gl_FragColor = t + bias; \n"
   "} \n";
#else
   /* XXX temporary shader for mesa testing */
   "uniform vec4 bias; \n"
   "  uniform sampler2D tex; \n"
   "void main() \n"
   "{ \n"
   "   vec4 t = vec4(texture2D(tex, gl_TexCoord[0].xy)); \n"
   "   gl_FragColor = t + bias; \n"
   "} \n";
#endif


static GLuint FragShader, Program;



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


static void
fill_array(int comps, int texels, void *buf, int bpp, const int val[4])
{
   int i, j;

   switch (bpp) {
   case 8:
      {
         GLubyte *b = (GLubyte *) buf;
         for (i = 0; i < texels; i++) {
            for (j = 0; j < comps; j++) {
               b[i * comps + j] = val[j];
            }
         }
      }
      break;
   case 16:
      {
         GLushort *b = (GLushort *) buf;
         for (i = 0; i < texels; i++) {
            for (j = 0; j < comps; j++) {
               b[i * comps + j] = val[j];
            }
         }
      }
      break;
   case 32:
      {
         GLuint *b = (GLuint *) buf;
         for (i = 0; i < texels; i++) {
            for (j = 0; j < comps; j++) {
               b[i * comps + j] = val[j];
            }
         }
      }
      break;
   default:
      assert(0);
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
test_format(const struct format_info *info)
{
   const int max = get_max_val(info);
   const int comps = num_components(info->BaseFormat);
   const int texels = TexWidth * TexHeight;
   const GLenum type = get_datatype(info);
   const int w = piglit_width / 10;
   const int h = piglit_height / 10;
   const float error = 2.0 / 255.0; /* XXX fix */
   GLfloat expected[4];
   void *buf;
   int value[4];
   GLfloat result[4], bias[4];
   GLint f;

   /* pick random texture color */
   value[0] = rand() % max;
   value[1] = rand() % max;
   value[2] = rand() % max;
   value[3] = rand() % max;

   /* alloc, fill texture image */
   buf = malloc(comps * texels * info->BitsPerChannel / 8);
   fill_array(comps, texels, buf, info->BitsPerChannel, value);

   glTexImage2D(GL_TEXTURE_2D, 0, info->IntFormat, TexWidth, TexHeight, 0,
                info->BaseFormat, type, buf);

   if (check_error(__FILE__, __LINE__))
      return GL_FALSE;

   glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &f);
   assert(f == info->IntFormat);

   /* setup expected polygon color */
   expected[0] = 0.25;
   expected[1] = 0.50;
   expected[2] = 0.75;
   expected[3] = 1.00;

   /* need to swizzle things depending on texture format */
   switch (info->BaseFormat) {
   case GL_RGBA_INTEGER_EXT:
      /* nothing */
      break;
   case GL_RGB_INTEGER_EXT:
      expected[3] = 0.0;
      break;
   case GL_ALPHA_INTEGER_EXT:
      expected[0] = expected[1] = expected[2] = 0.0;
      expected[3] = 0.25;
      value[3] = value[0];
      break;
   case GL_LUMINANCE_INTEGER_EXT:
      expected[0] = expected[1] = expected[2] = 0.25;
      expected[3] = 1.0;
      value[1] = value[2] = value[0];
      value[3] = 1.0;
      break;
   case GL_LUMINANCE_ALPHA_INTEGER_EXT:
      expected[0] = expected[1] = expected[2] = 0.25;
      value[3] = value[1];
      value[1] = value[2] = value[0];
      break;
   case GL_RED_INTEGER_EXT:
      expected[0] = expected[1] = expected[2] = expected[3] = 0.25;
      value[1] = value[2] = value[3] = value[0];
      break;
   default:
      ;
   }

   /* compute, set test bias */
   bias[0] = expected[0] - value[0];
   bias[1] = expected[1] - value[1];
   bias[2] = expected[2] - value[2];
   bias[3] = expected[3] - value[3];
   glUniform4fv(BiasUniform, 1, bias);

   /* draw */
   glClearColor(0, 1, 1, 0);
   glClear(GL_COLOR_BUFFER_BIT);
   glBegin(GL_POLYGON);
   glTexCoord2f(0, 0);   glVertex2f(0, 0);
   glTexCoord2f(1, 0);   glVertex2f(w, 0);
   glTexCoord2f(1, 1);   glVertex2f(w, h);
   glTexCoord2f(0, 1);   glVertex2f(0, h);
   glEnd();

   if (check_error(__FILE__, __LINE__))
      return GL_FALSE;

   /* test */
   glReadPixels(w/2, h/2, 1, 1, GL_RGBA, GL_FLOAT, result);

   if (check_error(__FILE__, __LINE__))
      return GL_FALSE;

   if (fabsf(result[0] - expected[0]) > error ||
       fabsf(result[1] - expected[1]) > error ||
       fabsf(result[2] - expected[2]) > error ||
       fabsf(result[3] - expected[3]) > error) {
      fprintf(stderr, "%s: failure with format %s:\n", TestName, info->Name);
      fprintf(stderr, "  texture color = %d, %d, %d, %d\n",
              value[0], value[1], value[2], value[3]);
      fprintf(stderr, "  expected color = %g, %g, %g, %g\n",
              expected[0], expected[1], expected[2], expected[3]);
      fprintf(stderr, "  result color = %g, %g, %g, %g\n",
              result[0], result[1], result[2], result[3]);
      return GL_FALSE;
   }

   glutSwapBuffers();

   free(buf);

   return GL_TRUE;
}


static GLboolean
test_general_formats(void)
{
   int f, i;

   for (f = 0; f < NUM_FORMATS; f++) {
      for (i = 0; i < 5; i++) {
         if (!test_format(&Formats[f]))
            return GL_FALSE;
      }
   }
   return GL_TRUE;
}


static GLboolean
test_specific_formats(void)
{
   /* These format combinations should all work */
   struct {
      GLenum intFormat, srcFormat, srcType;
   } formats[] = {
      { GL_RGBA8UI_EXT, GL_RGBA_INTEGER, GL_UNSIGNED_BYTE },
      { GL_RGBA8UI_EXT, GL_RGBA_INTEGER, GL_SHORT },
      { GL_RGBA8UI_EXT, GL_RGBA_INTEGER, GL_UNSIGNED_INT_8_8_8_8 },
      { GL_RGBA8UI_EXT, GL_BGRA_INTEGER, GL_UNSIGNED_INT_8_8_8_8 },
      { GL_LUMINANCE8I_EXT, GL_RGBA_INTEGER, GL_UNSIGNED_INT_8_8_8_8 },
      { GL_RGB16I_EXT, GL_RGB_INTEGER, GL_UNSIGNED_SHORT_5_6_5 },
      { GL_RGB32I_EXT, GL_RGB_INTEGER, GL_UNSIGNED_SHORT_5_6_5 }
   };
   int i;
   GLenum err;
   GLboolean pass = GL_TRUE;

   while (glGetError() != GL_NO_ERROR)
      ;

   for (i = 0; i < ELEMENTS(formats); i++) {
      glTexImage2D(GL_TEXTURE_2D, 0, formats[i].intFormat,
                   16, 16, 0,
                   formats[i].srcFormat, formats[i].srcType, NULL);
      err = glGetError();
      if (err != GL_NO_ERROR) {
         fprintf(stderr, "%s failure: glTexImage2D(0x%x, 0x%x, 0x%x) generated"
                 " error 0x%x (case %d)\n",
                 TestName, formats[i].intFormat,
                 formats[i].srcFormat, formats[i].srcType, err, i);
         pass = GL_FALSE;
      }
   }

   return pass;
}


/** check that an expected error is actually generated */
static GLboolean
verify_error(const char *func, GLenum error)
{
   GLenum err = glGetError();
   if (err != error) {
      fprintf(stderr, "%s: %s didn't generate '%s' error, found '%s'.\n",
              TestName, func, gluErrorString(error), gluErrorString(err));
      return GL_FALSE;
   }
   return GL_TRUE;
}


/** Test the various error conditions which are defined in the extension spec */
static GLboolean
test_errors(void)
{
   /* clear any prev errors */
   while (glGetError())
      ;

   /* use a new tex obj */
   glBindTexture(GL_TEXTURE_2D, 42);

   /* Check that GL_FLOAT type is not accepted with integer formats */
   {
      static const GLfloat pixel[4] = {0, 0, 0, 0};

      glDrawPixels(1, 1, GL_RGBA_INTEGER_EXT, GL_FLOAT, pixel);
      if (!verify_error("glDrawPixels", GL_INVALID_ENUM))
         return GL_FALSE;

      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16UI_EXT, 1, 1, 0,
                   GL_RGBA_INTEGER, GL_FLOAT, pixel);
      if (!verify_error("glTexImage2D", GL_INVALID_ENUM))
         return GL_FALSE;
   }

   /* Check that GL_INVALID_OPERATION is generated by trying to mix
    * integer/float formats/types.
    */
   {
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 4, 4, 0,
                   GL_RGBA_INTEGER, GL_SHORT, NULL);
      if (!verify_error("glTexImage2D", GL_INVALID_OPERATION))
         return GL_FALSE;

      glTexSubImage2D(GL_TEXTURE_2D, 0,
                      0, 0, 4, 4,
                      GL_RGBA_INTEGER, GL_FLOAT, NULL);
      if (!verify_error("glTexSubImage2D", GL_INVALID_OPERATION))
         return GL_FALSE;
   }

   /* Check for GL_INVALID_OPERATION when trying to copy framebuffer pixels
    * to an integer texture when the framebuffer is not an integer format.
    */
   {
      /* make valid texture image here */
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16UI_EXT, 4, 4, 0,
                   GL_RGBA_INTEGER, GL_UNSIGNED_SHORT, NULL);

      glCopyTexSubImage2D(GL_TEXTURE_2D, 0,
                          0, 0, 0, 0, 4, 4);
      if (!verify_error("glCopyTexSubImage2D", GL_INVALID_OPERATION))
         return GL_FALSE;
   }

   /* Is GL_INVALID_ENUM generated by glReadPixels? */
   {
      GLfloat buf[64];
      glReadPixels(0, 0, 4, 4, GL_RGBA_INTEGER, GL_FLOAT, buf);
      if (glGetError() != GL_INVALID_ENUM) {
         fprintf(stderr,
                 "%s: glReadPixels didn't generate GL_INVALID_ENUM\n",
                 TestName);
         return GL_FALSE;
      }
   }

   /* Is GL_INVALID_OPERATION generated by glReadPixels? */
   {
      GLuint buf[64];
      glReadPixels(0, 0, 4, 4, GL_RGBA_INTEGER, GL_UNSIGNED_INT, buf);
      if (!verify_error("glReadPixels", GL_INVALID_OPERATION))
         return GL_FALSE;
   }

   return GL_TRUE;
}


/** test some glGetInteger queries */
static GLboolean
test_limits(void)
{
   GLint val = 0;

   glGetIntegerv(GL_MIN_PROGRAM_TEXEL_OFFSET, &val);
   if (val > -8) {
      fprintf(stderr,
              "%s failure: query of GL_MIN_PROGRAM_TEXEL_OFFSET "
              "returned %d\n",
              TestName, val);
      return GL_FALSE;
   }

   glGetIntegerv(GL_MAX_PROGRAM_TEXEL_OFFSET, &val);
   if (val < 7) {
      fprintf(stderr,
              "%s failure: query of GL_MAX_PROGRAM_TEXEL_OFFSET "
              "returned %d\n",
              TestName, val);
      return GL_FALSE;
   }

   return GL_TRUE;
}



enum piglit_result
piglit_display(void)
{
   if (!test_general_formats())
      return PIGLIT_FAIL;

   if (!test_specific_formats())
      return PIGLIT_FAIL;

   if (!test_errors())
      return PIGLIT_FAIL;

   if (!test_limits())
      return PIGLIT_FAIL;

   return PIGLIT_PASS;
}


void
piglit_init(int argc, char **argv)
{
   piglit_require_extension("GL_EXT_texture_integer");
   piglit_require_extension("GL_EXT_gpu_shader4");

   FragShader = piglit_compile_shader_text(GL_FRAGMENT_SHADER, FragShaderText);
   assert(FragShader);

   Program = piglit_link_simple_program(0, FragShader);

   glUseProgram(Program);

   BiasUniform = glGetUniformLocation(Program, "bias");
   TexUniform = glGetUniformLocation(Program, "tex");

   glUniform1i(TexUniform, 0);  /* tex unit zero */

   (void) check_error(__FILE__, __LINE__);

   glGenTextures(1, &Texture);
   glBindTexture(GL_TEXTURE_2D, Texture);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

   (void) check_error(__FILE__, __LINE__);

   piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);
}
