/*
 * Copyright (c) VMware, Inc.
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
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


/**
 * @file
 * Test texture unit state with respect to the different number of
 * texture coord units, image units, combined units, etc.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

#define MAX_UNITS 256

/** random number for checking state */
static GLfloat Random[MAX_UNITS][4];

static GLint MaxTextureCoordUnits;
static GLint MaxTextureVertexUnits;
static GLint MaxTextureImageUnits;
static GLint MaxTextureCombinedUnits;


static void
generate_random_numbers(void)
{
   int i, j;
   for (i = 0; i < ARRAY_SIZE(Random); i++) {
      for (j = 0; j < 4; j++) {
         /* values in [0, 1] */
         Random[i][j] = (rand() % 1000) * .001;
      }
   }
}


static GLboolean
equal4v(const GLfloat v1[4], const GLfloat v2[4])
{
   return (v1[0] == v2[0] &&
           v1[1] == v2[1] &&
           v1[2] == v2[2] &&
           v1[3] == v2[3]);
}


static GLboolean
equal16v(const GLfloat v1[16], const GLfloat v2[16])
{
   int i;
   for (i = 0; i < 16; i++) {
      if (v1[i] != v2[i])
         return GL_FALSE;
   }
   return GL_TRUE;
}


static void
report4v(const GLfloat exp[4], const GLfloat act[4])
{
   printf("Expected (%g, %g, %g, %g) but found (%g, %g, %g, %g)\n",
          exp[0], exp[1], exp[2], exp[3],
          act[0], act[1], act[2], act[3]);
}


static void
clear_errors()
{
   while (glGetError())
      ;
}


static GLboolean
test_rasterpos(void)
{
   int i;

   clear_errors();

   /* set current texcoords */
   for (i = 0; i < MaxTextureCoordUnits; i++) {
      glMultiTexCoord4fv(GL_TEXTURE0 + i, Random[i]);
   }

   /* query current texcoords */
   for (i = 0; i < MaxTextureCoordUnits; i++) {
      GLfloat v[4];
      glActiveTexture(GL_TEXTURE0 + i);
      glGetFloatv(GL_CURRENT_TEXTURE_COORDS, v);
      if (!equal4v(Random[i], v)) {
         printf("Get GL_CURRENT_TEXTURE_COORDS, unit %d failed\n", i);
         report4v(Random[i], v);
         return GL_FALSE;
      }
   }

   /* set raster pos to update raster tex coords */
   glRasterPos2i(0, 0);

   for (i = 0; i < MaxTextureCoordUnits; i++) {
      GLfloat v[4];
      glActiveTexture(GL_TEXTURE0 + i);
      glGetFloatv(GL_CURRENT_RASTER_TEXTURE_COORDS, v);
      if (!equal4v(Random[i], v)) {
         printf("Get GL_CURRENT_RASTER_TEXTURE_COORDS, unit %d failed\n", i);
         report4v(Random[i], v);
         return GL_FALSE;
      }
   }

   /* there should be no errors at this point */
   if (!piglit_check_gl_error(GL_NO_ERROR)) {
      return GL_FALSE;
   }

   /* this should generate an error */
   {
      GLfloat v[4];
      glActiveTexture(GL_TEXTURE0 + MaxTextureCoordUnits);
      if (MaxTextureCoordUnits == MaxTextureCombinedUnits) {
         /* INVALID_ENUM is expected */
         if (!piglit_check_gl_error(GL_INVALID_ENUM)) {
            return GL_FALSE;
         }
      }
      else {
         /* INVALID_OPERATION is expected */
         glGetFloatv(GL_CURRENT_RASTER_TEXTURE_COORDS, v);
         if (!piglit_check_gl_error(GL_INVALID_OPERATION)) {
            return GL_FALSE;
         }
      }
   }

   return GL_TRUE;
}


static GLboolean
test_texture_matrix(void)
{
   int i;

   clear_errors();

   /* set tex matrices */
   for (i = 0; i < MaxTextureCoordUnits; i++) {
      glActiveTexture(GL_TEXTURE0 + i);
      glMatrixMode(GL_TEXTURE);
      glLoadMatrixf((GLfloat *) Random + (i * 4) % 124);
   }

   /* query matrices */
   for (i = 0; i < MaxTextureCoordUnits; i++) {
      GLfloat m[16];
      glActiveTexture(GL_TEXTURE0 + i);
      glGetFloatv(GL_TEXTURE_MATRIX, m);
      if (!equal16v((GLfloat *) Random + (i * 4) % 124, m)) {
         printf("Get texture matrix unit %d failed\n", i);
         return GL_FALSE;
      }
   }

   /* there should be no errors at this point */
   if (!piglit_check_gl_error(GL_NO_ERROR)) {
      return GL_FALSE;
   }

   /* this should generate an error */
   {
      GLfloat m[16];
      glActiveTexture(GL_TEXTURE0 + MaxTextureCoordUnits);
      if (MaxTextureCoordUnits == MaxTextureCombinedUnits) {
         /* INVALID_ENUM is expected */
         if (!piglit_check_gl_error(GL_INVALID_ENUM)) {
            return GL_FALSE;
         }
      }
      else {
         /* INVALID_OPERATION is expected */
         glGetFloatv(GL_TEXTURE_MATRIX, m);
         if (!piglit_check_gl_error(GL_INVALID_OPERATION)) {
            return GL_FALSE;
         }
      }
   }

   return GL_TRUE;
}


