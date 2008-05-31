// BEGIN_COPYRIGHT -*- glean -*-
// 
// Copyright (C) 2001  Allen Akin   All Rights Reserved.
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


// treadpix.h:  ReadPixels tests.

#ifndef __treadpix_h__
#define __treadpix_h__

#include <iomanip>
#include "tbase.h"

namespace GLEAN {

class ReadPixSanityResult: public BaseResult {
public:
	bool pass;

	bool passRGBA;
	int xRGBA;
	int yRGBA;
	float errRGBA;
	GLfloat expectedRGBA[4];
	GLfloat actualRGBA[4];

	bool passDepth;
	int xDepth;
	int yDepth;
	float errDepth;
	GLdouble expectedDepth;
	GLdouble actualDepth;

	bool passStencil;
	int xStencil;
	int yStencil;
	GLuint expectedStencil;
	GLuint actualStencil;

	bool passIndex;
	int xIndex;
	int yIndex;
	GLuint expectedIndex;
	GLuint actualIndex;

	ReadPixSanityResult() {
		pass = true;

		passRGBA = true;
		xRGBA = yRGBA = 0;
		errRGBA = 0.0;
		expectedRGBA[0] = expectedRGBA[1] = expectedRGBA[2]
			= expectedRGBA[3] = 0.0;
		actualRGBA[0] = actualRGBA[1] = actualRGBA[2]
			= actualRGBA[3] = 0.0;
			
		passDepth = true;
		xDepth = yDepth = 0;
		errDepth = 0.0;
		expectedDepth = 0.0;
		actualDepth = 0.0;

		passStencil = true;
		xStencil = yStencil = 0;
		expectedStencil = 0;
		actualStencil = 0;

		passIndex = true;
		xIndex = yIndex = 0;
		expectedIndex = 0;
		actualIndex = 0;
	}
	
	void putresults(ostream& s) const {
		s
		  << pass << '\n'

		  << passRGBA << '\n'
		  << xRGBA << ' ' << yRGBA << '\n'
		  << errRGBA << '\n'
		  << expectedRGBA[0] << ' ' << expectedRGBA[1] << ' '
		  	<< expectedRGBA[2] << ' ' << expectedRGBA[3] << '\n'
		  << actualRGBA[0] << ' ' << actualRGBA[1] << ' '
		  	<< actualRGBA[2] << ' ' << actualRGBA[3] << '\n'

		  << passDepth << '\n'
		  << xDepth << ' ' << yDepth << '\n'
		  << errDepth << '\n'
		  << setprecision(16)
		  << expectedDepth << '\n'
		  << actualDepth << '\n'

		  << passStencil << '\n'
		  << xStencil << ' ' << yStencil << '\n'
		  << expectedStencil << '\n'
		  << actualStencil << '\n'

		  << passIndex << '\n'
		  << xIndex << ' ' << yIndex << '\n'
		  << expectedIndex << '\n'
		  << actualIndex << '\n'
		  ;
	}
	
	bool getresults(istream& s) {
		s >> pass

		  >> passRGBA
		  >> xRGBA >> yRGBA
		  >> errRGBA
		  >> expectedRGBA[0] >> expectedRGBA[1] >> expectedRGBA[2]
		  	>> expectedRGBA[3]
		  >> actualRGBA[0] >> actualRGBA[1] >> actualRGBA[2]
		  	>> actualRGBA[3]

		  >> passDepth
		  >> xDepth >> yDepth
		  >> errDepth
		  >> expectedDepth
		  >> actualDepth

		  >> passStencil
		  >> xStencil >> yStencil
		  >> expectedStencil
		  >> actualStencil

		  >> passIndex
		  >> xIndex >> yIndex
		  >> expectedIndex
		  >> actualIndex
		  ;
		return s.good();
	}
};

#define READPIX_SANITY_WIN_SIZE 32

class ReadPixSanityTest: public BaseTest<ReadPixSanityResult> {
public:
	GLEAN_CLASS_WH(ReadPixSanityTest, ReadPixSanityResult,
		READPIX_SANITY_WIN_SIZE, READPIX_SANITY_WIN_SIZE);

	void checkRGBA(ReadPixSanityResult& r, Window& w);
	void checkDepth(ReadPixSanityResult& r, Window& w);
	void checkStencil(ReadPixSanityResult& r, Window& w);
	void checkIndex(ReadPixSanityResult& r, Window& w);
	void summarize(const char* label, bool oldPass, bool newPass);
}; // class ReadPixSanityTest
extern ReadPixSanityTest readPixSanityTest;




class ExactRGBAResult: public BaseResult {
public:
	struct Flavor {
		bool pass;
		int x;
		int y;
		GLuint err;
		GLuint expected[4];
		GLuint actual[4];

		bool operator== (const Flavor& f) const {
			return pass == f.pass
				&& x == f.x
				&& y == f.y
				&& err == f.err
				&& expected[0] == f.expected[0]
				&& expected[1] == f.expected[1]
				&& expected[2] == f.expected[2]
				&& expected[3] == f.expected[3]
				&& actual[0] == f.actual[0]
				&& actual[1] == f.actual[1]
				&& actual[2] == f.actual[2]
				&& actual[3] == f.actual[3]
				;
		}

		Flavor() {
			pass = true;
			x = y = 0;
			err = 0;
			expected[0] = expected[1] = expected[2]
				= expected[3] = 0;
			actual[0] = actual[1] = actual[2] = actual[3] = 0;
		}

		void put(ostream& s) const {
			s
			  << pass << '\n'
			  << x << ' ' << y << '\n'
			  << err << '\n'
			  << expected[0] << ' '
			  	<< expected[1] << ' '
				<< expected[2] << ' '
				<< expected[3] << '\n'
			  << actual[0] << ' '
			  	<< actual[1] << ' '
				<< actual[2] << ' '
				<< actual[3] << '\n'
			  ;
		}
		void get(istream& s) {
			s
			  >> pass
			  >> x >> y
			  >> err
			  >> expected[0]
			  	>> expected[1]
				>> expected[2]
				>> expected[3]
			  >> actual[0]
			  	>> actual[1]
				>> actual[2]
				>> actual[3]
			  ;
		}
	};

	bool skipped;
	bool pass;

	Flavor ub;
	Flavor us;
	Flavor ui;

	ExactRGBAResult(): ub(), us(), ui() {
		skipped = false;
		pass = true;
	}
	
	void putresults(ostream& s) const {
		s
		  << skipped << '\n'
		  << pass << '\n'
		  ;
		ub.put(s);
		us.put(s);
		ui.put(s);
	}
	
	bool getresults(istream& s) {
		s
		  >> skipped
		  >> pass
		  ;
		ub.get(s);
		us.get(s);
		ui.get(s);
		return s.good();
	}
};

#define EXACT_RGBA_WIN_SIZE (512+2)

class ExactRGBATest: public BaseTest<ExactRGBAResult> {
public:
	GLEAN_CLASS_WH(ExactRGBATest, ExactRGBAResult,
		EXACT_RGBA_WIN_SIZE, EXACT_RGBA_WIN_SIZE);

	void summarize(const char* label, const ExactRGBAResult::Flavor& o,
	    const ExactRGBAResult::Flavor& n);
	void logFlavor(const char* label, const ExactRGBAResult::Flavor& r);
}; // class ExactRGBATest
extern ExactRGBATest exactRGBATest;

} // namespace GLEAN

#endif // __treadpix_h__
