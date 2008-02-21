// BEGIN_COPYRIGHT -*- glean -*-
// 
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

// tdepthstencil.h:  Test GL_EXT_packed_depth_stencil extension.
// Brian Paul  1 October 2005

#ifndef __tdepthstencil_h__
#define __tdepthstencil_h__

#include "tbase.h"

namespace GLEAN {

#define drawingSize 1000
#define windowSize (drawingSize + 2)

class DepthStencilResult: public BaseResult
{
public:
	bool pass;
	double readDepthStencilRate;  // pixels/second
	double readDepthUintRate;  // pixels/second
	double readDepthUshortRate;  // pixels/second

        DepthStencilResult();

	virtual void putresults(ostream& s) const;
	virtual bool getresults(istream& s);
};


class DepthStencilTest: public BaseTest<DepthStencilResult>
{
public:
	GLEAN_CLASS_WH(DepthStencilTest, DepthStencilResult,
		       windowSize, windowSize);

private:
        int depthBits, stencilBits;
        GLenum errorCode;
        const char *errorPos;
        char errorMsg[1000];

        bool checkError(const char *where);
        void setup(void);
        bool testInsufficientVisual(void);
        bool testErrorDetection(void);
        bool testDrawAndRead(void);
	bool testTextureOperations(void);
	void testPerformance(DepthStencilResult &r);
	double readPixelsRate(GLenum format, GLenum type);
};

} // namespace GLEAN

#endif // __tdepthstencil_h__

