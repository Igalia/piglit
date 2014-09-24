/*
 * Copyright 2012 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

/*
 * file draw-pixels.c
 *
 * Test to verify functionality of glDrawPixels() with various pixel formats
 * and data types
 *
 * author: Anuj Phogat
 */

#include "piglit-util-gl.h"

/* Data conversions as used in mesa */
/** Convert GLubyte in [0,255] to GLfloat in [0.0,1.0] */
#define UBYTE_TO_FLOAT(u) ((float) u / 255.0F)

/** Convert GLbyte in [-128,127] to GLfloat in [-1.0,1.0] */
#define BYTE_TO_FLOAT(B)    ((2.0F * (B) + 1.0F) * (1.0F/255.0F))

/** Convert GLushort in [0,65535] to GLfloat in [0.0,1.0] */
#define USHORT_TO_FLOAT(S)  ((GLfloat) (S) * (1.0F / 65535.0F))

/** Convert GLshort in [-32768,32767] to GLfloat in [-1.0,1.0] */
#define SHORT_TO_FLOAT(S)   ((2.0F * (S) + 1.0F) * (1.0F/65535.0F))

/** Convert GLuint in [0,4294967295] to GLfloat in [0.0,1.0] */
#define UINT_TO_FLOAT(U)    ((GLfloat) ((U) * (1.0F / 4294967295.0)))

/** Convert GLint in [-2147483648,2147483647] to GLfloat in [-1.0,1.0] */
#define INT_TO_FLOAT(I)     ((GLfloat) ((2.0F * (I) + 1.0F) * (1.0F/4294967294.0)))

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DEPTH | PIGLIT_GL_VISUAL_STENCIL;

PIGLIT_GL_TEST_CONFIG_END

const GLuint idx0 = 0, idx1 = 1, idx2 = 2, idx3 = 3;
static GLfloat expected[100][4];

/*As per OpenGL 3.0 specification integer formats are not allowed in
 * glDrawPixels
 */
static GLenum pixel_formats[] = {
	GL_RED,
	GL_GREEN,
	GL_BLUE,
	GL_ALPHA,
	GL_RG,
	GL_RGB,
	GL_BGR,
	GL_RGBA,
	GL_BGRA,
	GL_LUMINANCE,
	GL_LUMINANCE_ALPHA,
	GL_DEPTH_COMPONENT,
	GL_STENCIL_INDEX };

static GLenum data_types[] = {
	GL_BYTE,
	GL_UNSIGNED_BYTE,
	GL_SHORT,
	GL_UNSIGNED_SHORT,
	GL_INT,
	GL_UNSIGNED_INT,
	GL_FLOAT,
	GL_UNSIGNED_BYTE_3_3_2,
	GL_UNSIGNED_BYTE_2_3_3_REV,
	GL_UNSIGNED_SHORT_5_6_5,
	GL_UNSIGNED_SHORT_5_6_5_REV,
	GL_UNSIGNED_SHORT_4_4_4_4,
	GL_UNSIGNED_SHORT_4_4_4_4_REV,
	GL_UNSIGNED_SHORT_5_5_5_1,
	GL_UNSIGNED_SHORT_1_5_5_5_REV,
	GL_UNSIGNED_INT_8_8_8_8,
	GL_UNSIGNED_INT_8_8_8_8_REV,
	GL_UNSIGNED_INT_10_10_10_2,
	GL_UNSIGNED_INT_2_10_10_10_REV };

typedef struct p_ops {
	GLenum pname;
	GLint param;
} p_ops;

static const p_ops pixel_ops[] = {
	{ GL_UNPACK_SWAP_BYTES,		0 },
	{ GL_UNPACK_SWAP_BYTES,		1 } };


void Swap2Byte(void *value)
{
	GLubyte *bytes = (GLubyte *) value;
	GLubyte tmp = bytes[0];
	bytes[0] = bytes[1];
	bytes[1] = tmp;
}

void Swap4Byte(void *value)
{
	GLubyte *bytes = (GLubyte *) value;
	GLubyte tmp = bytes[0];
	bytes[0] = bytes[3];
	bytes[3] = tmp;
	tmp = bytes[1];
	bytes[1] = bytes[2];
	bytes[2] = tmp;
}

