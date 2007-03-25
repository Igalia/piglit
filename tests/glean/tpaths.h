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


// tpaths.h:  Basic test of GL rendering paths.
// Author: Brian Paul (brianp@valinux.com)  November 2000


#ifndef __tpaths_h__
#define __tpaths_h__

#include "tbasic.h"

namespace GLEAN {

class PathsTest: public BasicTest {
    public:
	PathsTest(const char* testName, const char* filter,
		  const char* description):
	    BasicTest(testName, filter, description) {
	}

	virtual void runOne(BasicResult& r, Window& w);
	virtual void logOne(BasicResult& r);

    private:
	enum Path {
		ALPHA,
		BLEND,
		COLOR_MASK,
		DEPTH,
		LOGIC,
		SCISSOR,
		STENCIL,
		STIPPLE,
		TEXTURE,
		ZZZ  // end-of-list token
	};
	enum State {
		DISABLE,
		ALWAYS_PASS,
		ALWAYS_FAIL
	};

	const char *PathName(Path path) const;
	void FailMessage(BasicResult &r, Path path, State state,
			 GLfloat pixel[3]) const;
	void SetPathState(Path path, State state) const;

}; // class PathsTest

} // namespace GLEAN

#endif // __tpaths_h__
