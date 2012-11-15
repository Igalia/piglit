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

// ttexture_srgb.h:  Test GL_EXT_texture_sRGB extension.
// Brian Paul  August 2006

#ifndef __ttexture_srgb_h__
#define __ttexture_srgb_h__

#include "tbase.h"

namespace GLEAN {

#define windowSize 128

class TextureSRGBResult: public BaseResult
{
public:
	bool pass;

	virtual void putresults(ostream& s) const;
	virtual bool getresults(istream& s);
};


class TextureSRGBTest: public BaseTest<TextureSRGBResult>
{
public:
	GLEAN_CLASS_WH(TextureSRGBTest, TextureSRGBResult,
		       windowSize, windowSize);

private:
        GLenum errorCode;
        const char *errorPos;
        char errorMsg[1000];

        bool testImageTransfer(void);
	bool testTextureFormat(GLenum intFormat, GLint components,
                               GLEAN::Environment &env);
        bool testTexturing(void);
};

} // namespace GLEAN

#endif // __ttexture_srgb_h__