bool is_format_type_mismatch(GLenum format, GLenum type)
{
	if (((type == GL_UNSIGNED_BYTE_3_3_2 ||
	      type == GL_UNSIGNED_BYTE_2_3_3_REV ||
	      type == GL_UNSIGNED_SHORT_5_6_5 ||
	      type == GL_UNSIGNED_SHORT_5_6_5_REV) &&
	     (format != GL_RGB)) ||

	    ((type == GL_UNSIGNED_SHORT_4_4_4_4 ||
	      type == GL_UNSIGNED_SHORT_4_4_4_4_REV ||
	      type == GL_UNSIGNED_SHORT_5_5_5_1 ||
	      type == GL_UNSIGNED_SHORT_1_5_5_5_REV ||
	      type == GL_UNSIGNED_INT_8_8_8_8 ||
	      type == GL_UNSIGNED_INT_8_8_8_8_REV ||
	      type == GL_UNSIGNED_INT_10_10_10_2 ||
	      type == GL_UNSIGNED_INT_2_10_10_10_REV) &&
	     (format != GL_RGBA &&
	      format != GL_BGRA)))
		return true;

	return false;
}

static void *
allocPixels(GLenum format, GLenum type, GLuint components)
{
	GLint i, j;
	GLvoid *pixels;
	GLuint npixels = piglit_width * piglit_height;

	switch(type) {
	case GL_BYTE:
		pixels = calloc(npixels * components, sizeof(GLbyte));
		for (i = 0; i < npixels; i++) {
			for (j = 0; j < components; j++)
				((GLbyte *)pixels)[i * components + j] = 50 + j * 4;
		}
		break;

	case GL_UNSIGNED_BYTE:
		pixels = calloc(npixels * components, sizeof(GLubyte));
		for (i = 0; i < npixels; i++) {
			for (j = 0; j < components; j++)
				((GLubyte *)pixels)[i * components + j] = 100 + j * 4;
		}
		break;

	case GL_UNSIGNED_BYTE_3_3_2:
	case GL_UNSIGNED_BYTE_2_3_3_REV:
		pixels = calloc(npixels, sizeof(GLubyte));
		for (i = 0; i < npixels; i++) {
			((GLubyte *)pixels)[i] = 0x99;
		}
		break;

	case GL_SHORT:
		pixels = calloc(npixels * components, sizeof(GLshort));
		for (i = 0; i < npixels; i++) {
			for (j = 0; j < components; j++) {
				((GLshort *)pixels)[i * components + j] = 0x1234;
			}
		}
		break;

	case GL_UNSIGNED_SHORT:
		pixels = calloc(npixels * components, sizeof(GLushort));
		for (i = 0; i < npixels; i++) {
			for (j = 0; j < components; j++) {
				((GLushort *)pixels)[i * components + j] = 0x4321;
			}
		}
		break;

	case GL_UNSIGNED_SHORT_5_6_5:
	case GL_UNSIGNED_SHORT_5_6_5_REV:
	case GL_UNSIGNED_SHORT_4_4_4_4:
	case GL_UNSIGNED_SHORT_4_4_4_4_REV:
	case GL_UNSIGNED_SHORT_5_5_5_1:
	case GL_UNSIGNED_SHORT_1_5_5_5_REV:
		pixels = calloc(npixels, sizeof(GLushort));
		for (i = 0; i < npixels; i++)
			((GLushort *)pixels)[i] = 0x9b59;
		break;

	case GL_INT:
		pixels = calloc(npixels * components, sizeof(GLint));
		for (i = 0; i < npixels; i++) {
			for (j = 0; j < components; j++) {
				((GLint *)pixels)[i * components + j] = 0x12345678;
			}
		}
		break;

	case GL_UNSIGNED_INT:
		pixels = calloc(npixels * components, sizeof(GLuint));
		for (i = 0; i < npixels; i++) {
			for (j = 0; j < components; j++) {
				((GLuint *)pixels)[i * components + j] = 0x87654321;
			}
		}
		break;

	case GL_UNSIGNED_INT_8_8_8_8:
	case GL_UNSIGNED_INT_8_8_8_8_REV:
	case GL_UNSIGNED_INT_10_10_10_2:
	case GL_UNSIGNED_INT_2_10_10_10_REV:
		pixels = calloc(npixels, sizeof(GLuint));
		for (i = 0; i < npixels; i++)
			((GLuint *)pixels)[i] = 0x1a4b5a4b;
		break;

	case GL_FLOAT:
		pixels = calloc(npixels * components, sizeof(GLfloat));
		for (i = 0; i < npixels; i++) {
			for (j = 0; j < components; j++) {
				if (format == GL_STENCIL_INDEX)
					((GLfloat *)pixels)[i * components + j] =
					0x1020;
				else
					((GLfloat *)pixels)[i * components + j] =
					0.5 - j * 0.1;
			}
		}
		break;

	default:
		assert(!"Unexpected data type");
		pixels = NULL;
		break;
	}
	return pixels;
}

