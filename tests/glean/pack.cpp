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




// Data packing utilities.  Note that these map component values per
// the usual OpenGL conversions.  Also, see comments in unpack.cpp.

#include "image.h"

namespace {

#define SCALE (static_cast<double>(num) / static_cast<double>(denom))
#define BIAS (static_cast<double>(bias) / static_cast<double>(denom))

// The original implementation of packing functions (using function
// templates) wouldn't compile under VC6, but this slight variant
// using static member functions in a class template will compile.

template<class component, int num, unsigned int denom, int bias>
class Pack
{
public :
	// pack_l
	static void pack_l(GLsizei n, char* dst, double* rgba) 
	{
		component* out = reinterpret_cast<component*>(dst);
		double* end = rgba + 4 * n;
		for (; rgba != end; rgba += 4) {
			if (bias)
				out[0] = static_cast<component>(SCALE * rgba[0] - BIAS);
			else
				out[0] = static_cast<component>(SCALE * rgba[0]);
			out += 1;
		}
	}

	// pack_la
	static void pack_la(GLsizei n, char* dst, double* rgba) 
	{
		component* out = reinterpret_cast<component*>(dst);
		double* end = rgba + 4 * n;
		for (; rgba != end; rgba += 4) {
			if (bias) {
				out[0] = static_cast<component>(SCALE * rgba[0] - BIAS);
				out[1] = static_cast<component>(SCALE * rgba[3] - BIAS);
			} else {
				out[0] = static_cast<component>(SCALE * rgba[0]);
				out[1] = static_cast<component>(SCALE * rgba[3]);
			}
			out += 2;
		}
	}

	// pack_rga
	static void pack_rgb(GLsizei n, char* dst, double* rgba) 
	{
		component* out = reinterpret_cast<component*>(dst);
		double* end = rgba + 4 * n;
		for (; rgba != end; rgba += 4) {
			if (bias) {
				out[0] = static_cast<component>(SCALE * rgba[0] - BIAS);
				out[1] = static_cast<component>(SCALE * rgba[1] - BIAS);
				out[2] = static_cast<component>(SCALE * rgba[2] - BIAS);
			} else {
				out[0] = static_cast<component>(SCALE * rgba[0]);
				out[1] = static_cast<component>(SCALE * rgba[1]);
				out[2] = static_cast<component>(SCALE * rgba[2]);
			}
			out += 3;
		}
	}

	// pack_rgba
	static void pack_rgba(GLsizei n, char* dst, double* rgba) 
	{
		component* out = reinterpret_cast<component*>(dst);
		double* end = rgba + 4 * n;
		for (; rgba != end; rgba += 4) {
			if (bias) {
				out[0] = static_cast<component>(SCALE * rgba[0] - BIAS);
				out[1] = static_cast<component>(SCALE * rgba[1] - BIAS);
				out[2] = static_cast<component>(SCALE * rgba[2] - BIAS);
				out[3] = static_cast<component>(SCALE * rgba[3] - BIAS);
			} else {
				out[0] = static_cast<component>(SCALE * rgba[0]);
				out[1] = static_cast<component>(SCALE * rgba[1]);
				out[2] = static_cast<component>(SCALE * rgba[2]);
				out[3] = static_cast<component>(SCALE * rgba[3]);
			}
			out += 4;
		}
	}

};	// class Pack

#undef SCALE
#undef BIAS

}; // anonymous namespace


