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




// image.h:  image data and attributes, image I/O

// This class encapsulates OpenGL information related to images (size,
// format, etc.) and provides utilities for transferring images to and
// from files.


#ifndef __image_h__
#define __image_h__

#include <string>
#include "glwrap.h"
#include "stats.h"

namespace GLEAN {

class Image {

    private:

	GLsizei _width;
	GLsizei _height;
	GLenum _format;
	GLenum _type;
	char* _pixels;
	GLsizei _alignment;
	GLsizei _rowSizeInBytes;
	GLsizei _pixelSizeInBytes;

	enum {				// validation bits, for lazy validation
		vbRowSizeInBytes = 1,
		vbPixelSizeInBytes = 2,
		vbPacker = 4,
		vbUnpacker = 8,
		vbAll = ~0
	};
	int _invalid;
	inline bool invalid(int bit) const { return _invalid & bit; }
	inline bool valid(int bit) const { return !invalid(bit); }
	inline void invalidate(int bits) { _invalid |= bits; }
	inline void validate(int bits) { _invalid &= ~bits; }

	GLsizei validateRowSizeInBytes();
	GLsizei validatePixelSizeInBytes();

	typedef void Unpacker(GLsizei n, double* rgba, char* nextPixel);
	Unpacker* _unpacker;
	Unpacker* validateUnpacker();
	typedef void Packer(GLsizei n, char* nextPixel, double* rgba);
	Packer* _packer;
	Packer* validatePacker();

	// For now, we will require that:
	// 1.  All images are in native byte order (so that byte swapping
	//     at the OpenGL level is unnecessary).
	// 2.  The image width and height above describe the entire image
	//     (so that there is no need to specify row length
	//     independently).
	// 3.  We have no need to specify subimages at this level (so
	//     there is no need for SKIP_ROWS and SKIP_PIXELS attributes).

	// Should construction fix the format and type for all time?
	// That would eliminate synchronization problems between data and
	// descriptive information that might arise when an Image is reused,
	// and might permit use of template functions instead of lots of
	// switches.  Probably not; it would make dynamic type assignment
	// (such as reading a TIFF file) quite awkward.

    public:

	// Exceptions:

	struct Error { };		// Base class for all image errors.
	struct BadFormat: public Error {	// Bad image format.
		GLenum format;
		BadFormat(GLenum f) {format = f;}
	};
	struct BadType: public Error {		// Bad image type.
		GLenum type;
		BadType(GLenum t) {type = t;}
	};
	struct CantOpen: public Error {		// Can't open file.
		const char* filename;
		CantOpen(const char* p) {filename = p;}
	};
	struct UnsupportedTIFF: public Error {	// TIFF we can't handle.
	};
	struct RefImageTooLarge: public Error {	// Can't register ref image.
	};

	// Constructors/Destructor:

	Image();
	Image(int aWidth, int aHeight, GLenum aFormat, GLenum aType);
	Image(int aWidth, int aHeight, GLenum aFormat, GLenum aType,
		double r, double g, double b, double a);
	Image(Image& i);
	Image& operator= (Image& i);
	~Image();

	// Reserve space for the pixel array:

	void reserve();

	// Get/Set attributes.  These attributes are useful for calls
	// to glDrawPixels, glTexImage2D, etc.  Note the alignment
	// value; passing it to glPixelStore is essential before using
	// one of the other OpenGL commands.

	inline GLsizei width() const	// Image width, in pixels
		{ return _width; }
	inline void width(GLsizei w)
		{ _width = w; invalidate(vbRowSizeInBytes); }

	inline GLsizei height() const	// Image height, in pixels.
		{ return _height; }
	inline void height(GLsizei h)
		{ _height = h; }

	inline GLenum format() const	// Image format.  Currently
		{ return _format; }	// these formats are supported:
					// GL_LUMINANCE,
					// GL_LUMINANCE_ALPHA,
					// GL_RGB, GL_RGBA.
					// It may be easiest to treat
					// stencil, depth, etc. images
					// as luminance images.
	inline void format(GLenum f) {
		_format = f;
		invalidate(
			  vbRowSizeInBytes
			| vbPixelSizeInBytes
			| vbPacker
			| vbUnpacker);
	}

	inline GLenum type() const	// Pixel data type.  Currently
		{ return _type; }	// these types are supported:
					// GL_BYTE, GL_UNSIGNED_BYTE,
					// GL_SHORT, GL_UNSIGNED_SHORT,
					// GL_INT, GL_UNSIGNED_INT, GL_FLOAT.
	inline void type(GLenum t) {
		_type = t;
		invalidate(
			  vbRowSizeInBytes
			| vbPixelSizeInBytes
			| vbPacker
			| vbUnpacker);
	}

	inline char* pixels() 		// The pixels.
		{ return _pixels; }
	inline const char* pixels() const
		{ return const_cast<const char*>(_pixels); }
	void pixels(char* p);

	inline GLsizei alignment() const	// Alignment.  See glPixelStore.
		{ return _alignment; }
	inline void alignment(GLsizei a)
		{ _alignment = a; invalidate(vbRowSizeInBytes); }

	inline GLsizei rowSizeInBytes() {	// Size of scanline, in bytes
		return valid(vbRowSizeInBytes)?
			_rowSizeInBytes: validateRowSizeInBytes();
	}

	inline GLsizei pixelSizeInBytes() {	// Size of pixel, in bytes
		return valid(vbPixelSizeInBytes)?
			_pixelSizeInBytes: validatePixelSizeInBytes();
	}

	// XXX Utilities to determine component size in bits/bytes?
	// XXX Component range (min neg, max neg, min pos, max pos, eps?)

	// Pixel packing/unpacking utilities:

	void unpack(GLsizei n, double* rgba, char* nextPixel);
	void pack(GLsizei n, char* nextPixel, double* rgba);
	// XXX get(x, y, double* rgba);
	// XXX put(x, y, double* rgba);

	// Image registration.  The utility compares a reference image
	// to the current image (which must be at least as large as the
	// reference image in both dimensions), determines the offset at
	// which the reference image minus the current image has minimum
	// mean absolute error (summed over R, G, B, and A), and returns
	// an object specifying the offset and corresponding statistics.

	struct Registration {
	    	int wOffset;		// offset in width (x)
		int hOffset;		// offset in height (y)
		BasicStats stats[4];	// stats for absolute error in
					// R, G, B, and A
	};
	Registration reg(Image& ref);

	// Image arithmetic
	// XXX type and format conversions, with appropriate scaling.
	// XXX image difference
	// XXX minmax, histogram, contrast stretch?

	// TIFF I/O utilities:

	void readTIFF(const char* filename);
	inline void readTIFF(const std::string& s)  { readTIFF(s.c_str()); }
	void writeTIFF(const char* filename);
	inline void writeTIFF(const std::string& s)  { writeTIFF(s.c_str()); }

	// GL operation utilities:

	void draw();				// Invoke glDrawPixels.
	void read(GLint x, GLint y);		// Invoke glReadPixels.
	void makeMipmaps(GLenum intFormat);	// Load texture mipmaps.

}; // class Image

} // namespace GLEAN

#endif // __image_h__
