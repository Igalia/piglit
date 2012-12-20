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


// ttexenv.h:  Test the basic texture env modes
// Author: Brian Paul (brianp@valinux.com)  April 2001


#ifndef __ttexenv_h__
#define __ttexenv_h__

#include "tbasic.h"

namespace GLEAN {

class TexEnvTest: public BasicTest {
    public:
	TexEnvTest(const char* testName, const char* filter,
		       const char* description):
	    BasicTest(testName, filter, description),
	    mTolerance() {
	}

	virtual void runOne(BasicResult& r, Window& w);
	virtual void logOne(BasicResult& r);

    private:
	bool TestColor(const GLfloat c1[3], const GLfloat c2[3]);
	void ComputeExpectedColor(GLenum envMode, GLenum texFormat,
		const GLfloat texColor[4], const GLfloat fragColor[4],
		const GLfloat envColor[4], GLfloat result[4]);
	void MakeTexImage(GLenum baseFormat, int numColors,
		const GLfloat colors[][4]);
	bool MatrixTest(GLenum envMode, GLenum texFormat,
		const char *envName, const char *formatName,
		int numColors, const GLfloat colors[][4],
		const GLfloat envColor[4], Window &w);
	GLfloat mTolerance[3];

}; // class TexEnvTest

} // namespace GLEAN

#endif // __ttexenv_h__
