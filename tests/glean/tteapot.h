// BEGIN_COPYRIGHT -*- glean -*-
// 
// Copyright (C) 2000  Adam Haberlach   All Rights Reserved.
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

#ifndef __tteapot_h_
#define __tteapot_h_

#include "tbase.h"


// Simple teapot-drawing benchmark provided by Adam Haberlach.

namespace GLEAN {

class TeapotResult: public BaseResult {
public:
	bool pass;
	double fTps; // speed in "Teapots per Second"

	void putresults(ostream& s) const {
		s << pass << '\n';
		s << fTps << '\n';
	}
	
	bool getresults(istream& s) {
		s >> pass;
		s >> fTps;
		return s.good();
	}
};

class TeapotTest: public BaseTest<TeapotResult> {
public:
	GLEAN_CLASS_WH(TeapotTest, TeapotResult, 300, 315);
};

} // namespace GLEAN

#endif // __tteapot_h_
