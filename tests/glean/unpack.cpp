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




// Data unpacking utilities.  Note that these map component values per
// the usual OpenGL conventions.

// XXX The construction of SCALE and BIAS is clumsy, and the need to
// test bias is really unfortunate, but egcs 1.1.2 won't propagate
// floating-point constant expressions from equivalent const
// declarations.

#include "image.h"

namespace {

#define SCALE (static_cast<double>(num) / static_cast<double>(denom))
#define BIAS (static_cast<double>(bias) / static_cast<double>(denom))

// See comments in pack.cpp concerning this workaround for a VC6 problem.

template<class component, int num, unsigned int denom, int bias>
class Unpack
{
public :
	// unpack_l
	static void unpack_l(GLsizei n, double* rgba, char* src) 
	{
		component* in = reinterpret_cast<component*>(src);
			// XXX It seems to me that static_cast should be sufficient,
			// but egcs 1.1.2 thinks otherwise.

		double* end = rgba + 4 * n;
		for (; rgba != end; rgba += 4) {
			if (bias)
				rgba[0] = SCALE * in[0] + BIAS;
			else
				rgba[0] = SCALE * in[0];
			rgba[1] = rgba[2] = rgba[3] = 0.0;
			in += 1;
		}
	}

	// unpack_la
	static void unpack_la(GLsizei n, double* rgba, char* src) 
	{
		component* in = reinterpret_cast<component*>(src);
		double* end = rgba + 4 * n;
		for (; rgba != end; rgba += 4) {
			if (bias) {
				rgba[0] = SCALE * in[0] + BIAS;
				rgba[3] = SCALE * in[1] + BIAS;
			} else {
				rgba[0] = SCALE * in[0];
				rgba[3] = SCALE * in[1];
			}
			rgba[1] = rgba[2] = 0.0;
			in += 2;
		}
	}

	// unpack_rgb
	static void unpack_rgb(GLsizei n, double* rgba, char* src) 
	{
		component* in = reinterpret_cast<component*>(src);
		double* end = rgba + 4 * n;
		for (; rgba != end; rgba += 4) {
			if (bias) {
				rgba[0] = SCALE * in[0] + BIAS;
				rgba[1] = SCALE * in[1] + BIAS;
				rgba[2] = SCALE * in[2] + BIAS;
			} else {
				rgba[0] = SCALE * in[0];
				rgba[1] = SCALE * in[1];
				rgba[2] = SCALE * in[2];
			}
			rgba[3] = 0.0;
			in += 3;
		}
	}

	// unpack_rgba
	static void unpack_rgba(GLsizei n, double* rgba, char* src) 
	{
		component* in = reinterpret_cast<component*>(src);
		double* end = rgba + 4 * n;
		for (; rgba != end; rgba += 4) {
			if (bias) {
				rgba[0] = SCALE * in[0] + BIAS;
				rgba[1] = SCALE * in[1] + BIAS;
				rgba[2] = SCALE * in[2] + BIAS;
				rgba[3] = SCALE * in[3] + BIAS;
			} else {
				rgba[0] = SCALE * in[0];
				rgba[1] = SCALE * in[1];
				rgba[2] = SCALE * in[2];
				rgba[3] = SCALE * in[3];
			}
			in += 4;
		}
	}

};	// class Unpack

#undef SCALE
#undef BIAS

}; // anonymous namespace


