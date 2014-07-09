/* Copyright © 2011 Intel Corporation
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

#include "piglit-util-gl.h"
#include "sized-internalformats.h"

#define FORMAT(token, r, g, b, a, l, i, d, s) \
	{ #token, token, { r, g, b, a, l, i, d, s } }

/* This list has to match up with the enums in sized-internalformats.h */
static const struct {
	int size;
	GLenum type;
} sized_format_bits[BITS_MAX] = {
	{ 0, GL_NONE },
  
	{ 32, GL_UNSIGNED_NORMALIZED },
	{ 32, GL_FLOAT },
	{ 32, GL_INT },
	{ 32, GL_UNSIGNED_INT },

	{ 24, GL_UNSIGNED_NORMALIZED },

	{ 16, GL_UNSIGNED_NORMALIZED },
	{ 16, GL_SIGNED_NORMALIZED },
	{ 16, GL_FLOAT },
	{ 16, GL_INT },
	{ 16, GL_UNSIGNED_INT },

	{ 10, GL_UNSIGNED_INT },
	{ 2, GL_UNSIGNED_INT },

	{ 12, GL_UNSIGNED_NORMALIZED },
	{ 10, GL_UNSIGNED_NORMALIZED },

	{ 8, GL_UNSIGNED_NORMALIZED },
	{ 8, GL_SIGNED_NORMALIZED },
	{ 8, GL_INT },
	{ 8, GL_UNSIGNED_INT },

	{ 6, GL_UNSIGNED_NORMALIZED },
	{ 5, GL_UNSIGNED_NORMALIZED },
	{ 4, GL_UNSIGNED_NORMALIZED },
	{ 3, GL_UNSIGNED_NORMALIZED },
	{ 2, GL_UNSIGNED_NORMALIZED },
	{ 1, GL_UNSIGNED_NORMALIZED },

	{ 11, GL_FLOAT },
	{ 10, GL_FLOAT },
	{ 9, GL_FLOAT },

	{ ~0, GL_UNSIGNED_NORMALIZED },
	{ ~0, GL_SIGNED_NORMALIZED },
};

