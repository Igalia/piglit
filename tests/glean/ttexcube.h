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


// ttexcube.h:  Test the GL_ARB_texture_cube_map extension
// Author: Brian Paul (brianp@valinux.com)  March 2001


#ifndef __ttexcube_h__
#define __ttexcube_h__

#include "tbasic.h"

namespace GLEAN {

class TexCubeTest: public BasicTest {
    public:
	TexCubeTest(const char* testName, const char* filter,
		       const char* description):
	    BasicTest(testName, filter, "GL_ARB_texture_cube_map",
		      description) {
	}

	virtual void runOne(BasicResult& r, Window& w);
	virtual void logOne(BasicResult& r);

    private:
	bool TestColor(const GLfloat c1[3], const GLfloat c2[3]);
	void BuildTexImage(GLenum target, const GLfloat color[4][3]);
	bool TestNormalMap(bool testTexCoords, const char *modeName);
	bool TestReflectionMap(const char *modeName);

	GLfloat mColors[6][4][3];
	GLfloat mTolerance[3];

}; // class TexCubeTest

} // namespace GLEAN

#endif // __ttexcube_h__
