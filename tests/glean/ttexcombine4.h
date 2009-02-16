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

// ttexcombine4.h:  Test GL_NV_texture_env_combine4

#ifndef __ttexcombine4_h__
#define __ttexcombine4_h__

#include "tbase.h"
#include "rand.h"


namespace GLEAN {

#define NUM_POINTS 1000
#define WINDOW_SIZE 100
#define NUM_TESTS 200


class TexCombine4Result: public BaseResult
{
public:
        TexCombine4Result();

	virtual void putresults(ostream& s) const;
	virtual bool getresults(istream& s);

	bool pass;
};


class TexCombine4Test: public BaseTest<TexCombine4Result>
{
public:
	GLEAN_CLASS_WH(TexCombine4Test, TexCombine4Result,
		       WINDOW_SIZE, WINDOW_SIZE);

private:
        struct combine_state
        {
           GLenum CombineMode;
           GLenum Source[4];
           GLenum OperandRGB[4];
           GLenum OperandA[4];
           GLfloat PrimaryColor[4];
           GLfloat ConstantColor[4];
           GLfloat TextureColor[4];
        };

	RandomDouble rand;

        void generate_state(struct combine_state &state);
        void evaluate_state(const struct combine_state &state, GLfloat result[4]);
        bool render_state(const struct combine_state &state, GLfloat result[4]);
        void report_state(const struct combine_state &state);

        void reportError(const char *msg);
};

} // namespace GLEAN

#endif // __ttexcombine4_h__