const struct sized_internalformat sized_internalformats[] = {
	/* Sized internal color formats, table 3.16 of the GL 3.0
	 * specification.
	 */
	FORMAT(GL_ALPHA4, NONE, NONE, NONE, UN4, NONE, NONE, NONE, NONE),
	FORMAT(GL_ALPHA8, NONE, NONE, NONE, UN8, NONE, NONE, NONE, NONE),
	FORMAT(GL_ALPHA12, NONE, NONE, NONE, UN12, NONE, NONE, NONE, NONE),
	FORMAT(GL_ALPHA16, NONE, NONE, NONE, UN16, NONE, NONE, NONE, NONE),
	FORMAT(GL_R8, UN8, NONE, NONE, NONE, NONE, NONE, NONE, NONE),
	FORMAT(GL_R16, UN16, NONE, NONE, NONE, NONE, NONE, NONE, NONE),
	FORMAT(GL_RG8, UN8, UN8, NONE, NONE, NONE, NONE, NONE, NONE),
	FORMAT(GL_RG16, UN16, UN16, NONE, NONE, NONE, NONE, NONE, NONE),
	FORMAT(GL_R3_G3_B2, UN3, UN3, UN2, NONE, NONE, NONE, NONE, NONE),
	FORMAT(GL_RGB4, UN4, UN4, UN4, NONE, NONE, NONE, NONE, NONE),
	FORMAT(GL_RGB5, UN5, UN5, UN5, NONE, NONE, NONE, NONE, NONE),
	FORMAT(GL_RGB8, UN8, UN8, UN8, NONE, NONE, NONE, NONE, NONE),
	FORMAT(GL_RGB10, UN10, UN10, UN10, NONE, NONE, NONE, NONE, NONE),
	FORMAT(GL_RGB12, UN12, UN12, UN12, NONE, NONE, NONE, NONE, NONE),
	FORMAT(GL_RGB16, UN16, UN16, UN16, NONE, NONE, NONE, NONE, NONE),
	FORMAT(GL_RGBA2, UN2, UN2, UN2, UN2, NONE, NONE, NONE, NONE),
	FORMAT(GL_RGBA4, UN4, UN4, UN4, UN4, NONE, NONE, NONE, NONE),
	FORMAT(GL_RGB5_A1, UN5, UN5, UN5, UN1, NONE, NONE, NONE, NONE),
	FORMAT(GL_RGBA8, UN8, UN8, UN8, UN8, NONE, NONE, NONE, NONE),
	FORMAT(GL_RGB10_A2, UN10, UN10, UN10, UN2, NONE, NONE, NONE, NONE),
	FORMAT(GL_RGB10_A2UI, U10, U10, U10, U2, NONE, NONE, NONE, NONE),
	FORMAT(GL_RGBA12, UN12, UN12, UN12, UN12, NONE, NONE, NONE, NONE),
	FORMAT(GL_RGBA16, UN16, UN16, UN16, UN16, NONE, NONE, NONE, NONE),
	FORMAT(GL_SRGB8, UN8, UN8, UN8, NONE, NONE, NONE, NONE, NONE),
	FORMAT(GL_SRGB8_ALPHA8, UN8, UN8, UN8, UN8, NONE, NONE, NONE, NONE),
	FORMAT(GL_R16F, F16, NONE, NONE, NONE, NONE, NONE, NONE, NONE),
	FORMAT(GL_RG16F, F16, F16, NONE, NONE, NONE, NONE, NONE, NONE),
	FORMAT(GL_RGB16F, F16, F16, F16, NONE, NONE, NONE, NONE, NONE),
	FORMAT(GL_RGBA16F, F16, F16, F16, F16, NONE, NONE, NONE, NONE),
	FORMAT(GL_R32F, F32, NONE, NONE, NONE, NONE, NONE, NONE, NONE),
	FORMAT(GL_RG32F, F32, F32, NONE, NONE, NONE, NONE, NONE, NONE),
	FORMAT(GL_RGB32F, F32, F32, F32, NONE, NONE, NONE, NONE, NONE),
	FORMAT(GL_RGBA32F, F32, F32, F32, F32, NONE, NONE, NONE, NONE),
	FORMAT(GL_R11F_G11F_B10F, F11, F11, F10, NONE, NONE, NONE, NONE, NONE),
	FORMAT(GL_RGB565, UN5, UN6, UN5, NONE, NONE, NONE, NONE, NONE),
	FORMAT(GL_RGB9_E5, F9, F9, F9, NONE, NONE, NONE, NONE, NONE),
	FORMAT(GL_R8I, I8, NONE, NONE, NONE, NONE, NONE, NONE, NONE),
	FORMAT(GL_R8UI, U8, NONE, NONE, NONE, NONE, NONE, NONE, NONE),
	FORMAT(GL_R16I, I16, NONE, NONE, NONE, NONE, NONE, NONE, NONE),
	FORMAT(GL_R16UI, U16, NONE, NONE, NONE, NONE, NONE, NONE, NONE),
	FORMAT(GL_R32I, I32, NONE, NONE, NONE, NONE, NONE, NONE, NONE),
	FORMAT(GL_R32UI, U32, NONE, NONE, NONE, NONE, NONE, NONE, NONE),
	FORMAT(GL_RG8I, I8, I8, NONE, NONE, NONE, NONE, NONE, NONE),
	FORMAT(GL_RG8UI, U8, U8, NONE, NONE, NONE, NONE, NONE, NONE),
	FORMAT(GL_RG16I, I16, I16, NONE, NONE, NONE, NONE, NONE, NONE),
	FORMAT(GL_RG16UI, U16, U16, NONE, NONE, NONE, NONE, NONE, NONE),
	FORMAT(GL_RG32I, I32, I32, NONE, NONE, NONE, NONE, NONE, NONE),
	FORMAT(GL_RG32UI, U32, U32, NONE, NONE, NONE, NONE, NONE, NONE),
	FORMAT(GL_RGB8I, I8, I8, I8, NONE, NONE, NONE, NONE, NONE),
	FORMAT(GL_RGB8UI, U8, U8, U8, NONE, NONE, NONE, NONE, NONE),
	FORMAT(GL_RGB16I, I16, I16, I16, NONE, NONE, NONE, NONE, NONE),
	FORMAT(GL_RGB16UI, U16, U16, U16, NONE, NONE, NONE, NONE, NONE),
	FORMAT(GL_RGB32I, I32, I32, I32, NONE, NONE, NONE, NONE, NONE),
	FORMAT(GL_RGB32UI, U32, U32, U32, NONE, NONE, NONE, NONE, NONE),
	FORMAT(GL_RGBA8I, I8, I8, I8, I8, NONE, NONE, NONE, NONE),
	FORMAT(GL_RGBA8UI, U8, U8, U8, U8, NONE, NONE, NONE, NONE),
	FORMAT(GL_RGBA16I, I16, I16, I16, I16, NONE, NONE, NONE, NONE),
	FORMAT(GL_RGBA16UI, U16, U16, U16, U16, NONE, NONE, NONE, NONE),
	FORMAT(GL_RGBA32I, I32, I32, I32, I32, NONE, NONE, NONE, NONE),
	FORMAT(GL_RGBA32UI, U32, U32, U32, U32, NONE, NONE, NONE, NONE),

	/* SNORM formats introduced as required sized texture formats
	 * in GL 3.1, but didn't get sizes actually specified until GL
	 * 3.2's table 3.12.
	 */
	FORMAT(GL_R8_SNORM, SN8, NONE, NONE, NONE, NONE, NONE, NONE, NONE),
	FORMAT(GL_R16_SNORM, SN16, NONE, NONE, NONE, NONE, NONE, NONE, NONE),
	FORMAT(GL_RG8_SNORM, SN8, SN8, NONE, NONE, NONE, NONE, NONE, NONE),
	FORMAT(GL_RG16_SNORM, SN16, SN16, NONE, NONE, NONE, NONE, NONE, NONE),
	FORMAT(GL_RGB8_SNORM, SN8, SN8, SN8, NONE, NONE, NONE, NONE, NONE),
	FORMAT(GL_RGB16_SNORM, SN16, SN16, SN16, NONE, NONE, NONE, NONE, NONE),
	FORMAT(GL_RGBA8_SNORM, SN8, SN8, SN8, SN8, NONE, NONE, NONE, NONE),
	FORMAT(GL_RGBA16_SNORM, SN16, SN16, SN16, SN16, NONE, NONE, NONE, NONE),

	/* Sized internal luminance formats, table 3.17 of the GL 3.0
	 * specification.
	 */
	FORMAT(GL_LUMINANCE4, NONE, NONE, NONE, NONE, UN4, NONE, NONE, NONE),
	FORMAT(GL_LUMINANCE8, NONE, NONE, NONE, NONE, UN8, NONE, NONE, NONE),
	FORMAT(GL_LUMINANCE12, NONE, NONE, NONE, NONE, UN12, NONE, NONE, NONE),
	FORMAT(GL_LUMINANCE16, NONE, NONE, NONE, NONE, UN16, NONE, NONE, NONE),
	FORMAT(GL_LUMINANCE4_ALPHA4, NONE, NONE, NONE, UN4, UN4, NONE, NONE, NONE),
	FORMAT(GL_LUMINANCE8_ALPHA8, NONE, NONE, NONE, UN8, UN8, NONE, NONE, NONE),
	FORMAT(GL_LUMINANCE12_ALPHA12, NONE, NONE, NONE, UN12, UN12, NONE, NONE, NONE),
	FORMAT(GL_LUMINANCE16_ALPHA16, NONE, NONE, NONE, UN16, UN16, NONE, NONE, NONE),
	FORMAT(GL_INTENSITY4, NONE, NONE, NONE, NONE, NONE, UN4, NONE, NONE),
	FORMAT(GL_INTENSITY8, NONE, NONE, NONE, NONE, NONE, UN8, NONE, NONE),
	FORMAT(GL_INTENSITY12, NONE, NONE, NONE, NONE, NONE, UN12, NONE, NONE),
	FORMAT(GL_INTENSITY16, NONE, NONE, NONE, NONE, NONE, UN16, NONE, NONE),
	FORMAT(GL_SLUMINANCE, NONE, NONE, NONE, NONE, UN8, NONE, NONE, NONE),
	FORMAT(GL_SLUMINANCE8_ALPHA8, NONE, NONE, NONE, UN8, UN8, NONE, NONE, NONE),

	/* Sized internal depth and stencil formats, table 3.18 of the
	 * GL 3.0 specification.
	 */
	FORMAT(GL_DEPTH_COMPONENT16, NONE, NONE, NONE, NONE, NONE, NONE,
	       UN16, NONE),
	FORMAT(GL_DEPTH_COMPONENT24, NONE, NONE, NONE, NONE, NONE, NONE,
	       UN24, NONE),
	FORMAT(GL_DEPTH_COMPONENT32, NONE, NONE, NONE, NONE, NONE, NONE,
	       UN32, NONE),
	FORMAT(GL_DEPTH_COMPONENT32F, NONE, NONE, NONE, NONE, NONE, NONE,
	       F32, NONE),
	FORMAT(GL_DEPTH24_STENCIL8, NONE, NONE, NONE, NONE, NONE, NONE,
	       UN24, UN8),
	FORMAT(GL_DEPTH32F_STENCIL8, NONE, NONE, NONE, NONE, NONE, NONE,
	       F32, UN8),

	/* Specific compressed internal formats, table 3.19 of the GL
	 * 3.0 specification.
	 */
	FORMAT(GL_COMPRESSED_RG_RGTC2, UCMP, UCMP, NONE, NONE, NONE, NONE, NONE, NONE),
	FORMAT(GL_COMPRESSED_SIGNED_RG_RGTC2, SCMP, SCMP, NONE, NONE, NONE, NONE, NONE, NONE),
	FORMAT(GL_COMPRESSED_RED_RGTC1, UCMP, NONE, NONE, NONE, NONE, NONE, NONE, NONE),
	FORMAT(GL_COMPRESSED_SIGNED_RED_RGTC1, SCMP, NONE, NONE, NONE, NONE, NONE, NONE, NONE),

	{ GL_NONE },
};

