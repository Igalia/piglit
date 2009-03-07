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

// tTexSwizzle.h:  Test GL_EXT_texture_swizzle

#ifndef __ttexswizzle_h__
#define __ttexswizzle_h__

#include "tbase.h"
#include "rand.h"

namespace GLEAN {

#define windowSize 50

class TexSwizzleResult: public BaseResult
{
public:
	bool pass;

	TexSwizzleResult();

	virtual void putresults(ostream& s) const;
	virtual bool getresults(istream& s);
};


class TexSwizzleTest: public BaseTest<TexSwizzleResult>
{
public:
	GLEAN_CLASS_WH(TexSwizzleTest, TexSwizzleResult,
		       windowSize, windowSize);

private:
	RandomBase rand;

	void RandomColor(GLubyte *color);
	void SetTextureColor(const GLubyte *color);
	GLubyte Swizzle(const GLubyte *texColor, GLenum swizzle);
	void ComputeExpectedColor(const GLubyte *texColor,
				  GLenum swizzleR,
				  GLenum swizzleG,
				  GLenum swizzleB,
				  GLenum swizzleA,
				  GLubyte *expectedColor);
	const char *SwizzleString(GLenum swizzle);
	void ReportFailure(GLenum swizzleR,
			   GLenum swizzleG,
			   GLenum swizzleB,
			   GLenum swizzleA,
			   const GLubyte *texColor,
			   const GLubyte *actual,
			   const GLubyte *expected);
	bool TestAPI(void);
	bool TestSwizzles(void);
	bool TestSwizzlesWithProgram(void);
	void Setup(void);
};

} // namespace GLEAN

#endif // __ttexswizzle_h__

