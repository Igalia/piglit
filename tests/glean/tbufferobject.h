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

#ifndef __tbufferobject_h__
#define __tbufferobject_h__

#include "tbase.h"
#include "rand.h"

namespace GLEAN {

#define windowSize 100

class BufferObjectResult: public BaseResult
{
public:
   bool pass;

   BufferObjectResult();

   virtual void putresults(ostream& s) const;
   virtual bool getresults(istream& s);
};


class BufferObjectTest: public BaseTest<BufferObjectResult>
{
public:
   GLEAN_CLASS_WH(BufferObjectTest, BufferObjectResult,
                  windowSize, windowSize);

private:
   bool have_ARB_vertex_buffer_object;
   bool have_ARB_pixel_buffer_object;
   bool have_ARB_copy_buffer;
   bool have_ARB_map_buffer_range;

   GLenum target1, target2;

   RandomBase rand;

   bool setup(void);

   bool testCopyBuffer(void);
   bool testMapBufferRange(void);
};

} // namespace GLEAN

#endif // __tbufferobject_h__