const struct required_format required_formats[] = {
	{ GL_RGBA32F, 30, true },
	{ GL_RGBA32I, 30, true },
	{ GL_RGBA32UI, 30, true },
	{ GL_RGBA16, 30, true },
	{ GL_RGBA16F, 30, true },
	{ GL_RGBA16I, 30, true },
	{ GL_RGBA16UI, 30, true },
	{ GL_RGBA8, 30, true },
	{ GL_RGBA8I, 30, true },
	{ GL_RGBA8UI, 30, true },
	{ GL_SRGB8_ALPHA8, 30, true },
	{ GL_RGB10_A2, 30, true },

	{ GL_RGB10_A2UI, 33, true },

	{ GL_RGB5_A1, 42, true },
	{ GL_RGBA4, 42, true },

	{ GL_R11F_G11F_B10F, 30, true },

	{ GL_RGB565, 42 },

	{ GL_RG32F, 30, true },
	{ GL_RG32I, 30, true },
	{ GL_RG32UI, 30, true },
	{ GL_RG16, 30, true },
	{ GL_RG16F, 30, true },
	{ GL_RG16I, 30, true },
	{ GL_RG16UI, 30, true },
	{ GL_RG8, 30, true },
	{ GL_RG8I, 30, true },
	{ GL_RG8UI, 30, true },
	{ GL_R32F, 30, true },
	{ GL_R32I, 30, true },
	{ GL_R32UI, 30, true },
	{ GL_R16F, 30, true },
	{ GL_R16I, 30, true },
	{ GL_R16UI, 30, true },
	{ GL_R16, 30, true },
	{ GL_R8, 30, true },
	{ GL_R8I, 30, true },
	{ GL_R8UI, 30, true },

	{ GL_ALPHA8, 30, true }, /* deprecated */

	/* Required color formats (texture-only): */

	{ GL_RGBA16_SNORM, 31, false },
	{ GL_RGBA8_SNORM, 31, false },
	{ GL_RGB32F, 30, false },
	{ GL_RGB32I, 30, false },
	{ GL_RGB32UI, 30, false },

	{ GL_RGB16_SNORM, 31 },
	{ GL_RGB16F, 30, false },
	{ GL_RGB16I, 30, false },
	{ GL_RGB16UI, 30, false },
	{ GL_RGB16, 30, false },

	{ GL_RGB8_SNORM, 31 },
	{ GL_RGB8, 30, false },
	{ GL_RGB8I, 30, false },
	{ GL_RGB8UI, 30, false },
	{ GL_SRGB8, 30, false },

	{ GL_RGB9_E5, 30, false },

	{ GL_RG16_SNORM, 31, false },
	{ GL_RG8_SNORM, 31, false },

	{ GL_COMPRESSED_RG_RGTC2, 30, false },
	{ GL_COMPRESSED_SIGNED_RG_RGTC2, 30, false },

	{ GL_R16_SNORM, 31, false },
	{ GL_R8_SNORM, 31, false },

	{ GL_COMPRESSED_RED_RGTC1, 30, false },
	{ GL_COMPRESSED_SIGNED_RED_RGTC1, 30, false },

	{ GL_DEPTH_COMPONENT32F, 30, false },
	{ GL_DEPTH_COMPONENT24, 30, false },
	{ GL_DEPTH_COMPONENT16, 30, false },

	{ GL_DEPTH32F_STENCIL8, 30, false },
	{ GL_DEPTH24_STENCIL8, 30, false },

	{ GL_NONE }
};

