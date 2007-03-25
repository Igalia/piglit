// BEGIN_COPYRIGHT -*- glean -*-
// 
// Copyright (C) 2000  Allen Akin   All Rights Reserved.
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




// tbinding.h:  Test functions in the window-system binding

#ifndef __tbinding_h__
#define __tbinding_h__

#include "tbasic.h"

class DrawingSurfaceConfig;		// Forward reference.
class GLEAN::Window;

namespace GLEAN {

#define drawingSize 64

class MakeCurrentResult: public BaseResult {
public:
	bool pass;
	// Short descriptions of the rendering contexts:
	vector<const char*> descriptions;
	// Complete record of rendering contexts made "current" during
	// the test:
	vector<int> testSequence;

	void putresults(ostream& s) const {
		s << pass << '\n';
	}
	
	bool getresults(istream& s) {
		s >> pass;
		return s.good();
	}
};

class MakeCurrentTest: public BaseTest<MakeCurrentResult> {
public:
	GLEAN_CLASS_WH(MakeCurrentTest, MakeCurrentResult,
		       drawingSize, drawingSize);
}; // class MakeCurrentTest

} // namespace GLEAN

#endif // __tbinding_h__
