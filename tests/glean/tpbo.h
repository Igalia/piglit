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
// tfbo.h:  Test OpenGL Extension GL_ARB_pixel_buffer_objec

#ifndef __pfbo_h__
#define __pfbo_h__

#include "tmultitest.h"

namespace GLEAN {

#define windowSize 100


class PBOTest: public MultiTest
{
public:
	PBOTest(const char* testName, const char* filter,
                 const char *extensions, const char* description):
		MultiTest(testName, filter, extensions, description)
	{
	}

	virtual void runOne(MultiTestResult &r, Window &w);

private:
        typedef bool (PBOTest::*TestFunc)(MultiTestResult &r);
        typedef bool (PBOTest::*SubTestFunc)(void);

	GLfloat tolerance[5];

        bool testFunctionality(MultiTestResult &r);
	bool testPerformance(MultiTestResult &r);
        bool testSanity(void);
	bool testErrorHandling(void);
	bool testDrawPixels(void);
	bool testPixelMap(void);
	bool testBitmap(void);
        bool testTexImage(void);
	bool testTexSubImage(void);
	bool testPolygonStip(void);

        void runSubTests(MultiTestResult &r);

	bool setup(void);
	bool checkResult(const GLfloat exp[4], const int depth, const int stencil) const;
	bool equalColors(const GLfloat a[4], const GLfloat b[4]) const;
	bool equalColors1(const GLubyte a[4], const GLubyte b[4]) const;

	void reportFailure(const char *msg, int line) const;
	void reportFailure(const char *msg, GLenum target, int line) const;
};

} // namespace GLEAN

#endif // __tpbo_h__
