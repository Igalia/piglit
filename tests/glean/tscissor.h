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


// tscissor.h:  Basic test of OpenGL scissor.
//
// Author: Gareth Hughes <gareth@valinux.com>  December 2000


#ifndef __tscissor_h__
#define __tscissor_h__

#include "tbasic.h"

namespace GLEAN {

class ScissorTest: public BaseTest<BasicResult> {
public:
	GLEAN_CLASS_WH( ScissorTest, BasicResult, 10, 10 );

	void FailMessage( BasicResult &r, int err, int x, int y,
			  int sx, int sy, int sw, int sh ) const;
}; // class ScissorTest

} // namespace GLEAN

#endif // __tscissor_h__
