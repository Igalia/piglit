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

/**
 * @file sampler-objects.c
 *
 * Test GL_ARB_sampler_objects
 * Brian Paul
 * April 2011
 */


#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

static const char *Prog = "sampler-objects";


static GLboolean
check_error(int line)
{
   GLenum err = glGetError();
   if (err != GL_NO_ERROR) {
      fprintf(stderr, "%s: unexpected error 0x%x at line %d\n", Prog, err, line);
      return GL_TRUE;
   }
   return GL_FALSE;
}


/**
 * Test the sampler object gen/bind/delete functions.
 */
static enum piglit_result
test_objects(void)
{
   GLuint samplers[4], i;

   glGenSamplers(4, samplers);
   if (check_error(__LINE__))
      return PIGLIT_FAIL;

   for (i = 0; i < 4; i++) {
      if (samplers[i] == 0)
         return PIGLIT_FAIL;
      if (i > 1 && samplers[i] == samplers[i-1])
         return PIGLIT_FAIL;
      if (!glIsSampler(samplers[i]))
         return PIGLIT_FAIL;
   }

   for (i = 0; i < 4; i++) {
      glBindSampler(i, samplers[i]);
      if (check_error(__LINE__))
         return PIGLIT_FAIL;
   }

   glDeleteSamplers(4, samplers);
   if (check_error(__LINE__))
      return PIGLIT_FAIL;

   for (i = 0; i < 4; i++) {
      if (glIsSampler(samplers[i]))
         return PIGLIT_FAIL;
   }

   return PIGLIT_PASS;
}


static const GLubyte mipmap_colors[10][4] = {
   { 255,   0,   0, 255 },
   {   0, 255,   0, 255 },
   {   0,   0, 255, 255 },
   {   0, 255, 255, 255 },
   { 255,   0, 255, 255 },
   { 255, 255,   0, 255 },
   { 255, 255, 255, 255 },
   { 128, 128, 128, 255 },
   { 255, 128,   0, 255 },
   {   0, 255, 128, 255 }
};


/** XXX this could be a piglit util function */
static GLuint
generate_mipmap(GLuint numLevels)
{
   int l;
   GLuint tex;
   assert(numLevels <= 10);
   glGenTextures(1, &tex);
   glBindTexture(GL_TEXTURE_2D, tex);
   glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
   for (l = 0; l < numLevels; l++) {
      int w = 1 << (numLevels - l - 1);
      int h = w;
      GLubyte *buf = malloc(w * h * 4);
      int i;
      for (i = 0; i < w * h; i++) {
         buf[i*4+0] = mipmap_colors[l][0];
         buf[i*4+1] = mipmap_colors[l][1];
         buf[i*4+2] = mipmap_colors[l][2];
         buf[i*4+3] = mipmap_colors[l][3];
      }
      glTexImage2D(GL_TEXTURE_2D, l, GL_RGBA, w, h, 0,
                   GL_RGBA, GL_UNSIGNED_BYTE, buf);
      free(buf);
      if (l == numLevels - 1) {
         assert(w == 1);
      }
   }
   return tex;
}


#define NUM_SAMPLERS 8

/**
 * Test sampler object operation.  Create a mipmap texture with each
 * level a different color.  Create a number of samplers with each
 * one's GL_TEXTURE_MIN_LOD = GL_TEXTURE_MAX_LOD = i to force sampling
 * from mipmap level i.  Draw a textured rect with each sampler object
 * and test that the rect's color matches the mipmap level.
 * XXX we should also test texcoord wrap modes, lod bias, filters, etc.
 */
static enum piglit_result
test_samplers(void)
{
   const GLenum minFilt = GL_NEAREST_MIPMAP_NEAREST, magFilt = GL_NEAREST;
   GLuint tex = generate_mipmap(9);
   GLuint samplers[NUM_SAMPLERS], i;

   glGenSamplers(NUM_SAMPLERS, samplers);
   if (check_error(__LINE__))
      return PIGLIT_FAIL;

   /* Create samplers which clamp lod to a particular mipmap level) */
   for (i = 0; i < NUM_SAMPLERS; i++) {
      glSamplerParameteri(samplers[i], GL_TEXTURE_MIN_LOD, (float) i);
      glSamplerParameteri(samplers[i], GL_TEXTURE_MAX_LOD, (float) i);
      glSamplerParameteri(samplers[i], GL_TEXTURE_MIN_FILTER, minFilt);
      glSamplerParameteri(samplers[i], GL_TEXTURE_MAG_FILTER, magFilt);
   }

   /* Test sampler queries */
   for (i = 0; i < NUM_SAMPLERS; i++) {
      GLint v;
      glGetSamplerParameteriv(samplers[i], GL_TEXTURE_MIN_LOD, &v);
      if (v != i) {
         fprintf(stderr, "%s: GL_TEXTURE_MIN_LOD query failed\n", Prog);
         return PIGLIT_FAIL;
      }
      glGetSamplerParameteriv(samplers[i], GL_TEXTURE_MAX_LOD, &v);
      if (v != i) {
         fprintf(stderr, "%s: GL_TEXTURE_MAX_LOD query failed\n", Prog);
         return PIGLIT_FAIL;
      }
      glGetSamplerParameteriv(samplers[i], GL_TEXTURE_MIN_FILTER, &v);
      if (v != minFilt) {
         fprintf(stderr, "%s: GL_TEXTURE_MIN_LOD query failed\n", Prog);
         return PIGLIT_FAIL;
      }
      glGetSamplerParameteriv(samplers[i], GL_TEXTURE_MAG_FILTER, &v);
      if (v != magFilt) {
         fprintf(stderr, "%s: GL_TEXTURE_MIN_LOD query failed\n", Prog);
         return PIGLIT_FAIL;
      }
   }

   /* draw test rects */
   glEnable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D, tex);

   for (i = 0; i < NUM_SAMPLERS; i++) {
      GLint p;
      GLfloat exp[4];

      glBindSampler(0, samplers[i]);
      glClear(GL_COLOR_BUFFER_BIT);
      piglit_draw_rect_tex(0, 0, piglit_width, piglit_height,
                           0.0, 0.0, 1.0, 1.0);

      exp[0] = mipmap_colors[i][0] / 255.0;
      exp[1] = mipmap_colors[i][1] / 255.0;
      exp[2] = mipmap_colors[i][2] / 255.0;
      exp[3] = mipmap_colors[i][3] / 255.0;

      p = piglit_probe_pixel_rgba(10, 10, exp);

      piglit_present_results();

      if (!p) {
         fprintf(stderr, "%s failed for sampler %d\n", Prog, i);
         return PIGLIT_FAIL;
      }
   }
   glDisable(GL_TEXTURE_2D);

   return PIGLIT_PASS;
}


enum piglit_result
piglit_display(void)
{
   enum piglit_result res;
   
   res = test_objects();
   if (res != PIGLIT_PASS)
      return res;

   res = test_samplers();
   if (res != PIGLIT_PASS)
      return res;

   return res;
}


void
piglit_init(int argc, char**argv)
{
   piglit_require_extension("GL_ARB_sampler_objects");

   piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);
}
