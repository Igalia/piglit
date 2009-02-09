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

// tglsl1.h:  Test OpenGL shading language
// Brian Paul  6 March 2007

#ifndef __tglsl1_h__
#define __tglsl1_h__

#include "tmultitest.h"

namespace GLEAN {

#define windowSize 100


class ShaderProgram
{
public:
	const char *name;
	const char *vertShaderString;
	const char *fragShaderString;
	GLfloat expectedColor[4];
	GLfloat expectedZ;
	int flags;
};



class GLSLTest: public MultiTest
{
public:
	GLSLTest(const char* testName, const char* filter,
                 const char *extensions, const char* description):
		MultiTest(testName, filter, extensions, description)
	{
	}

	virtual void runOne(MultiTestResult &r, Window &w);

private:
	GLfloat tolerance[5];
	GLfloat looseTolerance[5];
        GLfloat glsl_120;   // GLSL 1.20 or higher supported?
        bool getFunctions(void);
        void setupTextures(void);
        void setupTextureMatrix1(void);
	bool setup(void);
	bool equalColors(const GLfloat a[4], const GLfloat b[4], int flags) const;
	bool equalDepth(GLfloat z0, GLfloat z1) const;
        GLuint loadAndCompileShader(GLenum target, const char *str);
        bool checkCompileStatus(GLenum target, GLuint shader,
                                const ShaderProgram &p);
	bool testProgram(const ShaderProgram &p);
	void reportFailure(const char *programName,
                           const GLfloat expectedColor[4],
                           const GLfloat actualColor[4] ) const;
	void reportZFailure(const char *programName,
			    GLfloat expectedZ, GLfloat actualZ) const;

};

} // namespace GLEAN

#endif // __tglsl1_h__
