/*
 * Copyright © 2011 Intel Corporation
 * Copyright (c) 2010 VMware, Inc.
 * Copyright (c) 2011 Dave Airlie
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NON-INFRINGEMENT.  IN NO EVENT SHALL VMWARE AND/OR THEIR SUPPLIERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/**
 * @file getteximage-clamping.c
 *
 * From the GL_EXT_texture_integer spec:
 *
 *
 *     "(modify the section labeled "Final Conversion", p. 222)
 *
 *      For a floating-point RGBA color, each component is first
 *      clamped to [0, 1]. Then the appropriate conversion formula
 *      from table 4.7 is applied to the component.  For an integer
 *      RGBA color, each component is clamped to the representable
 *      range of <type>."
 *
 * This test checks the conversion and clamping by making a texture of
 * every sized internal format it can, and reads it to every
 * format/type combo it can.
 */


#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

struct format_info {
	const char *name;
	GLenum internal_format, base_format;
	int size;
	bool sign;
};

/* FINISHME: I'd like this set of texture formats to be sanitized a
 * bit, and it should probably be shared with the other testcases in
 * this directory.
 */
static const struct format_info formats[] = {
	{ "GL_RGBA8I",   GL_RGBA8I,   GL_RGBA_INTEGER, 8,  true  },
	{ "GL_RGBA8UI",  GL_RGBA8UI , GL_RGBA_INTEGER, 8,  false },
	{ "GL_RGBA16I",  GL_RGBA16I,  GL_RGBA_INTEGER, 16, true  },
	{ "GL_RGBA16UI", GL_RGBA16UI, GL_RGBA_INTEGER, 16, false },
	{ "GL_RGBA32I",  GL_RGBA32I,  GL_RGBA_INTEGER, 32, true  },
	{ "GL_RGBA32UI", GL_RGBA32UI, GL_RGBA_INTEGER, 32, false },

	{ "GL_RGBA8I (bgra)",   GL_RGBA8I,   GL_BGRA_INTEGER, 8,  true  },
	{ "GL_RGBA8UI (bgra)",  GL_RGBA8UI , GL_BGRA_INTEGER, 8,  false },
	{ "GL_RGBA16I (bgra)",  GL_RGBA16I,  GL_BGRA_INTEGER, 16, true  },
	{ "GL_RGBA16UI (bgra)", GL_RGBA16UI, GL_BGRA_INTEGER, 16, false },
	{ "GL_RGBA32I (bgra)",  GL_RGBA32I,  GL_BGRA_INTEGER, 32, true  },
	{ "GL_RGBA32UI (bgra)", GL_RGBA32UI, GL_BGRA_INTEGER, 32, false },

	{ "GL_RGB8I",   GL_RGB8I,   GL_RGB_INTEGER, 8,  true  },
	{ "GL_RGB8UI",  GL_RGB8UI , GL_RGB_INTEGER, 8,  false },
	{ "GL_RGB16I",  GL_RGB16I,  GL_RGB_INTEGER, 16, true  },
	{ "GL_RGB16UI", GL_RGB16UI, GL_RGB_INTEGER, 16, false },
	{ "GL_RGB32I",  GL_RGB32I,  GL_RGB_INTEGER, 32, true  },
	{ "GL_RGB32UI", GL_RGB32UI, GL_RGB_INTEGER, 32, false },

	{ "GL_ALPHA8I_EXT",   GL_ALPHA8I_EXT,   GL_ALPHA_INTEGER_EXT, 8,  true  },
	{ "GL_ALPHA8UI_EXT",  GL_ALPHA8UI_EXT , GL_ALPHA_INTEGER_EXT, 8,  false },
	{ "GL_ALPHA16I_EXT",  GL_ALPHA16I_EXT,  GL_ALPHA_INTEGER_EXT, 16, true  },
	{ "GL_ALPHA16UI_EXT", GL_ALPHA16UI_EXT, GL_ALPHA_INTEGER_EXT, 16, false },
	{ "GL_ALPHA32I_EXT",  GL_ALPHA32I_EXT,  GL_ALPHA_INTEGER_EXT, 32, true  },
	{ "GL_ALPHA32UI_EXT", GL_ALPHA32UI_EXT, GL_ALPHA_INTEGER_EXT, 32, false },

	{ "GL_LUMINANCE8I_EXT",   GL_LUMINANCE8I_EXT,   GL_LUMINANCE_INTEGER_EXT, 8,  true  },
	{ "GL_LUMINANCE8UI_EXT",  GL_LUMINANCE8UI_EXT , GL_LUMINANCE_INTEGER_EXT, 8,  false },
	{ "GL_LUMINANCE16I_EXT",  GL_LUMINANCE16I_EXT,  GL_LUMINANCE_INTEGER_EXT, 16, true  },
	{ "GL_LUMINANCE16UI_EXT", GL_LUMINANCE16UI_EXT, GL_LUMINANCE_INTEGER_EXT, 16, false },
	{ "GL_LUMINANCE32I_EXT",  GL_LUMINANCE32I_EXT,  GL_LUMINANCE_INTEGER_EXT, 32, true  },
	{ "GL_LUMINANCE32UI_EXT", GL_LUMINANCE32UI_EXT, GL_LUMINANCE_INTEGER_EXT, 32, false },

	{ "GL_LUMINANCE_ALPHA8I_EXT",   GL_LUMINANCE_ALPHA8I_EXT,   GL_LUMINANCE_ALPHA_INTEGER_EXT, 8,  true  },
	{ "GL_LUMINANCE_ALPHA8UI_EXT",  GL_LUMINANCE_ALPHA8UI_EXT , GL_LUMINANCE_ALPHA_INTEGER_EXT, 8,  false },
	{ "GL_LUMINANCE_ALPHA16I_EXT",  GL_LUMINANCE_ALPHA16I_EXT,  GL_LUMINANCE_ALPHA_INTEGER_EXT, 16, true  },
	{ "GL_LUMINANCE_ALPHA16UI_EXT", GL_LUMINANCE_ALPHA16UI_EXT, GL_LUMINANCE_ALPHA_INTEGER_EXT, 16, false },
	{ "GL_LUMINANCE_ALPHA32I_EXT",  GL_LUMINANCE_ALPHA32I_EXT,  GL_LUMINANCE_ALPHA_INTEGER_EXT, 32, true  },
	{ "GL_LUMINANCE_ALPHA32UI_EXT", GL_LUMINANCE_ALPHA32UI_EXT, GL_LUMINANCE_ALPHA_INTEGER_EXT, 32, false },

	{ "GL_INTENSITY8I_EXT",   GL_INTENSITY8I_EXT,   GL_RED_INTEGER_EXT, 8,  true  },
	{ "GL_INTENSITY8UI_EXT",  GL_INTENSITY8UI_EXT , GL_RED_INTEGER_EXT, 8,  false },
	{ "GL_INTENSITY16I_EXT",  GL_INTENSITY16I_EXT,  GL_RED_INTEGER_EXT, 16, true  },
	{ "GL_INTENSITY16UI_EXT", GL_INTENSITY16UI_EXT, GL_RED_INTEGER_EXT, 16, false },
	{ "GL_INTENSITY32I_EXT",  GL_INTENSITY32I_EXT,  GL_RED_INTEGER_EXT, 32, true  },
	{ "GL_INTENSITY32UI_EXT", GL_INTENSITY32UI_EXT, GL_RED_INTEGER_EXT, 32, false },

	{ "GL_RG8I",   GL_RG8I,   GL_RG_INTEGER, 8,  true  },
	{ "GL_RG8UI",  GL_RG8UI , GL_RG_INTEGER, 8,  false },
	{ "GL_RG16I",  GL_RG16I,  GL_RG_INTEGER, 16, true  },
	{ "GL_RG16UI", GL_RG16UI, GL_RG_INTEGER, 16, false },
	{ "GL_RG32I",  GL_RG32I,  GL_RG_INTEGER, 32, true  },
	{ "GL_RG32UI", GL_RG32UI, GL_RG_INTEGER, 32, false },
	{ "GL_R8I",   GL_R8I,   GL_RED_INTEGER, 8,  true  },
	{ "GL_R8UI",  GL_R8UI , GL_RED_INTEGER, 8,  false },
	{ "GL_R16I",  GL_R16I,  GL_RED_INTEGER, 16, true  },
	{ "GL_R16UI", GL_R16UI, GL_RED_INTEGER, 16, false },
	{ "GL_R32I",  GL_R32I,  GL_RED_INTEGER, 32, true  },
	{ "GL_R32UI", GL_R32UI, GL_RED_INTEGER, 32, false },

	{ "GL_RGB10_A2UI", GL_RGB10_A2UI, GL_RGBA_INTEGER, 10, false },
	{ "GL_RGB10_A2UI (bgra)", GL_RGB10_A2UI, GL_BGRA_INTEGER, 10, false },
	{ "GL_RGB10_A2UI (rev)", GL_RGB10_A2UI, GL_RGBA_INTEGER, 10, true },
	{ "GL_RGB10_A2UI (rev bgra)", GL_RGB10_A2UI, GL_BGRA_INTEGER, 10, true },
};

