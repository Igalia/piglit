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

// A note on principles of operation:  The OpenGL spec normally allows
// a reasonable amount of slop when converting user-specified colors
// to a hardware-dependent representation in the framebuffer.  One
// exception to this lenience is when lighting is disabled and the
// color is specified as an unsigned byte, short, or int.  In this
// case the conversion from user-supplied color to hardware-determined
// color must be exact, up to the number of bits in the framebuffer or
// in the value supplied by the user (whichever is smaller).  This is
// intended to allow object identification numbers to be encoded as
// colors, so that applications can implement object selection by
// drawing objects and reading back the image to determine the object
// ID of the closest visible object.  glean uses this property in a
// number of cases, for example, where it needs to draw a large number
// of primitives and verify that all of them were actually drawn.  See
// the OpenGL spec, version 1.2.1, section 2.13.9 (page 55) for the
// description of this convertibility requirement.

#ifndef __codedid_h__
#define __codedid_h__

using namespace std;

#include <vector>
#include "glwrap.h"

namespace GLEAN {

class Image;				// forward reference

class RGBCodedID {
	int rBits, gBits, bBits;	// number of signif. bits in channels
	int nsRBits, nsGBits, nsBBits;	// non-significant bits in each
	int rMask, gMask, bMask;	// masks for significant bits in each
    public:
	RGBCodedID(int r, int g, int b);
	~RGBCodedID();

	// Return the maximum ID number that the caller can use:
	int maxID() const;

	// Map an ID number to an RGB triple:
	void toRGB(int id, GLubyte& r, GLubyte& g, GLubyte& b) const;

	// Map an RGB triple to the equivalent ID number:
	int toID(GLubyte r, GLubyte g, GLubyte b) const;

	// Histogram an UNSIGNED_BYTE RGB image:
	void histogram(Image& img, vector<int>& hist) const;

	// Are all of a range of IDs present in an RGB image?
	bool allPresent(Image& img, int first, int last) const;

}; // RGBCodedID

// XXX Might want an IndexCodedID class for use with color index drawing
// surfaces, even though such a class would be trivial.

} // namespace GLEAN

#endif // __codedid_h__
