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

#include "piglit-util.h"


#define MAX( A, B )   ( (A)>(B) ? (A) : (B) )


static int Width = 128, Height = 128;
static int Automatic = 0;


/** random number for checking state */
static GLfloat Random[128][4];

static GLint MaxTextureCoordUnits;
static GLint MaxTextureVertexUnits;
static GLint MaxTextureImageUnits;
static GLint MaxTextureCombinedUnits;


static void
generate_random_numbers(void)
{
   int i, j;
   for (i = 0; i < 128; i++) {
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
   GLenum err;
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
   err = glGetError();
   if (err != GL_NO_ERROR) {
      printf("Unexpected GL error in %s(): 0x%x\n", __FUNCTION__, err);
   }

   /* this should generate an error */
   {
      GLfloat v[4];
      glActiveTexture(GL_TEXTURE0 + MaxTextureCoordUnits);
      if (MaxTextureCoordUnits == MaxTextureCombinedUnits) {
         /* INVALID_ENUM is expected */
         err = glGetError();
         if (err != GL_INVALID_ENUM) {
            printf("GL failed to raise GL_INVALID_ENUM setting texture unit\n");
            return GL_FALSE;
         }
      }
      else {
         /* INVALID_OPERATION is expected */
         glGetFloatv(GL_CURRENT_RASTER_TEXTURE_COORDS, v);
         if (glGetError() != GL_INVALID_OPERATION) {
            printf("GL failed to raise GL_INVALID_OPERATION quering invalid raster tex coords\n");
            return GL_FALSE;
         }
      }
   }

   return GL_TRUE;
}


static GLboolean
test_texture_matrix(void)
{
   GLenum err;
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
   err = glGetError();
   if (err != GL_NO_ERROR) {
      printf("Unexpected GL error in %s(): 0x%x\n", __FUNCTION__, err);
   }

   /* this should generate an error */
   {
      GLfloat m[16];
      glActiveTexture(GL_TEXTURE0 + MaxTextureCoordUnits);
      if (MaxTextureCoordUnits == MaxTextureCombinedUnits) {
         /* INVALID_ENUM is expected */
         err = glGetError();
         if (err != GL_INVALID_ENUM) {
            printf("GL failed to raise GL_INVALID_ENUM setting texture unit\n");
            return GL_FALSE;
         }
      }
      else {
         /* INVALID_OPERATION is expected */
         glGetFloatv(GL_TEXTURE_MATRIX, m);
         if (glGetError() != GL_INVALID_OPERATION) {
            printf("GL failed to raise GL_INVALID_OPERATION querying invalid texture matrix\n");
            return GL_FALSE;
         }
      }
   }

   return GL_TRUE;
}


static GLboolean
test_texture_params(void)
{
   GLuint tex[100];
   GLenum err;
   int i;
   int maxUnit;

   clear_errors();

   glGenTextures(MaxTextureCombinedUnits, tex);

   /* set per-unit state */
   for (i = 0; i < MaxTextureCombinedUnits; i++) {
      glActiveTexture(GL_TEXTURE0 + i);
      glBindTexture(GL_TEXTURE_2D, tex[i]);
      glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, Random[i]);
   }

   /* check per-unit state */
   for (i = 0; i < MaxTextureCombinedUnits; i++) {
      GLfloat v[4];
      glActiveTexture(GL_TEXTURE0 + i);
      glBindTexture(GL_TEXTURE_2D, tex[i]);
      /* any per-unit state will do: */
      glGetTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, v);
      if (!equal4v(v, Random[i])) {
         printf("Setting per-unit param state failed for unit %d\n", i);
         report4v(Random[i], v);
         return GL_FALSE;
      }
   }

   /* there should be no errors at this point */
   err = glGetError();
   if (err != GL_NO_ERROR) {
      printf("Unexpected GL error in %s(): 0x%x\n", __FUNCTION__, err);
   }

   maxUnit = MAX(MaxTextureCombinedUnits, MaxTextureCoordUnits);

   /* this should generate an error */
   glActiveTexture(GL_TEXTURE0 + maxUnit);
   /* INVALID_ENUM is expected */
   err = glGetError();
   if (err != GL_INVALID_ENUM) {
      printf("GL failed to raise GL_INVALID_ENUM setting texture unit\n");
      return GL_FALSE;
   }

   return GL_TRUE;
}


