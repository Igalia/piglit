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

// timer.h:  Simple benchmark timing utilities based on the previous timer.h
// Modified from timer.h by Rickard E. (Rik) Faith <faith@valinux.com>

// Timer objects provide a framework for measuring the rate at which an
// operation can be performed.

#ifndef __timer_h__
#define __timer_h__

namespace GLEAN {

class Timer {
	double overhead;	// Overhead (in seconds) of initial op,
				// final op, and timer access.

	int    calibrated;	// Has calibrate been called?

	double chooseRunTime();	// Select a runtime that will reduce random
				// timing error to an acceptable level.
	
public:
	virtual void   premeasure() {}; // called in measure(), before time()
	virtual void   postmeasure() {}; // called in measure(), after time()
	virtual void   preop()  {}; // before op, in each loop in time()
	virtual void   op()     {}; // in each loop in time()
	virtual void   postop() {}; // after op, in each loop in time()
	virtual double compute(double t) {
		// modify measure()'s result -- e.g., by computing a rate
		return t;
	}
	
	void         calibrate();
	double       time();
	double       getClock();    // Get wall-clock time, in seconds
	double       waitForTick(); // Wait for next clock tick; return time
	void         measure(int count,
			     double* low, double* avg, double* high);
	
	Timer() { overhead = 0.0; calibrated = false; }
        virtual ~Timer() { /* just silence warning */ }

}; // class Timer

} // namespace GLEAN

#endif // __timer_h__
