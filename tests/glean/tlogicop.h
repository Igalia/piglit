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

// tlogicop.h:  Test RGBA logic op functions.
// Based on Allen's blendFunc test.
// Brian Paul  10 May 2001

#ifndef __tlogicop_h__
#define __tlogicop_h__

#include "tbase.h"

namespace GLEAN {

#define drawingSize 64
#define windowSize (drawingSize + 2)

class LogicopFuncResult: public BaseResult {
public:
	bool pass;	// not written to log file

	struct PartialResult {
		GLenum logicop;	// The logic op
		float rbErr;	// Max readback error, in bits.
		float opErr;	// Max logicop error, in bits.
	};
	vector<PartialResult> results;
	
	virtual void putresults(ostream& s) const;
	virtual bool getresults(istream& s);
};

class LogicopFuncTest: public BaseTest<LogicopFuncResult> {
public:
	GLEAN_CLASS_WH(LogicopFuncTest, LogicopFuncResult,
		       windowSize, windowSize);
}; // class LogicopFuncTest

} // namespace GLEAN

#endif // __tlogicop_h__