const struct sized_internalformat *
get_sized_internalformat(GLenum token)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(sized_internalformats); i++) {
		if (sized_internalformats[i].token == token)
			return &sized_internalformats[i];
	}

	return NULL;
}

int
get_channel_size(const struct sized_internalformat *f, enum channel c)
{
	return sized_format_bits[f->bits[c]].size;
}

GLenum
get_channel_type(const struct sized_internalformat *f, enum channel c)
{
	return sized_format_bits[f->bits[c]].type;
}

void
print_bits(int size, GLenum type)
{
	/* For compressed formats, there is no particular value for
	 * the channel size specified.
	 */
	if (size == ~0)
		printf("??");
	else
		printf("%2d", size);

	if (type == GL_FLOAT)
		printf("f ");
	else if (type == GL_INT)
		printf("i ");
	else if (type == GL_UNSIGNED_INT)
		printf("ui");
	else if (type == GL_SIGNED_NORMALIZED)
		printf("s ");
	else if (type == GL_UNSIGNED_NORMALIZED ||
		 (size == 0 && type == GL_NONE))
		printf("  ");
	else
		printf("??");
}

static bool
string_starts_with(const char *string, const char *start)
{
	return !strncmp(string, start, strlen(start));
}

bool
valid_for_gl_version(const struct required_format *format, int target_version)
{
	if (format->version > target_version)
		return false;

	/* Since we have a core context for 3.1, don't test
	 * deprecated formats there.
	 */
	if (piglit_get_gl_version() >= 31 &&
	    !piglit_is_extension_supported("GL_ARB_compatibility")) {
		const char *name = piglit_get_gl_enum_name(format->token);
		if (string_starts_with(name, "GL_ALPHA") ||
		    string_starts_with(name, "GL_LUMINANCE_ALPHA") ||
		    string_starts_with(name, "GL_LUMINANCE") ||
		    string_starts_with(name, "GL_INTENSITY")) {
			return false;
		}
	}

	return true;
}

static void
usage(const char *name)
{
	fprintf(stderr, "usage: %s <30 | 31 | 33 | 42>\n", name);
	piglit_report_result(PIGLIT_FAIL);
}

/**
 * Sets up the test config for the 3 required size tests across GL
 * compat/core versions.
 */
void
setup_required_size_test(int argc, char **argv,
			 struct piglit_gl_test_config *config)
{
	int target_version;

	if (argc < 2)
		usage(argv[0]);

	target_version = strtol(argv[1], NULL, 0);

	switch (target_version) {
	case 30:
		config->supports_gl_compat_version = 30;
		break;
	case 31:
	case 33:
	case 42:
		config->supports_gl_core_version = target_version;
		break;
	default:
		usage(argv[0]);
	}

	config->window_width = 32;
	config->window_height = 32;
	config->window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

}
