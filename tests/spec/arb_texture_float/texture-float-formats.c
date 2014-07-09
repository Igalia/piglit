/*
 * Copyright (c) 2011 VMware, Inc.
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
 * Tests GL_ARB_texture_float floating point formats
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static const char *TestName = "texture-float-formats";
static GLint TexWidth = 16, TexHeight = 16;
static GLint BiasUniform = -1, ScaleUniform = -1, TexUniform = -1;
static const float Scale = 1.0 / 2000.0, Bias = 0.5;
static GLboolean HaveRG;


struct format_info
{
   const char *Name;
   GLenum IntFormat, BaseFormat;
   GLuint BitsPerChannel;
};


static const struct format_info Formats[] = {
   { "GL_RGBA32F_ARB", GL_RGBA32F_ARB, GL_RGBA, 32 },
   { "GL_RGB32F_ARB", GL_RGB32F_ARB, GL_RGB, 32 },
   { "GL_ALPHA32F_ARB", GL_ALPHA32F_ARB, GL_ALPHA, 32 },
   { "GL_INTENSITY32F_ARB", GL_INTENSITY32F_ARB, GL_INTENSITY, 32 },
   { "GL_LUMINANCE32F_ARB", GL_LUMINANCE32F_ARB, GL_LUMINANCE, 32 },
   { "GL_LUMINANCE_ALPHA32F_ARB", GL_LUMINANCE_ALPHA32F_ARB, GL_LUMINANCE, 32 },

   { "GL_RGBA16F_ARB", GL_RGBA16F_ARB, GL_RGBA, 16 },
   { "GL_RGB16F_ARB", GL_RGB16F_ARB, GL_RGB, 16 },
   { "GL_ALPHA16F_ARB", GL_ALPHA16F_ARB, GL_ALPHA, 16 },
   { "GL_INTENSITY16F_ARB", GL_INTENSITY16F_ARB, GL_INTENSITY, 16 },
   { "GL_LUMINANCE16F_ARB", GL_LUMINANCE16F_ARB, GL_LUMINANCE, 16 },
   { "GL_LUMINANCE_ALPHA16F_ARB", GL_LUMINANCE_ALPHA16F_ARB, GL_LUMINANCE, 16 },

   /* These require GL_ARB_texture_rg */
   { "GL_R32F", GL_R32F, GL_RED, 32 },
   { "GL_RG32F", GL_RG32F, GL_RG, 32 },
   { "GL_R16F", GL_R16F, GL_RED, 16 },
   { "GL_RG16F", GL_RG16F, GL_RG, 16 },
};


static const char *FragShaderText =
   "uniform float bias, scale; \n"
   "uniform sampler2D tex; \n"
   "void main() \n"
   "{ \n"
   "   vec4 t = vec4(texture2D(tex, gl_TexCoord[0].xy)); \n"
   "   gl_FragColor = t * scale + bias; \n"
   "} \n";


static GLuint Program;



static int
num_components(GLenum format)
{
   switch (format) {
   case GL_RGBA:
      return 4;
   case GL_RGB:
      return 3;
   case GL_ALPHA:
      return 1;
   case GL_INTENSITY:
      return 1;
   case GL_LUMINANCE:
      return 1;
   case GL_LUMINANCE_ALPHA:
      return 2;
   case GL_RED:
      return 1;
   case GL_RG:
      return 2;
   default:
      assert(0);
      return 0;
   }
}


static void
fill_array(int comps, int texels, void *buf, const float val[4])
{
   GLfloat *f = (GLfloat *) buf;
   int i, j;

   for (i = 0; i < texels; i++) {
      for (j = 0; j < comps; j++) {
         f[i * comps + j] = val[j];
      }
   }
}


static GLboolean
check_error(const char *file, int line)
{
   GLenum err = glGetError();
   if (err) {
      printf("%s: error 0x%x at %s:%d\n", TestName, err, file, line);
      return GL_TRUE;
   }
   return GL_FALSE;
}


/** Scale a float in [-1000, 1000] to [0, 1] */
static float
scale_and_bias(float val)
{
   return val * Scale + Bias;
}


