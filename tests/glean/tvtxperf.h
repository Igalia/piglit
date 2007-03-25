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

// tvtxperf.h:  Test performance of various ways to specify vertex data

#ifndef __tvtxperf_h__
#define __tvtxperf_h__

#include "tbase.h"

namespace GLEAN {

#define drawingSize 256

// Auxiliary struct for holding a vertex-performance result:
class VPSubResult {
public:
	double tps;		// Triangles Per Second
	double tpsLow;		// Low end of tps range
	double tpsHigh;		// High end of tps range
	bool imageOK;		// Image sanity-check status
	bool imageMatch;	// Image comparison status

	VPSubResult() {
		tps = tpsLow = tpsHigh = 0.0;
		imageOK = imageMatch = true;
	}
	
	void put(ostream& s) const {
		s << tps
		  << ' ' << tpsLow
		  << ' ' << tpsHigh
		  << ' ' << imageOK
		  << ' ' << imageMatch
		  << '\n';
	}
	
	void get(istream& s) {
		s >> tps >> tpsLow >> tpsHigh >> imageOK >> imageMatch;
	}
};

class VPResult: public BaseResult {
public:
	bool	    skipped;	// prerequisite tests failed
	bool        pass;
	
	VPSubResult imTri;	// immediate-mode independent triangles
	VPSubResult dlTri;	// display-listed independent triangles
	VPSubResult daTri;	// DrawArrays independent triangles
	VPSubResult ldaTri;	// Locked DrawArrays independent tris
	VPSubResult deTri;	// DrawElements independent triangles
	VPSubResult ldeTri;	// Locked DrawElements ind. tris
	
	VPSubResult imTS;	// immediate-mode triangle strip
	VPSubResult dlTS;	// display-listed triangle strip
	VPSubResult daTS;	// DrawArrays triangle strip
	VPSubResult ldaTS;	// Locked DrawArrays triangle strip
	VPSubResult deTS;	// DrawElements triangle strip
	VPSubResult ldeTS;	// Locked DrawElements triangle strip
		
	virtual void putresults(ostream& s) const {
		s
			<< skipped << '\n'
			<< pass << '\n'
			;

		imTri.put(s);
		dlTri.put(s);
		daTri.put(s);
		ldaTri.put(s);
		deTri.put(s);
		ldeTri.put(s);

		imTS.put(s);
		dlTS.put(s);
		daTS.put(s);
		ldaTS.put(s);
		deTS.put(s);
		ldeTS.put(s);
	}
	
	virtual bool getresults(istream& s) {
		s
			>> skipped
			>> pass
			;
		imTri.get(s);
		dlTri.get(s);
		daTri.get(s);
		ldaTri.get(s);
		deTri.get(s);
		ldeTri.get(s);

		imTS.get(s);
		dlTS.get(s);
		daTS.get(s);
		ldaTS.get(s);
		deTS.get(s);
		ldeTS.get(s);
		
		return s.good();
	}
};

class ColoredLitPerf: public BaseTest<VPResult> {
public:
	GLEAN_CLASS_WHO(ColoredLitPerf, VPResult,
			drawingSize, drawingSize, true);
	void logStats(VPResult& r, GLEAN::Environment* env);
}; // class ColoredLitPerf

class ColoredTexPerf: public BaseTest<VPResult> {
public:
	GLEAN_CLASS_WHO(ColoredTexPerf, VPResult,
			drawingSize, drawingSize, true);
	void logStats(VPResult& r, GLEAN::Environment* env);
}; // class ColoredTexPerf

} // namespace GLEAN

#endif // __tvtxperf_h__
