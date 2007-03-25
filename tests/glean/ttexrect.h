// BEGIN_COPYRIGHT -*- glean -*-

/*
 * Copyright © 2006 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *    Eric Anholt <eric@anholt.net>
 *
 */


// ttexenv.h:  Test basic ARB_texture_rectangle support.
// Author: Eric Anholt <eric@anholt.net>

#ifndef __ttexrect_h__
#define __ttexrect_h__

#include "tbasic.h"

#define windowSize 256
namespace GLEAN {

class TexRectTest: public BasicTest {
    public:
	TexRectTest(const char* testName, const char* filter,
		    const char *prereqs, const char* description):
	    BasicTest(testName, filter, prereqs, description) {
	}

	virtual void runOne(BasicResult& r, Window& w);
	virtual void logOne(BasicResult& r);

    private:
	bool TestColor(const GLfloat c1[3], const GLfloat c2[3]);
	void CalculateTolerance();
	GLfloat mTolerance[3];

}; // class TexRectTest

} // namespace GLEAN

#endif // __ttexrect_h__
