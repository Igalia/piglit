// BEGIN_COPYRIGHT -*- glean -*-
// 
// Copyright (C) 1999-2000  Allen Akin   All Rights Reserved.
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

// timer.cpp: Implementation of simple benchmark timer utilities.

// This particular implementation is derived from the one in libpdb,
// part of the isfast library for OpenGL.

// XXXWIN as of 5/8/99: The code for Windows timing is taken from
// Michael Gold's implementation of libpdb.  I've probably introduced
// some bugs in the translation, unfortunately.  [Allen]

// Modified from original timer.cpp by Rickard E. (Rik) Faith
// <faith@valinux.com>, December 2000

#include "timer.h"
#include <vector>
#include <algorithm>
using namespace std;

#if defined(__UNIX__)
#    include <sys/time.h>	// for gettimeofday, used by getClock
#elif defined(__MS__)
#    include <windows.h>
#    include <sys/types.h>
#    include <sys/timeb.h>	// for _ftime(), used by getClock
#endif

namespace GLEAN {

///////////////////////////////////////////////////////////////////////////////
// calibrate:  Determine overhead of measurement, initialization routine,
//	and finalization routine
///////////////////////////////////////////////////////////////////////////////
void
Timer::calibrate() {
	double runTime = chooseRunTime();

	preop();

	long reps = 0;
	double current;
	double start = waitForTick();
	do {
		postop();
		++reps;
	} while ((current = getClock()) < start + runTime);

	overhead   = (current - start) / (double) reps;
	calibrated = true;
} // Timer::calibrate

///////////////////////////////////////////////////////////////////////////////
// chooseRunTime:  Select an appropriate runtime for benchmarks.
//	By running for at least 10000 ticks, and attempting to keep timing
//	accurate to one tick, we hope to make our results repeatable.
//	(ignoring all the other stuff that might be going on in	the system,
//	of course).  Long runs reduce the effect of measurement error, but
//	short runs reduce the chance that some other process on the system
//	will steal time.
///////////////////////////////////////////////////////////////////////////////
double
Timer::chooseRunTime() {
	double start = getClock();
	double finish;

	// Wait for next tick:
	while ((finish = getClock()) == start)
		;
	
	// Run for 10000 ticks, clamped to [0.1 sec, 5.0 sec]:
	double runTime = 10000.0 * (finish - start);
	if (runTime < 0.1)
		runTime = 0.1;
	else if (runTime > 5.0)
		runTime = 5.0;

	return runTime;
} // Timer::chooseRunTime

///////////////////////////////////////////////////////////////////////////////
// getClock - get current wall-clock time (expressed in seconds)
///////////////////////////////////////////////////////////////////////////////
double
Timer::getClock() {
#if defined(__MS__)
	static int once = 1;
	static double freq;

	if (once) {
	    LARGE_INTEGER fr;
	    freq = (double) (QueryPerformanceFrequency(&fr) ?
			     1.0 / fr.QuadPart : 0);
	    once = 0;
	}

	// Use high-resolution counter, if available
	if (freq) {
	    LARGE_INTEGER pc;
	    QueryPerformanceCounter(&pc);
	    return freq * (double) pc.QuadPart;
	} else {
	    struct _timeb t;

	    _ftime(&t);

	    return (double) t.time + (double) t.millitm * 1E-3;
	}
#elif defined(__UNIX__)
	struct timeval t;

	// XXX gettimeofday is different on SysV, if I remember correctly
	gettimeofday(&t, 0);

	return (double) t.tv_sec + (double) t.tv_usec * 1E-6;
#endif
} // Timer::getClock

///////////////////////////////////////////////////////////////////////////////
// waitForTick:  wait for beginning of next system clock tick; return the time.
///////////////////////////////////////////////////////////////////////////////
double
Timer::waitForTick() {
	double start;
	double current;

	start = getClock();

	// Wait for next tick:
	while ((current = getClock()) == start)
		;

	// Start timing:
	return current;
} // Timer::waitForTick

///////////////////////////////////////////////////////////////////////////////
// time:  measure time (in seconds) to perform caller's operation
///////////////////////////////////////////////////////////////////////////////
double
Timer::time() {
	// Select a run time that's appropriate for our timer resolution:
	double runTime = chooseRunTime();

	// Measure successively larger batches of operations until we find
	// one that's long enough to meet our runtime target:
	long reps = 1;
	double start;
	double current;
	for (;;) {
		preop();

		start = waitForTick();

		for (long i = reps; i > 0; --i) op();


		postop();

		current = getClock();
		if (current >= start + runTime + overhead)
			break;

		// Try to reach runtime target in one fell swoop:
		long newReps;
		if (current > start + overhead)
			newReps = static_cast<long> (reps *
			       (0.5 + runTime / (current - start - overhead)));
		else
			newReps = reps * 2;
		if (newReps == reps)
			reps += 1;
		else
			reps = newReps;
		}

	// Subtract overhead to determine the final operation rate:
	return (current - start - overhead) / reps;
} // Timer::time

///////////////////////////////////////////////////////////////////////////////
// measure:  measure several results for performing caller's operation
///////////////////////////////////////////////////////////////////////////////
void
Timer::measure(int count, double* low, double* avg, double* high)
{
	vector<double> m;
	double         sum = 0.0;

	if (!calibrated) calibrate();
	if (count < 3) count = 3;
	premeasure();
	for (int i = 0; i < count; i++) {
		preop();
		double t = time();
		postop();
		m.push_back(compute(t));
	}
	postmeasure();
	sort(m.begin(), m.end());
	for (int j = 1; j < count - 1; j++) sum += m[j];
	*avg  = sum / (count - 2);
	*low  = m[1];
	*high = m[count - 2];
} // Timer::measure

} // namespace GLEAN
