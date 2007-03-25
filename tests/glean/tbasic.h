// BEGIN_COPYRIGHT -*- glean -*-
// 
// Copyright (C) 1999-2000  Allen Akin   All Rights Reserved.
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



// tbasic.h:  Example class for basic tests

// This class illustrates the use of the BaseResult class and BaseTest
// template class for constructing straightforward portable tests. 
// See the file tbase.h for a discussion of this process.

// BasicTest simply runs on all drawing surface configurations that
// permit the creation of a window, and always passes.


#ifndef __tbasic_h__
#define __tbasic_h__

#include "tbase.h"

namespace GLEAN {

class BasicResult: public BaseResult {
public:
	bool pass;

	void putresults(ostream& s) const {
		s << pass << '\n';
	}
	
	bool getresults(istream& s) {
		s >> pass;
		return s.good();
	}
};

class BasicTest: public BaseTest<BasicResult> {
public:
	GLEAN_CLASS(BasicTest, BasicResult);
};

} // namespace GLEAN

#endif // __tbasic_h__
