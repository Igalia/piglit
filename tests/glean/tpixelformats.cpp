// BEGIN_COPYRIGHT -*- glean -*-
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


#include <stdlib.h>
#include <cassert>
#include <cmath>
#include "tpixelformats.h"
#include "../util/rgb9e5.h"


// Set to 1 to help debug test failures:
// Also, when a test failure is found, rearrange the order of the
// formats, types, internformats below so the failure case comes first.
#define DEBUG 0


// just for extra debugging
// Maybe add fragment program path as a 3rd envMode (below) someday.
#define USE_FRAG_PROG 0

#define abort() do { printf("%s:%i\n", __FUNCTION__, __LINE__); abort(); } while (0)

namespace GLEAN {


struct NameTokenComps {
	const char *Name;
	GLenum Token;
	GLuint Components;
};


static const NameTokenComps Types[] =
{
	{ "GL_UNSIGNED_BYTE", GL_UNSIGNED_BYTE, 0 },
	{ "GL_BYTE", GL_BYTE, 0 },
	{ "GL_UNSIGNED_INT", GL_UNSIGNED_INT, 0 },
	{ "GL_SHORT", GL_SHORT, 0 },
	{ "GL_UNSIGNED_SHORT", GL_UNSIGNED_SHORT, 0 },
	{ "GL_INT", GL_INT, 0 },
	{ "GL_FLOAT", GL_FLOAT, 0 },
	{ "GL_HALF_FLOAT_ARB", GL_HALF_FLOAT_ARB, 0 },

	{ "GL_UNSIGNED_INT_8_8_8_8", GL_UNSIGNED_INT_8_8_8_8, 4 },
	{ "GL_UNSIGNED_INT_8_8_8_8_REV", GL_UNSIGNED_INT_8_8_8_8_REV, 4 },
	{ "GL_UNSIGNED_INT_10_10_10_2", GL_UNSIGNED_INT_10_10_10_2, 4 },
	{ "GL_UNSIGNED_INT_2_10_10_10_REV", GL_UNSIGNED_INT_2_10_10_10_REV, 4 },
	{ "GL_UNSIGNED_SHORT_5_5_5_1", GL_UNSIGNED_SHORT_5_5_5_1, 4 },
	{ "GL_UNSIGNED_SHORT_1_5_5_5_REV", GL_UNSIGNED_SHORT_1_5_5_5_REV, 4 },
	{ "GL_UNSIGNED_SHORT_4_4_4_4", GL_UNSIGNED_SHORT_4_4_4_4, 4 },
	{ "GL_UNSIGNED_SHORT_4_4_4_4_REV", GL_UNSIGNED_SHORT_4_4_4_4_REV, 4 },
	{ "GL_UNSIGNED_SHORT_5_6_5", GL_UNSIGNED_SHORT_5_6_5, 3 },
	{ "GL_UNSIGNED_SHORT_5_6_5_REV", GL_UNSIGNED_SHORT_5_6_5_REV, 3 },
	{ "GL_UNSIGNED_BYTE_3_3_2", GL_UNSIGNED_BYTE_3_3_2, 3 },
	{ "GL_UNSIGNED_BYTE_2_3_3_REV", GL_UNSIGNED_BYTE_2_3_3_REV, 3 },

	{ "GL_UNSIGNED_INT_5_9_9_9_REV", GL_UNSIGNED_INT_5_9_9_9_REV, 3 }
};

#define NUM_TYPES (sizeof(Types) / sizeof(Types[0]))


static const NameTokenComps Formats[] =
{
	{ "GL_RGBA", GL_RGBA, 4 },
	{ "GL_BGRA", GL_BGRA, 4 },
	{ "GL_RGB", GL_RGB, 3 },
	{ "GL_BGR", GL_BGR, 3 },
	{ "GL_RED", GL_RED, 1 },
	{ "GL_GREEN", GL_GREEN, 1 },
	{ "GL_BLUE", GL_BLUE, 1 },
	{ "GL_ALPHA", GL_ALPHA, 1 },
	{ "GL_LUMINANCE", GL_LUMINANCE, 1 },
	{ "GL_LUMINANCE_ALPHA", GL_LUMINANCE_ALPHA, 2 },
	{ "GL_ABGR_EXT", GL_ABGR_EXT, 4 },
	{ "GL_RG", GL_RG, 2 }
};

#define NUM_FORMATS (sizeof(Formats) / sizeof(Formats[0]))


static const NameTokenComps InternalFormats[] =
{
	{ "glDrawPixels", 0, 4 },  // special case for glDrawPixels

	{ "4", 4, 4 },
	{ "GL_RGBA", GL_RGBA, 4 },
	{ "GL_RGBA2", GL_RGBA2, 4 },
	{ "GL_RGBA4", GL_RGBA4, 4 },
	{ "GL_RGB5_A1", GL_RGB5_A1, 4 },
	{ "GL_RGBA8", GL_RGBA8, 4 },
	{ "GL_RGB10_A2", GL_RGB10_A2, 4 },
	{ "GL_RGBA12", GL_RGBA12, 4 },
	{ "GL_RGBA16", GL_RGBA16, 4 },
	{ "GL_SRGB_ALPHA_EXT", GL_SRGB_ALPHA_EXT, 4 },
	{ "GL_SRGB8_ALPHA8_EXT", GL_SRGB8_ALPHA8_EXT, 4 },

	{ "3", 3, 3 },
	{ "GL_RGB", GL_RGB, 3 },
	{ "GL_R3_G3_B2", GL_R3_G3_B2, 3 },
	{ "GL_RGB4", GL_RGB4, 3 },
	{ "GL_RGB5", GL_RGB5, 3 },
	{ "GL_RGB8", GL_RGB8, 3 },
	{ "GL_RGB10", GL_RGB10, 3 },
	{ "GL_RGB12", GL_RGB12, 3 },
	{ "GL_RGB16", GL_RGB16, 3 },
	{ "GL_SRGB_EXT", GL_SRGB_EXT, 3 },
	{ "GL_SRGB8_EXT", GL_SRGB8_EXT, 3 },

	{ "2", 2, 2 },
	{ "GL_LUMINANCE_ALPHA", GL_LUMINANCE_ALPHA, 2 },
	{ "GL_LUMINANCE4_ALPHA4", GL_LUMINANCE4_ALPHA4, 1 },
	{ "GL_LUMINANCE6_ALPHA2", GL_LUMINANCE6_ALPHA2, 1 },
	{ "GL_LUMINANCE8_ALPHA8", GL_LUMINANCE8_ALPHA8, 1 },
	{ "GL_LUMINANCE12_ALPHA4", GL_LUMINANCE12_ALPHA4, 1 },
	{ "GL_LUMINANCE12_ALPHA12", GL_LUMINANCE12_ALPHA12, 1 },
	{ "GL_LUMINANCE16_ALPHA16", GL_LUMINANCE16_ALPHA16, 1 },
	{ "GL_SLUMINANCE_ALPHA_EXT", GL_SLUMINANCE_ALPHA_EXT, 3 },
	{ "GL_SLUMINANCE8_ALPHA8_EXT", GL_SLUMINANCE8_ALPHA8_EXT, 3 },

	{ "1", 1, 1 },
	{ "GL_LUMINANCE", GL_LUMINANCE, 1 },
	{ "GL_LUMINANCE4", GL_LUMINANCE4, 1 },
	{ "GL_LUMINANCE8", GL_LUMINANCE8, 1 },
	{ "GL_LUMINANCE12", GL_LUMINANCE12, 1 },
	{ "GL_LUMINANCE16", GL_LUMINANCE16, 1 },
	{ "GL_SLUMINANCE_EXT", GL_SLUMINANCE_EXT, 3 },
	{ "GL_SLUMINANCE8_EXT", GL_SLUMINANCE8_EXT, 3 },

	{ "GL_ALPHA", GL_ALPHA, 1 },
	{ "GL_ALPHA4", GL_ALPHA4, 1 },
	{ "GL_ALPHA8", GL_ALPHA8, 1 },
	{ "GL_ALPHA12", GL_ALPHA12, 1 },
	{ "GL_ALPHA16", GL_ALPHA16, 1 },

	{ "GL_INTENSITY", GL_INTENSITY, 1 },
	{ "GL_INTENSITY4", GL_INTENSITY4, 1 },
	{ "GL_INTENSITY8", GL_INTENSITY8, 1 },
	{ "GL_INTENSITY12", GL_INTENSITY12, 1 },
	{ "GL_INTENSITY16", GL_INTENSITY16, 1 },

	{ "GL_RED", GL_RED, 1 },
	{ "GL_RG", GL_RG, 2 },
	{ "GL_R8", GL_R8, 1 },
	{ "GL_RG8", GL_RG8, 2 },
	{ "GL_R16", GL_R16, 1},
	{ "GL_RG16", GL_RG16, 2 },
	{ "GL_R16F", GL_R16F, 1 },
	{ "GL_RG16F", GL_RG16F, 2 },
	{ "GL_R32F", GL_R32F, 1},
	{ "GL_RG32F", GL_RG32F, 2 },

	{ "GL_RED_SNORM", GL_RED_SNORM, 1 },
	{ "GL_RG_SNORM", GL_RG_SNORM, 2 },
	{ "GL_RGB_SNORM", GL_RGB_SNORM, 3 },
	{ "GL_RGBA_SNORM", GL_RGBA_SNORM, 4 },
	{ "GL_ALPHA_SNORM", GL_ALPHA_SNORM, 1 },
	{ "GL_LUMINANCE_SNORM", GL_LUMINANCE_SNORM, 1 },
	{ "GL_LUMINANCE_ALPHA_SNORM", GL_LUMINANCE_ALPHA_SNORM, 2 },
	{ "GL_INTENSITY_SNORM", GL_INTENSITY_SNORM, 1 },

	{ "GL_R8_SNORM", GL_R8_SNORM, 1 },
	{ "GL_RG8_SNORM", GL_RG8_SNORM, 2 },
	{ "GL_RGB8_SNORM", GL_RGB8_SNORM, 3 },
	{ "GL_RGBA8_SNORM", GL_RGBA8_SNORM, 4 },
	{ "GL_ALPHA8_SNORM", GL_ALPHA8_SNORM, 1 },
	{ "GL_LUMINANCE8_SNORM", GL_LUMINANCE8_SNORM, 1 },
	{ "GL_LUMINANCE8_ALPHA8_SNORM", GL_LUMINANCE8_ALPHA8_SNORM, 2 },
	{ "GL_INTENSITY8_SNORM", GL_INTENSITY8_SNORM, 1 },

	{ "GL_R16_SNORM", GL_R16_SNORM, 1 },
	{ "GL_RG16_SNORM", GL_RG16_SNORM, 2 },
	{ "GL_RGB16_SNORM", GL_RGB16_SNORM, 3 },
	{ "GL_RGBA16_SNORM", GL_RGBA16_SNORM, 4 },
	{ "GL_ALPHA16_SNORM", GL_ALPHA16_SNORM, 1 },
	{ "GL_LUMINANCE16_SNORM", GL_LUMINANCE16_SNORM, 1 },
	{ "GL_LUMINANCE16_ALPHA16_SNORM", GL_LUMINANCE16_ALPHA16_SNORM, 2 },
	{ "GL_INTENSITY16_SNORM", GL_INTENSITY16_SNORM, 1 },

	{ "GL_RGB9_E5", GL_RGB9_E5, 3 }

	// XXX maybe add compressed formats too...
};

#define NUM_INT_FORMATS (sizeof(InternalFormats) / sizeof(InternalFormats[0]))


static const char *EnvModes[] = {
	"GL_REPLACE",
	"GL_COMBINE_ARB"
};


// Return four bitmasks indicating which bits correspond to the
// 1st, 2nd, 3rd and 4th components in a packed datatype.
// Set all masks to zero for non-packed types.
static void
ComponentMasks(GLenum datatype, GLuint masks[4])
{
	switch (datatype) {
	case GL_UNSIGNED_BYTE:
	case GL_BYTE:
	case GL_UNSIGNED_SHORT:
	case GL_SHORT:
	case GL_UNSIGNED_INT:
	case GL_INT:
	case GL_FLOAT:
	case GL_HALF_FLOAT_ARB:
	case GL_UNSIGNED_INT_5_9_9_9_REV: /* special case, handled separately */
		masks[0] =
		masks[1] =
		masks[2] =
		masks[3] = 0x0;
		break;
	case GL_UNSIGNED_INT_8_8_8_8:
		masks[0] = 0xff000000;
		masks[1] = 0x00ff0000;
		masks[2] = 0x0000ff00;
		masks[3] = 0x000000ff;
		break;
	case GL_UNSIGNED_INT_8_8_8_8_REV:
		masks[0] = 0x000000ff;
		masks[1] = 0x0000ff00;
		masks[2] = 0x00ff0000;
		masks[3] = 0xff000000;
		break;
	case GL_UNSIGNED_INT_10_10_10_2:
		masks[0] = 0xffc00000;
		masks[1] = 0x003ff000;
		masks[2] = 0x00000ffc;
		masks[3] = 0x00000003;
		break;
	case GL_UNSIGNED_INT_2_10_10_10_REV:
		masks[0] = 0x000003ff;
		masks[1] = 0x000ffc00;
		masks[2] = 0x3ff00000;
		masks[3] = 0xc0000000;
		break;
	case GL_UNSIGNED_SHORT_5_5_5_1:
		masks[0] = 0xf800;
		masks[1] = 0x07c0;
		masks[2] = 0x003e;
		masks[3] = 0x0001;
		break;
	case GL_UNSIGNED_SHORT_1_5_5_5_REV:
		masks[0] = 0x001f;
		masks[1] = 0x03e0;
		masks[2] = 0x7c00;
		masks[3] = 0x8000;
		break;
	case GL_UNSIGNED_SHORT_4_4_4_4:
		masks[0] = 0xf000;
		masks[1] = 0x0f00;
		masks[2] = 0x00f0;
		masks[3] = 0x000f;
		break;
	case GL_UNSIGNED_SHORT_4_4_4_4_REV:
		masks[0] = 0x000f;
		masks[1] = 0x00f0;
		masks[2] = 0x0f00;
		masks[3] = 0xf000;
		break;
	case GL_UNSIGNED_SHORT_5_6_5:
		masks[0] = 0xf800;
		masks[1] = 0x07e0;
		masks[2] = 0x001f;
		masks[3] = 0;
		break;
	case GL_UNSIGNED_SHORT_5_6_5_REV:
		masks[0] = 0x001f;
		masks[1] = 0x07e0;
		masks[2] = 0xf800;
		masks[3] = 0;
		break;
	case GL_UNSIGNED_BYTE_3_3_2:
		masks[0] = 0xe0;
		masks[1] = 0x1c;
		masks[2] = 0x03;
		masks[3] = 0;
		break;
	case GL_UNSIGNED_BYTE_2_3_3_REV:
		masks[0] = 0x07;
		masks[1] = 0x38;
		masks[2] = 0xc0;
		masks[3] = 0;
		break;
	default:
		abort();
	}
}


// Return four values indicating the ordering of the Red, Green, Blue and
// Alpha components for the given image format.
// For example: GL_BGRA = {2, 1, 0, 3}.
static void
ComponentPositions(GLenum format, GLint pos[4])
{
	switch (format) {
	case GL_RGBA:
		pos[0] = 0;
		pos[1] = 1;
		pos[2] = 2;
		pos[3] = 3;
		break;
	case GL_BGRA:
		pos[0] = 2;
		pos[1] = 1;
		pos[2] = 0;
		pos[3] = 3;
		break;
	case GL_RGB:
		pos[0] = 0;
		pos[1] = 1;
		pos[2] = 2;
		pos[3] = -1;
		break;
	case GL_BGR:
		pos[0] = 2;
		pos[1] = 1;
		pos[2] = 0;
		pos[3] = -1;
		break;
	case GL_LUMINANCE:
		pos[0] = 0;
		pos[1] = -1;
		pos[2] = -1;
		pos[3] = -1;
		break;
	case GL_LUMINANCE_ALPHA:
		pos[0] = 0;
		pos[1] = -1;
		pos[2] = -1;
		pos[3] = 1;
		break;
	case GL_RED:
		pos[0] = 0;
		pos[1] = -1;
		pos[2] = -1;
		pos[3] = -1;
		break;
	case GL_GREEN:
		pos[0] = -1;
		pos[1] = 0;
		pos[2] = -1;
		pos[3] = -1;
		break;
	case GL_BLUE:
		pos[0] = -1;
		pos[1] = -1;
		pos[2] = 0;
		pos[3] = -1;
		break;
	case GL_ALPHA:
		pos[0] = -1;
		pos[1] = -1;
		pos[2] = -1;
		pos[3] = 0;
		break;
	case GL_ABGR_EXT:
		pos[0] = 3;
		pos[1] = 2;
		pos[2] = 1;
		pos[3] = 0;
		break;
	case GL_RG:
		pos[0] = 0;
		pos[1] = 1;
		pos[2] = -1;
		pos[3] = -1;
		break;
	default:
		abort();
	}
}


// Given a texture internal format, return the corresponding base format.
static GLenum
BaseTextureFormat(GLint intFormat)
{
	switch (intFormat) {
	case 0:
		return 0;  // for glDrawPixels
	case GL_ALPHA:
	case GL_ALPHA4:
	case GL_ALPHA8:
	case GL_ALPHA12:
	case GL_ALPHA16:
	case GL_ALPHA_SNORM:
	case GL_ALPHA8_SNORM:
	case GL_ALPHA16_SNORM:
		return GL_ALPHA;
	case 1:
	case GL_LUMINANCE:
	case GL_LUMINANCE4:
	case GL_LUMINANCE8:
	case GL_LUMINANCE12:
	case GL_LUMINANCE16:
	case GL_LUMINANCE_SNORM:
	case GL_LUMINANCE8_SNORM:
	case GL_LUMINANCE16_SNORM:
		return GL_LUMINANCE;
	case 2:
	case GL_LUMINANCE_ALPHA:
	case GL_LUMINANCE4_ALPHA4:
	case GL_LUMINANCE6_ALPHA2:
	case GL_LUMINANCE8_ALPHA8:
	case GL_LUMINANCE12_ALPHA4:
	case GL_LUMINANCE12_ALPHA12:
	case GL_LUMINANCE16_ALPHA16:
	case GL_LUMINANCE_ALPHA_SNORM:
	case GL_LUMINANCE8_ALPHA8_SNORM:
	case GL_LUMINANCE16_ALPHA16_SNORM:
		return GL_LUMINANCE_ALPHA;
	case GL_INTENSITY:
	case GL_INTENSITY4:
	case GL_INTENSITY8:
	case GL_INTENSITY12:
	case GL_INTENSITY16:
	case GL_INTENSITY_SNORM:
	case GL_INTENSITY8_SNORM:
	case GL_INTENSITY16_SNORM:
		return GL_INTENSITY;

	case GL_RED:
	case GL_R8:
	case GL_R16:
	case GL_R16F:
	case GL_R32F:
	case GL_RED_SNORM:
	case GL_R8_SNORM:
	case GL_R16_SNORM:
		return GL_RED;

	case GL_RG:
	case GL_RG8:
	case GL_RG16:
	case GL_RG16F:
	case GL_RG32F:
	case GL_RG_SNORM:
	case GL_RG8_SNORM:
	case GL_RG16_SNORM:
		return GL_RG;

	case 3:
	case GL_RGB:
	case GL_R3_G3_B2:
	case GL_RGB4:
	case GL_RGB5:
	case GL_RGB8:
	case GL_RGB10:
	case GL_RGB12:
	case GL_RGB16:
	case GL_RGB_SNORM:
	case GL_RGB8_SNORM:
	case GL_RGB16_SNORM:
	case GL_RGB9_E5:
		return GL_RGB;
	case 4:
	case GL_RGBA:
	case GL_RGBA2:
	case GL_RGBA4:
	case GL_RGB5_A1:
	case GL_RGBA8:
	case GL_RGB10_A2:
	case GL_RGBA12:
	case GL_RGBA16:
	case GL_RGBA_SNORM:
	case GL_RGBA8_SNORM:
	case GL_RGBA16_SNORM:
		return GL_RGBA;

	case GL_SRGB_EXT:
	case GL_SRGB8_EXT:
	case GL_COMPRESSED_SRGB_EXT:
	case GL_COMPRESSED_SRGB_S3TC_DXT1_EXT:
		return GL_RGB;
	case GL_SRGB_ALPHA_EXT:
	case GL_SRGB8_ALPHA8_EXT:
	case GL_COMPRESSED_SRGB_ALPHA_EXT:
	case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT:
	case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT:
	case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT:
		return GL_RGBA;
	case GL_SLUMINANCE_ALPHA_EXT:
	case GL_SLUMINANCE8_ALPHA8_EXT:
	case GL_COMPRESSED_SLUMINANCE_EXT:
	case GL_COMPRESSED_SLUMINANCE_ALPHA_EXT:
		return GL_LUMINANCE_ALPHA;
	case GL_SLUMINANCE_EXT:
	case GL_SLUMINANCE8_EXT:
		return GL_LUMINANCE;
	default:
		abort();
	}
}




// Return number components in the given datatype.  This is 3 or 4 for
// packed types and zero for non-packed types
// Ex: GL_UNSIGNED_SHORT_5_5_5_1 = 4
// Ex: GL_INT = 0
static int
NumberOfComponentsInPackedType(GLenum datatype)
{
	for (unsigned i = 0; i < NUM_TYPES; i++) {
		if (Types[i].Token == datatype)
			return Types[i].Components;
	}
	abort();
}


static int
IsPackedType(GLenum datatype)
{
	return NumberOfComponentsInPackedType(datatype) > 0;
}


// Return number components in the given image format.
// Ex: GL_BGR = 3
static int
NumberOfComponentsInFormat(GLenum format)
{
	for (unsigned i = 0; i < NUM_FORMATS; i++) {
		if (Formats[i].Token == format)
			return Formats[i].Components;
	}
	abort();
}


// Return size, in bytes, of given datatype.
static int
SizeofType(GLenum datatype)
{
	switch (datatype) {
	case GL_UNSIGNED_INT_10_10_10_2:
	case GL_UNSIGNED_INT_2_10_10_10_REV:
	case GL_UNSIGNED_INT_8_8_8_8:
	case GL_UNSIGNED_INT_8_8_8_8_REV:
	case GL_UNSIGNED_INT_5_9_9_9_REV:
	case GL_UNSIGNED_INT:
	case GL_INT:
	case GL_FLOAT:
		return 4;
	case GL_UNSIGNED_SHORT_5_5_5_1:
	case GL_UNSIGNED_SHORT_1_5_5_5_REV:
	case GL_UNSIGNED_SHORT_4_4_4_4:
	case GL_UNSIGNED_SHORT_4_4_4_4_REV:
	case GL_UNSIGNED_SHORT_5_6_5:
	case GL_UNSIGNED_SHORT_5_6_5_REV:
	case GL_UNSIGNED_SHORT:
	case GL_SHORT:
	case GL_HALF_FLOAT_ARB:
		return 2;
	case GL_UNSIGNED_BYTE_3_3_2:
	case GL_UNSIGNED_BYTE_2_3_3_REV:
	case GL_UNSIGNED_BYTE:
	case GL_BYTE:
		return 1;
	default:
		abort();
	}
}


// Check if the given image format and datatype are compatible.
// Also check for types/formats defined by GL extensions here.
bool
PixelFormatsTest::CompatibleFormatAndType(GLenum format, GLenum datatype) const
{
	// Special case: GL_BGR can't be used with packed types!
	// This has to do with putting the most color bits in red and green,
	// not blue.
	if (format == GL_BGR && IsPackedType(datatype))
		return false;

	if (datatype == GL_HALF_FLOAT_ARB && !haveHalfFloat)
		return false;

	if (format == GL_ABGR_EXT && !haveABGR)
		return false;

	// Special case: GL_ABGR_EXT can't be used with packed types
	// because the packed formats specs (which were all written after the
	// GL_EXT_abgr) explicitly say that the packed formats can only be used
	// with GL_RGB, GL_BGR, GL_RGBA, or GL_BGRA and do not mention
	// GL_ABGR_EXT.
	if (format == GL_ABGR_EXT && IsPackedType(datatype))
		return false;

	if (format == GL_RG && !haveRG)
		return false;

	if (datatype == GL_UNSIGNED_INT_5_9_9_9_REV && !haveTexSharedExp)
		return false;

	const int formatComps = NumberOfComponentsInFormat(format);
	const int typeComps = NumberOfComponentsInPackedType(datatype);
	return formatComps == typeComps || typeComps == 0;
}


bool
PixelFormatsTest::SupportedIntFormat(GLint intFormat) const
{
	switch (intFormat) {
	case GL_SRGB_ALPHA_EXT:
	case GL_SRGB8_ALPHA8_EXT:
	case GL_SRGB_EXT:
	case GL_SRGB8_EXT:
	case GL_SLUMINANCE_ALPHA_EXT:
	case GL_SLUMINANCE8_ALPHA8_EXT:
	case GL_SLUMINANCE_EXT:
	case GL_SLUMINANCE8_EXT:
		return haveSRGB;
	case GL_RED:
	case GL_RG:
	case GL_R8:
	case GL_RG8:
	case GL_R16:
	case GL_RG16:
		return haveRG;
	case GL_R16F:
	case GL_RG16F:
	case GL_R32F:
	case GL_RG32F:
		return haveRG && haveFloat;
	case GL_RED_SNORM:
	case GL_R8_SNORM:
	case GL_R16_SNORM:
	case GL_RG_SNORM:
	case GL_RG8_SNORM:
	case GL_RG16_SNORM:
	case GL_RGB_SNORM:
	case GL_RGB8_SNORM:
	case GL_RGB16_SNORM:
	case GL_RGBA_SNORM:
	case GL_RGBA8_SNORM:
	case GL_RGBA16_SNORM:
	case GL_ALPHA_SNORM:
	case GL_ALPHA8_SNORM:
	case GL_ALPHA16_SNORM:
	case GL_LUMINANCE_SNORM:
	case GL_LUMINANCE8_SNORM:
	case GL_LUMINANCE16_SNORM:
	case GL_LUMINANCE_ALPHA_SNORM:
	case GL_LUMINANCE8_ALPHA8_SNORM:
	case GL_LUMINANCE16_ALPHA16_SNORM:
	case GL_INTENSITY_SNORM:
	case GL_INTENSITY8_SNORM:
	case GL_INTENSITY16_SNORM:
		return haveSnorm;
	case GL_RGB9_E5:
		return haveTexSharedExp;
	default:

		return true;
	}
}


// Determine if the ith pixel is in the upper-right quadrant of the
// rectangle of size 'width' x 'height'.
static bool
IsUpperRight(int i, int width, int height)
{
	const int y = i / width, x = i % width;
	return (x >= width / 2 && y >= height / 2);
}



// Create an image buffer and fill it so that a single image channel is
// the max value (1.0) while the other channels are zero.  For example,
// if fillComponent==2 and we're filling a four-component image, the
// pixels will be (0, 0, max, 0).
//
// We always leave the upper-right quadrant black/zero.  This is to help
// detect any image conversion issues related to stride, packing, etc.
static GLubyte *
MakeImage(int width, int height, GLenum format, GLenum type,
		  int fillComponent)
{
	assert(fillComponent < 4);

	if (type == GL_UNSIGNED_INT_5_9_9_9_REV) {
		GLubyte *image = new GLubyte [width * height * 4];
		int i;

		assert(format == GL_RGB);

		GLuint *ui = (GLuint *) image;
		for (i = 0; i < width * height; i++) {
			float p[3] = {0, 0, 0};

			if (!IsUpperRight(i, width, height))
				p[fillComponent] =  1;

			ui[i] = float3_to_rgb9e5(p);
		}

		return image;
	}
	else if (IsPackedType(type)) {
		const int bpp = SizeofType(type);
		GLubyte *image = new GLubyte [width * height * bpp];
		GLuint masks[4];
		int pos[4];
		int i;

		ComponentMasks(type, masks);
		ComponentPositions(format, pos);

		const GLuint value = masks[fillComponent];

		switch (bpp) {
		case 1:
			for (i = 0; i < width * height; i++) {
				if (IsUpperRight(i, width, height))
					image[i] = 0;
				else
					image[i] = (GLubyte) value;
			}
			break;
		case 2:
			{
				GLushort *image16 = (GLushort *) image;
				for (i = 0; i < width * height; i++) {
					if (IsUpperRight(i, width, height))
						image16[i] = 0;
					else
						image16[i] = (GLushort) value;
				}
			}
			break;
		case 4:
			{
				GLuint *image32 = (GLuint *) image;
				for (i = 0; i < width * height; i++) {
					if (IsUpperRight(i, width, height))
						image32[i] = 0;
					else
						image32[i] = (GLuint) value;
				}
			}
			break;
		default:
			abort();
		}

		return image;
	}
	else {
		const int comps = NumberOfComponentsInFormat(format);
		const int bpp = comps * SizeofType(type);
		assert(bpp > 0);
		GLubyte *image = new GLubyte [width * height * bpp];
		int i;

		switch (type) {
		case GL_UNSIGNED_BYTE:
			for (i = 0; i < width * height * comps; i++) {
				if (i % comps == fillComponent && !IsUpperRight(i/comps, width, height))
					image[i] = 0xff;
				else
					image[i] = 0x0;
			}
			break;
		case GL_BYTE:
			{
				GLbyte *b = (GLbyte *) image;
				for (i = 0; i < width * height * comps; i++) {
					if (i % comps == fillComponent && !IsUpperRight(i/comps, width, height))
						b[i] = 0x7f;
					else
						b[i] = 0x0;
				}
			}
			break;
		case GL_UNSIGNED_SHORT:
			{
				GLushort *us = (GLushort *) image;
				for (i = 0; i < width * height * comps; i++) {
					if (i % comps == fillComponent && !IsUpperRight(i/comps, width, height))
						us[i] = 0xffff;
					else
						us[i] = 0x0;
				}
			}
			break;
		case GL_SHORT:
			{
				GLshort *s = (GLshort *) image;
				for (i = 0; i < width * height * comps; i++) {
					if (i % comps == fillComponent && !IsUpperRight(i/comps, width, height))
						s[i] = 0x7fff;
					else
						s[i] = 0x0;
				}
			}
			break;
		case GL_UNSIGNED_INT:
			{
				GLuint *ui = (GLuint *) image;
				for (i = 0; i < width * height * comps; i++) {
					if (i % comps == fillComponent && !IsUpperRight(i/comps, width, height))
						ui[i] = 0xffffffff;
					else
						ui[i] = 0x0;
				}
			}
			break;
		case GL_INT:
			{
				GLint *in = (GLint *) image;
				for (i = 0; i < width * height * comps; i++) {
					if (i % comps == fillComponent && !IsUpperRight(i/comps, width, height))
						in[i] = 0x7fffffff;
					else
						in[i] = 0x0;
				}
			}
			break;
		case GL_FLOAT:
			{
				GLfloat *f = (GLfloat *) image;
				for (i = 0; i < width * height * comps; i++) {
					if (i % comps == fillComponent && !IsUpperRight(i/comps, width, height))
						f[i] = 1.0;
					else
						f[i] = 0.0;
				}
			}
			break;
		case GL_HALF_FLOAT_ARB:
			{
				GLhalfARB *f = (GLhalfARB *) image;
				for (i = 0; i < width * height * comps; i++) {
					if (i % comps == fillComponent && !IsUpperRight(i/comps, width, height))
						f[i] = 0x3c00;  /* == 1.0 */
					else
						f[i] = 0;
				}
			}
			break;
		default:
			abort();
		}
		return image;
	}
}


bool
PixelFormatsTest::CheckError(const char *where) const
{
	GLint err = glGetError();
	if (err) {
		char msg[1000];
		sprintf(msg, "GL Error: %s (0x%x) in %s\n",
				gluErrorString(err), err, where);
		env->log << msg;
		return true;
	}
	return false;
}


// Draw the given image, either as a texture quad or glDrawPixels.
// Return true for success, false if GL error detected.
bool
PixelFormatsTest::DrawImage(int width, int height,
							GLenum format, GLenum type, GLint intFormat,
							const GLubyte *image) const
{
	if (intFormat) {
		glEnable(GL_TEXTURE_2D);
		glViewport(0, 0, width, height);
		glTexImage2D(GL_TEXTURE_2D, 0, intFormat, width, height, 0,
					 format, type, image);
		if (CheckError("glTexImage2D"))
			return false;
#if USE_FRAG_PROG
		glEnable(GL_FRAGMENT_PROGRAM_ARB);
#endif
		glBegin(GL_POLYGON);
		glTexCoord2f(0, 0);  glVertex2f(-1, -1);
		glTexCoord2f(1, 0);  glVertex2f(1, -1);
		glTexCoord2f(1, 1);  glVertex2f(1, 1);
		glTexCoord2f(0, 1);  glVertex2f(-1, 1);
		glEnd();
		glDisable(GL_TEXTURE_2D);
#if USE_FRAG_PROG
		glDisable(GL_FRAGMENT_PROGRAM_ARB);
#endif
	}
	else {
		// glDrawPixels
		glDrawPixels(width, height, format, type, image);
		if (CheckError("glDrawPixels"))
			return false;
	}
	return true;
}


static bool
ColorsEqual(const GLubyte img[4], const GLubyte expected[4])
{
	const int tolerance = 1;
	if ((abs(img[0] - expected[0]) > tolerance) ||
		(abs(img[1] - expected[1]) > tolerance) ||
		(abs(img[2] - expected[2]) > tolerance) ||
		(abs(img[3] - expected[3]) > tolerance)) {
		return false;
	}
	else {
		return true;
	}
}


// Compute the expected RGBA color we're expecting to find with glReadPixels
// if the texture was defined with the given image format and texture
// internal format.  'testChan' indicates which of the srcFormat's image
// channels was set (to 1.0) when the image was filled.
void
PixelFormatsTest::ComputeExpected(GLenum srcFormat, int testChan,
								  GLint intFormat, GLubyte exp[4]) const
{
	const GLenum baseIntFormat = BaseTextureFormat(intFormat);

	switch (srcFormat) {

	case GL_RGBA:
	case GL_BGRA:
	case GL_ABGR_EXT:
		assert(testChan < 4);
		switch (baseIntFormat) {
		case 0: // == glReadPixels
			// fallthrough
		case GL_RGBA:
			exp[0] = 0;
			exp[1] = 0;
			exp[2] = 0;
			exp[3] = 0;
			exp[testChan] = 255;
			break;
		case GL_RGB:
			exp[0] = 0;
			exp[1] = 0;
			exp[2] = 0;
			exp[testChan] = 255;
			exp[3] = defaultAlpha; // fragment alpha or texture alpha
			break;
		case GL_RG:
			exp[0] = 0;
			exp[1] = 0;
			exp[testChan] = 255;
			exp[2] = 0;
			exp[3] = defaultAlpha; // fragment alpha or texture alpha
			break;
		case GL_RED:
			exp[0] = testChan == 0 ? 255 : 0;
			exp[1] = 0;
			exp[2] = 0;
			exp[3] = defaultAlpha; // fragment alpha or texture alpha
			break;
		case GL_ALPHA:
			exp[0] = 0;
			exp[1] = 0;
			exp[2] = 0;
			exp[3] = testChan == 3 ? 255 : 0;
			break;
		case GL_LUMINANCE:
			exp[0] =
			exp[1] =
			exp[2] = testChan == 0 ? 255 : 0;
			exp[3] = defaultAlpha; // fragment alpha or texture alpha
			break;
		case GL_LUMINANCE_ALPHA:
			exp[0] =
			exp[1] =
			exp[2] = testChan == 0 ? 255 : 0;
			exp[3] = testChan == 3 ? 255 : 0;
			break;
		case GL_INTENSITY:
			exp[0] =
			exp[1] =
			exp[2] =
			exp[3] = testChan == 0 ? 255 : 0;
			break;
		default:
			abort();
		}
		break;

	case GL_RGB:
	case GL_BGR:
		assert(testChan < 3);
		switch (baseIntFormat) {
		case 0:
		case GL_RGBA:
			exp[0] = 0;
			exp[1] = 0;
			exp[2] = 0;
			exp[testChan] = 255;
			exp[3] = 255; // texture's alpha
			break;
		case GL_RGB:
			exp[0] = 0;
			exp[1] = 0;
			exp[2] = 0;
			exp[testChan] = 255;
			exp[3] = defaultAlpha; // fragment alpha or texture alpha
			break;
		case GL_RG:
			exp[0] = 0;
			exp[1] = 0;
			exp[testChan] = 255;
			exp[2] = 0;
			exp[3] = defaultAlpha; // fragment alpha or texture alpha
			break;
		case GL_RED:
			exp[0] = testChan == 0 ? 255 : 0;
			exp[1] = 0;
			exp[2] = 0;
			exp[3] = defaultAlpha; // fragment alpha or texture alpha
			break;
		case GL_ALPHA:
			exp[0] = 0;
			exp[1] = 0;
			exp[2] = 0;
			exp[3] = 255; // texture's alpha
			break;
		case GL_LUMINANCE:
			exp[0] =
			exp[1] =
			exp[2] = testChan == 0 ? 255 : 0;
			exp[3] = defaultAlpha; // fragment alpha or texture alpha
			break;
		case GL_LUMINANCE_ALPHA:
			exp[0] =
			exp[1] =
			exp[2] = testChan == 0 ? 255 : 0;
			exp[3] = 255;  // texture's alpha
			break;
		case GL_INTENSITY:
			exp[0] =
			exp[1] =
			exp[2] =
			exp[3] = testChan == 0 ? 255 : 0;
			break;
		default:
			abort();
		}
		break;

	case GL_RG:
		assert(testChan < 2);
		switch (baseIntFormat) {
		case 0:
		case GL_RGBA:
			exp[0] = 0;
			exp[1] = 0;
			exp[testChan] = 255;
			exp[2] = 0;
			exp[3] = 255; // texture's alpha
			break;
		case GL_RGB:
			exp[0] = 0;
			exp[1] = 0;
			exp[testChan] = 255;
			exp[2] = 0;
			exp[3] = defaultAlpha; // fragment alpha or texture alpha
			break;
		case GL_RG:
			exp[0] = 0;
			exp[1] = 0;
			exp[testChan] = 255;
			exp[2] = 0;
			exp[3] = defaultAlpha; // fragment alpha or texture alpha
			break;
		case GL_RED:
			exp[0] = testChan == 0 ? 255 : 0;
			exp[1] = 0;
			exp[2] = 0;
			exp[3] = defaultAlpha; // fragment alpha or texture alpha
			break;
		case GL_ALPHA:
			exp[0] = 0;
			exp[1] = 0;
			exp[2] = 0;
			exp[3] = 255; // texture's alpha
			break;
		case GL_LUMINANCE:
			exp[0] =
			exp[1] =
			exp[2] = testChan == 0 ? 255 : 0;
			exp[3] = defaultAlpha; // fragment alpha or texture alpha
			break;
		case GL_LUMINANCE_ALPHA:
			exp[0] =
			exp[1] =
			exp[2] = testChan == 0 ? 255 : 0;
			exp[3] = 255;  // texture's alpha
			break;
		case GL_INTENSITY:
			exp[0] =
			exp[1] =
			exp[2] =
			exp[3] = testChan == 0 ? 255 : 0;
			break;
		default:
			abort();
		}
		break;

	case GL_RED:
		assert(testChan == 0);
		switch (baseIntFormat) {
		case 0:
		case GL_RGBA:
			exp[0] = 255;
			exp[1] = 0;
			exp[2] = 0;
			exp[3] = 255; // texture's alpha
			break;
		case GL_RGB:
			exp[0] = 255;
			exp[1] = 0;
			exp[2] = 0;
			exp[3] = defaultAlpha; // fragment alpha or texture alpha
			break;
		case GL_RG:
			exp[0] = 255;
			exp[1] = 0;
			exp[2] = 0;
			exp[3] = defaultAlpha; // fragment alpha or texture alpha
			break;
		case GL_RED:
			exp[0] = 255;
			exp[1] = 0;
			exp[2] = 0;
			exp[3] = defaultAlpha; // fragment alpha or texture alpha
			break;
		case GL_ALPHA:
			exp[0] = 0;
			exp[1] = 0;
			exp[2] = 0;
			exp[3] = 255; // texture's alpha
			break;
		case GL_LUMINANCE:
			exp[0] =
			exp[1] =
			exp[2] = 255;
			exp[3] = defaultAlpha; // fragment alpha or texture alpha
			break;
		case GL_LUMINANCE_ALPHA:
			exp[0] =
			exp[1] =
			exp[2] = 255;
			exp[3] = 255; // texture's alpha
			break;
		case GL_INTENSITY:
			exp[0] =
			exp[1] =
			exp[2] = 255;
			exp[3] = 255; // texture's alpha
			break;
		default:
			abort();
		}
		break;

	case GL_GREEN:
	case GL_BLUE:
		assert(testChan == 0);
		switch (baseIntFormat) {
		case 0:
		case GL_RGBA:
			exp[0] = 0;
			exp[1] = 255;
			exp[2] = 0;
			exp[3] = 255; // texture's alpha
			break;
		case GL_RGB:
			exp[0] = 0;
			exp[1] = 255;
			exp[2] = 0;
			exp[3] = defaultAlpha; // fragment alpha or texture alpha
			break;
		case GL_ALPHA:
			exp[0] = 0;
			exp[1] = 0;
			exp[2] = 0;
			exp[3] = 255; // texture's alpha
			break;
		case GL_LUMINANCE:
			exp[0] =
			exp[1] =
			exp[2] = 0;
			exp[3] = defaultAlpha; // fragment alpha or texture alpha
			break;
		case GL_LUMINANCE_ALPHA:
			exp[0] =
			exp[1] =
			exp[2] = 0;
			exp[3] = 255; // texture's alpha
			break;
		case GL_INTENSITY:
			exp[0] =
			exp[1] =
			exp[2] = 0;
			exp[3] = 0; // texture's alpha
			break;
		default:
			abort();
		}
		break;

	case GL_ALPHA:
		assert(testChan == 0);
		switch (baseIntFormat) {
		case 0:
		case GL_RGBA:
			exp[0] = 0;
			exp[1] = 0;
			exp[2] = 0;
			exp[3] = 255; // texture's alpha
			break;
		case GL_RGB:
			exp[0] = 0;
			exp[1] = 0;
			exp[2] = 0;
			exp[3] = defaultAlpha; // fragment alpha or texture alpha
			break;
		case GL_ALPHA:
			exp[0] = 0;
			exp[1] = 0;
			exp[2] = 0;
			exp[3] = 255; // texture's alpha
			break;
		case GL_LUMINANCE:
			exp[0] =
			exp[1] =
			exp[2] = 0;
			exp[3] = defaultAlpha; // fragment alpha or texture alpha
			break;
		case GL_LUMINANCE_ALPHA:
			exp[0] =
			exp[1] =
			exp[2] = 0;
			exp[3] = 255; // texture's alpha
			break;
		case GL_INTENSITY:
			exp[0] =
			exp[1] =
			exp[2] = 0;
			exp[3] = defaultAlpha; // fragment alpha or texture alpha
			break;
		default:
			abort();
		}
		break;

	case GL_LUMINANCE:
		assert(testChan == 0);
		switch (baseIntFormat) {
		case 0:
		case GL_RGBA:
			exp[0] = 255;
			exp[1] = 255;
			exp[2] = 255;
			exp[3] = 255; // texture's alpha
			break;
		case GL_RGB:
			exp[0] = 255;
			exp[1] = 255;
			exp[2] = 255;
			exp[3] = defaultAlpha; // fragment alpha or texture alpha
			break;
		case GL_RG:
			exp[0] = 255;
			exp[1] = 255;
			exp[2] = 0;
			exp[3] = defaultAlpha; // fragment alpha or texture alpha
			break;
		case GL_RED:
			exp[0] = 255;
			exp[1] = 0;
			exp[2] = 0;
			exp[3] = defaultAlpha; // fragment alpha or texture alpha
			break;
		case GL_ALPHA:
			exp[0] = 0;
			exp[1] = 0;
			exp[2] = 0;
			exp[3] = 255; // texture's alpha
			break;
		case GL_LUMINANCE:
			exp[0] = 255;
			exp[1] = 255;
			exp[2] = 255;
			exp[3] = defaultAlpha; // fragment alpha or texture alpha
			break;
		case GL_LUMINANCE_ALPHA:
			exp[0] = 255;
			exp[1] = 255;
			exp[2] = 255;
			exp[3] = 255; // texture's alpha
			break;
		case GL_INTENSITY:
			exp[0] = 255;
			exp[1] = 255;
			exp[2] = 255;
			exp[3] = 255; // texture's alpha
			break;
		default:
			abort();
		}
		break;

	case GL_LUMINANCE_ALPHA:
		assert(testChan < 2);
		switch (baseIntFormat) {
		case 0:
		case GL_RGBA:
			exp[0] =
			exp[1] =
			exp[2] = testChan == 0 ? 255 : 0;
			exp[3] = testChan == 1 ? 255 : 0;
			break;
		case GL_RGB:
			exp[0] = 
			exp[1] = 
			exp[2] = testChan == 0 ? 255 : 0;
			exp[3] = defaultAlpha; // fragment alpha or texture alpha
			break;
		case GL_RG:
			exp[0] =
			exp[1] = testChan == 0 ? 255 : 0;
			exp[2] = 0;
			exp[3] = defaultAlpha; // fragment alpha or texture alpha
			break;
		case GL_RED:
			exp[0] = testChan == 0 ? 255 : 0;
			exp[1] = 0;
			exp[2] = 0;
			exp[3] = defaultAlpha; // fragment alpha or texture alpha
			break;
		case GL_ALPHA:
			exp[0] =
			exp[1] =
			exp[2] = 0; // fragment color
			exp[3] = testChan == 1 ? 255 : 0;
			break;
		case GL_LUMINANCE:
			exp[0] = 
			exp[1] = 
			exp[2] = testChan == 0 ? 255 : 0;
			exp[3] = defaultAlpha; // fragment alpha or texture alpha
			break;
		case GL_LUMINANCE_ALPHA:
			exp[0] = testChan == 0 ? 255 : 0;
			exp[1] = testChan == 0 ? 255 : 0;
			exp[2] = testChan == 0 ? 255 : 0;
			exp[3] = testChan == 1 ? 255 : 0;
			break;
		case GL_INTENSITY:
			exp[0] =
			exp[1] =
			exp[2] =
			exp[3] = testChan == 0 ? 255 : 0;
			break;
		default:
			abort();
		}
		break;

	default:
		abort();
	}
}


// Read framebuffer and check that region [width x height] is the expected
// solid color, except the upper-right quadrant will always be black/zero.
// comp: which color channel in src image was set (0 = red, 1 = green,
//  2 = blue, 3 = alpha), other channels are zero.
// format is the color format we're testing.
bool
PixelFormatsTest::CheckRendering(int width, int height, int comp,
								 GLenum format, GLint intFormat) const
{
	const int checkAlpha = alphaBits > 0;
	GLubyte *image = new GLubyte [width * height * 4];
	GLboolean ok = 1;
	GLubyte expected[4];
	int i;

	assert(comp >= 0 && comp < 4);

	glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, image);
	for (i = 0; i < width * height; i += 4) {

		ComputeExpected(format, comp, intFormat, expected);
		if (IsUpperRight(i/4, width, height)) {
			expected[0] =
			expected[1] =
			expected[2] =
			expected[3] = 0;
		}

		if (!checkAlpha) {
			expected[3] = 0xff;
		}

		// do the color check
		if (!ColorsEqual(image+i, expected)) {
			// report failure info
			char msg[1000];
			env->log << name;
			sprintf(msg, " failed at pixel (%d,%d), color channel %d:\n",
					i/width, i%width, comp);
			env->log << msg;
			sprintf(msg, "  Expected: 0x%02x 0x%02x 0x%02x 0x%02x\n",
					expected[0], expected[1], expected[2], expected[3]);
			env->log << msg;
			sprintf(msg, "  Found:    0x%02x 0x%02x 0x%02x 0x%02x\n", 
					image[i + 0], image[i + 1], image[i + 2], image[i + 3]);
			env->log << msg;
			ok = false;
			break;
		}
	}
	delete [] image;
	return ok;
}