namespace GLEAN {

///////////////////////////////////////////////////////////////////////////////
// Public interface
///////////////////////////////////////////////////////////////////////////////
void
Image::unpack(GLsizei n, double* rgba, char* nextPixel) {
	(*(valid(vbUnpacker)? _unpacker: validateUnpacker()))
		(n, rgba, nextPixel);
}

///////////////////////////////////////////////////////////////////////////////
// validateUnpacker - select appropriate pixel-unpacking utility
///////////////////////////////////////////////////////////////////////////////
Image::Unpacker*
Image::validateUnpacker() {
	switch (format()) {
	case GL_LUMINANCE:
		switch (type()) {
		case GL_BYTE:
			_unpacker = Unpack<GLbyte, 2, 255, 1>::unpack_l;
			break;
		case GL_UNSIGNED_BYTE:
			_unpacker = Unpack<GLubyte, 1, 255, 0>::unpack_l;
			break;
		case GL_SHORT:
			_unpacker = Unpack<GLshort, 2, 65535, 1>::unpack_l;
			break;
		case GL_UNSIGNED_SHORT:
			_unpacker = Unpack<GLushort, 1, 65535, 0>::unpack_l;
			break;
		case GL_INT:
			_unpacker = Unpack<GLint, 2, 4294967295U, 1>::unpack_l;
			break;
		case GL_UNSIGNED_INT:
			_unpacker = Unpack<GLuint, 1, 4294967295U, 0>::unpack_l;
			break;
		case GL_FLOAT:
			_unpacker = Unpack<GLfloat, 1, 1, 0>::unpack_l;
			break;
		default:
			throw BadType(type());
		}
		break;
	case GL_LUMINANCE_ALPHA:
		switch (type()) {
		case GL_BYTE:
			_unpacker = Unpack<GLbyte, 2, 255, 1>::unpack_la;
			break;
		case GL_UNSIGNED_BYTE:
			_unpacker = Unpack<GLubyte, 1, 255, 0>::unpack_la;
			break;
		case GL_SHORT:
			_unpacker = Unpack<GLshort, 2, 65535, 1>::unpack_la;
			break;
		case GL_UNSIGNED_SHORT:
			_unpacker = Unpack<GLushort, 1, 65535, 0>::unpack_la;
			break;
		case GL_INT:
			_unpacker = Unpack<GLint, 2, 4294967295U, 1>::unpack_la;
			break;
		case GL_UNSIGNED_INT:
			_unpacker = Unpack<GLuint, 2, 4294967295U, 0>::unpack_la;
			break;
		case GL_FLOAT:
			_unpacker = Unpack<GLfloat, 1, 1, 0>::unpack_la;
			break;
		default:
			throw BadType(type());
		}
		break;
	case GL_RGB:
		switch (type()) {
		case GL_BYTE:
			_unpacker = Unpack<GLbyte, 2, 255, 1>::unpack_rgb;
			break;
		case GL_UNSIGNED_BYTE:
			_unpacker = Unpack<GLubyte, 1, 255, 0>::unpack_rgb;
			break;
		case GL_SHORT:
			_unpacker = Unpack<GLshort, 2, 65535, 1>::unpack_rgb;
			break;
		case GL_UNSIGNED_SHORT:
			_unpacker = Unpack<GLushort, 1, 65535, 0>::unpack_rgb;
			break;
		case GL_INT:
			_unpacker = Unpack<GLint, 2, 4294967295U, 1>::unpack_rgb;
			break;
		case GL_UNSIGNED_INT:
			_unpacker = Unpack<GLuint, 1, 4294967295U, 0>::unpack_rgb;
			break;
		case GL_FLOAT:
			_unpacker = Unpack<GLfloat, 1, 1, 0>::unpack_rgb;
			break;
		default:
			throw BadType(type());
		}
		break;
	case GL_RGBA:
		switch (type()) {
		case GL_BYTE:
			_unpacker = Unpack<GLbyte, 2, 255, 1>::unpack_rgba;
			break;
		case GL_UNSIGNED_BYTE:
			_unpacker = Unpack<GLubyte, 1, 255, 0>::unpack_rgba;
			break;
		case GL_SHORT:
			_unpacker = Unpack<GLshort, 2, 65535, 1>::unpack_rgba;
			break;
		case GL_UNSIGNED_SHORT:
			_unpacker = Unpack<GLushort, 1, 65535, 0>::unpack_rgba;
			break;
		case GL_INT:
			_unpacker = Unpack<GLint, 2, 4294967295U, 1>::unpack_rgba;
			break;
		case GL_UNSIGNED_INT:
			_unpacker = Unpack<GLuint, 1, 4294967295U, 0>::unpack_rgba;
			break;
		case GL_FLOAT:
			_unpacker = Unpack<GLfloat, 1, 1, 0>::unpack_rgba;
			break;
		default:
			throw BadType(type());
		}
		break;
	default:
		throw BadFormat(format());
	}

	validate(vbUnpacker);
	return _unpacker;
}

}; // namespace GLEAN