static GLboolean
test_texture_env(void)
{
   /* Texture Environment state is fixed-function; not used by shaders */
   GLenum err;
   int i;

   clear_errors();

   /* set per-unit state */
   for (i = 0; i < MaxTextureImageUnits; i++) {
      glActiveTexture(GL_TEXTURE0 + i);
      glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, Random[i]);
      err = glGetError();
      if (err) {
         fprintf(stderr, "unit %d glTexEnvfv error: 0x%x\n", i, err);
         return GL_FALSE;
      }
   }

   /* check per-unit state */
   for (i = 0; i < MaxTextureImageUnits; i++) {
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
   err = glGetError();
   if (err != GL_NO_ERROR) {
      printf("Unexpected GL error in %s(): 0x%x\n", __FUNCTION__, err);
   }

   /* this should generate an error */
   {
      glActiveTexture(GL_TEXTURE0 + MaxTextureImageUnits);
      if (MaxTextureImageUnits == MaxTextureCombinedUnits) {
         /* INVALID_ENUM is expected */
         err = glGetError();
         if (err != GL_INVALID_ENUM) {
            printf("GL failed to raise GL_INVALID_ENUM setting texture unit\n");
            return GL_FALSE;
         }
      }
      else {
         glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, Random[0]);
         /* INVALID_OPERATION is expected */
         err = glGetError();
         if (err != GL_INVALID_OPERATION) {
            printf("GL failed to raise GL_INVALID_OPERATION setting texture env\n");
            return GL_FALSE;
         }
      }
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


static void
redisplay(void)
{
   if (Automatic) {
      GLboolean pass = GL_TRUE;

      pass = test_rasterpos() && pass;
      pass = test_texture_matrix() && pass;
      pass = test_texture_params() && pass;
      pass = test_texture_env() && pass;

      if (pass) {
         piglit_report_result(PIGLIT_SUCCESS);
      }
      else {
         report_info();
         piglit_report_result(PIGLIT_FAILURE);
      }
      exit(1);
   }
}


static void
reshape(int width, int height)
{
   Width = width;
   Height = height;
   glViewport(0, 0, width, height);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glOrtho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
}


static void
key(unsigned char key, int x, int y)
{
   if (key == 27) {
      exit(0);
   }
   glutPostRedisplay();
}


static void
init(void)
{
   if (glutExtensionSupported("GL_ARB_fragment_program")) {
      glGetIntegerv(GL_MAX_TEXTURE_COORDS, &MaxTextureCoordUnits);
      glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &MaxTextureImageUnits);
      glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &MaxTextureVertexUnits);
      glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &MaxTextureCombinedUnits);
   }
   else {
      glGetIntegerv(GL_MAX_TEXTURE_UNITS, &MaxTextureCoordUnits);
      MaxTextureImageUnits =
      MaxTextureVertexUnits =
      MaxTextureCombinedUnits = MaxTextureCoordUnits;
   }

   if (0)
      report_info();

   generate_random_numbers();

   reshape(Width, Height);
}


int
main(int argc, char *argv[])
{
   glutInit(&argc, argv);
   if (argc == 2 && !strcmp(argv[1], "-auto"))
      Automatic = 1;

   glutInitWindowPosition(0, 0);
   glutInitWindowSize(Width, Height);
   glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
   glutCreateWindow(argv[0]);

   glewInit();

   if (!GLEW_VERSION_1_3) {
	   printf("Requires OpenGL 1.3\n");
	   piglit_report_result(PIGLIT_SKIP);
   }

   glutReshapeFunc(reshape);
   glutDisplayFunc(redisplay);
   if (!Automatic) {
      glutKeyboardFunc(key);
   }
   init();
   glutMainLoop();
   return 0;
}

