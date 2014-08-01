/*
 * Copyright (c) 2011 VMware, Inc.
 * Copyright (c) 2014 Intel Corporation
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
 * Tests for:
 *  GL_OES_texture_float,
 *  GL_OES_texture_half_float,
 *  GL_OES_texture_float_linear and
 *  GL_OES_texture_half_float_linear
 *
 * Usage: oes_texture_float [half] [linear]
 *
 */


#include <stdio.h>

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_es_version = 20;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static GLint TexWidth = 16, TexHeight = 16;
static GLint BiasUniform = -1, ScaleUniform = -1, TexUniform = -1;
static const float Scale = 1.0 / 2000.0, Bias = 0.5;



struct format_info
{
   const char *Name;
   GLenum Format;
   int NumComponents;
};

static const struct format_info Formats[] = {
  { "GL_RGBA",            GL_RGBA,            4 },
  { "GL_RGB",             GL_RGB,             3 },
  { "GL_ALPHA",           GL_ALPHA,           1 },
  { "GL_LUMINANCE",       GL_LUMINANCE,       1 },
  { "GL_LUMINANCE_ALPHA", GL_LUMINANCE_ALPHA, 2 },
};

static const char *VertShaderText =
  "attribute highp vec2 coord01; \n"
  "varying mediump vec2 tex_coord; \n"
  "void main() \n"
  "{ \n"
  "  tex_coord = coord01; \n"
  "  gl_Position =vec4(coord01*2.0 - vec2(1.0, 1.0), 0.0, 1.0); \n"
  "} \n";

static const char *FragShaderText =
   "uniform mediump float bias, scale; \n"
   "uniform mediump sampler2D tex; \n"
   "varying mediump vec2 tex_coord; \n"
   "void main() \n"
   "{ \n"
   "   mediump vec4 t = vec4(texture2D(tex, tex_coord.xy)); \n"
   "   gl_FragColor = t * scale + bias; \n"
   "} \n";


static GLuint Program, AttributeLoc;



static void
fill_array_fp32(int comps, int texels, void *buf, const float val[4])
{
   GLfloat *f = (GLfloat*) buf;
   int i, j;

   for (i = 0; i < texels; i++) {
      for (j = 0; j < comps; j++) {
         f[i * comps + j] = val[j];
      }
   }
}


/* Taken from Mesa: src/mesa/main/imports.h */
typedef union { GLfloat f; GLint i; GLuint u; } fi_type;
static inline int IROUND(float f)
{
   return (int) ((f >= 0.0F) ? (f + 0.5F) : (f - 0.5F));
}


/* Taken from Mesa: src/mesa/main/imports.c */
static int
_mesa_round_to_even(float val)
{
   int rounded = IROUND(val);

   if (val - floor(val) == 0.5) {
      if (rounded % 2 != 0)
         rounded += val > 0 ? -1 : 1;
   }

   return rounded;
}