static void *
pixelsInit(GLenum format, GLenum type)
{
	switch(format) {
	case GL_RED:
	case GL_GREEN:
	case GL_BLUE:
	case GL_ALPHA:
	case GL_LUMINANCE:
	case GL_DEPTH_COMPONENT:
	case GL_STENCIL_INDEX:
		return (allocPixels(format, type, 1));
	case GL_LUMINANCE_ALPHA:
	case GL_RG:
		return (allocPixels(format, type, 2));
	case GL_RGB:
	case GL_BGR:
		return (allocPixels(format, type, 3));
	case GL_RGBA:
	case GL_BGRA:
		return (allocPixels(format, type, 4));
	default:
		printf("format = %s not allowed in glDrawPixels()\n",
		       piglit_get_gl_enum_name(format));
		piglit_report_result(PIGLIT_FAIL);
	}
	return NULL;
}

static float
typeToFloat(GLenum format, GLenum type, GLvoid *src,
	    GLuint index, p_ops pixelops)
{
	/* Scale factors */
	GLuint pi, pui, mask; GLushort pus; GLshort ps;
	GLfloat pf; GLint stencil_bits; GLbyte pb; GLubyte pub;
	const GLuint *uisrc; const GLushort *ussrc; const GLubyte *ubsrc;
	GLfloat rs = 1.0f, gs = 1.0f, bs = 1.0f, as = 1.0f;

	GLboolean swap = (pixelops.pname == GL_UNPACK_SWAP_BYTES) ?
			 pixelops.param : false;

	if (format == GL_STENCIL_INDEX) {

		glGetIntegerv(GL_STENCIL_BITS, &stencil_bits);
		/* Clamp the return value to the size of stencil buffer */
		mask = 0xffffffff >> (sizeof(GLuint) *  8 - stencil_bits);

		switch(type) {
		case GL_BYTE:
			pb = ((GLbyte *)src)[index];
			return pb & mask;
		case GL_UNSIGNED_BYTE:
			pub = ((GLubyte *)src)[index];
			return pub & mask;
		case GL_SHORT:
			ps = ((GLshort *)src)[index];
			if (swap)
				Swap2Byte(&ps);
			return ps & mask;
		case GL_UNSIGNED_SHORT:
			pus = ((GLushort *)src)[index];
			if (swap)
				Swap2Byte(&pus);
			return pus & mask;
		case GL_INT:
			pi = ((GLint *)src)[index];
			if (swap)
				Swap4Byte(&pi);
			return pi & mask;
		case GL_UNSIGNED_INT:
			pui = ((GLuint *)src)[index];
			if (swap)
				Swap4Byte(&pui);
			return pui & mask;
		case GL_FLOAT:
			pf = ((GLfloat *)src)[index];
			if (swap)
				Swap4Byte(&pf);
			return (GLfloat)((GLuint)pf & mask);
		default:
			printf("type = %s not allowed in glDrawPixels()\n",
			       piglit_get_gl_enum_name(type));
			piglit_report_result(PIGLIT_FAIL);
		}
	}
	else {
		switch(type) {
		case GL_BYTE:
			return BYTE_TO_FLOAT(((GLbyte *)src)[index]);

		case GL_UNSIGNED_BYTE:
			return UBYTE_TO_FLOAT(((GLubyte *)src)[index]);

		case GL_UNSIGNED_BYTE_3_3_2:
			ubsrc = (const GLubyte *) src;
			rs = 1.0F / 7.0F;
			gs = 1.0F / 7.0F;
			bs = 1.0F / 3.0F;
			pub = ubsrc[index];
			if (index == idx0)
				return (((pub >> 5)      ) * rs);
			else if (index == idx1)
				return (((pub >> 2) & 0x7) * gs);
			else if (index == idx2)
				return (((pub     ) & 0x3) * bs);
			else
				return 1.0F;

		case GL_UNSIGNED_BYTE_2_3_3_REV:
			ubsrc = (const GLubyte *) src;
			rs = 1.0F / 7.0F;
			gs = 1.0F / 7.0F;
			bs = 1.0F / 3.0F;
			pub = ubsrc[index];
			if (index == idx0)
				return (((pub     ) & 0x7) * rs);
			else if (index == idx1)
				return (((pub >> 3) & 0x7) * gs);
			else if (index == idx2)
				return (((pub >> 6)      ) * bs);
			else
				return 1.0F;

		case GL_SHORT:
			ps = ((GLshort *)src)[index];
			if (swap)
				Swap2Byte(&ps);
			return (SHORT_TO_FLOAT(ps));

		case GL_UNSIGNED_SHORT:
			pus = ((GLushort *)src)[index];
			if (swap)
				Swap2Byte(&pus);
			return (USHORT_TO_FLOAT(pus));

		case GL_UNSIGNED_SHORT_5_6_5:
			ussrc = (const GLushort *) src;
			rs = 1.0F / 31.0F;
			gs = 1.0F / 63.0F;
			bs = 1.0F / 31.0F;
			pus = ussrc[index];
			if (swap)
				Swap2Byte(&pus);
			if (index == idx0)
				return (((pus >> 11)       ) * rs);
			else if (index == idx1)
				return (((pus >>  5) & 0x3f) * gs);
			else if (index == idx2)
				return (((pus      ) & 0x1f) * bs);
			else
				return 1.0F;

		case GL_UNSIGNED_SHORT_5_6_5_REV:
			ussrc = (const GLushort *) src;
			rs = 1.0F / 31.0F;
			gs = 1.0F / 63.0F;
			bs = 1.0F / 31.0F;
			pus = ussrc[index];
			if (swap)
				Swap2Byte(&pus);
			if (index == idx0)
				return (((pus      ) & 0x1f) * rs);
			else if (index == idx1)
				return (((pus >>  5) & 0x3f) * gs);
			else if (index == idx2)
				return (((pus >> 11)       ) * bs);
			else
				return 1.0F;

		case GL_UNSIGNED_SHORT_4_4_4_4:
			ussrc = (const GLushort *) src;
			rs = gs = bs = as = 1.0F / 15.0F;
			pus = ussrc[index];
			if (swap)
				Swap2Byte(&pus);
			if (index == idx0)
				return (((pus >> 12)      ) * rs);
			else if (index == idx1)
				return (((pus >> 8) & 0xf ) * gs);
			else if (index == idx2)
				return (((pus >> 4) & 0xf ) * bs);
			else
				return (((pus     ) & 0xf ) * as);

		case GL_UNSIGNED_SHORT_4_4_4_4_REV:
			ussrc = (const GLushort *) src;
			rs = gs = bs = as = 1.0F / 15.0F;
			pus = ussrc[index];
			if (swap)
				Swap2Byte(&pus);
			if (index == idx0)
				return (((pus     ) & 0xf ) * rs);
			else if (index == idx1)
				return (((pus >> 4) & 0xf ) * gs);
			else if (index == idx2)
				return (((pus >> 8) & 0xf ) * bs);
			else
				return (((pus >> 12)      ) * as);

		case GL_UNSIGNED_SHORT_5_5_5_1:
			ussrc = (const GLushort *) src;
			rs = gs = bs = 1.0F / 31.0F;
			pus = ussrc[index];
			if (swap)
				Swap2Byte(&pus);
			if (index == idx0)
				return (((pus >> 11)       ) * rs);
			else if (index == idx1)
				return (((pus >>  6) & 0x1f) * gs);
			else if (index == idx2)
				return (((pus >>  1) & 0x1f) * bs);
			else
				return (((pus      ) & 0x1)  * as);

		case GL_UNSIGNED_SHORT_1_5_5_5_REV:
			ussrc = (const GLushort *) src;
			rs = gs = bs = 1.0F / 31.0F;
			pus = ussrc[index];
			if (swap)
				Swap2Byte(&pus);
			if (index == idx0)
				return (((pus      ) & 0x1f) * rs);
			else if (index == idx1)
				return (((pus >>  5) & 0x1f) * gs);
			else if (index == idx2)
				return (((pus >> 10) & 0x1f) * bs);
			else
				return (((pus >> 15)       ) * as);

		case GL_INT:
			pi = ((GLint *)src)[index];
			if (swap)
				Swap4Byte(&pi);
			return INT_TO_FLOAT(pi);

		case GL_UNSIGNED_INT:
			pui = ((GLuint *)src)[index];
			if (swap)
				Swap4Byte(&pui);
			return UINT_TO_FLOAT(pui);

		case GL_UNSIGNED_INT_8_8_8_8:
			uisrc = (const GLuint *) src;
			pui = uisrc[index];
			if (swap)
				Swap4Byte(&pui);
			if (index == idx0)
				return UBYTE_TO_FLOAT(((pui >> 24)       ));
			else if (index == idx1)
				return UBYTE_TO_FLOAT(((pui >> 16) & 0xff));
			else if (index == idx2)
				return UBYTE_TO_FLOAT(((pui >>  8) & 0xff));
			else
				return UBYTE_TO_FLOAT(((pui      ) & 0xff));

		case GL_UNSIGNED_INT_8_8_8_8_REV:
			uisrc = (const GLuint *) src;
			pui = uisrc[index];
			if (swap)
				Swap4Byte(&pui);
			if (index == idx0)
				return UBYTE_TO_FLOAT(((pui      ) & 0xff));
			else if (index == idx1)
				return UBYTE_TO_FLOAT(((pui >>  8) & 0xff));
			else if (index == idx2)
				return UBYTE_TO_FLOAT(((pui >> 16) & 0xff));
			else
				return UBYTE_TO_FLOAT(((pui >> 24)       ));

		case GL_UNSIGNED_INT_10_10_10_2:
			uisrc = (const GLuint *) src;
			pui = uisrc[index];
			rs = 1.0F / 1023.0F;
			gs = 1.0F / 1023.0F;
			bs = 1.0F / 1023.0F;
			as = 1.0F / 3.0F;
			if (swap)
				Swap4Byte(&pui);
			if (index == idx0)
				return (((pui >> 22)        ) * rs);
			else if (index == idx1)
				return (((pui >> 12) & 0x3ff) * gs);
			else if (index == idx2)
				return (((pui >>  2) & 0x3ff) * bs);
			else
				return (((pui      ) & 0x3  ) * as);

		case GL_UNSIGNED_INT_2_10_10_10_REV:
			uisrc = (const GLuint *) src;
			pui = uisrc[index];
			rs = 1.0F / 1023.0F;
			gs = 1.0F / 1023.0F;
			bs = 1.0F / 1023.0F;
			as = 1.0F / 3.0F;
			if (swap)
				Swap4Byte(&pui);
			if (index == idx0)
				return (((pui      ) & 0x3ff) * rs);
			else if (index == idx1)
				return (((pui >> 10) & 0x3ff) * gs);
			else if (index == idx2)
				return (((pui >> 20) & 0x3ff) * bs);
			else
				return (((pui >> 30)        ) * as);

		case GL_FLOAT:
			pf = ((GLfloat *)src)[index];
			if (swap)
				Swap4Byte(&pf);
			return pf;
		default:
			printf("type = %s not supported in glDrawPixels()\n",
			       piglit_get_gl_enum_name(format));
			piglit_report_result(PIGLIT_FAIL);
		}
	}
	return 0.0F;
}

