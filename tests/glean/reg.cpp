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




// Image registration.

#include <cfloat>
#include "image.h"

#include <cmath>	// for fabs


namespace GLEAN {

///////////////////////////////////////////////////////////////////////////////
// register:  compare a reference image to the current (``test'') image.
//
//	The reference image must be no larger than the current image, in
//	both dimensions.  Type doesn't matter, as both images will be
//	converted to RGBA.
//
//	The reference image will be slid into all possible positions over
//	the current image, and the sum of the mean absolute errors for all
//	four color channels computed at each position.
//
//	Returns an Image::Registration struct that specifies the position at
//	which the sum of mean absolute errors was minimal, plus the statistics
//	at that position.
///////////////////////////////////////////////////////////////////////////////
Image::Registration
Image::reg(Image& ref) {
	int wt = width();		// Width of test image, in pixels.
	int ht = height();		// Height of test image, in pixels.
	int wr = ref.width();		// Width of reference image, in pixels.
	int hr = ref.height();		// Height of test image, in pixels.
	int dh = ht - hr;		// Difference in heights, in pixels.
	int dw = wt - wr;		// Difference in widths, in pixels.
	int i;

	if (dh < 0 || dw < 0)
		throw RefImageTooLarge();

	int wt4 = 4 * wt;		// Width of test image, in RGBA samples.
	int wr4 = 4 * wr;		// Width of ref image, in RGBA samples.
	int dw4 = 4 * dw;		// Difference in widths, in samples.

	double** testPix;		// Buffers containing all the rows of
					// the test image that need to be
					// accessed concurrently.
	// XXX sure would be nice to use auto_ptr to allocate this stuff,
	// but it isn't supported in the STL that came with egcs 1.1.2.
	
	// XXX testPix = new (double*) [dh + 1];
	// VC 6 seems to misinterpret this as a c-style cast
	testPix = new double* [dh + 1];

	
	for (/*int */i = 0; i <= dh; ++i)
		testPix[i] = new double [wt4];

	double* refPix = new double [wr4];
					// Buffer containing the one row of
					// the reference image that's accessed
					// at any given time.

	BasicStats** stats;		// Buffers containing a statistics-
					// gathering structure for each of
					// the possible reference image
					// positions.
	// XXX stats = new (BasicStats*) [dh + 1];
	// VC 6 seems to misinterpret this as a c-style cast
	stats = new BasicStats * [dh + 1];

	for (/*int*/ i = 0; i <= dh; ++i)
		stats[i] = new BasicStats [dw4 + 4];

	// Prime the pump by unpacking the first few rows of the test image:
	char* testRow = pixels();
	for (/*int*/ i = 0; i < dh; ++i) {
		unpack(wt, testPix[i], testRow);
		testRow += rowSizeInBytes();
	}

	// Now accumulate statistics for one row of the reference image
	// at a time, in all possible positions:
	char* refRow = ref.pixels();
	for (/*int*/ i = 0; i < hr; ++i) {
		// Get the next row of the reference image:
		ref.unpack(wr, refPix, refRow);
		refRow += ref.rowSizeInBytes();

		// Get the next row of the test image:
		unpack(wt, testPix[dh], testRow);
		testRow += rowSizeInBytes();

		// Accumulate absolute error for R,G,B,A in all positions:
		for (int j = 0; j <= dh; ++j)
			for (int k = 0; k <= dw4; k += 4)
				for (int m = 0; m < wr4; m += 4) {
					stats[j][k+0].sample( fabs(
						refPix[m+0]-testPix[j][m+k+0]));
					stats[j][k+1].sample( fabs(
						refPix[m+1]-testPix[j][m+k+1]));
					stats[j][k+2].sample( fabs(
						refPix[m+2]-testPix[j][m+k+2]));
					stats[j][k+3].sample( fabs(
						refPix[m+3]-testPix[j][m+k+3]));
				}
	}

	// Now find the position for which the sum of the mean absolute errors
	// is minimal:
	double minErrorSum = DBL_MAX;
	int minI = 0;
	int minJ = 0;
	for (/*int*/ i = 0; i <= dh; ++i)
		for (int j = 0; j <= dw4; j += 4) {
			double errorSum = stats[i][j+0].mean()
				+ stats[i][j+1].mean()
				+ stats[i][j+2].mean()
				+ stats[i][j+3].mean();
			if (errorSum < minErrorSum) {
				minErrorSum = errorSum;
				minI = i;
				minJ = j;
			}
		}

	Registration r;
	r.wOffset = minJ / 4;
	r.hOffset = minI;
	r.stats[0] = stats[minI][minJ+0];
	r.stats[1] = stats[minI][minJ+1];
	r.stats[2] = stats[minI][minJ+2];
	r.stats[3] = stats[minI][minJ+3];

	// Clean up:
	for (/*int*/ i = 0; i <= dh; ++i)
		delete[] testPix[i];
	delete[] testPix;
	delete[] refPix;
	for (/*int*/ i = 0; i <= dh; ++i)
		delete[] stats[i];
	delete[] stats;

	return r;
} // Image::register

}; // namespace GLEAN
