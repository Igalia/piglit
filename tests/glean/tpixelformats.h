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

// Brian Paul  September 2006

#ifndef __tpixelformats_h__
#define __tpixelformats_h__

#include "tmultitest.h"

namespace GLEAN {

#define windowSize 100


class PixelFormatsTest: public MultiTest
{
public:
	PixelFormatsTest(const char* testName, const char* filter,
					 const char *extensions, const char* description)
		: MultiTest(testName, filter, extensions, description)
	{
		alphaBits = 0;
		defaultAlpha = 0;
		haveHalfFloat = false;
		haveABGR = false;
		haveSRGB = false;
		haveCombine = false;
	}

	virtual void runOne(MultiTestResult &r, Window &w);

private:
	int alphaBits;
	int defaultAlpha;  // depends on texture env mode
	// extensions
	bool haveHalfFloat;
	bool haveABGR;
	bool haveSRGB;
	bool haveCombine;
	bool haveRG;
	bool haveFloat;
	bool haveSnorm;
	bool haveTexSharedExp;

	bool CheckError(const char *where) const;

	bool CompatibleFormatAndType(GLenum format, GLenum datatype) const;

	bool SupportedIntFormat(GLint intFormat) const;

	bool DrawImage(int width, int height,
				   GLenum format, GLenum type, GLint intFormat,
				   const GLubyte *image) const;

	void ComputeExpected(GLenum srcFormat, int testChan,
						 GLint intFormat, GLubyte exp[4]) const;

	bool CheckRendering(int width, int height, int color,
			    GLenum format, GLint intFormat) const;

	bool TestCombination(GLenum format, GLenum type, GLint intFormat);

	void setup(void);
};

} // namespace GLEAN

#endif // __tpixelformats_h__