struct read_format_info {
	const char *format_name, *type_name;
	GLenum format, type;
	int size;
	bool sign;
};

#define READ_FORMAT(format, type, size, sign) \
	{ #format, #type, format, type, size, sign }

/* Integer formats from table 3.5 and 3.6 of the GL 3.0 specification */
static const struct read_format_info read_formats[] = {
	READ_FORMAT(GL_RGBA_INTEGER, GL_UNSIGNED_INT,   32, false),
	READ_FORMAT(GL_RGBA_INTEGER, GL_INT,            32, true),
	READ_FORMAT(GL_RGBA_INTEGER, GL_UNSIGNED_SHORT, 16, false),
	READ_FORMAT(GL_RGBA_INTEGER, GL_SHORT,          16, true),
	READ_FORMAT(GL_RGBA_INTEGER, GL_UNSIGNED_BYTE,   8, false),
	READ_FORMAT(GL_RGBA_INTEGER, GL_BYTE,            8, true),

	READ_FORMAT(GL_RED_INTEGER, GL_UNSIGNED_INT,   32, false),
	READ_FORMAT(GL_RED_INTEGER, GL_INT,            32, true),
	READ_FORMAT(GL_RED_INTEGER, GL_UNSIGNED_SHORT, 16, false),
	READ_FORMAT(GL_RED_INTEGER, GL_SHORT,          16, true),
	READ_FORMAT(GL_RED_INTEGER, GL_UNSIGNED_BYTE,   8, false),
	READ_FORMAT(GL_RED_INTEGER, GL_BYTE,            8, true),

	READ_FORMAT(GL_GREEN_INTEGER, GL_UNSIGNED_INT,   32, false),
	READ_FORMAT(GL_GREEN_INTEGER, GL_INT,            32, true),
	READ_FORMAT(GL_GREEN_INTEGER, GL_UNSIGNED_SHORT, 16, false),
	READ_FORMAT(GL_GREEN_INTEGER, GL_SHORT,          16, true),
	READ_FORMAT(GL_GREEN_INTEGER, GL_UNSIGNED_BYTE,   8, false),
	READ_FORMAT(GL_GREEN_INTEGER, GL_BYTE,            8, true),

	READ_FORMAT(GL_BLUE_INTEGER, GL_UNSIGNED_INT,   32, false),
	READ_FORMAT(GL_BLUE_INTEGER, GL_INT,            32, true),
	READ_FORMAT(GL_BLUE_INTEGER, GL_UNSIGNED_SHORT, 16, false),
	READ_FORMAT(GL_BLUE_INTEGER, GL_SHORT,          16, true),
	READ_FORMAT(GL_BLUE_INTEGER, GL_UNSIGNED_BYTE,   8, false),
	READ_FORMAT(GL_BLUE_INTEGER, GL_BYTE,            8, true),

	READ_FORMAT(GL_ALPHA_INTEGER, GL_UNSIGNED_INT,   32, false),
	READ_FORMAT(GL_ALPHA_INTEGER, GL_INT,            32, true),
	READ_FORMAT(GL_ALPHA_INTEGER, GL_UNSIGNED_SHORT, 16, false),
	READ_FORMAT(GL_ALPHA_INTEGER, GL_SHORT,          16, true),
	READ_FORMAT(GL_ALPHA_INTEGER, GL_UNSIGNED_BYTE,   8, false),
	READ_FORMAT(GL_ALPHA_INTEGER, GL_BYTE,            8, true),

	READ_FORMAT(GL_RG_INTEGER, GL_UNSIGNED_INT,   32, false),
	READ_FORMAT(GL_RG_INTEGER, GL_INT,            32, true),
	READ_FORMAT(GL_RG_INTEGER, GL_UNSIGNED_SHORT, 16, false),
	READ_FORMAT(GL_RG_INTEGER, GL_SHORT,          16, true),
	READ_FORMAT(GL_RG_INTEGER, GL_UNSIGNED_BYTE,   8, false),
	READ_FORMAT(GL_RG_INTEGER, GL_BYTE,            8, true),

	READ_FORMAT(GL_RGB_INTEGER, GL_UNSIGNED_INT,   32, false),
	READ_FORMAT(GL_RGB_INTEGER, GL_INT,            32, true),
	READ_FORMAT(GL_RGB_INTEGER, GL_UNSIGNED_SHORT, 16, false),
	READ_FORMAT(GL_RGB_INTEGER, GL_SHORT,          16, true),
	READ_FORMAT(GL_RGB_INTEGER, GL_UNSIGNED_BYTE,   8, false),
	READ_FORMAT(GL_RGB_INTEGER, GL_BYTE,            8, true),

	/* RGBA was put at the top so that the more obvious failures come first. */

	READ_FORMAT(GL_BGR_INTEGER, GL_UNSIGNED_INT,   32, false),
	READ_FORMAT(GL_BGR_INTEGER, GL_INT,            32, true),
	READ_FORMAT(GL_BGR_INTEGER, GL_UNSIGNED_SHORT, 16, false),
	READ_FORMAT(GL_BGR_INTEGER, GL_SHORT,          16, true),
	READ_FORMAT(GL_BGR_INTEGER, GL_UNSIGNED_BYTE,   8, false),
	READ_FORMAT(GL_BGR_INTEGER, GL_BYTE,            8, true),

	READ_FORMAT(GL_BGRA_INTEGER, GL_UNSIGNED_INT,   32, false),
	READ_FORMAT(GL_BGRA_INTEGER, GL_INT,            32, true),
	READ_FORMAT(GL_BGRA_INTEGER, GL_UNSIGNED_SHORT, 16, false),
	READ_FORMAT(GL_BGRA_INTEGER, GL_SHORT,          16, true),
	READ_FORMAT(GL_BGRA_INTEGER, GL_UNSIGNED_BYTE,   8, false),
	READ_FORMAT(GL_BGRA_INTEGER, GL_BYTE,            8, true),

	/* FINISHME: Add more RGB10_A2UI.  Note the other packed formats
	 * besides 10/10/10/2 included in the spec!
	 */
	READ_FORMAT(GL_RGBA_INTEGER, GL_UNSIGNED_INT_10_10_10_2,   32, false),
	READ_FORMAT(GL_RGBA_INTEGER, GL_UNSIGNED_INT_2_10_10_10_REV,   32, false),
};

static bool test_rg = false;
static bool test_rgb10_a2ui = false;

static void
usage(void)
{
	fprintf(stderr, "usage:\n");
	fprintf(stderr,
		"ext_texture_integer-teximage-clamping "
		"[GL_ARB_texture_rg | GL_ARB_texture_rgb10_a2ui]\n");
	exit(1);
}

static const uint32_t texels_u[][4] = {
	{ 0xfffffff0, 0x00000000, 0x00000000, 0x00000000 },
	{ 0x00000000, 0xfffffff0, 0x00000000, 0x00000000 },
	{ 0x00000000, 0x00000000, 0xfffffff0, 0x00000000 },
	{ 0x00000000, 0x00000000, 0x00000000, 0xfffffff0 },
	{ 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff },
};

static const uint32_t texels_s[][4] = {
	{ 0x7fffffff, 0x80000000, 0x00000000, 0x00000000 },
	{ 0x00000000, 0x7fffffff, 0x80000000, 0x00000000 },
	{ 0x00000000, 0x00000000, 0x7fffffff, 0x80000000 },
	{ 0x80000000, 0x00000000, 0x00000000, 0x7fffffff },
	{ 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff },
};

static void
print_packed(const struct read_format_info *read_info, char *read,
	     int pixel, int chans)
{
	int i;

	char *read_pixel = read + pixel * chans * read_info->size / 8;

	switch (read_info->type) {
	case GL_UNSIGNED_INT:
	case GL_INT:
		for (i = 0; i < chans; i++)
			fprintf(stderr, " 0x%08x", ((uint32_t *)read_pixel)[i]);
		for (; i < 4; i++)
			fprintf(stderr, "           ");
		break;

	case GL_UNSIGNED_SHORT:
	case GL_SHORT:
		for (i = 0; i < chans; i++)
			fprintf(stderr, " 0x%04x", ((uint16_t *)read)[i]);
		for (; i < 4; i++)
			fprintf(stderr, "       ");
		break;

	case GL_UNSIGNED_BYTE:
	case GL_BYTE:
		for (i = 0; i < chans; i++)
			fprintf(stderr, " 0x%02x", ((uint8_t *)read)[i]);
		for (; i < 4; i++)
			fprintf(stderr, "     ");
		break;

	default:
		/* FINISHME: GL_texture_rgb10_a2ui */
		abort();
	}
}

static void
report_fail(const struct format_info *tex_info,
	    const struct read_format_info *read_info,
	    uint32_t texels[][4],
	    void *read, void *expected,
	    int pixels, int chans)
{
	int i;

	fprintf(stderr, "Failure reading from %s to %s/%s\n",
		tex_info->name, read_info->format_name, read_info->type_name);

	/* 10/channel + 3 spaces. */
	fprintf(stderr, "  %43s", "expected RGBA in texels");
	if (read_info->size == 32)
		fprintf(stderr, "  %43s", "expected packed");
	else if (read_info->size == 16)
		fprintf(stderr, "  %31s", "expected packed");
	else if (read_info->size == 8)
		fprintf(stderr, "  %19s", "expected packed");
	else
		abort(); /* FINISHME: GL_ARB_texture_rgb10_a2ui */
	fprintf(stderr, "  read values");
	fprintf(stderr, "\n");
	for (i = 0; i < pixels; i++) {
		fprintf(stderr, "  0x%08x 0x%08x 0x%08x 0x%08x",
			texels[i][0],
			texels[i][1],
			texels[i][2],
			texels[i][3]);

		fprintf(stderr, " ");
		print_packed(read_info, expected, i, chans);
		fprintf(stderr, " ");
		print_packed(read_info, read, i, chans);

		fprintf(stderr, "\n");
	}
}

static void
pack(const struct read_format_info *read_info, void *packed,
     int pixel, int chan,
     int values_per_pixel, uint32_t value)
{
	int32_t ivalue = value;
	void *pack_dst = ((char *)packed) + ((pixel * values_per_pixel +
					      chan) * read_info->size / 8);

	switch (read_info->type) {
	case GL_UNSIGNED_INT:
	case GL_INT:
		*(uint32_t *)pack_dst = value;
		break;

	case GL_UNSIGNED_SHORT:
		if (value > 0xffff)
			value = 0xffff;
		*(uint16_t *)pack_dst = value;
		break;

	case GL_SHORT:
		if (ivalue > 32767)
			ivalue = 32767;
		else if (ivalue < -32768)
			ivalue = -32768;
		*(int16_t *)pack_dst = ivalue;
		break;

	case GL_UNSIGNED_BYTE:
		if (value > 0xff)
			value = 0xff;
		*(uint8_t *)pack_dst = value;
		break;

	case GL_BYTE:
		if (ivalue > 127)
			ivalue = 127;
		else if (ivalue < -128)
			ivalue = -128;
		*(int8_t *)pack_dst = ivalue;
		break;

	default:
		/* FINISHME: GL_texture_rgb10_a2ui */
		abort();
	}
}

static enum piglit_result
read_format(const struct format_info *tex_info,
	    const struct read_format_info *read_info,
	    uint32_t texels[][4], int num_texels)
{
	size_t texels_size = num_texels * 4 * sizeof(uint32_t);
	char *expected;
	char *read;
	int i;
	int chans = 0;
	enum piglit_result result;

	if (!test_rg && (read_info->format == GL_RED_INTEGER ||
			 read_info->format == GL_RG_INTEGER)) {
		return PIGLIT_SKIP;
	}

	if (!test_rgb10_a2ui) {
		/* Packed integer pixel formats were introduced with
		 * GL_texture_rgb10_a2ui.
		 */
		switch (read_info->type) {
		case GL_INT:
		case GL_UNSIGNED_INT:
		case GL_SHORT:
		case GL_UNSIGNED_SHORT:
		case GL_BYTE:
		case GL_UNSIGNED_BYTE:
			break;
		default:
			return PIGLIT_SKIP;
		}
	}

	/* FINISHME: Again, not really sure how to handle sign conversion. */
	if (tex_info->sign != read_info->sign)
		return PIGLIT_SKIP;


	printf("Reading from %s to %s/%s\n", tex_info->name,
	       read_info->format_name, read_info->type_name);

	expected = (char *)malloc(texels_size);
	read = (char *)malloc(texels_size);

	memset(expected, 0xd0, texels_size);
	memset(read, 0xd0, texels_size);

	glGetTexImage(GL_TEXTURE_2D, 0,
		      read_info->format, read_info->type, read);

	switch (read_info->format) {
	case GL_RGBA_INTEGER:
		chans = 4;
		for (i = 0; i < num_texels; i++) {
			pack(read_info, expected, i, 0, chans, texels[i][0]);
			pack(read_info, expected, i, 1, chans, texels[i][1]);
			pack(read_info, expected, i, 2, chans, texels[i][2]);
			pack(read_info, expected, i, 3, chans, texels[i][3]);
		}
		break;

	case GL_BGRA_INTEGER:
		chans = 4;
		for (i = 0; i < num_texels; i++) {
			pack(read_info, expected, i, 0, chans, texels[i][2]);
			pack(read_info, expected, i, 1, chans, texels[i][1]);
			pack(read_info, expected, i, 2, chans, texels[i][0]);
			pack(read_info, expected, i, 3, chans, texels[i][3]);
		}
		break;

	case GL_RGB_INTEGER:
		chans = 3;
		for (i = 0; i < num_texels; i++) {
			pack(read_info, expected, i, 0, chans, texels[i][0]);
			pack(read_info, expected, i, 1, chans, texels[i][1]);
			pack(read_info, expected, i, 2, chans, texels[i][2]);
		}
		break;

	case GL_BGR_INTEGER:
		chans = 3;
		for (i = 0; i < num_texels; i++) {
			pack(read_info, expected, i, 0, chans, texels[i][2]);
			pack(read_info, expected, i, 1, chans, texels[i][1]);
			pack(read_info, expected, i, 2, chans, texels[i][0]);
		}
		break;

	case GL_RED_INTEGER:
		chans = 1;
		for (i = 0; i < num_texels; i++) {
			pack(read_info, expected, i, 0, chans, texels[i][0]);
		}
		break;

	case GL_GREEN_INTEGER:
		chans = 1;
		for (i = 0; i < num_texels; i++) {
			pack(read_info, expected, i, 0, chans, texels[i][1]);
		}
		break;

	case GL_BLUE_INTEGER:
		chans = 1;
		for (i = 0; i < num_texels; i++) {
			pack(read_info, expected, i, 0, chans, texels[i][2]);
		}
		break;

	case GL_ALPHA_INTEGER:
		chans = 1;
		for (i = 0; i < num_texels; i++) {
			pack(read_info, expected, i, 0, chans, texels[i][3]);
		}
		break;

	case GL_RG_INTEGER:
		chans = 2;
		for (i = 0; i < num_texels; i++) {
			pack(read_info, expected, i, 0, chans, texels[i][0]);
			pack(read_info, expected, i, 1, chans, texels[i][1]);
		}
		break;

	default:
		assert(0);
		return PIGLIT_SKIP;
	}

	if (memcmp(expected, read, num_texels * chans * read_info->size / 8)) {
		report_fail(tex_info, read_info, texels, read, expected,
			    num_texels, chans);
		result = PIGLIT_FAIL;
	} else {
		result = PIGLIT_PASS;
	}

	free(read);
	free(expected);

	return result;
}

static enum piglit_result
test_format(const struct format_info *info)
{
	int num_texels = ARRAY_SIZE(texels_s);
	uint32_t texels[ARRAY_SIZE(texels_s)][4];
	GLenum type;
	int lbits, abits, ibits, rbits, gbits, bbits;
	int i, readf;
	enum piglit_result result = PIGLIT_SKIP;

	if (!test_rg && ((info->base_format == GL_RED_INTEGER &&
			  !strstr(info->name, "GL_INTENSITY")) ||
			 info->base_format == GL_RG_INTEGER)) {
		return PIGLIT_SKIP;
	}

	if (!test_rgb10_a2ui && info->internal_format == GL_RGB10_A2UI)
		return PIGLIT_SKIP;

	/* FINISHME: We only test conversion from large signed to
	 * small signed or large unsigned to small unsigned.  The
	 * rules just say that when reading pixels, the value is
	 * clamped to the representable range, not how/when sign
	 * extension occurs, and whether the clamping applies before
	 * or after
	 */
	if (!strstr(info->name, "32"))
		return PIGLIT_SKIP;

	if (info->sign) {
		memcpy(texels, texels_s, sizeof(texels_s));
		type = GL_INT;
	} else {
		memcpy(texels, texels_u, sizeof(texels_u));
		type = GL_UNSIGNED_INT;
	}
	glTexImage2D(GL_TEXTURE_2D, 0, info->internal_format, num_texels, 1, 0,
		     GL_RGBA_INTEGER_EXT, type, texels);

	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0,
				 GL_TEXTURE_LUMINANCE_SIZE, &lbits);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0,
				 GL_TEXTURE_ALPHA_SIZE, &abits);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0,
				 GL_TEXTURE_INTENSITY_SIZE, &ibits);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0,
				 GL_TEXTURE_RED_SIZE, &rbits);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0,
				 GL_TEXTURE_GREEN_SIZE, &gbits);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0,
				 GL_TEXTURE_BLUE_SIZE, &bbits);

	/* Compute the RGBA channels that should be read from the
	 * texture given the input RGBA texels we gave.  See Table 6.1
	 * ("Texture, table, and filter return values") of the GL 3.0
	 * specification.  Note that input R is always mapped to L or
	 * I, and comes back out in R (except for ALPHA).
	 */
	if (ibits || lbits) {
		for (i = 0; i < num_texels; i++) {
			texels[i][1] = 0;
			texels[i][2] = 0;
		}
	} else {
		if (!rbits) {
			for (i = 0; i < num_texels; i++)
				texels[i][0] = 0;
		}
		if (!gbits) {
			for (i = 0; i < num_texels; i++)
				texels[i][1] = 0;
		}
		if (!bbits) {
			for (i = 0; i < num_texels; i++)
				texels[i][2] = 0;
		}
	}
	/* Everybody's consistent on A bits in table 6.1. */
	if (!abits) {
		for (i = 0; i < num_texels; i++)
			texels[i][3] = 1;
	}

	for (readf = 0; readf < ARRAY_SIZE(read_formats); readf++) {
		piglit_merge_result(&result, read_format(info,
							 &read_formats[readf],
							 texels, num_texels));

		if (result == PIGLIT_FAIL)
			return result;
	}

	return result;
}


enum piglit_result
piglit_display(void)
{
	/* unreached */
	return PIGLIT_FAIL;
}


void
piglit_init(int argc, char **argv)
{
	GLuint tex;
	int i, texf;
	enum piglit_result result = PIGLIT_SKIP;

	/* Note: We test later extensions by testing them against all
	 * combinations of (storage format, read format) including the
	 * formats from previous extensions.
	 */
	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "GL_ARB_texture_rg") == 0) {
			piglit_require_extension(argv[i]);
			test_rg = true;
		} else if (strcmp(argv[i], "GL_ARB_texture_rgb10_a2ui") == 0) {
			piglit_require_extension(argv[i]);
			test_rg = true;
			test_rgb10_a2ui = true;
		} else {
			usage();
			exit(1);
		}
	}

	piglit_require_extension("GL_EXT_texture_integer");

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	for (texf = 0; texf < ARRAY_SIZE(formats); texf++)
		piglit_merge_result(&result, test_format(&formats[texf]));

	piglit_report_result(result);
}