// Exercise a particular combination of image format, type and internal
// texture format.
// Return true for success, false for failure.
bool
PixelFormatsTest::TestCombination(GLenum format, GLenum type, GLint intFormat)
{
	const int numComps = NumberOfComponentsInFormat(format);
	const int width = 16;
	const int height = 16;
	int colorPos[4];
	ComponentPositions(format, colorPos);

	for (int comp = 0; comp < numComps; comp++) {
		if (colorPos[comp] >= 0) {
			// make original/incoming image
			const int comp2 = colorPos[comp];
			GLubyte *image = MakeImage(width, height, format, type, comp2);

			// render with image (texture / glDrawPixels)
			bool ok = DrawImage(width, height, format, type, intFormat, image);

			if (ok) {
				// check rendering
				ok = CheckRendering(width, height, comp, format, intFormat);
			}

			delete [] image;

			if (!ok) {
				return false;
			}
		}
	}

	return true;
}


// Per visual setup.
void
PixelFormatsTest::setup(void)
{
	haveHalfFloat = GLUtils::haveExtensions("GL_ARB_half_float_pixel");
	haveABGR = GLUtils::haveExtensions("GL_EXT_abgr");
	haveSRGB = GLUtils::haveExtensions("GL_EXT_texture_sRGB");
	haveCombine = GLUtils::haveExtensions("GL_ARB_texture_env_combine");
	haveRG = GLUtils::haveExtensions("GL_ARB_texture_rg");
	haveFloat = GLUtils::haveExtensions("GL_ARB_texture_float");
	haveSnorm = GLUtils::haveExtensions("GL_EXT_texture_snorm");
	haveTexSharedExp = GLUtils::haveExtensions("GL_EXT_texture_shared_exponent");

	glGetIntegerv(GL_ALPHA_BITS, &alphaBits);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glDrawBuffer(GL_FRONT);
	glReadBuffer(GL_FRONT);

	glColor4f(0, 0, 0, 0);

#if USE_FRAG_PROG
	{
		PFNGLPROGRAMSTRINGARBPROC glProgramStringARB_func;
		PFNGLBINDPROGRAMARBPROC glBindProgramARB_func;
		static const char *progText =
			"!!ARBfp1.0\n"
			"TEX result.color, fragment.texcoord[0], texture[0], 2D; \n"
			"END \n"
			;
		glProgramStringARB_func = (PFNGLPROGRAMSTRINGARBPROC)
			GLUtils::getProcAddress("glProgramStringARB");
		assert(glProgramStringARB_func);
		glBindProgramARB_func = (PFNGLBINDPROGRAMARBPROC)
			GLUtils::getProcAddress("glBindProgramARB");
		assert(glBindProgramARB_func);
		glBindProgramARB_func(GL_FRAGMENT_PROGRAM_ARB, 1);
		glProgramStringARB_func(GL_FRAGMENT_PROGRAM_ARB,
								GL_PROGRAM_FORMAT_ASCII_ARB,
								strlen(progText), (const GLubyte *) progText);
		if (glGetError()) {
			fprintf(stderr, "Bad fragment program, error: %s\n",
					(const char *) glGetString(GL_PROGRAM_ERROR_STRING_ARB));
			exit(0);
		}
	}
#endif
}



