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

// tbasicperf.cpp:  implementation of example class for basic
// performance tests

// To customize this for benchmarking a particular function,
// create a new performance test object of type GLEAN::Perf,
// overriding the preop(), op(), and postop() methods as needed.
// (For OpenGL timing tests preop() and postop() will both call
// glFinish(), but other pre- and post-ops may be used for
// timing things other than OpenGL.)  Then invoke the object's
// calibrate() and time() methods as shown in runOne().

#include "tbasicperf.h"
#include "timer.h"
#include <algorithm>

namespace {
class MyPerf : public GLEAN::Timer {
public:
	int msec;

	void preop()  { glFinish(); }
	void op()     {
#ifdef __WIN__
		Sleep(msec);	/* milliseconds */
#else
		usleep(msec*1000); /* microseconds */
#endif
	}
	void postop() { glFinish(); }

	MyPerf() { msec = 100; }
};


// Complex results helper functions

void
diffHeader(bool& same, const string& name,
           GLEAN::DrawingSurfaceConfig* config, GLEAN::Environment* env) {
        if (same) {
                same = false;
                env->log << name << ":  DIFF "
                         << config->conciseDescription() << '\n';
        }
} // diffHeader

}

namespace GLEAN {

///////////////////////////////////////////////////////////////////////////////
// runOne:  Run a single test case
///////////////////////////////////////////////////////////////////////////////
void
BasicPerfTest::runOne(BasicPerfResult& r, Window &w) {
	MyPerf perf;

	perf.calibrate();
	vector<float> m;
	for (int i = 0; i < 5; i++) {
		env->quiesce();
		double t = perf.time();
		w.swap();	// So user can see something
		m.push_back(t);
	}
	sort(m.begin(), m.end());
	r.timeAvg  = (m[1] + m[2] + m[3]) / 3.0;
	r.timeLow  = m[1];
	r.timeHigh = m[3];
	r.pass        = true;
} // BasicPerfTest::runOne

///////////////////////////////////////////////////////////////////////////////
// logOne:  Log a single test case
///////////////////////////////////////////////////////////////////////////////
void
BasicPerfTest::logOne(BasicPerfResult& r) {
	logPassFail(r);
	logConcise(r);
	logStats(r);
} // BasicPerfTest::logOne

///////////////////////////////////////////////////////////////////////////////
// compareOne:  Compare results for a single test case
///////////////////////////////////////////////////////////////////////////////
void
BasicPerfTest::compareOne(BasicPerfResult& oldR, BasicPerfResult& newR) {
	bool same = true;
	const char *title = "100mS sleep";

	if (newR.timeLow < oldR.timeLow) {
		double percent = (100.0
				  * (oldR.timeLow - newR.timeLow)
				  / newR.timeLow + 0.5);
		if (percent >= 5.0) {
			diffHeader(same, name, oldR.config, env);
			env->log << '\t'
				 << env->options.db1Name
				 << " may be "
				 << percent
				 << "% faster on "
				 << title
				 << '\n';
		}
        }
	if (newR.timeHigh > oldR.timeHigh) {
		double percent = (100.0
				  * (newR.timeHigh - oldR.timeHigh)
				  / oldR.timeHigh + 0.5);
		if (percent >= 5.0) {
			diffHeader(same, name, oldR.config, env);
			env->log << '\t'
				 << env->options.db2Name
				 << " may be "
				 << percent
				 << "% faster on "
				 << title
				 << '\n';
		}
        }

	if (same && env->options.verbosity) {
		env->log << name
			 << ": SAME "
			 << newR.config->conciseDescription()
			 << "\n\t"
			 << env->options.db2Name
			 << " test time falls within the"
			 << " valid measurement range of\n\t"
			 << env->options.db1Name
			 << " test time.\n";
	}
	if (env->options.verbosity) {
		env->log << env->options.db1Name << ':';
		logStats(oldR);
		env->log << env->options.db2Name << ':';
		logStats(newR);
	}
} // BasicPerfTest::compareOne

void
BasicPerfTest::logStats(BasicPerfResult& r) {
        env->log << "\tAverage = "
		 << r.timeAvg
		 << "\tRange = ["
		 << r.timeLow
		 << ", "
		 << r.timeHigh
		 << "]\n";
}

///////////////////////////////////////////////////////////////////////////////
// The test object itself:
///////////////////////////////////////////////////////////////////////////////
BasicPerfTest basicPerfTest("basicPerf", "window",
	"This trivial test simply verifies the internal support for basic\n"
	"performance tests.  It is run on every OpenGL-capable drawing surface\n"
	"configuration that supports creation of a window.  If everything is\n"
	"working correctly, each result should be close to 0.1 second.\n");

} // namespace GLEAN
