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

// tvertarraybgra.h:  Test GL_EXT_vertex_array bgra

#ifndef __tvertarraybgra_h__
#define __tvertarraybgra_h__

#include "tbase.h"


namespace GLEAN {

#define NUM_POINTS 1000
#define WINDOW_SIZE 100

class VertArrayBGRAResult: public BaseResult
{
public:
        VertArrayBGRAResult();

	virtual void putresults(ostream& s) const;
	virtual bool getresults(istream& s);

	bool pass;
};


class VertArrayBGRATest: public BaseTest<VertArrayBGRAResult>
{
public:
	GLEAN_CLASS_WH(VertArrayBGRATest, VertArrayBGRAResult,
		       WINDOW_SIZE, WINDOW_SIZE);

private:
        float mPos[NUM_POINTS][2];
        GLubyte mRGBA[NUM_POINTS][4];
        GLubyte mBGRA[NUM_POINTS][4];

        void reportError(const char *msg);
        bool testAPI(void);
        void setupPoints(void);
        void renderPoints(bool useBGRA);
};

} // namespace GLEAN

#endif // __tvertarraybgra_h__

