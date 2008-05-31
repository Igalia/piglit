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

// tapi2.h:  Test OpenGL 2.x API functions/features
// Brian Paul  9 March 2007

#ifndef __tapi2_h__
#define __tapi2_h__

#include "tmultitest.h"

namespace GLEAN {

#define windowSize 100


class API2Test: public MultiTest
{
public:
	API2Test(const char* testName, const char* filter,
                 const char *extensions, const char* description):
		MultiTest(testName, filter, extensions, description)
	{
	}

	virtual void runOne(MultiTestResult &r, Window &w);

private:
        typedef bool (API2Test::*TestFunc)(void);

	GLfloat tolerance[5];
        bool getFunctions_2_0(char **errorFunc);

        GLuint loadAndCompileShader(GLenum target, const char *str);
        GLuint createProgram(GLuint vertShader, GLuint fragShader);

        void renderQuad(GLfloat *pixel) const;
        void renderQuadWithArrays(GLint attr, const GLfloat value[4],
                                  GLfloat *pixel) const;

        bool testStencilFuncSeparate(void);
        bool testStencilOpSeparate(void);
        bool testStencilMaskSeparate(void);
        bool testBlendEquationSeparate(void);
        bool testDrawBuffers(void);
        bool testShaderObjectFuncs(void);
        bool testUniformfFuncs(void);
        bool testUniformiFuncs(void);
        bool testShaderAttribs(void);

        void runSubTests(MultiTestResult &r);

	bool setup(void);
	bool equalColors(const GLfloat a[4], const GLfloat b[4]) const;

	void reportFailure(const char *msg, int line) const;
	void reportFailure(const char *msg, GLenum target, int line) const;
};

} // namespace GLEAN

#endif // __tglsl1_h__
