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


// ttexcombing.h:  Test the GL_EXT_texture_env_combine extension
// Author: Brian Paul (brianp@valinux.com)  September 2000


#ifndef __ttexcombine_h__
#define __ttexcombine_h__

#include "tbasic.h"

namespace GLEAN {

#define MAX_TEX_UNITS 8

class TexCombineTest: public BasicTest {
    public:
	TexCombineTest(const char* testName, const char* filter,
		       const char* description):
#if (__AGL__)
	    BasicTest(testName, filter, "GL_ARB_texture_env_combine",
		      description) {
#else
	    BasicTest(testName, filter, "GL_EXT_texture_env_combine",
		      description) {
#endif
		fWidth = 2;
		fHeight = 2;
	}

	virtual void runOne(BasicResult& r, Window& w);
	virtual void logOne(BasicResult& r);

    private:
	// Our model of GL machine state
	struct glmachine {
		GLenum COMBINE_RGB[MAX_TEX_UNITS];
		GLenum COMBINE_ALPHA[MAX_TEX_UNITS];
		GLenum SOURCE0_RGB[MAX_TEX_UNITS];
		GLenum SOURCE1_RGB[MAX_TEX_UNITS];
		GLenum SOURCE2_RGB[MAX_TEX_UNITS];
		GLenum SOURCE0_ALPHA[MAX_TEX_UNITS];
		GLenum SOURCE1_ALPHA[MAX_TEX_UNITS];
		GLenum SOURCE2_ALPHA[MAX_TEX_UNITS];
		GLenum OPERAND0_RGB[MAX_TEX_UNITS];
		GLenum OPERAND1_RGB[MAX_TEX_UNITS];
		GLenum OPERAND2_RGB[MAX_TEX_UNITS];
		GLenum OPERAND0_ALPHA[MAX_TEX_UNITS];
		GLenum OPERAND1_ALPHA[MAX_TEX_UNITS];
		GLenum OPERAND2_ALPHA[MAX_TEX_UNITS];
		GLfloat RGB_SCALE[MAX_TEX_UNITS];
		GLfloat ALPHA_SCALE[MAX_TEX_UNITS];
		GLfloat FragColor[4];                  // fragment color
		GLfloat EnvColor[MAX_TEX_UNITS][4];    // texture env color
		GLfloat TexColor[MAX_TEX_UNITS][4];    // texture image color
		GLenum TexFormat[MAX_TEX_UNITS];       // texture base format
		int NumTexUnits;
	};

	// describes possible state combinations
	struct test_param {
		GLenum target;
		GLenum validValues[6];
	};

	glmachine Machine;
	static test_param ReplaceParams[];
	static test_param ModulateParams[];
	static test_param AddParams[];
	static test_param AddSignedParams[];
	static test_param InterpolateParams[];
	static test_param Dot3RGBParams[];
	static test_param Dot3RGBAParams[];
	static test_param MultitexParams[];
	static test_param CrossbarParams[];
	bool haveDot3;
	bool haveCrossbar;
	GLfloat mTolerance[4];
	GLuint mTextures[MAX_TEX_UNITS];
	int testStride;

	void ResetMachine(glmachine &machine);
	void ComputeTexCombine(const glmachine &machine, int texUnit,
		const GLfloat prevColor[4], GLfloat result[4]) const;
	void PrintMachineState(const glmachine &machine) const;
	bool VerifyMachineState(const glmachine &machine) const;
	void ReportFailure(const glmachine &machine, const GLfloat expected[4],
		const GLfloat rendered[4], BasicResult &r, const char *where);
	void TexEnv(glmachine &machine, int texUnit, GLenum target,
		GLenum value);
	void SetupTestEnv(glmachine &machine, int texUnit, int testNum,
		const test_param testParams[]);
	void SetupColors(struct glmachine &machine);
	int CountTestCombinations(const test_param testParams[]) const;
	bool RunSingleTextureTest(glmachine &machine,
		const test_param testParams[], BasicResult &r, Window &w);
        int CountMultiTextureTestCombinations(const glmachine &machine) const;
	bool RunMultiTextureTest(glmachine &machine, BasicResult &r, Window &w);
	int CountCrossbarCombinations() const;
	bool RunCrossbarTest(glmachine &machine, BasicResult &r, Window &w);

	PFNGLACTIVETEXTUREARBPROC p_glActiveTextureARB;
	PFNGLMULTITEXCOORD2FARBPROC p_glMultiTexCoord2fARB;

}; // class TexCombineTest

} // namespace GLEAN

#endif // __ttexcombine_h__
