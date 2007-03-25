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


// tpgos.h:  Polygon offset tests.
// Derived in part from tests written by Angus Dorbie <dorbie@sgi.com>
// in September 2000 and Rickard E. (Rik) Faith <faith@valinux.com> in
// October 2000.

#ifndef __tpgos_h__
#define __tpgos_h__

#include "tbase.h"
#include <iomanip>

namespace GLEAN {

// Auxiliary struct for holding a glPolygonOffset result
class POResult: public BaseResult {
public:
	bool pass;
	double nextToNear;
	double nextToFar;
	double idealMRDNear;
	double idealMRDFar;
	double actualMRDNear;
	double actualMRDFar;
	bool bigEnoughMRD;
	bool smallEnoughMRD;
	bool slopeOffsetsPassed;
	float failingAngle;
	float failingAxis[3];
	double failingOffset;
	double minGoodOffset;
	double maxGoodOffset;

	POResult() {
		pass = true;
		nextToNear = 1.0;
		nextToFar = 2.0;
		idealMRDNear = 0.1;
		idealMRDFar = 0.1;
		actualMRDNear = 0.1;
		actualMRDFar = 0.1;
		bigEnoughMRD = true;
		smallEnoughMRD = true;
		slopeOffsetsPassed = true;
		failingAngle = 0.0;
		failingAxis[0] = failingAxis[1] = failingAxis[2] = 0.0;
		failingOffset = 1.0;
		minGoodOffset = 0.1;
		maxGoodOffset = 0.1;
	}
	
	void putresults(ostream& s) const {
		s << setprecision(16)
		  << pass << '\n'
		  << nextToNear << ' ' << nextToFar << '\n'
		  << idealMRDNear << ' ' << idealMRDFar << '\n'
		  << actualMRDNear << ' ' << actualMRDFar << '\n'
		  << bigEnoughMRD << ' ' << smallEnoughMRD << '\n'
		  << slopeOffsetsPassed << '\n'
		  << failingAngle << '\n'
		  << failingAxis[0] << ' ' << failingAxis[1] << ' '
		  	<< failingAxis[2] << '\n'
		  << failingOffset << ' ' << minGoodOffset << ' '
		  	<< maxGoodOffset << '\n'
		  ;
	}
	
	bool getresults(istream& s) {
		s >> pass
		  >> nextToNear
		  >> nextToFar
		  >> idealMRDNear
		  >> idealMRDFar
		  >> actualMRDNear
		  >> actualMRDFar
		  >> bigEnoughMRD
		  >> smallEnoughMRD
		  >> slopeOffsetsPassed
		  >> failingAngle
		  >> failingAxis[0] >> failingAxis[1] >> failingAxis[2]
		  >> failingOffset >> minGoodOffset >> maxGoodOffset
		  ;
		return s.good();
	}
};

#define PGOS_WIN_SIZE 128

class PgosTest: public BaseTest<POResult> {
public:
	GLEAN_CLASS_WH(PgosTest, POResult, PGOS_WIN_SIZE, PGOS_WIN_SIZE);
}; // class PgosTest

} // namespace GLEAN

#endif // __tpgos_h__