static uint16_t
_mesa_float_to_half(float val)
{
   const fi_type fi = {val};
   const int flt_m = fi.i & 0x7fffff;
   const int flt_e = (fi.i >> 23) & 0xff;
   const int flt_s = (fi.i >> 31) & 0x1;
   int s, e, m = 0;
   uint16_t result;

   /* sign bit */
   s = flt_s;

   /* handle special cases */
   if ((flt_e == 0) && (flt_m == 0)) {
      /* zero */
      /* m = 0; - already set */
      e = 0;
   }
   else if ((flt_e == 0) && (flt_m != 0)) {
      /* denorm -- denorm float maps to 0 half */
      /* m = 0; - already set */
      e = 0;
   }
   else if ((flt_e == 0xff) && (flt_m == 0)) {
      /* infinity */
      /* m = 0; - already set */
      e = 31;
   }
   else if ((flt_e == 0xff) && (flt_m != 0)) {
      /* NaN */
      m = 1;
      e = 31;
   }
   else {
      /* regular number */
      const int new_exp = flt_e - 127;
      if (new_exp < -14) {
         /* The float32 lies in the range (0.0, min_normal16) and is rounded
          * to a nearby float16 value. The result will be either zero, subnormal,
          * or normal.
          */
         e = 0;
         m = _mesa_round_to_even((1 << 24) * fabsf(fi.f));
      }
      else if (new_exp > 15) {
         /* map this value to infinity */
         /* m = 0; - already set */
         e = 31;
      }
      else {
         /* The float32 lies in the range
          *   [min_normal16, max_normal16 + max_step16)
          * and is rounded to a nearby float16 value. The result will be
          * either normal or infinite.
          */
         e = new_exp + 15;
         m = _mesa_round_to_even(flt_m / (float) (1 << 13));
      }
   }

   assert(0 <= m && m <= 1024);
   if (m == 1024) {
      /* The float32 was rounded upwards into the range of the next exponent,
       * so bump the exponent. This correctly handles the case where f32
       * should be rounded up to float16 infinity.
       */
      ++e;
      m = 0;
   }

   result = (s << 15) | (e << 10) | m;
   return result;

}

static void
fill_array_fp16(int comps, int texels, void *buf, const float val[4])
{
   uint16_t *f = (uint16_t*) buf;
   int i, j;

   for (i = 0; i < texels; i++) {
      for (j = 0; j < comps; j++) {
        f[i * comps + j] = _mesa_float_to_half(val[j]);
      }
   }
}


struct texture_float_info
{
  void (*fill_array)(int comps, int texels, void *buf, const float val[4]);
  GLenum TextureType;
  size_t SizeOfType;
  const char *TestName;
  const char *ExtensionName;
  const char *ExtensionNameLinearFilter;
};

const struct texture_float_info Tests[]=
  {
    {
      fill_array_fp32,
      GL_FLOAT, 4,
      "oes-texture-float",
      "GL_OES_texture_float",
      "GL_OES_texture_float_linear"
    },

    {
      fill_array_fp16,
      GL_HALF_FLOAT_OES, 2,
      "oes-texture-half-float",
      "GL_OES_texture_half_float",
      "GL_OES_texture_half_float_linear"
    },
  };


