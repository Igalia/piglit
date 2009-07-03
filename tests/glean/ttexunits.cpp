// BEGIN_COPYRIGHT -*- glean -*-
// 
// Copyright (C) 2008  VMWare, Inc.  All Rights Reserved.
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

// Test texture unit things
// We're generally just testing API-related things, not rendering.
// Brian Paul  31 Dec 2008

#define GL_GLEXT_PROTOTYPES

#include <cstring>
#include <cassert>
#include <math.h>
#include "ttexunits.h"


namespace GLEAN {

static PFNGLACTIVETEXTUREPROC glActiveTexture_func = NULL;
static PFNGLCLIENTACTIVETEXTUREPROC glClientActiveTexture_func = NULL;


void
TexUnitsTest::reportFailure(const char *msg) const
{
   env->log << "FAILURE:\n";
   env->log << "\t" << msg << "\n";
}


void
TexUnitsTest::reportFailure(const char *msg, GLint unit) const
{
   char s[100];
#if defined(_MSC_VER)
   _snprintf(s, sizeof(s), msg, unit);
#else
   snprintf(s, sizeof(s), msg, unit);
#endif
   env->log << "FAILURE:\n";
   env->log << "\t" << s << "\n";
}


bool
TexUnitsTest::setup(void)
{
   // check that we have OpenGL 2.x or 3.x
   const char *verString = (const char *) glGetString(GL_VERSION);

   if (verString[0] != '2' && verString[0] != '3') {
      env->log << "OpenGL 2.x or 3.x not supported\n";
      return false;
   }

   glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxImageUnits);
   glGetIntegerv(GL_MAX_TEXTURE_COORDS, &maxCoordUnits);
   glGetIntegerv(GL_MAX_TEXTURE_UNITS, &maxUnits);

   glActiveTexture_func = (PFNGLACTIVETEXTUREPROC) GLUtils::getProcAddress("glActiveTexture");
   assert(glActiveTexture_func);
   glClientActiveTexture_func = (PFNGLCLIENTACTIVETEXTUREPROC) GLUtils::getProcAddress("glClientActiveTexture");
   assert(glClientActiveTexture_func);

   return true;
}


bool
TexUnitsTest::testLimits(void)
{
   if (maxImageUnits < maxUnits) {
      reportFailure("GL_MAX_TEXTURE_IMAGE_UNITS < GL_MAX_TEXTURE_UNITS");
      return false;
   }
   if (maxCoordUnits < maxUnits) {
      reportFailure("GL_MAX_TEXTURE_COORD_UNITS < GL_MAX_TEXTURE_UNITS");
      return false;
   }
   return true;
}


bool
TexUnitsTest::testActiveTexture(void)
{
   GLint i;
   GLint maxUnits;

   if (maxImageUnits > maxCoordUnits)
      maxUnits = maxImageUnits;
   else
      maxUnits = maxCoordUnits;

   // clear any error state
   while (glGetError())
      ;

   // test glActiveTexture()
   for (i = 0; i < maxUnits; i++) {
      glActiveTexture_func(GL_TEXTURE0 + i);
      if (glGetError()) {
         reportFailure("glActiveTexture(GL_TEXTURE%d) failed", i);
         return false;
      }

      GLint unit;
      glGetIntegerv(GL_ACTIVE_TEXTURE, &unit);
      if (unit != GL_TEXTURE0 + i || glGetError()) {
         reportFailure("glGetIntegerv(GL_ACTIVE_TEXTURE) failed");
         return false;
      }
   }

   // this should fail:
   glActiveTexture_func(GL_TEXTURE0 + maxUnits);
   if (glGetError() != GL_INVALID_ENUM) {
      reportFailure("glActiveTexture(GL_TEXTURE%d) failed to generate an error",
                    maxUnits);
      return false;
   }


   // test glClientActiveTexture()
   for (i = 0; i < maxCoordUnits; i++) {
      glClientActiveTexture_func(GL_TEXTURE0 + i);
      if (glGetError()) {
         reportFailure("glClientActiveTexture(GL_TEXTURE%d) failed", i);
         return false;
      }

      GLint unit;
      glGetIntegerv(GL_CLIENT_ACTIVE_TEXTURE, &unit);
      if (unit != GL_TEXTURE0 + i || glGetError()) {
         reportFailure("glGetIntegerv(GL_CLIENT_ACTIVE_TEXTURE) failed");
         return false;
      }
   }

   // this should fail:
   glClientActiveTexture_func(GL_TEXTURE0 + maxUnits);
   if (glGetError() != GL_INVALID_ENUM) {
      reportFailure("glClientActiveTexture(GL_TEXTURE%d) failed to generate an error", maxUnits);
      return false;
   }

   return true;
}


