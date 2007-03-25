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



// codedid.h:  tool to map integer IDs into colors, and vice-versa

using namespace std;

#include <algorithm>
#include <vector>
#include "codedid.h"
#include "image.h"

namespace GLEAN {

///////////////////////////////////////////////////////////////////////////////
// RGBCodedID: Create an object that maps integer identification numbers
//	to RGB triples, and vice-versa
///////////////////////////////////////////////////////////////////////////////
RGBCodedID::RGBCodedID(int r, int g, int b) {
	rBits = min(8, r);	// 8 because we use GLubyte color values
	gBits = min(8, g);
	bBits = min(8, b);
	nsRBits = 8 - rBits;
	nsGBits = 8 - gBits;
	nsBBits = 8 - bBits;
	rMask = (1 << rBits) - 1;
	gMask = (1 << gBits) - 1;
	bMask = (1 << bBits) - 1;
} // RGBCodedID::RGBCodedID

RGBCodedID::~RGBCodedID() {
} // RGBCodedID::~RGBCodedID

///////////////////////////////////////////////////////////////////////////////
// maxID: Return the maximum allowable integer ID number
///////////////////////////////////////////////////////////////////////////////
int
RGBCodedID::maxID() const {
	return (1 << (rBits + gBits + bBits)) - 1;
} // RGBCodedID::maxID

///////////////////////////////////////////////////////////////////////////////
// toRGB: Convert integer ID number to RGB triple
///////////////////////////////////////////////////////////////////////////////
void
RGBCodedID::toRGB(int id, GLubyte& r, GLubyte& g, GLubyte& b) const {
	b = (id & bMask) << nsBBits;
	id >>= bBits;
	g = (id & gMask) << nsGBits;
	id >>= gBits;
	r = (id & rMask) << nsRBits;
} // RGBCodedID::toRGB

///////////////////////////////////////////////////////////////////////////////
// toID: Convert RGB triple to integer ID number
///////////////////////////////////////////////////////////////////////////////
int
RGBCodedID::toID(GLubyte r, GLubyte g, GLubyte b) const {
	int id = 0;
	id |= (r >> nsRBits) & rMask;
	id <<= gBits;
	id |= (g >> nsGBits) & gMask;
	id <<= bBits;
	id |= (b >> nsBBits) & bMask;
	return id;
} // RGBCodedID::toID

///////////////////////////////////////////////////////////////////////////////
// histogram: Compute histogram of coded IDs in an UNSIGNED_BYTE RGB image
///////////////////////////////////////////////////////////////////////////////
void
RGBCodedID::histogram(Image& img, vector<int>& hist) const {
	if (img.format() != GL_RGB || img.type() != GL_UNSIGNED_BYTE) {
		hist.resize(0);
		return;
	}

	int max = maxID();
	hist.resize(max + 1);
	for (vector<int>::iterator p = hist.begin(); p != hist.end(); ++p)
		*p = 0;

	GLubyte* row = reinterpret_cast<GLubyte*>(img.pixels());
	for (GLsizei r = 0; r < img.height(); ++r) {
		GLubyte* pix = row;
		for (GLsizei c = 0; c < img.width(); ++c) {
			int id = toID(pix[0], pix[1], pix[2]);
			if (id <= max)
				++hist[id];
			pix += 3;
		}
		row += img.rowSizeInBytes();
	}
} // RGBCodedID::histogram

///////////////////////////////////////////////////////////////////////////////
// allPresent: See if all of a range of IDs are present in a given RGB image
///////////////////////////////////////////////////////////////////////////////
bool
RGBCodedID::allPresent(Image& img, int first, int last) const {
	vector<int> hist;
	histogram(img, hist);
	if (hist.size() == 0)
		return false;
	if (first >= static_cast<int>(hist.size()))
		return false;
	if (last >= static_cast<int>(hist.size()))
		return false;
	if (first > last)
		return false;

	for (vector<int>::const_iterator p = hist.begin() + first;
	    p != hist.begin() + last + 1; ++p)
		if (*p == 0)
			return false;
	return true;
} // RGBCodedID::allPresent

} // namespace GLEAN