// Test all possible image formats, types and internal texture formats.
// Result will indicate number of passes and failures.
void
PixelFormatsTest::runOne(MultiTestResult &r, Window &w)
{
	int testNum = 0, testStride;
	(void) w;  // silence warning

	setup();

	if (env->options.quick)
		testStride = 13;  // a prime number
	else
		testStride = 1;

	const unsigned numEnvModes = haveCombine ? 2 : 1;

	for (unsigned envMode = 0; envMode < numEnvModes; envMode++) {
		if (envMode == 0) {
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
			// When the texture internal format is GL_LUMINANCE or GL_RGB,
			// GL_REPLACE takes alpha from the fragment, which we set to zero
			// with glColor4f(0,0,0,0).
#if USE_FRAG_PROG
			defaultAlpha = 255;
#else
			defaultAlpha = 0;
#endif
		}
		else {
			assert(haveCombine);
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_REPLACE);
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_ARB, GL_REPLACE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_TEXTURE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_ARB, GL_TEXTURE);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_ARB, GL_SRC_COLOR);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_ARB, GL_SRC_ALPHA);
			// For this GL_COMBINE mode, when sampling a texture that does
			// not have an alpha channel, alpha is effectively 1.0.
			defaultAlpha = 255;
		}

		for (unsigned formatIndex = 0; formatIndex < NUM_FORMATS; formatIndex++) {
			for (unsigned typeIndex = 0; typeIndex < NUM_TYPES; typeIndex++) {

				if (CompatibleFormatAndType(Formats[formatIndex].Token,
											Types[typeIndex].Token)) {

					for (unsigned intFormat = 0; intFormat < NUM_INT_FORMATS; intFormat++) {

						if (!SupportedIntFormat(InternalFormats[intFormat].Token))
							continue;

#if DEBUG
						env->log << "testing "
								 << testNum
								 << ":\n";
						env->log << "  Format:    " << Formats[formatIndex].Name << "\n";
						env->log << "  Type:      " << Types[typeIndex].Name << "\n";
						env->log << "  IntFormat: " << InternalFormats[intFormat].Name << "\n";

#endif
						bool ok;

						if (testNum % testStride == 0) {
							ok = TestCombination(Formats[formatIndex].Token,
												 Types[typeIndex].Token,
												 InternalFormats[intFormat].Token);
						}
						else {
							// skip
							ok = true;
						}

						if (!ok) {
							// error was reported to log, add format info here:
							env->log << "  Format: " << Formats[formatIndex].Name << "\n";
							env->log << "  Type: " << Types[typeIndex].Name << "\n";
							env->log << "  Internal Format: " << InternalFormats[intFormat].Name << "\n";
							env->log << "  EnvMode: " << EnvModes[envMode] << "\n";
							r.numFailed++;
						}
						else {
							r.numPassed++;
						}
						testNum++;
					}
				}
			}
		}
	}

	r.pass = (r.numFailed == 0);
}


// The test object itself:
PixelFormatsTest pixelFormatsTest("pixelFormats", "window, rgb",
				"",
	"Test that all the various pixel formats/types (like\n"
	"GL_BGRA/GL_UNSIGNED_SHORT_4_4_4_4_REV) operate correctly.\n"
	"Test both glTexImage and glDrawPixels.\n"
	"For textures, also test all the various internal texture formats.\n"
	"Thousands of combinations are possible!\n"
	);


} // namespace GLEAN
