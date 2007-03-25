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



// tbasicperf.h:  Example class for basic performance tests

// This class provides a framework for performance tests.  Like most
// glean tests, it's derived from the BaseResult class and BaseTest
// template class; see tbase.h for further information.  However, it
// is specialized to include member variables and functions that show
// how to perform timing operations, save results, and compare
// results.

// To produce a customized benchmark, one can derive a new class from
// this one and override the runOne() function to perform the timing
// measurements.  For more information, see tbasicperf.cpp.


#ifndef __tbasicperf_h__
#define __tbasicperf_h__

#include "tbase.h"

class DrawingSurfaceConfig;		// Forward reference.

namespace GLEAN {

class BasicPerfResult: public BaseResult {
public:
	bool   pass;
	double timeAvg, timeLow, timeHigh;
	
	BasicPerfResult() {
		timeAvg = timeLow = timeHigh = 0.0;
	}

	void putresults(ostream& s) const {
		s << pass
		  << ' ' << timeAvg
		  << ' ' << timeLow
		  << ' ' << timeHigh
		  << '\n';
	}

	bool getresults(istream& s) {
		s >> pass >> timeAvg >> timeLow >> timeHigh;
		return s.good();
	}
};

class BasicPerfTest: public BaseTest<BasicPerfResult> {
public:
	GLEAN_CLASS(BasicPerfTest, BasicPerfResult);
	void logStats(BasicPerfResult& r);
}; // class BasicPerfTest

} // namespace GLEAN

#endif // __tbasicperf_h__
