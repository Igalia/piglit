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




#include "image.h"
#include "tiffio.h"

namespace GLEAN {

///////////////////////////////////////////////////////////////////////////////
// readTIFF - read image from TIFF file, set attributes to match the file
///////////////////////////////////////////////////////////////////////////////
void
Image::readTIFF(const char* filename) {
	// XXX Things we explicitly don't handle:
	//	Varying number of bits per sample.  Sam's library doesn't
	//		handle that.
	//	Bits per sample other than 8, 16, or 32.
	//	Tile-oriented TIFF files.  We only do strip-oriented files.
	//	Planar configurations other than contiguous (R,G,B,R,G,B,...).
	//	Premultiplied alpha.  If there's a fourth color channel,
	//		we just assume it's non-premultiplied alpha.
	// Eventually would be good to add a ``validation'' function which
	// checks a file before attempting to read the image it contains.
	// Also:  need error-reporting code.

	TIFF* tf = TIFFOpen(filename, "r");
	if (!tf)
		throw CantOpen(filename);

	uint32 u32;

	TIFFGetFieldDefaulted(tf, TIFFTAG_IMAGELENGTH, &u32);
	height(u32);

	TIFFGetFieldDefaulted(tf, TIFFTAG_IMAGEWIDTH, &u32);
	width(u32);

	uint16 samplesPerPixel;
	TIFFGetFieldDefaulted(tf, TIFFTAG_SAMPLESPERPIXEL, &samplesPerPixel);
	switch (samplesPerPixel) {
	case 1:
		format(GL_LUMINANCE);
		break;
	case 2:
		format(GL_LUMINANCE_ALPHA);
		break;
	case 3:
		format(GL_RGB);
		break;
	case 4:
		format(GL_RGBA);
		break;
	default:
		TIFFClose(tf);
		throw UnsupportedTIFF();
	}

	uint16 bitsPerSample;
	TIFFGetFieldDefaulted(tf, TIFFTAG_BITSPERSAMPLE, &bitsPerSample);
	uint16 sampleFormat;
	if (TIFFGetField(tf, TIFFTAG_SAMPLEFORMAT, &sampleFormat) != 1)
		sampleFormat = SAMPLEFORMAT_UINT;
	switch ((sampleFormat << 8) | bitsPerSample) {
	case (SAMPLEFORMAT_UINT << 8) | 8:
		type(GL_UNSIGNED_BYTE);
		break;
	case (SAMPLEFORMAT_UINT << 8) | 16:
		type(GL_UNSIGNED_SHORT);
		break;
	case (SAMPLEFORMAT_UINT << 8) | 32:
		type(GL_UNSIGNED_INT);
		break;
	case (SAMPLEFORMAT_INT << 8) | 8:
		type(GL_BYTE);
		break;
	case (SAMPLEFORMAT_INT << 8) | 16:
		type(GL_SHORT);
		break;
	case (SAMPLEFORMAT_INT << 8) | 32:
		type(GL_INT);
		break;
	case (SAMPLEFORMAT_IEEEFP << 8) | 32:
		type(GL_FLOAT);
		break;
	default:
		TIFFClose(tf);
		throw UnsupportedTIFF();
	}

	// At the moment it's not obvious whether we should pad
	// scanlines to achieve a preferred alignment, so we'll just
	// return an alignment that matches the data.
	alignment(1);
	switch (rowSizeInBytes() & 0x7) {
	case 0:
		alignment(8);
		break;
	case 4:
		alignment(4);
		break;
	case 2:
	case 6:
		alignment(2);
		break;
	case 1:
	case 3:
	case 5:
	case 7:
		alignment(1);
		break;
	}

	reserve();

	{
		// Store rows in reverse order, so that the default TIFF
		// orientation won't result in an upside-down image for
		// OpenGL:
		GLsizei rowStep = rowSizeInBytes();
		char* row = pixels() + (height() - 1) * rowStep;
		for (GLsizei r = 0; r < height(); ++r, row -= rowStep)
			TIFFReadScanline(tf, row, r);
	}

	TIFFClose(tf);
} // Image::readTIFF

}; // namespace GLEAN
