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


// tgetstr.h:  Check OpenGL vendor, renderer, version, and extension strings

// See tbasic.cpp for the basic test structure.


#ifndef __tgetstr_h__
#define __tgetstr_h__

#include "tbase.h"

namespace GLEAN {

class DrawingSurfaceConfig;		// Forward reference.

class GetStringResult: public BaseResult {
public:
	bool pass;
	string vendor;
	string renderer;
	string version;
	string extensions;

	void putresults(ostream& s) const {
		s << vendor << '\n';
		s << renderer << '\n';
		s << version << '\n';
		s << extensions << '\n';
	}
	
	bool getresults(istream& s) {
		getline(s, vendor);
		getline(s, renderer);
		getline(s, version);
		getline(s, extensions);
		return s.good();
	}
};

class GetStringTest: public BaseTest<GetStringResult> {
public:
	GLEAN_CLASS(GetStringTest, GetStringResult);
}; // class GetStringTest

} // namespace GLEAN

#endif // __tgetstr_h__