static float
clampColor(float f)
{
	return ((f > 1.0f) ? 1.0f : ((f < 0.0f ? 0.0f : f)));
}

static void
computeExpected(GLenum format, GLenum type, GLuint index,
		p_ops pixelops, GLvoid *pixels)
{
	int j = index;
	GLvoid * src = pixels;
	GLfloat fval;

	switch(format) {
	case GL_RED:
		fval = typeToFloat(format, type, src, idx0, pixelops);
		expected[j][0] = clampColor(fval);
		expected[j][1] = 0.0;
		expected[j][2] = 0.0;
		expected[j][3] = 1.0;
		break;
	case GL_GREEN:
		fval = typeToFloat(format, type, src, idx0, pixelops);
		expected[j][0] = 0.0;
		expected[j][1] = clampColor(fval);
		expected[j][2] = 0.0;
		expected[j][3] = 1.0;
		break;
	case GL_BLUE:
		fval = typeToFloat(format, type, src, idx0, pixelops);
		expected[j][0] = 0.0;
		expected[j][1] = 0.0;
		expected[j][2] = clampColor(fval);
		expected[j][3] = 1.0;
		break;

	case GL_ALPHA:
		fval = typeToFloat(format, type, src, idx0, pixelops);
		expected[j][0] = 0.0;
		expected[j][1] = 0.0;
		expected[j][2] = 0.0;
		expected[j][3] = clampColor(fval);
		break;

	case GL_LUMINANCE:
		fval = typeToFloat(format, type, src, idx0, pixelops);
		expected[j][0] = clampColor(fval);
		expected[j][1] = clampColor(fval);
		expected[j][2] = clampColor(fval);
		expected[j][3] = 1.0;
		break;

	case GL_LUMINANCE_ALPHA:
		fval = typeToFloat(format, type, src, idx0, pixelops);
		expected[j][0] = clampColor(fval);
		expected[j][1] = clampColor(fval);
		expected[j][2] = clampColor(fval);
		fval = typeToFloat(format, type, src, idx1, pixelops);
		expected[j][3] = clampColor(fval);
		break;

	case GL_RG:
		fval = typeToFloat(format, type, src, idx0, pixelops);
		expected[j][0] = clampColor(fval);
		fval = typeToFloat(format, type, src, idx1, pixelops);
		expected[j][1] = clampColor(fval);
		expected[j][2] = 0.0;
		expected[j][3] = 1.0;
		break;

	case GL_RGB:
		fval = typeToFloat(format, type, src, idx0, pixelops);
		expected[j][0] = clampColor(fval);
		fval = typeToFloat(format, type, src, idx1, pixelops);
		expected[j][1] = clampColor(fval);
		fval = typeToFloat(format, type, src, idx2, pixelops);
		expected[j][2] = clampColor(fval);
		expected[j][3] = 1.0;
		break;

	case GL_BGR:
		fval = typeToFloat(format, type, src, idx2, pixelops);
		expected[j][0] = clampColor(fval);
		fval = typeToFloat(format, type, src, idx1, pixelops);
		expected[j][1] = clampColor(fval);
		fval = typeToFloat(format, type, src, idx0, pixelops);
		expected[j][2] = clampColor(fval);
		expected[j][3] = 1.0;
		break;

	case GL_RGBA:
		fval = typeToFloat(format, type, src, idx0, pixelops);
		expected[j][0] = clampColor(fval);
		fval = typeToFloat(format, type, src, idx1, pixelops);
		expected[j][1] = clampColor(fval);
		fval = typeToFloat(format, type, src, idx2, pixelops);
		expected[j][2] = clampColor(fval);
		fval = typeToFloat(format, type, src, idx3, pixelops);
		expected[j][3] = clampColor(fval);
		break;

	case GL_BGRA:
		fval = typeToFloat(format, type, src, idx2, pixelops);
		expected[j][0] = clampColor(fval);
		fval = typeToFloat(format, type, src, idx1, pixelops);
		expected[j][1] = clampColor(fval);
		fval = typeToFloat(format, type, src, idx0, pixelops);
		expected[j][2] = clampColor(fval);
		fval = typeToFloat(format, type, src, idx3, pixelops);
		expected[j][3] = clampColor(fval);
		break;

	case GL_DEPTH_COMPONENT:
		fval = typeToFloat(format, type, src, idx0, pixelops);
		expected[j][0] = clampColor(fval);
		break;

	case GL_STENCIL_INDEX:
		fval = typeToFloat(format, type, src, idx0, pixelops);
		expected[j][0] = fval;
		break;
	}
}

