// BEGIN_COPYRIGHT -*- glean -*-
// 
// Copyright (C) 2008  VMware, Inc.  All rights reserved.
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


#ifndef __ttexunits_h__
#define __ttexunits_h__

#include "tmultitest.h"

namespace GLEAN {

#define windowSize 100


class TexUnitsTest: public MultiTest
{
public:
	TexUnitsTest(const char* testName, const char* filter,
                 const char *extensions, const char* description):
		MultiTest(testName, filter, extensions, description)
	{
	}

	virtual void runOne(MultiTestResult &r, Window &w);

private:
        GLint maxCombinedUnits;
        GLint maxImageUnits;
        GLint maxCoordUnits;
        GLint maxUnits;

	void reportFailure(const char *msg) const;
	void reportFailure(const char *msg, GLint unit) const;

        bool setup(void);
        bool testLimits(void);
        bool testActiveTexture(void);
        bool testTextureMatrices(void);
        bool testTextureCoordGen(void);
        bool testTexcoordArrays(void);
};


} // namespace GLEAN

#endif // __ttexunits_h__
