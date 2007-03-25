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

// tpointatten.h:  Test GL_ARB_point_parameters extension.
// Brian Paul  6 October 2005

#ifndef __tpointatten_h__
#define __tpointatten_h__

#include "tbasic.h"

namespace GLEAN {

#define drawingSize 101   // yes, odd
#define windowSize (drawingSize + 2)


class PointAttenuationTest: public BasicTest
{
public:
	PointAttenuationTest(const char *testName,
			     const char *filter,
			     const char *extensions,
			     const char *description);

	virtual void runOne(BasicResult& r, Window& w);
	virtual void logOne(BasicResult& r);

private:
	GLenum errorCode;
	const char *errorPos;
	GLfloat aliasedLimits[2];  // min/max
	GLfloat smoothLimits[2];   // min/max

	void setup(void);
	bool testPointRendering(GLboolean smooth);
	void reportFailure(GLfloat initSize,
			   const GLfloat attenuation[3],
			   GLfloat min, GLfloat max,
			   GLfloat eyeZ, GLboolean smooth,
			   GLfloat expected, GLfloat actual) const;
	void reportSuccess(int count, GLboolean smooth) const;
	GLfloat expectedSize(GLfloat initSize,
			     const GLfloat attenuation[3],
			     GLfloat min, GLfloat max,
			     GLfloat eyeZ, GLboolean smooth) const;
	GLfloat measureSize() const;
};

} // namespace GLEAN

#endif // __tpointatten_h__