static void
report_failure(GLenum format, GLenum type)
{
	printf("  Failed with format %s, type %s\n",
	       piglit_get_gl_enum_name(format),
	       piglit_get_gl_enum_name(type));
}

enum piglit_result
piglit_display(void)
{
	int i, j, k;
	GLenum format, type;
	GLvoid *pixels = NULL;
	bool pass = true, p;
	GLfloat black[4] = {0.0, 0.0, 0.0, 1.0};
	GLfloat red[4] = {1.0, 0.0, 0.0, 1.0};

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	for (i = 0; i < ARRAY_SIZE(data_types); i++) {
		for (k = 0; k < ARRAY_SIZE(pixel_ops); k++) {
			for (j = 0; j < ARRAY_SIZE(pixel_formats); j++) {

				format = pixel_formats[j];
				type = data_types[i];

				if (is_format_type_mismatch(format, type)) {
					glDrawPixels(piglit_width, piglit_height,
						     format, type, pixels);
					/* Here GL_INVALID_OPERATION is an
					 * expected GL error
					 */
					pass = piglit_check_gl_error(
					       GL_INVALID_OPERATION)
					       && pass;
					continue;
				}

				if (type == GL_UNSIGNED_BYTE_3_3_2 ||
				    type == GL_UNSIGNED_BYTE_2_3_3_REV)
					piglit_set_tolerance_for_bits(7, 7, 7, 7);
				else
					piglit_set_tolerance_for_bits(8, 8, 8, 8);

				if (!piglit_automatic)
					printf("Format = %s, Type = %s,"
					       " Swap Bytes = %d\n",
					       piglit_get_gl_enum_name(format),
					       piglit_get_gl_enum_name(type),
					       pixel_ops[k].param);

				pixels = pixelsInit(format, type);
				computeExpected(format, type, j, pixel_ops[k], pixels);

				glClear(GL_COLOR_BUFFER_BIT);
				/* Enable/Disable byte swap while unpacking pixels */
				glPixelStorei(pixel_ops[k].pname, pixel_ops[k].param);

				switch(format) {

				case GL_RG:
					if (!piglit_is_extension_supported(
					   "GL_ARB_texture_rg")) {
						   if (!piglit_automatic)
							printf("GL_RG skipped\n");
						continue;
					}

				case GL_RED:
				case GL_GREEN:
				case GL_BLUE:
				case GL_ALPHA:
				case GL_LUMINANCE:
				case GL_LUMINANCE_ALPHA:
				case GL_RGB:
				case GL_BGR:
				case GL_RGBA:
				case GL_BGRA:
					glDrawPixels(piglit_width, piglit_height,
						     format, type, pixels);

					pass = piglit_check_gl_error(GL_NO_ERROR)
					       && pass;
					p = piglit_probe_rect_rgba(0, 0,
					       piglit_width, piglit_height,
					       expected[j]);
					if (!p) {
						report_failure(format, type);
						pass = GL_FALSE;
					}
					break;

				case GL_DEPTH_COMPONENT:
					glEnable(GL_DEPTH_TEST);
					glClearDepth(0.0);
					glDepthFunc(GL_ALWAYS);
					glClear(GL_DEPTH_BUFFER_BIT);
					glDrawPixels(piglit_width, piglit_height,
						     format, type, pixels);

					pass = piglit_check_gl_error(GL_NO_ERROR)
					       && pass;
					p = piglit_probe_rect_depth(0, 0,
					       piglit_width, piglit_height,
					       expected[j][0]);
					if (!p) {
						report_failure(format, type);
						pass = GL_FALSE;
					}
					glDisable(GL_DEPTH_TEST);
					break;

				case GL_STENCIL_INDEX:
					glClearStencil(0.0);
					glClear(GL_STENCIL_BUFFER_BIT);
					glDrawPixels(piglit_width, piglit_height,
						     format, type, pixels);

					pass = piglit_check_gl_error(GL_NO_ERROR)
					       && pass;
					/* Probe stencil buffer */
					p = piglit_probe_rect_stencil(0, 0,
								piglit_width,
								piglit_height,
								expected[j][0]);
					if (!p) {
						report_failure(format, type);
						pass = GL_FALSE;
					}

					glEnable(GL_STENCIL_TEST);
					glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
					glStencilFunc(GL_EQUAL, 1, ~0);
					glColor4f(1.0, 0.0, 0.0, 1.0);
					piglit_draw_rect(0, 0, piglit_width,
							 piglit_height);

					/* Probe color buffer. Color buffer will
					 * stay unaffected by piglit_draw_rect()
					 */
					p = piglit_probe_rect_rgba(0, 0,
					       piglit_width, piglit_height,
					       black);
					if (!p) {
						report_failure(format, type);
						pass = GL_FALSE;
					}

					glStencilFunc(GL_EQUAL, expected[j][0], ~0);
					piglit_draw_rect(0, 0, piglit_width,
							 piglit_height);
					p = piglit_probe_rect_rgba(0, 0,
					       piglit_width, piglit_height,
					       red);
					if (!p) {
						report_failure(format, type);
						pass = GL_FALSE;
					}

					glDisable(GL_STENCIL_TEST);
					break;
				}
				free(pixels);
				pixels = NULL;

				if (!pass) {
					piglit_present_results();
				}
			}
		}
	}
	return (pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

void
piglit_init(int argc, char **argv)
{
	glClearColor(0.0, 0.0, 0.0, 1.0);
	piglit_ortho_projection(piglit_width, piglit_height, GL_TRUE);
}
