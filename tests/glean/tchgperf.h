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


// tchgperf.h:  Some basic tests of attribute-change performance.

#ifndef __tchgperf_h__
#define __tchgperf_h__

#include "tbase.h"

namespace GLEAN {

#define drawingSize 128	// must be power-of-2, 128 or greater

class TexBindPerfResult: public BaseResult {
public:
	bool pass;
	double bindTime;
        double lowerBound;
	double upperBound;

	TexBindPerfResult() { bindTime = lowerBound = upperBound = 0.0; }

	void putresults(ostream& s) const {
		s << bindTime
		  << ' ' << lowerBound
		  << ' ' << upperBound
		  << '\n';
	}
	
	bool getresults(istream& s) {
		s >> bindTime >> lowerBound >> upperBound;
		return s.good();
	}

};

class TexBindPerf: public BaseTest<TexBindPerfResult> {
public:
	GLEAN_CLASS_WH(TexBindPerf, TexBindPerfResult,
	       drawingSize, drawingSize);
}; // class TexBindPerf

} // namespace GLEAN

#endif // __tchgperf_h__
