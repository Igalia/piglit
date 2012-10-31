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

// tstencil2.h:  Test two-sided stencil features

#ifndef __tstencil2_h__
#define __tstencil2_h__

#include "tbase.h"

namespace GLEAN {

#define windowSize 100

class Stencil2Result: public BaseResult
{
public:
	bool pass;

	Stencil2Result();

	virtual void putresults(ostream& s) const;
	virtual bool getresults(istream& s);
};


class Stencil2Test: public BaseTest<Stencil2Result>
{
public:
	GLEAN_CLASS_WH(Stencil2Test, Stencil2Result,
		       windowSize, windowSize);

	bool isApplicable() const;

private:
	GLint stencilBits, stencilMax;

	void get_ext_functions();

	bool have_ATI_separate_stencil(void) const;
	bool have_EXT_stencil_two_side(void) const;
	bool have_GL2_stencil_two_side(void) const;
	bool have_stencil_wrap(void) const;

	bool render_test(GLuint expectedFront, GLuint expectedBack);

	bool compare_state(int method, GLenum found, GLenum expected, const char *msg);

	bool set_stencil_state(int method,
			       GLenum frontStencilFail,
			       GLenum backStencilFail,
			       GLenum frontZFail,
			       GLenum backZFail,
			       GLenum frontZPass,
			       GLenum backZPass,
			       GLenum frontFunc,
			       GLenum backFunc,
			       GLint frontRef,
			       GLint backRef,
			       GLuint frontMask,
			       GLuint backMask,
			       GLuint frontWriteMask,
			       GLuint backWriteMask);

	bool set_stencil_state(int method,
			       GLenum frontStencilFail,
			       GLenum backStencilFail,
			       GLenum frontZFail,
			       GLenum backZFail,
			       GLenum frontZPass,
			       GLenum backZPass,
			       GLenum frontFunc,
			       GLenum backFunc,
			       GLint ref,
			       GLuint mask,
			       GLuint writeMask = ~0);

	void reset_stencil_state(int method);

	bool test_stencil(GLint method);

};

} // namespace GLEAN

#endif // __tstencil2_h__

