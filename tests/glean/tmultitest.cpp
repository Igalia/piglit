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

#include <cstring>
#include "tmultitest.h"

namespace GLEAN {


MultiTestResult::MultiTestResult()
{
	numPassed = numFailed = 0;
	pass = true;
}


void
MultiTestResult::putresults(ostream &s) const
{
	if (pass) {
		s << "PASS\n";
	}
	else {
		s << "FAIL\n";
	}
	s << numPassed << '\n';
	s << numFailed << '\n';
}


bool
MultiTestResult::getresults(istream &s)
{
	char result[1000];
	s >> result;
	if (strcmp(result, "FAIL") == 0) {
		pass = false;
	}
	else {
		pass = true;
	}
	s >> numPassed;
	s >> numFailed;
	return s.good();
}


// VERY IMPORTANT: this function _must_ be defined here, even though
// it's never used.  Otherwise, you'll get linker errors like this:
// tmultitest.h:83: undefined reference to `vtable for GLEAN::MultiTest'
void
MultiTest::runOne(MultiTestResult &r, Window &)
{
	r.numPassed = r.numFailed = 0;
	r.pass = true;
}


void
MultiTest::logOne(MultiTestResult &r)
{
	if (r.numPassed == 0 && r.numFailed == 0) {
		// non-applicable test
		env->log << name << ":  NOTE ";
		logConcise(r);
		env->log << "\tTest skipped/non-applicable\n";
	}
	else {
		logPassFail(r);
		logConcise(r);
		env->log << "\t"
				 << r.numPassed << " tests passed, "
				 << r.numFailed << " tests failed.\n";
	}
}


void
MultiTest::compareOne(MultiTestResult &oldR,
			    MultiTestResult &newR)
{
	if (oldR.numPassed != newR.numPassed ||
		oldR.numFailed != newR.numFailed) {
		env->log << "Different results: passed: "
				 << oldR.numPassed
				 << " vs. "
				 << newR.numPassed
				 << "  failed: "
				 << oldR.numFailed
				 << " vs. "
				 << newR.numFailed
				 << "\n";
	}
}


#if 0
MultiTest multiTest("multi", "window",
					"",
	"Base class for multi pass/fail tests\n"
	);
#endif


} // namespace GLEAN
