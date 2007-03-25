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
#include <string.h>

namespace GLEAN {

///////////////////////////////////////////////////////////////////////////////
// Constructors/Destructor
///////////////////////////////////////////////////////////////////////////////
// An empty image:
Image::Image() {
	_width = _height = 0;
	_format = GL_RGB;
	_type = GL_UNSIGNED_BYTE;
	_pixels = 0;
	_alignment = 4;
	_packer = 0;
	_unpacker = 0;
	_invalid = vbAll;
} // Image::Image

// An unitialized image of the given type and size:
Image::Image(int aWidth, int aHeight, GLenum aFormat, GLenum aType) {
	_width = aWidth;
	_height = aHeight;
	_format = aFormat;
	_type = aType;
	_pixels = 0;
	_alignment = 4;
	_packer = 0;
	_unpacker = 0;
	_invalid = vbAll;
	reserve();
} // Image::Image(aWidth, aHeight, aFormat, aType)

// An image of the given type and size, initialized to a solid color:
Image::Image(int aWidth, int aHeight, GLenum aFormat, GLenum aType,
    double r, double g, double b, double a) {
	_width = aWidth;
	_height = aHeight;
	_format = aFormat;
	_type = aType;
	_pixels = 0;
	_alignment = 4;
	_packer = 0;
	_unpacker = 0;
	_invalid = vbAll;
	reserve();
	int i;		// VC++ 6 doesn't handle the definition of variables in a 
				// for-statement properly

	double* solidColor = new double[4 * width()];
	for (/*int */i = 0; i < 4 * width(); i += 4) {
		solidColor[i + 0] = r;
		solidColor[i + 1] = g;
		solidColor[i + 2] = b;
		solidColor[i + 3] = a;
	}

	char* row = pixels();
	for (/*int */i = 0; i < height(); ++i) {
		pack(width(), row, solidColor);
		row += rowSizeInBytes();
	}
} // Image::Image(aWidth, aHeight, aFormat, aType)

// Copy constructor:
Image::Image(Image& i) {
	_width = i.width();
	_height = i.height();
	_format = i.format();
	_type = i.type();
	_alignment = i.alignment();
	_pixels = 0;
	_packer = 0;
	_unpacker = 0;
	_invalid = vbAll;
	reserve();
	memcpy(pixels(), i.pixels(), height() * rowSizeInBytes());
} // Image::Image(Image&)

/*Image::*/Image&
Image::operator= (Image& i) {
	if (this == &i)
		return *this;
	width(i.width());
	height(i.height());
	format(i.format());
	type(i.type());
	alignment(i.alignment());
	_invalid = vbAll;
	reserve();
	memcpy(pixels(), i.pixels(), height() * rowSizeInBytes());
	return *this;
} // Image::operator=

Image::~Image() {
	if (_pixels)
		delete[] _pixels;
}

///////////////////////////////////////////////////////////////////////////////
// pixels - set pointer to pixel array
///////////////////////////////////////////////////////////////////////////////
void
Image::pixels(char* p) {
	// We always own our pixels, so delete the old ones (if any) before
	// installing new ones:
	if (_pixels)
		delete[] _pixels;
	_pixels = p;
} // Image::pixels

///////////////////////////////////////////////////////////////////////////////
// reserve - reserve memory for image (assuming current type, format, and size)
///////////////////////////////////////////////////////////////////////////////
void
Image::reserve() {
	pixels(0);	// deallocate old pixel array
	pixels(new char[height() * rowSizeInBytes()]);
} // Image::reserve

///////////////////////////////////////////////////////////////////////////////
// validateRowSizeInBytes - compute image row size, measured in bytes
///////////////////////////////////////////////////////////////////////////////
GLsizei
Image::validateRowSizeInBytes() {
	_rowSizeInBytes =
		(width() * pixelSizeInBytes() + alignment() - 1)
		& ~(alignment() - 1);
	validate(vbRowSizeInBytes);
	return _rowSizeInBytes;
} // Image::calcRowSizeInBytes

///////////////////////////////////////////////////////////////////////////////
// validatePixelSizeInBytes - compute pixel size, measured in bytes
///////////////////////////////////////////////////////////////////////////////
GLsizei
Image::validatePixelSizeInBytes() {
	switch (format()) {
	case GL_LUMINANCE:
		_pixelSizeInBytes = 1;
		break;
	case GL_LUMINANCE_ALPHA:
		_pixelSizeInBytes = 2;
		break;
	case GL_RGB:
		_pixelSizeInBytes = 3;
		break;
	case GL_RGBA:
		_pixelSizeInBytes = 4;
		break;
	default:
		throw BadFormat(format());
	}

	switch (type()) {
	case GL_BYTE:
	case GL_UNSIGNED_BYTE:
		break;
	case GL_SHORT:
	case GL_UNSIGNED_SHORT:
		_pixelSizeInBytes <<= 1;
		break;
	case GL_INT:
	case GL_UNSIGNED_INT:
	case GL_FLOAT:
		_pixelSizeInBytes <<= 2;
		break;
	default:
		throw BadType(type());
	}

	validate(vbPixelSizeInBytes);
	return _pixelSizeInBytes;
}

}; // namespace GLEAN
