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




// Implementation of image data, attribute, and I/O

#include "image.h"
#include "tiffio.h"

namespace GLEAN {

///////////////////////////////////////////////////////////////////////////////
// writeTIFF - write image to TIFF file
///////////////////////////////////////////////////////////////////////////////
void
Image::writeTIFF(const char* filename) {
	static uint16 unassocAlpha[] = {EXTRASAMPLE_UNASSALPHA};
	GLsizei rowStep = rowSizeInBytes();

	TIFF* tf = TIFFOpen(filename, "w");
	if (!tf)
		throw CantOpen(filename);

	TIFFSetField(tf, TIFFTAG_IMAGELENGTH, height());
	TIFFSetField(tf, TIFFTAG_IMAGEWIDTH, width());
	TIFFSetField(tf, TIFFTAG_XRESOLUTION, 100.0);
	TIFFSetField(tf, TIFFTAG_YRESOLUTION, 100.0);
	TIFFSetField(tf, TIFFTAG_RESOLUTIONUNIT, RESUNIT_INCH);
	TIFFSetField(tf, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
	TIFFSetField(tf, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
	TIFFSetField(tf, TIFFTAG_COMPRESSION, COMPRESSION_NONE);
		// LZW would have been acceptable, were it not for patent
		// issues.
	TIFFSetField(tf, TIFFTAG_ROWSPERSTRIP, height());

	switch (format()) {
	case GL_LUMINANCE:
		TIFFSetField(tf, TIFFTAG_SAMPLESPERPIXEL, 1);
		TIFFSetField(tf, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
		break;
	case GL_LUMINANCE_ALPHA:
		TIFFSetField(tf, TIFFTAG_SAMPLESPERPIXEL, 2);
		TIFFSetField(tf, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
		TIFFSetField(tf, TIFFTAG_EXTRASAMPLES, 1, unassocAlpha);
		break;
	case GL_RGB:
		TIFFSetField(tf, TIFFTAG_SAMPLESPERPIXEL, 3);
		TIFFSetField(tf, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
		break;
	case GL_RGBA:
		TIFFSetField(tf, TIFFTAG_SAMPLESPERPIXEL, 4);
		TIFFSetField(tf, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
		TIFFSetField(tf, TIFFTAG_EXTRASAMPLES, 1, unassocAlpha);
		break;
	default:
		TIFFClose(tf);
		throw BadFormat(format());
	}

	switch (type()) {
	case GL_BYTE:
		TIFFSetField(tf, TIFFTAG_BITSPERSAMPLE, 8);
		TIFFSetField(tf, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_INT);
		break;
	case GL_UNSIGNED_BYTE:
		TIFFSetField(tf, TIFFTAG_BITSPERSAMPLE, 8);
		TIFFSetField(tf, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_UINT);
		break;
	case GL_SHORT:
		TIFFSetField(tf, TIFFTAG_BITSPERSAMPLE, 16);
		TIFFSetField(tf, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_INT);
		break;
	case GL_UNSIGNED_SHORT:
		TIFFSetField(tf, TIFFTAG_BITSPERSAMPLE, 16);
		TIFFSetField(tf, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_UINT);
		break;
	case GL_INT:
		TIFFSetField(tf, TIFFTAG_BITSPERSAMPLE, 32);
		TIFFSetField(tf, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_INT);
		break;
	case GL_UNSIGNED_INT:
		TIFFSetField(tf, TIFFTAG_BITSPERSAMPLE, 32);
		TIFFSetField(tf, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_UINT);
		break;
	case GL_FLOAT:
		TIFFSetField(tf, TIFFTAG_BITSPERSAMPLE, 32);
		TIFFSetField(tf, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_IEEEFP);
		break;
	default:
		TIFFClose(tf);
		throw BadType(type());
	}

	{
		// Write rows in reverse order, so that the usual OpenGL
		// orientation won't result in an upside-down image for
		// naive TIFF readers:
		char* row = pixels() + (height() - 1) * rowStep;
		for (GLsizei r = 0; r < height(); ++r, row -= rowStep)
			TIFFWriteScanline(tf, row, r, 0);
	}

	TIFFClose(tf);
}; // Image::writeTIFF


}; // namespace GLEAN
