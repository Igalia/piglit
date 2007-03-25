// BEGIN_COPYRIGHT
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




// basic.cpp:  basic statistics utilities for glean

#include <cfloat>
#include <cmath>
#include "stats.h"

namespace GLEAN {

void
BasicStats::init() {
	_min = DBL_MAX;
	_max = -DBL_MAX;
	_sum = _sum2 = 0.0;
	_n = 0;
}

double
BasicStats::mean() const {return _n? (_sum / _n): 0.0;}

double
BasicStats::variance() const {
	if (_n < 2)
		return 0.0;
	return (_sum2 - _sum * _sum / _n) / (_n - 1);
		// Not really numerically robust, but good enough for us.
}
double
BasicStats::deviation() const {
	const double v = variance();
	return (v < 0.0)? 0.0: sqrt(v);
}

} // namespace GLEAN