static GLboolean
test_texture_params(void)
{
   GLuint tex[MAX_UNITS];
   int i;
   int maxUnit;

   clear_errors();

   glCreateTextures(GL_TEXTURE_2D, MaxTextureCombinedUnits, tex);

   /* set per-unit state */
   for (i = 0; i < MaxTextureCombinedUnits; i++) {
      glBindTextureUnit(i, tex[i]);
      glTextureParameterfv(tex[i], GL_TEXTURE_BORDER_COLOR, Random[i]);
   }

   /* check per-unit state */
   for (i = 0; i < MaxTextureCombinedUnits; i++) {
      GLfloat v[4];
      glBindTextureUnit(i, tex[i]);
      /* any per-unit state will do: */
      glGetTextureParameterfv(tex[i], GL_TEXTURE_BORDER_COLOR, v);
      if (!equal4v(v, Random[i])) {
         printf("Setting per-unit param state failed for unit %d\n", i);
         report4v(Random[i], v);
         return GL_FALSE;
      }
   }

   /* there should be no errors at this point */
   if (!piglit_check_gl_error(GL_NO_ERROR)) {
      return GL_FALSE;
   }

   maxUnit = MAX2(MaxTextureCombinedUnits, MaxTextureCoordUnits);

   /* this should generate an error */
   glBindTextureUnit(maxUnit, tex[0]);
   /* INVALID_OPERATION is expected */
   /* (see the GL 4.4 spec for glBindTextures) */
   if (!piglit_check_gl_error(GL_INVALID_OPERATION)) {
      return GL_FALSE;
   }

   return GL_TRUE;
}


static GLboolean
test_texture_env(void)
{
   /* Texture Environment state is fixed-function; not used by shaders */
   int i;

   clear_errors();

   /* set per-unit state */
   for (i = 0; i < MaxTextureCombinedUnits; i++) {
      glActiveTexture(GL_TEXTURE0 + i);
      glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, Random[i]);
      if (!piglit_check_gl_error(GL_NO_ERROR)) {
         return GL_FALSE;
      }
   }

   /* check per-unit state */
   for (i = 0; i < MaxTextureCombinedUnits; i++) {
      GLfloat v[4];
      glActiveTexture(GL_TEXTURE0 + i);
      glGetTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, v);
      if (!equal4v(v, Random[i])) {
         printf("Setting per-unit env state failed for unit %d\n", i);
         report4v(Random[i], v);
         return GL_FALSE;
      }
   }

   /* there should be no errors at this point */
   if (!piglit_check_gl_error(GL_NO_ERROR)) {
      return GL_FALSE;
   }

   return GL_TRUE;
}


static void
report_info(void)
{
   printf("GL_RENDERER = %s\n", (char *) glGetString(GL_RENDERER));
   printf("GL_MAX_TEXTURE_COORDS = %d\n", MaxTextureCoordUnits);
   printf("GL_MAX_TEXTURE_IMAGE_UNITS = %d\n", MaxTextureImageUnits);
   printf("GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS = %d\n", MaxTextureVertexUnits);
   printf("GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS = %d\n", MaxTextureCombinedUnits);
}


enum piglit_result
piglit_display(void)
{
	GLboolean pass = GL_TRUE;

	pass = test_rasterpos() && pass;
	pass = test_texture_matrix() && pass;
	pass = test_texture_params() && pass;
	pass = test_texture_env() && pass;

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


static void
init(void)
{
   if (piglit_is_extension_supported("GL_ARB_vertex_shader")) {
      glGetIntegerv(GL_MAX_TEXTURE_COORDS, &MaxTextureCoordUnits);
      glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &MaxTextureImageUnits);
      glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &MaxTextureVertexUnits);
      glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &MaxTextureCombinedUnits);
   }
   else if (piglit_is_extension_supported("GL_ARB_fragment_shader") ||
            piglit_is_extension_supported("GL_ARB_fragment_program")) {
      glGetIntegerv(GL_MAX_TEXTURE_COORDS, &MaxTextureCoordUnits);
      glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &MaxTextureImageUnits);
      MaxTextureVertexUnits = 0;
      MaxTextureCombinedUnits = MaxTextureImageUnits;
   }
   else {
      glGetIntegerv(GL_MAX_TEXTURE_UNITS, &MaxTextureCoordUnits);
      MaxTextureImageUnits =
      MaxTextureCombinedUnits = MaxTextureCoordUnits;
      MaxTextureVertexUnits = 0;
   }

   report_info();

   if (MaxTextureCombinedUnits > MAX_UNITS) {
      /* Need to increase the MAX_UNITS limit */
      piglit_report_result(PIGLIT_WARN);
   }

   generate_random_numbers();

   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glOrtho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
}


void
piglit_init(int argc, char *argv[])
{
   piglit_require_gl_version(13);
   piglit_require_extension("GL_ARB_direct_state_access");

   init();
}

