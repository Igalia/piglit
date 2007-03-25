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

// tvertprog.h:  Test GL_ARB_vertex_program extension.
// Brian Paul  22 October 2005

#ifndef __tvertprog_h__
#define __tvertprog_h__

#include "tmultitest.h"

namespace GLEAN {

#define windowSize 100

// to indicate a looser tolerance test is needed
#define FLAG_NONE   0
#define FLAG_LOOSE  1

class VertexProgram
{
public:
	const char *name;
	const char *progString;
	const GLfloat expectedColor[4];
	const GLfloat expectedZ;
	int flags;
};



class VertexProgramTest: public MultiTest
{
public:
	VertexProgramTest(const char* testName, const char* filter,
			  const char *extensions, const char* description):
		MultiTest(testName, filter, extensions, description)
	{
	}

	virtual void runOne(MultiTestResult &r, Window &w);

private:
	GLfloat tolerance[5];
	GLfloat looseTolerance[5];
	void setup(void);
	bool equalColors(const GLfloat a[4], const GLfloat b[4], int flags) const;
	bool equalDepth(GLfloat z0, GLfloat z1) const;
	bool testProgram(const VertexProgram &p);
	void reportFailure(const char *programName,
                           const GLfloat expectedColor[4],
                           const GLfloat actualColor[4] ) const;
	void reportZFailure(const char *programName,
			    GLfloat expectedZ, GLfloat actualZ) const;

};

} // namespace GLEAN

#endif // __tvertprog_h__