bool
TexUnitsTest::testTextureMatrices(void)
{
   GLint i;

   glActiveTexture_func(GL_TEXTURE0);
   glMatrixMode(GL_TEXTURE);

   // set texture matrices
   for (i = 0; i < maxCoordUnits; i++) {
      glActiveTexture_func(GL_TEXTURE0 + i);

      // generate matrix
      GLfloat m[16];
      for (int j = 0; j < 16; j++) {
         m[j] = float(i * 100 + j);
      }

      glLoadMatrixf(m);
   }

   // query texture matrices
   for (i = 0; i < maxCoordUnits; i++) {
      glActiveTexture_func(GL_TEXTURE0 + i);

      // get matrix and check it
      GLfloat m[16];
      memset(m, 0, sizeof(m));
      glGetFloatv(GL_TEXTURE_MATRIX, m);

      if (glGetError()) {
         reportFailure("Query of texture matrix %d raised an error", i);
         return false;
      }

      for (int j = 0; j < 16; j++) {
         if (m[j] != float(i * 100 + j)) {
            reportFailure("Query of texture matrix %d failed", i);
            return false;
         }
      }
   }

   if (glGetError()) {
      reportFailure("GL error was generated while testing texture matrices");
      return false;
   }

   return true;
}


bool
TexUnitsTest::testTextureCoordGen(void)
{
   GLint i;

   glActiveTexture_func(GL_TEXTURE0);
   glMatrixMode(GL_TEXTURE);

   // test texgen enable/disable
   for (i = 0; i < maxUnits; i++) {
      glActiveTexture_func(GL_TEXTURE0 + i);

      glEnable(GL_TEXTURE_GEN_S);
      glEnable(GL_TEXTURE_GEN_T);
      glEnable(GL_TEXTURE_GEN_R);
      glEnable(GL_TEXTURE_GEN_Q);
      if (i < maxCoordUnits) {
         // should be no error
         if (glGetError()) {
            reportFailure("GL error was generated by enabling GL_TEXTURE_GEN_x, unit %d", i);
            return false;
         }
         glDisable(GL_TEXTURE_GEN_S);
         glDisable(GL_TEXTURE_GEN_T);
         glDisable(GL_TEXTURE_GEN_R);
         glDisable(GL_TEXTURE_GEN_Q);
      }
      else {
         // should be an error
         if (glGetError() != GL_INVALID_OPERATION) {
            reportFailure("GL error not generated by invalid enable of GL_TEXTURE_GEN_x, unit %d", i);
            return false;
         }
      }
   }

   return true;
}


bool
TexUnitsTest::testTexcoordArrays(void)
{
   GLint i;

   for (i = 0; i < maxCoordUnits; i++) {
      glClientActiveTexture_func(GL_TEXTURE0 + i);

      glEnableClientState(GL_TEXTURE_COORD_ARRAY);
      if (glGetError()) {
         reportFailure("GL error was generated by glEnableClientState for unit %d", i);
         return false;
      }
      glDisableClientState(GL_TEXTURE_COORD_ARRAY);

   }

   return true;
}


void
TexUnitsTest::runOne(MultiTestResult &r, Window &w)
{
   (void) w;

   if (!setup()) {
      r.pass = false;
      return;
   }

   if (testLimits())
      r.numPassed++;
   else
      r.numFailed++;

   if (testActiveTexture())
      r.numPassed++;
   else
      r.numFailed++;

   if (testTextureMatrices())
      r.numPassed++;
   else
      r.numFailed++;

   if (testTextureCoordGen())
      r.numPassed++;
   else
      r.numFailed++;

   if (testTexcoordArrays())
      r.numPassed++;
   else
      r.numFailed++;

   r.pass = (r.numFailed == 0);
}


// The test object itself:
TexUnitsTest texUnitTest("texUnits", "window, rgb",
                         "",  // no extension filter
                         "texUnits: test texture units.\n"
                         );



} // namespace GLEAN
