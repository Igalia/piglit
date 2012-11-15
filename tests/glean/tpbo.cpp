// BEGIN_COPYRIGHT -*- glean -*-
// 
// Copyrigth (C) 2007  Intel Corporation
// Copyright (C) 1999  Allen Akin   All Rights Reserved.
// 
// Permission is hereby granted, free of charge, to any person
// obtaining a copy of this software and associated documentation
// files (the "Software"), to deal in the Software without
// restriction, including without limitation the rights to use,
// copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following
// conditions:
// 
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the
// Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
// KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
// PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL ALLEN AKIN BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
// AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
// OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
// 
// END_COPYRIGHT
//
// Authors:
//  Shuang He <shuang.he@intel.com>
//
// tpbo.cpp:  Test OpenGL Extension GL_ARB_pixel_buffer_object


#define GL_GLEXT_PROTOTYPES

#include <stdlib.h>
#include <cstring>
#include <cassert>
#include <math.h>
#include "tpbo.h"


namespace GLEAN
{
// GL_VERSION_1_5
static PFNGLBINDBUFFERPROC glBindBuffer_func = NULL;
static PFNGLMAPBUFFERPROC glMapBuffer_func = NULL;
static PFNGLUNMAPBUFFERPROC glUnmapBuffer_func = NULL;

// GL_ARB_vertex_buffer_object
static PFNGLBINDBUFFERARBPROC glBindBufferARB_func = NULL;
static PFNGLDELETEBUFFERSARBPROC glDeleteBuffersARB_func = NULL;
static PFNGLGENBUFFERSARBPROC glGenBuffersARB_func = NULL;
static PFNGLISBUFFERARBPROC glIsBufferARB_func = NULL;
static PFNGLBUFFERDATAARBPROC glBufferDataARB_func = NULL;
static PFNGLMAPBUFFERARBPROC glMapBufferARB_func = NULL;
static PFNGLUNMAPBUFFERARBPROC glUnmapBufferARB_func = NULL;

static int usePBO;
#define BUFFER_OFFSET(i) ((char *)NULL + (i))

bool PBOTest::setup(void)
{
   glMatrixMode(GL_PROJECTION);

   glLoadIdentity();
   gluOrtho2D(0, 100, 0, 100);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   glDrawBuffer(GL_FRONT);
   glReadBuffer(GL_FRONT);

   // compute error tolerances (may need fine-tuning)
   int bufferBits[5];

   glGetIntegerv(GL_RED_BITS, &bufferBits[0]);
   glGetIntegerv(GL_GREEN_BITS, &bufferBits[1]);
   glGetIntegerv(GL_BLUE_BITS, &bufferBits[2]);
   glGetIntegerv(GL_ALPHA_BITS, &bufferBits[3]);
   glGetIntegerv(GL_DEPTH_BITS, &bufferBits[4]);

   tolerance[0] = 2.0 / (1 << bufferBits[0]);
   tolerance[1] = 2.0 / (1 << bufferBits[1]);
   tolerance[2] = 2.0 / (1 << bufferBits[2]);
   if (bufferBits[3])
      tolerance[3] = 2.0 / (1 << bufferBits[3]);
   else
      tolerance[3] = 1.0;
   if (bufferBits[4])
      tolerance[4] = 16.0 / (1 << bufferBits[4]);
   else
      tolerance[4] = 1.0;

   // Check if GL_ARB_pixel_buffer_object is supported
   if (!GLUtils::haveExtension("GL_ARB_pixel_buffer_object")) {
      //printf("GL_ARB_pixel_buffer_object is not supported\n");
      usePBO = 0;
      return false;
   }
   else {
      //printf("GL_ARB_pixel_buffer_object is supported\n");
      usePBO = 1;
   }

   glBindBuffer_func = (PFNGLBINDBUFFERPROC) GLUtils::getProcAddress("glBindBuffer");
   assert(glBindBuffer_func);
   glMapBuffer_func = (PFNGLMAPBUFFERPROC) GLUtils::getProcAddress("glMapBuffer");
   assert(glMapBuffer_func);
   glUnmapBuffer_func = (PFNGLUNMAPBUFFERPROC) GLUtils::getProcAddress("glUnmapBuffer");
   assert(glUnmapBuffer_func);

   glBindBufferARB_func = (PFNGLBINDBUFFERARBPROC) GLUtils::getProcAddress("glBindBufferARB");
   assert(glBindBufferARB_func);
   glDeleteBuffersARB_func = (PFNGLDELETEBUFFERSARBPROC) GLUtils::getProcAddress("glDeleteBuffersARB");
   assert(glDeleteBuffersARB_func);
   glGenBuffersARB_func = (PFNGLGENBUFFERSARBPROC) GLUtils::getProcAddress("glGenBuffersARB");
   assert(glGenBuffersARB_func);
   glIsBufferARB_func = (PFNGLISBUFFERARBPROC) GLUtils::getProcAddress("glIsBufferARB");
   assert(glIsBufferARB_func);
   glBufferDataARB_func = (PFNGLBUFFERDATAARBPROC) GLUtils::getProcAddress("glBufferDataARB");
   assert(glBufferDataARB_func);
   glMapBufferARB_func = (PFNGLMAPBUFFERARBPROC) GLUtils::getProcAddress("glMapBufferARB");
   assert(glMapBufferARB_func);
   glUnmapBufferARB_func = (PFNGLUNMAPBUFFERARBPROC) GLUtils::getProcAddress("glUnmapBufferARB");
   assert(glUnmapBufferARB_func);

   return true;
}


void
PBOTest::reportFailure(const char *msg, const int line) const
{
   env->log << "FAILURE: " << msg << " (at tpbo.cpp:" << line << ")\n";
}

void
PBOTest::reportFailure(const char *msg, const GLenum target, const int line) const
{
   env->log << "FAILURE: " << msg;
   if (target == GL_FRAGMENT_SHADER)
      env->log << " (fragment)";
   else
      env->log << " (vertex)";
   env->log << " (at tpbo.cpp:" << line << ")\n";
}

#define REPORT_FAILURE(MSG) reportFailure(MSG, __LINE__)
#define REPORT_FAILURE_T(MSG, TARGET) reportFailure(MSG, TARGET, __LINE__)
// Compare actual and expected colors 
bool PBOTest::equalColors(const GLfloat act[3], const GLfloat exp[3]) const
{
   if ((fabsf(act[0] - exp[0]) > tolerance[0])
       || (fabsf(act[1] - exp[1]) > tolerance[1])
       || (fabsf(act[2] - exp[2]) > tolerance[2])) {
      return false;
   }
   else
      return true;
}

bool PBOTest::equalColors1(const GLubyte act[3], const GLubyte exp[3]) const
{
   if ((act[0] != exp[0])
       || (act[1] != exp[1])
       || (act[2] != exp[2])) {
      return false;
   }
   else
      return true;
}



#define TEXSIZE 64

bool PBOTest::testSanity(void)
{
   GLuint pbs[1];
   GLuint pb_binding;

   if (!usePBO)
      return true;

   // Check default binding
   glGetIntegerv(GL_PIXEL_UNPACK_BUFFER_BINDING_ARB, (GLint *) & pb_binding);
   if (pb_binding != 0) {
      REPORT_FAILURE("Failed to bind unpack pixel buffer object");
      return false;
   }

   glGetIntegerv(GL_PIXEL_PACK_BUFFER_BINDING_ARB, (GLint *) & pb_binding);
   if (pb_binding != 0) {
      REPORT_FAILURE("Failed to bind pack pixel buffer object");
      return false;
   }

   glGenBuffersARB_func(1, pbs);

   if (glIsBufferARB_func(pbs[0]) != GL_FALSE) {
      REPORT_FAILURE("glIsBufferARB failed");
      return false;
   }

   glBindBufferARB_func(GL_PIXEL_UNPACK_BUFFER_ARB, pbs[0]);
   glGetIntegerv(GL_PIXEL_UNPACK_BUFFER_BINDING_ARB, (GLint *) & pb_binding);
   glBindBufferARB_func(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
   if (pb_binding != pbs[0]) {
      REPORT_FAILURE("Failed to bind unpack pixel buffer object");
      return false;
   }

   glBindBufferARB_func(GL_PIXEL_PACK_BUFFER_ARB, pbs[0]);
   glGetIntegerv(GL_PIXEL_PACK_BUFFER_BINDING_ARB, (GLint *) & pb_binding);
   glBindBufferARB_func(GL_PIXEL_PACK_BUFFER_ARB, 0);
   if (pb_binding != pbs[0]) {
      REPORT_FAILURE("Failed to bind unpack pixel buffer object");
      return false;
   }

   glDeleteBuffersARB_func(1, pbs);

   if (glIsBufferARB_func(pbs[0]) == GL_TRUE) {
      REPORT_FAILURE("glIsBufferARB failed");
      return false;
   }

   return true;
}


bool PBOTest::testDrawPixels(void)
{
   int useUnpackBuffer;
   int usePackBuffer;
   GLuint pb_pack[1];
   GLuint pb_unpack[1];
   GLubyte buf[windowSize * windowSize * 4];
   GLubyte t[TEXSIZE * TEXSIZE * 4];
   int i, j;
   GLubyte * pboPackMem = NULL;
   GLubyte black[3] = { 0, 0, 0 };

   glBindBuffer_func(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
   glBindBuffer_func(GL_PIXEL_PACK_BUFFER_ARB, 0);

   for (useUnpackBuffer = 0; useUnpackBuffer < usePBO + 1; useUnpackBuffer++) {
      for (usePackBuffer = 0; usePackBuffer < usePBO + 1; usePackBuffer++) {
         glClearColor(0.0, 0.0, 0.0, 1.0);
         glClear(GL_COLOR_BUFFER_BIT);
         if (useUnpackBuffer) {
            glGenBuffersARB_func(1, pb_unpack);
            glBindBufferARB_func(GL_PIXEL_UNPACK_BUFFER_ARB, pb_unpack[0]);
            glBufferDataARB_func(GL_PIXEL_UNPACK_BUFFER_ARB,
                         TEXSIZE * TEXSIZE * 4 * sizeof(GLubyte), NULL,
                         GL_STREAM_DRAW);
         }
         GLubyte *pboMem = NULL;
         if (useUnpackBuffer) {
            pboMem = (GLubyte *) glMapBufferARB_func(GL_PIXEL_UNPACK_BUFFER_ARB,
                                       GL_WRITE_ONLY);
         }
         else {
            pboMem = t;
         }

         for (i = 0; i < TEXSIZE; i++)
            for (j = 0; j < TEXSIZE; j++) {
               pboMem[4 * (i * TEXSIZE + j)] = i % 256;
               pboMem[4 * (i * TEXSIZE + j) + 1] = i % 256;
               pboMem[4 * (i * TEXSIZE + j) + 2] = i % 256;
               pboMem[4 * (i * TEXSIZE + j) + 3] = 0;
            }

         if (useUnpackBuffer) {
            glUnmapBufferARB_func(GL_PIXEL_UNPACK_BUFFER_ARB);
            glBindBufferARB_func(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
         }

         if (useUnpackBuffer) {
            glBindBufferARB_func(GL_PIXEL_UNPACK_BUFFER_ARB, pb_unpack[0]);
            glDrawPixels(TEXSIZE, TEXSIZE, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
            glBindBufferARB_func(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
         }
         else
            glDrawPixels(TEXSIZE, TEXSIZE, GL_BGRA, GL_UNSIGNED_BYTE, pboMem);

         // Check the result
         if (usePackBuffer) {
            glGenBuffersARB_func(1, pb_pack);
            glBindBufferARB_func(GL_PIXEL_PACK_BUFFER_ARB, pb_pack[0]);
            glBufferDataARB_func(GL_PIXEL_PACK_BUFFER_ARB,
                         windowSize * windowSize * 4 *
                         sizeof(GL_UNSIGNED_BYTE), NULL, GL_STREAM_DRAW);
            glReadPixels(0, 0, windowSize, windowSize, GL_BGRA,
                         GL_UNSIGNED_BYTE, NULL);
            pboPackMem = (GLubyte *) glMapBufferARB_func(GL_PIXEL_PACK_BUFFER_ARB,
                                       GL_READ_ONLY);
         }
         else {
            pboPackMem = buf;
            glReadPixels(0, 0, windowSize, windowSize, GL_BGRA,
                         GL_UNSIGNED_BYTE, pboPackMem);
         }

         for (j = 0; j < windowSize; j++) {
            for (i = 0; i < windowSize; i++) {
               GLubyte exp[3];
               exp[0] = j % 256;
               exp[1] = j % 256;
               exp[2] = j % 256;

               if (i < TEXSIZE && j < TEXSIZE) {
                  if (!equalColors1(&pboPackMem[(j * windowSize + i) * 4], exp)) {
                     REPORT_FAILURE("glDrawPixels failed");
                     printf("  got (%d, %d) = [%d, %d, %d], ", i, j,
                            pboPackMem[(j * windowSize + i) * 4],
                            pboPackMem[(j * windowSize + i) * 4 + 1],
                            pboPackMem[(j * windowSize + i) * 4 + 2]);
                     printf("should be [%d, %d, %d]\n",
                            exp[0], exp[1], exp[2]);

                     return false;
                  }
               }
               else {
                  if (!equalColors1(&pboPackMem[(j * windowSize + i) * 4], black)) {
                     REPORT_FAILURE("glDrawPixels failed");
                     printf("(%d, %d) = [%d, %d, %d], ", i, j,
                            pboPackMem[(j * windowSize + i) * 4],
                            pboPackMem[(j * windowSize + i) * 4 + 1],
                            pboPackMem[(j * windowSize + i) * 4 + 2]);
                     printf("should be [0.0, 0.0, 0.0]\n");
                     return false;
                  }

               }
            }
         }


         if (usePackBuffer) {
            glBindBuffer_func(GL_PIXEL_PACK_BUFFER_ARB, 0);
            glDeleteBuffersARB_func(1, pb_pack);
         }

         if (useUnpackBuffer) {
            glBindBuffer_func(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
            glDeleteBuffersARB_func(1, pb_unpack);
         }

      }
   }

   return true;
}


bool PBOTest::testPixelMap(void)
{
   int useUnpackBuffer;
   int usePackBuffer;
   GLuint pb_pack[1];
   GLuint pb_unpack[1];
   int i;
   int size;
   int max;

   glBindBufferARB_func(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
   glBindBufferARB_func(GL_PIXEL_PACK_BUFFER_ARB, 0);

   glGetIntegerv(GL_MAX_PIXEL_MAP_TABLE, &max);

   for (usePackBuffer = 0; usePackBuffer < usePBO + 1; usePackBuffer++) {
      for (useUnpackBuffer = 0; useUnpackBuffer < usePBO + 1;
           useUnpackBuffer++) {
         glClearColor(0.0, 0.0, 0.0, 1.0);
         glClear(GL_COLOR_BUFFER_BIT);
         if (useUnpackBuffer) {
            glGenBuffersARB_func(1, pb_unpack);
            glBindBufferARB_func(GL_PIXEL_UNPACK_BUFFER_ARB, pb_unpack[0]);
            glBufferDataARB_func(GL_PIXEL_UNPACK_BUFFER_ARB, max * sizeof(GLushort),
                         NULL, GL_STREAM_DRAW);
         }
         GLushort *pboMem = NULL;
         if (useUnpackBuffer) {
            pboMem = (GLushort *) glMapBufferARB_func(GL_PIXEL_UNPACK_BUFFER_ARB,
                                        GL_WRITE_ONLY);
         }
         else {
            pboMem = (GLushort *) malloc(sizeof(GLushort) * max);
         }
         for (i = 0; i < max; i++)
            pboMem[i] = max - i - 1;

         if (useUnpackBuffer) {
            glUnmapBufferARB_func(GL_PIXEL_UNPACK_BUFFER_ARB);
            glPixelMapusv(GL_PIXEL_MAP_R_TO_R, max, NULL);
            glPixelMapusv(GL_PIXEL_MAP_G_TO_G, max, NULL);
            glPixelMapusv(GL_PIXEL_MAP_B_TO_B, max, NULL);
            glPixelMapusv(GL_PIXEL_MAP_A_TO_A, max, NULL);
            glBindBufferARB_func(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
         }
         else {
            glPixelMapusv(GL_PIXEL_MAP_R_TO_R, max, pboMem);
            glPixelMapusv(GL_PIXEL_MAP_G_TO_G, max, pboMem);
            glPixelMapusv(GL_PIXEL_MAP_B_TO_B, max, pboMem);
            glPixelMapusv(GL_PIXEL_MAP_A_TO_A, max, pboMem);
            free(pboMem);
         }


         glGetIntegerv(GL_PIXEL_MAP_R_TO_R_SIZE, &size);
         if (size != max) {
            REPORT_FAILURE("glPixelMap failed");
            return false;
         }
         glPixelTransferi(GL_MAP_COLOR, GL_FALSE);

         // Read back pixel map
         if (usePackBuffer) {
            glGenBuffersARB_func(1, pb_pack);
            glBindBufferARB_func(GL_PIXEL_PACK_BUFFER_ARB, pb_pack[0]);
            glBufferDataARB_func(GL_PIXEL_PACK_BUFFER_ARB, max * sizeof(GLushort),
                         NULL, GL_STREAM_DRAW);
            glGetPixelMapusv(GL_PIXEL_MAP_R_TO_R, NULL);
            pboMem = (GLushort *) glMapBufferARB_func(GL_PIXEL_PACK_BUFFER_ARB,
                                        GL_READ_ONLY);
         }
         else {
            pboMem = (GLushort *) malloc(sizeof(GLushort) * max);
            glGetPixelMapusv(GL_PIXEL_MAP_R_TO_R, pboMem);
         }

         for (i = 0; i < max; i++) {
            if (pboMem[i] != (255 - i)) {
               REPORT_FAILURE("get PixelMap failed");
               return false;
            }
         }


         if (usePackBuffer) {
            glUnmapBufferARB_func(GL_PIXEL_PACK_BUFFER_ARB);
            glBindBufferARB_func(GL_PIXEL_PACK_BUFFER_ARB, 0);
            glDeleteBuffersARB_func(1, pb_pack);
         }
         else {
            free(pboMem);
         }

         if (useUnpackBuffer) {
            glBindBufferARB_func(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
            glDeleteBuffersARB_func(1, pb_unpack);
         }

      }
   }

   return true;
}

bool PBOTest::testBitmap(void)
{
   GLuint pb_unpack[1];
   GLuint pb_pack[1];
   int useUnpackBuffer = usePBO;
   int usePackBuffer = 0;
   GLubyte bitmap[TEXSIZE * TEXSIZE / 8];
   GLfloat buf[windowSize * windowSize * 3];
   GLfloat white[3] = { 1.0, 1.0, 1.0 };
   GLfloat black[3] = { 0.0, 0.0, 0.0 };
   int i, j;
   GLubyte *pboUnpackMem = NULL;
   GLfloat *pboPackMem = NULL;

   glBindBufferARB_func(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
   glBindBufferARB_func(GL_PIXEL_PACK_BUFFER_ARB, 0);

   for (usePackBuffer = 0; usePackBuffer < usePBO + 1; usePackBuffer++) {
      for (useUnpackBuffer = 0; useUnpackBuffer < usePBO + 1;
           useUnpackBuffer++) {
         glClearColor(0.0, 0.0, 0.0, 1.0);
         glClear(GL_COLOR_BUFFER_BIT);

         if (useUnpackBuffer) {
            glGenBuffersARB_func(1, pb_unpack);
            glBindBufferARB_func(GL_PIXEL_UNPACK_BUFFER_ARB, pb_unpack[0]);
            glBufferDataARB_func(GL_PIXEL_UNPACK_BUFFER_ARB, TEXSIZE * TEXSIZE, NULL,
                         GL_STREAM_DRAW);
            pboUnpackMem = (GLubyte *) glMapBufferARB_func(GL_PIXEL_UNPACK_BUFFER_ARB,
                                       GL_WRITE_ONLY);
         }
         else {
            pboUnpackMem = bitmap;
         }

         for (i = 0; i < TEXSIZE * TEXSIZE / 8; i++) {
            pboUnpackMem[i] = 0xAA;
         }


         glColor4f(1.0, 1.0, 1.0, 0.0);
         glRasterPos2f(0.0, 0.0);
         if (useUnpackBuffer) {
            glUnmapBufferARB_func(GL_PIXEL_UNPACK_BUFFER_ARB);
            glBitmap(TEXSIZE, TEXSIZE, 0, 0, 0, 0, NULL);
            glBindBufferARB_func(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
         }
         else
            glBitmap(TEXSIZE, TEXSIZE, 0, 0, 0, 0, pboUnpackMem);

         // Check the result
         if (usePackBuffer) {
            glGenBuffersARB_func(1, pb_pack);
            glBindBufferARB_func(GL_PIXEL_PACK_BUFFER_ARB, pb_pack[0]);
            glBufferDataARB_func(GL_PIXEL_PACK_BUFFER_ARB,
                            windowSize * windowSize * 4 * sizeof(GLfloat),
                            NULL,
                            GL_STREAM_DRAW);
            glReadPixels(0, 0, windowSize, windowSize, GL_RGB, GL_FLOAT,
                         NULL);
            pboPackMem =
               (GLfloat *) glMapBufferARB_func(GL_PIXEL_PACK_BUFFER_ARB,
                                       GL_READ_ONLY);
         }
         else {
            pboPackMem = buf;
            glReadPixels(0, 0, windowSize, windowSize, GL_RGB, GL_FLOAT,
                         pboPackMem);
         }

         for (j = 0; j < windowSize; j++) {
            for (i = 0; i < windowSize; i++) {
               const GLfloat *exp;
               if ((i & 1))
                  exp = black;
               else
                  exp = white;
               if (i < TEXSIZE && j < TEXSIZE) {
                  if (!equalColors(&pboPackMem[(j * windowSize + i) * 3], exp)) {
                     REPORT_FAILURE("glBitmap failed");
                     printf("  got (%d, %d) = [%f, %f, %f], ", i, j,
                            pboPackMem[(j * windowSize + i) * 3],
                            pboPackMem[(j * windowSize + i) * 3 + 1],
                            pboPackMem[(j * windowSize + i) * 3 + 2]);
                     printf("should be [%f, %f, %f]\n",
                            exp[0], exp[1], exp[2]);

                     return false;
                  }
               }
               else {
                  if (!equalColors(&pboPackMem[(j * windowSize + i) * 3],
                                   black)) {
                     REPORT_FAILURE("glBitmap failed");
                     printf("(%d, %d) = [%f, %f, %f], ", i, j,
                            pboPackMem[(j * windowSize + i) * 3],
                            pboPackMem[(j * windowSize + i) * 3 + 1],
                            pboPackMem[(j * windowSize + i) * 3 + 2]);
                     printf("should be [0.0, 0.0, 0.0]\n");
                     return false;
                  }

               }
            }
         }
         if (usePackBuffer) {
            glUnmapBuffer_func(GL_PIXEL_PACK_BUFFER_ARB);
            glBindBuffer_func(GL_PIXEL_PACK_BUFFER_ARB, 0);
            glDeleteBuffersARB_func(1, pb_pack);
         }

         if (useUnpackBuffer) {
            glBindBuffer_func(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
            glDeleteBuffersARB_func(1, pb_unpack);
         }
      }
   }
   return true;
}


bool PBOTest::testTexImage(void)
{
   int breakCOWPBO, breakCOWTexture;
   int useTexUnpackBuffer, useTexPackBuffer;
   GLuint unpack_pb[1];
   GLuint pack_pb[1];
   GLfloat t1[TEXSIZE * TEXSIZE * 3];
   GLfloat t2[TEXSIZE * TEXSIZE * 3];
   GLfloat *pboMem = NULL;
   int i, j;
   GLfloat green[3] = { 1.0, 1.0, 0.0 };
   GLfloat black[3] = { 0.0, 0.0, 0.0 };
   GLfloat buf[windowSize * windowSize * 3];

   glBindBufferARB_func(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
   glBindBufferARB_func(GL_PIXEL_PACK_BUFFER_ARB, 0);

   glClearColor(0.0, 0.0, 0.0, 1.0);
   glClear(GL_COLOR_BUFFER_BIT);

   for (useTexPackBuffer = 0; useTexPackBuffer < usePBO + 1;
        useTexPackBuffer++) {
      for (useTexUnpackBuffer = 0; useTexUnpackBuffer < usePBO + 1;
           useTexUnpackBuffer++) {
         for (breakCOWPBO = 0; breakCOWPBO < useTexUnpackBuffer + 1;
              breakCOWPBO++) {
            for (breakCOWTexture = 0;
                 breakCOWTexture < useTexUnpackBuffer + 1;
                 breakCOWTexture++) {
               if (useTexUnpackBuffer) {
                  glGenBuffersARB_func(1, unpack_pb);
                  glBindBufferARB_func(GL_PIXEL_UNPACK_BUFFER_ARB, unpack_pb[0]);
                  glBufferDataARB_func(GL_PIXEL_UNPACK_BUFFER_ARB,
                               TEXSIZE * TEXSIZE * 3 * sizeof(GLfloat), NULL,
                               GL_STREAM_DRAW);
               }

               glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                               GL_NEAREST);
               glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                               GL_NEAREST);

               if (useTexUnpackBuffer) {
                  pboMem =
                     (GLfloat *) glMapBufferARB_func(GL_PIXEL_UNPACK_BUFFER_ARB,
                                             GL_WRITE_ONLY);
               }
               else {
                  pboMem = t1;
               }

               for (i = 0; i < TEXSIZE * TEXSIZE; i++) {
                  pboMem[3 * i] = 1.0;
                  pboMem[3 * i + 1] = 1.0;
                  pboMem[3 * i + 2] = 0.0;
               }

               if (useTexUnpackBuffer) {
                  glUnmapBufferARB_func(GL_PIXEL_UNPACK_BUFFER_ARB);
                  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TEXSIZE, TEXSIZE, 0,
                               GL_RGB, GL_FLOAT, NULL);
                  glBindBufferARB_func(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
               }
               else
                  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TEXSIZE, TEXSIZE, 0,
                               GL_RGB, GL_FLOAT, pboMem);

               if (useTexUnpackBuffer) {
                  if (breakCOWPBO) {
                     glBindBufferARB_func(GL_PIXEL_UNPACK_BUFFER_ARB, unpack_pb[0]);
                     pboMem =
                        (GLfloat *) glMapBufferARB_func(GL_PIXEL_UNPACK_BUFFER_ARB,
                                                GL_WRITE_ONLY);
                     for (i = 0; i < TEXSIZE * TEXSIZE * 3; i++)
                        pboMem[i] = 0.2;
                     glUnmapBufferARB_func(GL_PIXEL_UNPACK_BUFFER_ARB);
                     glBindBufferARB_func(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
                  }
               }

               if (useTexUnpackBuffer) {
                  if (breakCOWTexture) {
                     GLfloat temp[1 * 1 * 3];
                     for (i = 0; i < 1 * 1 * 3; i++)
                        temp[i] = 0.8;
                     glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_RGB,
                                     GL_FLOAT, temp);
                  }
               }

               // Check PBO's content
               if (useTexUnpackBuffer) {
                  glBindBufferARB_func(GL_PIXEL_UNPACK_BUFFER_ARB, unpack_pb[0]);
                  pboMem = (GLfloat *) glMapBuffer_func(GL_PIXEL_UNPACK_BUFFER_ARB,
                                             GL_READ_ONLY);
                  if (breakCOWPBO) {
                     for (i = 0; i < TEXSIZE * TEXSIZE * 3; i++)
                        if (fabsf(pboMem[i] - 0.2) > tolerance[0]) {
                           REPORT_FAILURE
                              ("PBO modified by someone else, there must be something wrong");
                           return false;
                        }
                  }
                  glUnmapBufferARB_func(GL_PIXEL_UNPACK_BUFFER_ARB);
                  glBindBufferARB_func(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
               }


               // Read texture back
               if (useTexPackBuffer) {
                  glGenBuffersARB_func(1, pack_pb);
                  glBindBufferARB_func(GL_PIXEL_PACK_BUFFER_ARB, pack_pb[0]);
                  glBufferDataARB_func(GL_PIXEL_PACK_BUFFER_ARB,
                               TEXSIZE * TEXSIZE * 3 * sizeof(GLfloat), NULL,
                               GL_STREAM_DRAW);
                  glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, NULL);
                  pboMem = (GLfloat *) glMapBufferARB_func(GL_PIXEL_PACK_BUFFER_ARB,
                                             GL_READ_ONLY);
               }
               else {
                  glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, t2);
                  pboMem = t2;
               }

               // Check texture image
               for (i = 0; i < TEXSIZE * TEXSIZE; i++) {
                  if (i == 0 && breakCOWTexture && useTexUnpackBuffer) {
                     GLfloat exp[3] = { 0.8, 0.8, 0.8 };
                     if (!equalColors(&pboMem[i * 3], exp)) {
                        REPORT_FAILURE("glGetTexImage failed");
                        printf("  got (%d) = [%f, %f, %f], ", i,
                               pboMem[i * 3],
                               pboMem[i * 3 + 1], pboMem[i * 3 + 2]);
                        printf("should be [%f, %f, %f]\n",
                               exp[0], exp[1], exp[2]);

                        return false;
                     }
                  }
                  else {
                     GLfloat exp[3] = { 1.0, 1.0, 0.0 };
                     if (!equalColors(&pboMem[i * 3], exp)) {
                        REPORT_FAILURE("glGetTexImage failed");
                        printf("  got (%d) = [%f, %f, %f], ", i,
                               pboMem[i * 3],
                               pboMem[i * 3 + 1], pboMem[i * 3 + 2]);
                        printf("should be [%f, %f, %f]\n",
                               exp[0], exp[1], exp[2]);

                        return false;
                     }
                  }
               }

               if (useTexPackBuffer) {
                  glUnmapBufferARB_func(GL_PIXEL_PACK_BUFFER_ARB);
                  glBindBufferARB_func(GL_PIXEL_PACK_BUFFER_ARB, 0);
                  glDeleteBuffersARB_func(1, pack_pb);
               }
               if (useTexUnpackBuffer) {
                  glDeleteBuffersARB_func(1, unpack_pb);
               }

               glEnable(GL_TEXTURE_2D);
               glBegin(GL_POLYGON);
               glTexCoord2f(0, 0);
               glVertex2f(0, 0);
               glTexCoord2f(1, 0);
               glVertex2f(TEXSIZE, 0);
               glTexCoord2f(1, 1);
               glVertex2f(TEXSIZE, TEXSIZE);
               glTexCoord2f(0, 1);
               glVertex2f(0, TEXSIZE);
               glEnd();
               glDisable(GL_TEXTURE_2D);

               glReadPixels(0, 0, windowSize, windowSize, GL_RGB, GL_FLOAT,
                            buf);
               for (j = 0; j < windowSize; j++) {
                  for (i = 0; i < windowSize; i++) {
                     if (i == 0 && j == 0 && breakCOWTexture
                         && useTexUnpackBuffer) {
                        GLfloat exp[3] = { 0.8, 0.8, 0.8 };
                        if (!equalColors(&buf[(j * windowSize + i) * 3], exp)) {
                           REPORT_FAILURE("glTexImage failed");
                           printf("  got (%d, %d) = [%f, %f, %f], ", i, j,
                                  buf[(j * windowSize + i) * 3],
                                  buf[(j * windowSize + i) * 3 + 1],
                                  buf[(j * windowSize + i) * 3 + 2]);
                           printf("should be [%f, %f, %f]\n",
                                  exp[0], exp[1], exp[2]);

                           return false;
                        }
                     }
                     else if (i < TEXSIZE && j < TEXSIZE) {
                        if (!equalColors(&buf[(j * windowSize + i) * 3], green)) {
                           REPORT_FAILURE("glTexImage failed");
                           printf("  got (%d, %d) = [%f, %f, %f], ", i, j,
                                  buf[(j * windowSize + i) * 3],
                                  buf[(j * windowSize + i) * 3 + 1],
                                  buf[(j * windowSize + i) * 3 + 2]);
                           printf("should be [%f, %f, %f]\n",
                                  green[0], green[1], green[2]);

                           return false;
                        }
                     }
                     else {
                        if (!equalColors(&buf[(j * windowSize + i) * 3], black)) {
                           REPORT_FAILURE("glTexImage failed");
                           printf("(%d, %d) = [%f, %f, %f], ", i, j,
                                  buf[(j * windowSize + i) * 3],
                                  buf[(j * windowSize + i) * 3 + 1],
                                  buf[(j * windowSize + i) * 3 + 2]);
                           printf("should be [0.0, 0.0, 0.0]\n");

                           return false;
                        }
                     }
                  }
               }
            }
         }
      }
   }

   return true;
}

bool PBOTest::testTexSubImage(void)
{
   GLuint pbs[1];
   GLfloat t[TEXSIZE * TEXSIZE * 3];
   int i, j;
   int useUnpackBuffer = 0;
   GLfloat green[3] = { 0.0, 1.0, 0.0 };
   GLfloat black[3] = { 0.0, 0.0, 0.0 };

   glBindBufferARB_func(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
   glBindBufferARB_func(GL_PIXEL_PACK_BUFFER_ARB, 0);

   for (useUnpackBuffer = 0; useUnpackBuffer < usePBO + 1; useUnpackBuffer++) {
      glClearColor(0.0, 0.0, 0.0, 1.0);
      glClear(GL_COLOR_BUFFER_BIT);

      if (useUnpackBuffer) {
         glGenBuffersARB_func(1, pbs);
         glBindBufferARB_func(GL_PIXEL_UNPACK_BUFFER_ARB, pbs[0]);
         glBufferDataARB_func(GL_PIXEL_UNPACK_BUFFER_ARB, TEXSIZE * TEXSIZE * 3 * sizeof(GLfloat),
                      NULL, GL_STREAM_DRAW);
         glBindBufferARB_func(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
      }

      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TEXSIZE, TEXSIZE, 0, GL_RGB,
                   GL_FLOAT, NULL);

      GLfloat *pboMem = NULL;
      if (useUnpackBuffer) {
         glBindBufferARB_func(GL_PIXEL_UNPACK_BUFFER_ARB, pbs[0]);
         pboMem = (GLfloat *) glMapBufferARB_func(GL_PIXEL_UNPACK_BUFFER_ARB,
                                    GL_WRITE_ONLY);
      }
      else {
         pboMem = t;
      }

      for (i = 0; i < TEXSIZE * TEXSIZE; i++) {
         pboMem[3 * i] = 0.0;
         pboMem[3 * i + 1] = 1.0;
         pboMem[3 * i + 2] = 0.0;
      }

      if (useUnpackBuffer) {
         glUnmapBufferARB_func(GL_PIXEL_UNPACK_BUFFER_ARB);
         glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, TEXSIZE, TEXSIZE, GL_RGB,
                         GL_FLOAT, NULL);
         glBindBufferARB_func(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
      }
      else
         glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, TEXSIZE, TEXSIZE, GL_RGB,
                         GL_FLOAT, pboMem);

      glEnable(GL_TEXTURE_2D);
      glBegin(GL_POLYGON);
      glTexCoord2f(0, 0);
      glVertex2f(0, 0);
      glTexCoord2f(1, 0);
      glVertex2f(10, 0);
      glTexCoord2f(1, 1);
      glVertex2f(10, 10);
      glTexCoord2f(0, 1);
      glVertex2f(0, 10);
      glEnd();
      glDisable(GL_TEXTURE_2D);

      GLfloat buf[windowSize * windowSize * 3];

      glReadPixels(0, 0, windowSize, windowSize, GL_RGB, GL_FLOAT, buf);
      for (j = 0; j < windowSize; j++) {
         for (i = 0; i < windowSize; i++) {
            if (i < 10 && j < 10) {
               if (!equalColors(&buf[(j * windowSize + i) * 3], green)) {
                  REPORT_FAILURE("glTexSubImage failed");
                  printf("  got (%d, %d) = [%f, %f, %f], ", i, j,
                         buf[(j * windowSize + i) * 3],
                         buf[(j * windowSize + i) * 3 + 1],
                         buf[(j * windowSize + i) * 3 + 2]);
                  printf("should be [%f, %f, %f]\n",
                         green[0], green[1], green[2]);

                  return false;
               }
            }
            else {
               if (!equalColors(&buf[(j * windowSize + i) * 3], black)) {
                  REPORT_FAILURE("glTexSubImage failed");
                  printf("(%d, %d) = [%f, %f, %f], ", i, j,
                         buf[(j * windowSize + i) * 3],
                         buf[(j * windowSize + i) * 3 + 1],
                         buf[(j * windowSize + i) * 3 + 2]);
                  printf("should be [0.0, 0.0, 0.0]\n");

                  return false;
               }

            }
         }
      }
   }
   return true;
}

bool PBOTest::testPolygonStip(void)
{
   int useUnpackBuffer = 0;
   int usePackBuffer = 0;
   GLuint unpack_pb[1];
   GLuint pack_pb[1];
   GLubyte t1[32 * 32 / 8];
   GLubyte t2[32 * 32 / 8];
   GLubyte *pboMem = NULL;
   int i, j;
   GLfloat white[3] = { 1.0, 1.0, 1.0 };
   GLfloat black[3] = { 0.0, 0.0, 0.0 };

   glBindBufferARB_func(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
   glBindBufferARB_func(GL_PIXEL_PACK_BUFFER_ARB, 0);

   for (useUnpackBuffer = 0; useUnpackBuffer < usePBO + 1; useUnpackBuffer++) {
      for (usePackBuffer = 0; usePackBuffer < usePBO + 1; usePackBuffer++) {
         glClearColor(0.0, 0.0, 0.0, 1.0);
         glClear(GL_COLOR_BUFFER_BIT);

         if (useUnpackBuffer) {
            glGenBuffersARB_func(1, unpack_pb);
            glBindBufferARB_func(GL_PIXEL_UNPACK_BUFFER_ARB, unpack_pb[0]);
            glBufferDataARB_func(GL_PIXEL_UNPACK_BUFFER_ARB, 32 * 32 / 8, NULL,
                         GL_STREAM_DRAW);
            pboMem = (GLubyte *) glMapBufferARB_func(GL_PIXEL_UNPACK_BUFFER_ARB,
                                       GL_WRITE_ONLY);
         }
         else {
            pboMem = t1;
         }


         // Fill in the stipple pattern
         for (i = 0; i < 32 * 32 / 8; i++) {
            pboMem[i] = 0xAA;
         }

         if (useUnpackBuffer) {
            glUnmapBufferARB_func(GL_PIXEL_UNPACK_BUFFER_ARB);
            glPolygonStipple(NULL);
         }
         else {
            glPolygonStipple(pboMem);
         }

         // Read back the stipple pattern
         if (usePackBuffer) {
            glGenBuffersARB_func(1, pack_pb);
            glBindBufferARB_func(GL_PIXEL_PACK_BUFFER_ARB, pack_pb[0]);
            glBufferDataARB_func(GL_PIXEL_PACK_BUFFER_ARB, 32 * 32 / 8, NULL,
                         GL_STREAM_DRAW);
            glGetPolygonStipple(NULL);
            pboMem = (GLubyte *) glMapBufferARB_func(GL_PIXEL_PACK_BUFFER_ARB,
                                       GL_READ_ONLY);
         }
         else {
            glGetPolygonStipple(t2);
            pboMem = t2;
         }

         for (i = 0; i < 32 * 32 / 8; i++) {
            if (pboMem[i] != 0xAA) {
               REPORT_FAILURE("glGetPolygonStipple failed");
               return false;
            }
         }


         if (useUnpackBuffer) {
            glBindBufferARB_func(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
            glDeleteBuffersARB_func(1, unpack_pb);
         }
         if (usePackBuffer) {
            glBindBufferARB_func(GL_PIXEL_PACK_BUFFER_ARB, 0);
            glDeleteBuffersARB_func(1, pack_pb);
         }

         glEnable(GL_POLYGON_STIPPLE);
         glColor4f(1.0, 1.0, 1.0, 0.0);
         glBegin(GL_POLYGON);
         glVertex2f(0, 0);
         glVertex2f(10, 0);
         glVertex2f(10, 10);
         glVertex2f(0, 10);
         glEnd();

         glDisable(GL_POLYGON_STIPPLE);

         // Check the result
         GLfloat buf[windowSize * windowSize * 3];

         glReadPixels(0, 0, windowSize, windowSize, GL_RGB, GL_FLOAT, buf);

         for (j = 0; j < windowSize; j++) {
            for (i = 0; i < windowSize; i++) {
               const GLfloat *exp;
               if (i & 1)
                  exp = black;
               else
                  exp = white;
               if (i < 10 && j < 10) {
                  if (!equalColors(&buf[(j * windowSize + i) * 3], exp)) {
                     REPORT_FAILURE("glGetPolygonStipple failed");
                     printf("(%d, %d) = [%f, %f, %f], ", i, j,
                            buf[(j * windowSize + i) * 3],
                            buf[(j * windowSize + i) * 3 + 1],
                            buf[(j * windowSize + i) * 3 + 2]);
                     printf("should be [1.0, 1.0, 1.0]\n");
                     return false;
                  }
               }
               else {
                  if (!equalColors(&buf[(j * windowSize + i) * 3], black)) {
                     REPORT_FAILURE("glGetPolygonStipple failed");
                     printf("(%d, %d) = [%f, %f, %f], ", i, j,
                            buf[(j * windowSize + i) * 3],
                            buf[(j * windowSize + i) * 3 + 1],
                            buf[(j * windowSize + i) * 3 + 2]);
                     printf("should be [0.0, 0.0, 0.0]\n");
                     return false;
                  }

               }
            }
         }

      }
   }

   return true;
}


bool PBOTest::testErrorHandling(void)
{
   GLuint fbs[1];

   glBindBufferARB_func(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
   glBindBufferARB_func(GL_PIXEL_PACK_BUFFER_ARB, 0);

   if (usePBO) {
      /* test that glDrawPixels from too small of buffer raises error */
      glGenBuffersARB_func(1, fbs);
      glBindBufferARB_func(GL_PIXEL_UNPACK_BUFFER, fbs[0]);
      glBufferDataARB_func(GL_PIXEL_UNPACK_BUFFER_ARB, 32 * 32 * 4, NULL,
                   GL_STREAM_DRAW);
      glDrawPixels(32, 32 + 1, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
      if (glGetError() != GL_INVALID_OPERATION)
         return false;

      glDeleteBuffersARB_func(1, fbs);
      glBindBufferARB_func(GL_PIXEL_UNPACK_BUFFER, 0);

      /* test that glReadPixels into too small of buffer raises error */
      glGenBuffersARB_func(1, fbs);
      glBindBufferARB_func(GL_PIXEL_PACK_BUFFER, fbs[0]);
      glBufferDataARB_func(GL_PIXEL_PACK_BUFFER_ARB, 32 * 32 * 4, NULL,
                   GL_STREAM_DRAW);
      glReadPixels(0, 0, 32, 32 + 1, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
      if (glGetError() != GL_INVALID_OPERATION)
         return false;

      glDeleteBuffersARB_func(1, fbs);
      glBindBufferARB_func(GL_PIXEL_PACK_BUFFER, 0);
   }
   return true;
}

void
PBOTest::runOne(MultiTestResult & r, Window & w)
{
   static SubTestFunc
      funcs[] = {
      &GLEAN::PBOTest::testSanity,
      &GLEAN::PBOTest::testBitmap,
      &GLEAN::PBOTest::testDrawPixels,
      &GLEAN::PBOTest::testPixelMap,
      &GLEAN::PBOTest::testTexImage,
      &GLEAN::PBOTest::testTexSubImage,
      &GLEAN::PBOTest::testPolygonStip,
      &GLEAN::PBOTest::testErrorHandling,
      NULL
   };

   (void) w;

   if (!setup()) {
      r.pass = false;
      return;
   }

   for (int i = 0; funcs[i]; i++)
      if ((this->*funcs[i]) ())
         r.numPassed++;
      else
         r.numFailed++;

   r.pass = (r.numFailed == 0);
}

// The test object itself:
PBOTest pboTest("pbo", "window, rgb, z", "",    // no extension filter 
                "pbo test: Test OpenGL Extension GL_ARB_pixel_buffer_object\n");



}  // namespace GLEAN