/**
 * Get a color to use for filling the texture image.
 * Range of values is [-1000, 1000]
 */
static void
get_texture_color(GLfloat value[4])
{
   static const GLfloat colors[12][4] = {
      { 690.0, 633.0, -649.0, -975.0 },
      { 409.0, -678.0, 624.0, -976.0 },
      { -460.0, -102.0, -983.0, 120.0 },
      { 202.0, 75.0, 826.0, -339.0 },
      { -709.0, 620.0, 204.0, -666.0 },
      { 718.0, -299.0, 290.0, 383.0 },
      { 634.0, 235.0, 571.0, -651.0 },
      { -984.0, -99.0, 448.0, 263.0 },
      { -466.0, 356.0, -155.0, 500.0 },
      { 678.0, -531.0, 81.0, -783.0 },
      { -76.0, 98.0, -106.0, -875.0 },
      { 730.0, -723.0, -656.0, -980.0 }
   };
   static int i = 0;
   value[0] = colors[i][0];
   value[1] = colors[i][1];
   value[2] = colors[i][2];
   value[3] = colors[i][3];
   i = (i + 1) % 12;
}


/** \return GL_TRUE for pass, GL_FALSE for fail */
static GLboolean
test_format(const struct format_info *info)
{
   const int comps = num_components(info->BaseFormat);
   const int texels = TexWidth * TexHeight;
   const int w = piglit_width / 10;
   const int h = piglit_height / 10;
   GLfloat expected[4];
   void *image;
   float value[4];
   GLint f;
   GLenum userFormat;
   int p;

   if ((info->BaseFormat == GL_RED ||
        info->BaseFormat == GL_RG) && !HaveRG) {
      /* skip it */
      return GL_TRUE;
   }

   /*printf("Testing %s\n", info->Name);*/

   get_texture_color(value);

   /* alloc, fill texture image */
   image = malloc(comps * texels * sizeof(GLfloat));
   fill_array(comps, texels, image, value);

   /* GL_INTENSITY is not a legal src format */
   userFormat = info->BaseFormat == GL_INTENSITY ? GL_LUMINANCE : info->BaseFormat;

   glTexImage2D(GL_TEXTURE_2D, 0, info->IntFormat, TexWidth, TexHeight, 0,
                userFormat, GL_FLOAT, image);
   free(image);

   if (check_error(__FILE__, __LINE__))
      return GL_FALSE;

   /* check internal format */
   glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &f);
   if (f != info->IntFormat) {
      printf("%s: GL_TEXTURE_INTERNAL_FORMAT query failed for 0x%x\n",
             TestName,
             info->IntFormat);
      return GL_FALSE;
   }

   /* check texture datatype info */
   {
      static const GLenum queries[] = {
         GL_TEXTURE_RED_TYPE_ARB,
         GL_TEXTURE_GREEN_TYPE_ARB,
         GL_TEXTURE_BLUE_TYPE_ARB,
         GL_TEXTURE_ALPHA_TYPE_ARB,
         GL_TEXTURE_LUMINANCE_TYPE_ARB,
         GL_TEXTURE_INTENSITY_TYPE_ARB,
         GL_TEXTURE_DEPTH_TYPE_ARB
      };
      static const char *queryNames[] = {
         "GL_TEXTURE_RED_TYPE_ARB",
         "GL_TEXTURE_GREEN_TYPE_ARB",
         "GL_TEXTURE_BLUE_TYPE_ARB",
         "GL_TEXTURE_ALPHA_TYPE_ARB",
         "GL_TEXTURE_LUMINANCE_TYPE_ARB",
         "GL_TEXTURE_INTENSITY_TYPE_ARB",
         "GL_TEXTURE_DEPTH_TYPE_ARB"
      };
      int i;
      for (i = 0; i < ARRAY_SIZE(queries); i++) {
         GLint type = 1;
         glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, queries[i], &type);
         if (check_error(__FILE__, __LINE__))
            return GL_FALSE;
         if (type != GL_NONE && type != GL_FLOAT) {
            printf("%s: %s query failed (returned 0x%x)\n",
                   TestName, queryNames[i], type);
            return GL_FALSE;
         }
      }
   }

   /* compute expected color */
   switch (info->BaseFormat) {
   case GL_RGBA:
      expected[0] = scale_and_bias(value[0]);
      expected[1] = scale_and_bias(value[1]);
      expected[2] = scale_and_bias(value[2]);
      expected[3] = scale_and_bias(value[3]);
      break;
   case GL_RGB:
      expected[0] = scale_and_bias(value[0]);
      expected[1] = scale_and_bias(value[1]);
      expected[2] = scale_and_bias(value[2]);
      expected[3] = scale_and_bias(1.0);
      break;
   case GL_ALPHA:
      expected[0] =
      expected[1] =
      expected[2] = scale_and_bias(0.0);
      expected[3] = scale_and_bias(value[0]);
      break;
   case GL_LUMINANCE:
      expected[0] =
      expected[1] =
      expected[2] = scale_and_bias(value[0]);
      expected[3] = scale_and_bias(1.0);
      break;
   case GL_INTENSITY:
      expected[0] =
      expected[1] =
      expected[2] = 
      expected[3] = scale_and_bias(value[0]);
      break;
   case GL_LUMINANCE_ALPHA:
      expected[0] =
      expected[1] =
      expected[2] = scale_and_bias(value[0]);
      expected[3] = scale_and_bias(value[1]);
      break;
   case GL_RED:
      expected[0] = scale_and_bias(value[0]);
      expected[1] = scale_and_bias(0.0);
      expected[2] = scale_and_bias(0.0);
      expected[3] = scale_and_bias(1.0);
      break;
   case GL_RG:
      expected[0] = scale_and_bias(value[0]);
      expected[1] = scale_and_bias(value[1]);
      expected[2] = scale_and_bias(0.0);
      expected[3] = scale_and_bias(1.0);
      break;
   default:
      abort();
   }

   /* draw */
   glClearColor(0.5, 0.5, 0.5, 0.0);
   glClear(GL_COLOR_BUFFER_BIT);
   glBegin(GL_POLYGON);
   glTexCoord2f(0, 0);   glVertex2f(0, 0);
   glTexCoord2f(1, 0);   glVertex2f(w, 0);
   glTexCoord2f(1, 1);   glVertex2f(w, h);
   glTexCoord2f(0, 1);   glVertex2f(0, h);
   glEnd();

   /* test */
   p = piglit_probe_pixel_rgba(w/2, h/2, expected);
   if (!p) {
      int i;

      printf("  Failed with format %s:\n", info->Name);
      printf("  Texture color = ");
      for (i = 0; i < comps; i++) {
         printf("%f", value[i]);
         if (i + 1 < comps)
            printf(", ");
      }
      printf("\n");
   }

   piglit_present_results();

   return p;
}


enum piglit_result
piglit_display(void)
{
   int f;
   GLboolean pass = GL_TRUE;

   for (f = 0; f < ARRAY_SIZE(Formats); f++)
      if (!test_format(&Formats[f]))
         pass = GL_FALSE;

   return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


void
piglit_init(int argc, char **argv)
{
   GLuint Texture;

   piglit_require_extension("GL_ARB_texture_float");
   piglit_require_extension("GL_ARB_fragment_shader");

   HaveRG = piglit_is_extension_supported("GL_ARB_texture_rg");

   Program = piglit_build_simple_program(NULL, FragShaderText);

   glUseProgram(Program);

   BiasUniform = glGetUniformLocation(Program, "bias");
   ScaleUniform = glGetUniformLocation(Program, "scale");
   TexUniform = glGetUniformLocation(Program, "tex");

   glUniform1f(BiasUniform, Bias);
   glUniform1f(ScaleUniform, Scale);
   glUniform1i(TexUniform, 0);  /* tex unit zero */

   (void) check_error(__FILE__, __LINE__);

   glGenTextures(1, &Texture);
   glBindTexture(GL_TEXTURE_2D, Texture);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

   piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);
}