namespace GLEAN {

///////////////////////////////////////////////////////////////////////////////
// Public interface
///////////////////////////////////////////////////////////////////////////////
void
Image::pack(GLsizei n, char* nextPixel, double* rgba) {
	(*(valid(vbPacker)? _packer: validatePacker())) (n, nextPixel, rgba);
}

///////////////////////////////////////////////////////////////////////////////
// validatePacker - select appropriate pixel-packing utility
///////////////////////////////////////////////////////////////////////////////

Image::Packer*
Image::validatePacker() {
	switch (format()) {
	case GL_LUMINANCE:
		switch (type()) {
		case GL_BYTE:
			_packer = Pack<GLbyte, 255, 2, 1>::pack_l;
			break;
		case GL_UNSIGNED_BYTE:
			_packer = Pack<GLubyte, 255, 1, 0>::pack_l;
			break;
		case GL_SHORT:
			_packer = Pack<GLshort, 65535, 2, 1>::pack_l;
			break;
		case GL_UNSIGNED_SHORT:
			_packer = Pack<GLushort, 65535, 1, 0>::pack_l;
			break;
		case GL_INT:
			_packer = Pack<GLint, 4294967295U, 2, 1>::pack_l;
			break;
		case GL_UNSIGNED_INT:
			_packer = Pack<GLuint, 4294967295U, 1, 0>::pack_l;
			break;
		case GL_FLOAT:
			_packer = Pack<GLfloat, 1, 1, 0>::pack_l;
			break;
		default:
			throw BadType(type());
		}
		break;
	case GL_LUMINANCE_ALPHA:
		switch (type()) {
		case GL_BYTE:
			_packer = Pack<GLbyte, 255, 2, 1>::pack_la;
			break;
		case GL_UNSIGNED_BYTE:
			_packer = Pack<GLubyte, 255, 1, 0>::pack_la;
			break;
		case GL_SHORT:
			_packer = Pack<GLshort, 65535, 2, 1>::pack_la;
			break;
		case GL_UNSIGNED_SHORT:
			_packer = Pack<GLushort, 65535, 1, 0>::pack_la;
			break;
		case GL_INT:
			_packer = Pack<GLint, 4294967295U, 2, 1>::pack_la;
			break;
		case GL_UNSIGNED_INT:
			_packer = Pack<GLuint, 4294967295U, 1, 0>::pack_la;
			break;
		case GL_FLOAT:
			_packer = Pack<GLfloat, 1, 1, 0>::pack_la;
			break;
		default:
			throw BadType(type());
		}
		break;
	case GL_RGB:
		switch (type()) {
		case GL_BYTE:
			_packer = Pack<GLbyte, 255, 2, 1>::pack_rgb;
			break;
		case GL_UNSIGNED_BYTE:
			_packer = Pack<GLubyte, 255, 1, 0>::pack_rgb;
			break;
		case GL_SHORT:
			_packer = Pack<GLshort, 65535, 2, 1>::pack_rgb;
			break;
		case GL_UNSIGNED_SHORT:
			_packer = Pack<GLushort, 65535, 1, 0>::pack_rgb;
			break;
		case GL_INT:
			_packer = Pack<GLint, 4294967295U, 2, 1>::pack_rgb;
			break;
		case GL_UNSIGNED_INT:
			_packer = Pack<GLuint, 4294967295U, 1, 0>::pack_rgb;
			break;
		case GL_FLOAT:
			_packer = Pack<GLfloat, 1, 1, 0>::pack_rgb;
			break;
		default:
			throw BadType(type());
		}
		break;
	case GL_RGBA:
		switch (type()) {
		case GL_BYTE:
			_packer = Pack<GLbyte, 255, 2, 1>::pack_rgba;
			break;
		case GL_UNSIGNED_BYTE:
			_packer = Pack<GLubyte, 255, 1, 0>::pack_rgba;
			break;
		case GL_SHORT:
			_packer = Pack<GLshort, 65535, 2, 1>::pack_rgba;
			break;
		case GL_UNSIGNED_SHORT:
			_packer = Pack<GLushort, 65535, 1, 0>::pack_rgba;
			break;
		case GL_INT:
			_packer = Pack<GLint, 4294967295U, 2, 1>::pack_rgba;
			break;
		case GL_UNSIGNED_INT:
			_packer = Pack<GLuint, 4294967295U, 1, 0>::pack_rgba;
			break;
		case GL_FLOAT:
			_packer = Pack<GLfloat, 1, 1, 0>::pack_rgba;
			break;
		default:
			throw BadType(type());
		}
		break;
	default:
		throw BadFormat(format());
	}

	validate(vbPacker);
	return _packer;
}

}; // namespace GLEAN