static GLboolean
check_error(const char *file, int line, const struct texture_float_info *test)
{
   GLenum err = glGetError();
   if (err) {
      printf("%s: error 0x%x at %s:%d\n", test->TestName, err, file, line);
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
test_format(const struct format_info *info, const struct texture_float_info *test, GLboolean generate_mipmap)
{
   const int comps = info->NumComponents;
   const int texels = TexWidth * TexHeight;
   const int w = piglit_width / 10;
   const int h = piglit_height / 10;
   GLfloat expected[4];
   void *image;
   float value[4];
   int p;
   GLushort drawIndices[] =
     {
       0, 1, 2,
       0, 2, 3
     };

   /** printf("Testing %s of %s\n", info->Name, test->TestName); **/

   get_texture_color(value);

   /* alloc, fill texture image */
   image = malloc(comps * texels * test->SizeOfType);
   test->fill_array(comps, texels, image, value);


   glTexImage2D(GL_TEXTURE_2D, 0, info->Format, TexWidth, TexHeight, 0,
                info->Format, test->TextureType, image);
   if(generate_mipmap) {
     int w, h, l;
     for(w=TexWidth/2, h=TexHeight/2, l=1; w>0 || h>0; w/=2, h/=2, ++l) {
       int useW, useH;
       useW = (w!=0) ? w : 1;
       useH = (h!=0) ? h : 1;
       glTexImage2D(GL_TEXTURE_2D, l, info->Format, useW, useH, 0,
                    info->Format, test->TextureType, image);
     }
   }

   free(image);

   if (check_error(__FILE__, __LINE__, test))
      return GL_FALSE;

   /* compute expected color */
   switch (info->Format) {
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
   case GL_LUMINANCE_ALPHA:
      expected[0] =
      expected[1] =
      expected[2] = scale_and_bias(value[0]);
      expected[3] = scale_and_bias(value[1]);
      break;
   default:
      abort();
   }

   /* draw */

   glClearColor(0.5, 0.5, 0.5, 0.0);
   glClear(GL_COLOR_BUFFER_BIT);
   glDisable(GL_DEPTH_TEST);
   glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, drawIndices);

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

   piglit_swap_buffers();

   return p;
}

static GLboolean
test_each_format(const struct texture_float_info *test, GLboolean generate_mipmap)
{
  int f;
  GLboolean pass = GL_TRUE;
  for (f = 0; f < ARRAY_SIZE(Formats); f++) {
    if (!test_format(&Formats[f], test, generate_mipmap)) {
        pass = GL_FALSE;
     }
  }
  return pass;
}

static const struct texture_float_info *ActiveTest = NULL;
static GLboolean linearFilter = GL_FALSE;


enum piglit_result
piglit_display(void)
{
   GLboolean pass = GL_TRUE;
   GLenum minFilters[] =
     {
       GL_NEAREST,
       GL_LINEAR,
       GL_NEAREST_MIPMAP_NEAREST,
       GL_NEAREST_MIPMAP_LINEAR,
       GL_LINEAR_MIPMAP_NEAREST,
       GL_LINEAR_MIPMAP_LINEAR,
     };
   GLenum magFilters[] =
     {
       GL_NEAREST,
       GL_LINEAR,
     };

   if (linearFilter) {
      int i, j;
      GLenum magFilter, minFilter;

      for (i = 0; i < ARRAY_SIZE(magFilters); ++i) {
         magFilter = magFilters[i];
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
         for (j = 0; j < ARRAY_SIZE(minFilters); ++j) {
            minFilter = minFilters[j];
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
            if (!test_each_format(ActiveTest, minFilter!=GL_NEAREST && minFilter!=GL_LINEAR)) {
              pass = GL_FALSE;
            }
         }
      }
   } else {
     pass = test_each_format(ActiveTest, GL_FALSE);
   }

   return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

static GLfloat AttributeValues[] =
  {
    0.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f,
  };

void
piglit_init(int argc, char **argv)
{
   GLuint Texture;
   GLboolean half = GL_FALSE;
   int i;

   for (i = 1; i<argc; ++i) {
      if (!strcmp(argv[i], "half")) {
        half = GL_TRUE;
      } else if (!strcmp(argv[i], "linear")) {
        linearFilter = GL_TRUE;
      }
   }

   if (half) {
     ActiveTest = &Tests[1];
   } else {
     ActiveTest = &Tests[0];
   }

   if (linearFilter) {
     piglit_require_extension(ActiveTest->ExtensionNameLinearFilter);
   }
   piglit_require_extension(ActiveTest->ExtensionName);

   Program = piglit_build_simple_program(VertShaderText, FragShaderText);

   glUseProgram(Program);

   BiasUniform = glGetUniformLocation(Program, "bias");
   ScaleUniform = glGetUniformLocation(Program, "scale");
   TexUniform = glGetUniformLocation(Program, "tex");

   glUniform1f(BiasUniform, Bias);
   glUniform1f(ScaleUniform, Scale);
   glUniform1i(TexUniform, 0);  /* tex unit zero */

   AttributeLoc = glGetAttribLocation(Program, "coord01");
   glEnableVertexAttribArray(AttributeLoc);
   glVertexAttribPointer(AttributeLoc, 2, //slot, count
                         GL_FLOAT, GL_FALSE, //type, normalized
                         2*sizeof(float), //stride
                         AttributeValues); //pointer/offset

   (void) check_error(__FILE__, __LINE__, ActiveTest);


   glGenTextures(1, &Texture);
   glBindTexture(GL_TEXTURE_2D, Texture);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}
