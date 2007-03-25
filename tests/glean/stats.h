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




// stats.h: simple statistics-gathering utilities for glean

// These are rather simplistic.  For more robust implementations, consider
// using Numerical Recipes.

#ifndef __stats_h__
#define __stats_h__

#include <vector>

#ifdef __WIN__
using namespace std;
#endif

#if defined( __WIN__ )

#undef min
#undef max

#endif

namespace GLEAN {

class BasicStats {
	int _n;
	double _min;
	double _max;
	double _sum;
	double _sum2;
    public:
	void init();
	inline int n() const {return _n;}
	inline double min() const {return _min;}
	inline double max() const {return _max;}
	inline double sum() const {return _sum;}
	inline double sum2() const {return _sum2;}
	double mean() const;
	double variance() const;
	double deviation() const;
	inline void sample(double d) {
		++_n;
		if (d < _min)
			_min = d;
		if (d > _max)
			_max = d;
		_sum += d;
		_sum2 += d*d;
	}

	BasicStats() {init();}
	template<class T> BasicStats(std::vector<T>& v) {
		init();
		for (typename std::vector<T>::const_iterator p = v.begin(); p < v.end(); ++p)
			sample(*p);
	}
}; // class BasicStats

} // namespace GLEAN

#endif // __stats_h__
