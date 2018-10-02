/*
 * Copyright © 2018 Intel Corporation
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

static const struct astc_fmt {
	/** The GLenum for the ASTC format. */
	GLenum fmt;

	/** The block width. */
	int bw;

	/** The block height. */
	int bh;

	/** The number of bytes per block. */
	int bb;
} formats[] = {
	{GL_COMPRESSED_RGBA_ASTC_4x4_KHR          ,  4,  4, 16},
	{GL_COMPRESSED_RGBA_ASTC_5x4_KHR          ,  5,  4, 16},
	{GL_COMPRESSED_RGBA_ASTC_5x5_KHR          ,  5,  5, 16},
	{GL_COMPRESSED_RGBA_ASTC_6x5_KHR          ,  6,  5, 16},
	{GL_COMPRESSED_RGBA_ASTC_6x6_KHR          ,  6,  6, 16},
	{GL_COMPRESSED_RGBA_ASTC_8x5_KHR          ,  8,  5, 16},
	{GL_COMPRESSED_RGBA_ASTC_8x6_KHR          ,  8,  6, 16},
	{GL_COMPRESSED_RGBA_ASTC_8x8_KHR          ,  8,  8, 16},
	{GL_COMPRESSED_RGBA_ASTC_10x5_KHR         , 10,  5, 16},
	{GL_COMPRESSED_RGBA_ASTC_10x6_KHR         , 10,  6, 16},
	{GL_COMPRESSED_RGBA_ASTC_10x8_KHR         , 10,  8, 16},
	{GL_COMPRESSED_RGBA_ASTC_10x10_KHR        , 10, 10, 16},
	{GL_COMPRESSED_RGBA_ASTC_12x10_KHR        , 12, 10, 16},
	{GL_COMPRESSED_RGBA_ASTC_12x12_KHR        , 12, 12, 16},
	{GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR  ,  4,  4, 16},
	{GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR  ,  5,  4, 16},
	{GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR  ,  5,  5, 16},
	{GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR  ,  6,  5, 16},
	{GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR  ,  6,  6, 16},
	{GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR  ,  8,  5, 16},
	{GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR  ,  8,  6, 16},
	{GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR  ,  8,  8, 16},
	{GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR , 10,  5, 16},
	{GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR , 10,  6, 16},
	{GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR , 10,  8, 16},
	{GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR, 10, 10, 16},
	{GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR, 12, 10, 16},
	{GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR, 12, 12, 16}
};
