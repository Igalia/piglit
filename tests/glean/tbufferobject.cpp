// BEGIN_COPYRIGHT -*- glean -*-
// 
// Copyright (C) 2009  VMware, Inc. All Rights Reserved.
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
// PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL VMWARE BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
// AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
// OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
// 
// END_COPYRIGHT

// tbufferobject.c - test various buffer object features/extensions
// Author: Brian Paul


#define GL_GLEXT_PROTOTYPES
#include <cassert>
#include <cmath>
#include <cstring>
#include <stdlib.h>
#include "tbufferobject.h"


namespace GLEAN {


// GL_ARB_vertex/fragment_program
static PFNGLGENBUFFERSARBPROC glGenBuffersARB_func = NULL;
static PFNGLDELETEBUFFERSARBPROC glDeleteBuffersARB_func = NULL;
static PFNGLBINDBUFFERARBPROC glBindBufferARB_func = NULL;
static PFNGLBUFFERDATAARBPROC glBufferDataARB_func = NULL;
static PFNGLMAPBUFFERARBPROC glMapBufferARB_func = NULL;
static PFNGLUNMAPBUFFERARBPROC glUnmapBufferARB_func = NULL;

// GL_ARB_copy_buffer
static PFNGLCOPYBUFFERSUBDATAPROC glCopyBufferSubData_func = NULL;

// GL_ARB_map_buffer_range
static PFNGLMAPBUFFERRANGEPROC glMapBufferRange_func = NULL;
static PFNGLFLUSHMAPPEDBUFFERRANGEPROC glFlushMappedBufferRange_func = NULL;


BufferObjectResult::BufferObjectResult()
{
   pass = false;
}


bool
BufferObjectTest::setup(void)
{
   have_ARB_vertex_buffer_object = GLUtils::haveExtension("GL_ARB_vertex_buffer_object");
   have_ARB_pixel_buffer_object = GLUtils::haveExtension("GL_ARB_pixel_buffer_object");
   have_ARB_copy_buffer = GLUtils::haveExtension("GL_ARB_copy_buffer");
   have_ARB_map_buffer_range = GLUtils::haveExtension("GL_ARB_map_buffer_range");

   if (have_ARB_vertex_buffer_object) {
      target1 = GL_ARRAY_BUFFER_ARB;
      target2 = GL_ELEMENT_ARRAY_BUFFER_ARB;
   }
   else if (have_ARB_pixel_buffer_object) {
      target1 = GL_PIXEL_PACK_BUFFER_ARB;
      target2 = GL_PIXEL_UNPACK_BUFFER_ARB;
   }
   else {
      return false;
   }


   glGenBuffersARB_func = (PFNGLGENBUFFERSARBPROC) GLUtils::getProcAddress("glGenBuffersARB");
   glDeleteBuffersARB_func = (PFNGLDELETEBUFFERSARBPROC) GLUtils::getProcAddress("glDeleteBuffersARB");
   glBindBufferARB_func = (PFNGLBINDBUFFERARBPROC) GLUtils::getProcAddress("glBindBufferARB");
   glBufferDataARB_func = (PFNGLBUFFERDATAARBPROC) GLUtils::getProcAddress("glBufferDataARB");
   glMapBufferARB_func = (PFNGLMAPBUFFERARBPROC) GLUtils::getProcAddress("glMapBufferARB");
   glUnmapBufferARB_func = (PFNGLUNMAPBUFFERARBPROC) GLUtils::getProcAddress("glUnmapBufferARB");

   if (have_ARB_copy_buffer) {
      glCopyBufferSubData_func = (PFNGLCOPYBUFFERSUBDATAPROC) GLUtils::getProcAddress("glCopyBufferSubData");
   }

   if (have_ARB_map_buffer_range) {
      glMapBufferRange_func = (PFNGLMAPBUFFERRANGEPROC) GLUtils::getProcAddress("glMapBufferRange");
      glFlushMappedBufferRange_func = (PFNGLFLUSHMAPPEDBUFFERRANGEPROC) GLUtils::getProcAddress("glFlushMappedBufferRange");
   }

   return true;
}


// test GL_ARB_copy_buffer
bool
BufferObjectTest::testCopyBuffer(void)
{
   static const GLsizei size1 = 4200, size2 = 3800;
   GLubyte buf1[size1], buf2[size2];
   GLuint bufs[2];
   GLubyte *map;
   GLint i;
   bool pass;

   glGenBuffersARB_func(2, bufs);

   // setup first buffer
   glBindBufferARB_func(target1, bufs[0]);
   glBufferDataARB_func(target1, size1, NULL, GL_STATIC_DRAW);
   map = (GLubyte *) glMapBufferARB_func(target1, GL_WRITE_ONLY_ARB);
   for (i = 0; i < size1; i++) {
      map[i] = buf1[i] = i & 0xff;
   }
   glUnmapBufferARB_func(target1);

   // setup second buffer
   glBindBufferARB_func(target2, bufs[1]);
   glBufferDataARB_func(target2, size2, NULL, GL_STATIC_DRAW);
   map = (GLubyte *) glMapBufferARB_func(target2, GL_WRITE_ONLY_ARB);
   for (i = 0; i < size2; i++) {
      map[i] = buf2[i] = 0;
   }
   glUnmapBufferARB_func(target2);

   // copy random sections of first buffer to second buffer
   for (i = 0; i < 50; i++) {
      const int min = size1 < size2 ? size1 : size2;
      int size = rand.next() % min;
      int srcOffset = rand.next() % (size1 - size);
      int dstOffset = rand.next() % (size2 - size);

      assert(srcOffset + size <= size1);
      assert(dstOffset + size <= size2);

      // test copy from first buffer to second
      glCopyBufferSubData_func(target1,   // src target
                               target2,   // dst target
                               srcOffset, // src start
                               dstOffset, // dst start
                               size);

      // update the validation/reference buffer in the same way
      memcpy(buf2 + dstOffset, buf1 + srcOffset, size);
   }

   // no errors should have been generated.
   if (glGetError()) {
      env->log << "Unexpected GL error in copy buffer test.\n";
      return false;
   }

   // check results in second buffer object
   map = (GLubyte *) glMapBufferARB_func(target2, GL_READ_ONLY_ARB);
   pass = true;
   for (i = 0; i < size2; i++) {
      if (map[i] != buf2[i]) {
         printf("%d: %d != %d\n", i, map[i], buf2[i]);
         pass = false;
         break;
      }
   }
   glUnmapBufferARB_func(target2);

   glDeleteBuffersARB_func(2, bufs);

   return pass;
}


// Test GL_ARB_map_buffer_range
// This isn't exhaustive, but covers the basics.
bool
BufferObjectTest::testMapBufferRange(void)
{
   static const GLsizei size = 30000;
   GLubyte buf[size];
   GLuint buffer;
   GLubyte *map;
   GLint i, j;
   bool pass = true;

   // create buffer
   glGenBuffersARB_func(1, &buffer);
   glBindBufferARB_func(target1, buffer);
   glBufferDataARB_func(target1, size, NULL, GL_STATIC_DRAW);

   // initialize to zeros
   map = (GLubyte *) glMapBufferRange_func(target1, 0, size, GL_MAP_WRITE_BIT);
   for (i = 0; i < size; i++) {
      map[i] = buf[i] = 0;
   }
   glUnmapBufferARB_func(target1);

   // write to random ranges
   for (i = 0; i < 50; i++) {
      const int mapSize = rand.next() % size;
      const int mapOffset = rand.next() % (size - mapSize);

      assert(mapOffset + mapSize <= size);

      map = (GLubyte *)
         glMapBufferRange_func(target1, mapOffset, mapSize,
                               GL_MAP_WRITE_BIT | GL_MAP_FLUSH_EXPLICIT_BIT);

      for (j = 0; j < mapSize; j++) {
         map[j] = buf[mapOffset + j] = (mapOffset + j) & 0xff;
      }

      glFlushMappedBufferRange_func(target1, 0, mapSize);

      glUnmapBufferARB_func(target1);
   }

   if (glGetError())
      pass = false;

   // read/check random ranges
   for (i = 0; i < 50 && pass; i++) {
      const int mapSize = rand.next() % size;
      const int mapOffset = rand.next() % (size - mapSize);

      assert(mapOffset + mapSize <= size);

      map = (GLubyte *) glMapBufferRange_func(target1, mapOffset,
                                              mapSize, GL_MAP_READ_BIT);

      for (j = 0; j < mapSize; j++) {
         if (map[j] != buf[mapOffset + j]) {
            pass = false;
            break;
         }
      }
      glUnmapBufferARB_func(target1);
   }

   glDeleteBuffersARB_func(1, &buffer);

   if (glGetError())
      pass = false;

   return pass;
}


void
BufferObjectTest::runOne(BufferObjectResult &r, Window &w)
{
   (void) w;  // silence warning
   r.pass = true;

   if (!setup()) {
      // if neither GL_ARB_vertex/pixel_buffer_object are supported, do nothing
      r.pass = true;
      return;
   }

   if (r.pass && have_ARB_copy_buffer)
      r.pass = testCopyBuffer();

   if (r.pass && have_ARB_map_buffer_range)
      r.pass = testMapBufferRange();
}


void
BufferObjectTest::logOne(BufferObjectResult &r)
{
   logPassFail(r);
   logConcise(r);
}


void
BufferObjectResult::putresults(ostream &s) const
{
   if (pass) {
      s << "PASS\n";
   }
   else {
      s << "FAIL\n";
   }
}


bool
BufferObjectResult::getresults(istream &s)
{
   char result[1000];
   s >> result;

   if (strcmp(result, "FAIL") == 0) {
      pass = false;
   }
   else {
      pass = true;
   }
   return s.good();
}


// The test object itself:
BufferObjectTest bufferObjectTest("bufferObject",
                                  "window, rgb",
                                  "", // no extension tests
                                  "Test buffer object features and extensions such as:\n"
                                  "  GL_ARB_vertex_buffer_object\n"
                                  "  GL_ARB_pixel_buffer_object\n"
                                  "  GL_ARB_copy_buffer\n"
                                  "  GL_ARB_map_buffer_range\n");



} // namespace GLEAN


