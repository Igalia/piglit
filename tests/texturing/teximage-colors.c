/*
 * Copyright 2014 Intel Corporation
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

#define BENCHMARK_ITERATIONS 1000

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 13;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

struct texture_format {
	GLenum internal_format;
	const char *name;
	GLenum format;
	GLenum data_type;
};

struct texture_format formats[] = {
#define FORMAT(IF, F, D) { IF, #IF, F, D }
	FORMAT(GL_RED, GL_RED, GL_NONE),
	FORMAT(GL_R8, GL_RED, GL_UNSIGNED_BYTE),
	FORMAT(GL_R8_SNORM, GL_RED, GL_BYTE),
	FORMAT(GL_R16, GL_RED, GL_UNSIGNED_SHORT),
	FORMAT(GL_R16_SNORM, GL_RED, GL_SHORT),
	FORMAT(GL_R16F, GL_RED, GL_NONE),
	FORMAT(GL_R32F, GL_RED, GL_FLOAT),

	FORMAT(GL_RG, GL_RG, GL_NONE),
	FORMAT(GL_RG8, GL_RG, GL_UNSIGNED_BYTE),
	FORMAT(GL_RG8_SNORM, GL_RG, GL_BYTE),
	FORMAT(GL_RG16, GL_RG, GL_UNSIGNED_SHORT),
	FORMAT(GL_RG16_SNORM, GL_RG, GL_SHORT),
	FORMAT(GL_RG16F, GL_RG, GL_NONE),
	FORMAT(GL_RG32F, GL_RG, GL_FLOAT),

	FORMAT(GL_RGB, GL_RGB, GL_NONE),
	FORMAT(GL_R3_G3_B2, GL_RGB, GL_UNSIGNED_BYTE_3_3_2),
	FORMAT(GL_RGB4, GL_RGB, GL_NONE),
	FORMAT(GL_RGB5, GL_RGB, GL_NONE),
	FORMAT(GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE),
	FORMAT(GL_RGB8_SNORM, GL_RGB, GL_BYTE),
	FORMAT(GL_SRGB8, GL_RGB, GL_UNSIGNED_BYTE),
	FORMAT(GL_RGB10, GL_RGB, GL_NONE),
	FORMAT(GL_R11F_G11F_B10F, GL_RGB, GL_NONE),
	FORMAT(GL_RGB12, GL_RGB, GL_NONE),
	FORMAT(GL_RGB9_E5, GL_RGB, GL_NONE),
	FORMAT(GL_RGB16, GL_RGB, GL_UNSIGNED_SHORT),
	FORMAT(GL_RGB16F, GL_RGB, GL_NONE),
	FORMAT(GL_RGB16_SNORM, GL_RGB, GL_SHORT),
	FORMAT(GL_RGB32F, GL_RGB, GL_FLOAT),

	FORMAT(GL_RGBA, GL_RGBA, GL_NONE),
	FORMAT(GL_RGBA2, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4),
	FORMAT(GL_RGBA4, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4),
	FORMAT(GL_RGB5_A1, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1),
	FORMAT(GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE),
	FORMAT(GL_RGB10_A2, GL_RGBA, GL_UNSIGNED_INT_10_10_10_2),
	FORMAT(GL_RGBA8_SNORM, GL_RGBA, GL_BYTE),
	FORMAT(GL_SRGB8_ALPHA8, GL_RGBA, GL_UNSIGNED_BYTE),
	FORMAT(GL_RGBA12, GL_RGBA, GL_NONE),
	FORMAT(GL_RGBA16, GL_RGBA, GL_UNSIGNED_SHORT),
	FORMAT(GL_RGBA16_SNORM, GL_RGBA, GL_SHORT),
	FORMAT(GL_RGBA32F, GL_RGBA, GL_FLOAT),

	FORMAT(GL_ALPHA, GL_ALPHA, GL_NONE),
	FORMAT(GL_ALPHA4, GL_ALPHA, GL_NONE),
	FORMAT(GL_ALPHA8, GL_ALPHA, GL_UNSIGNED_BYTE),
	FORMAT(GL_ALPHA12, GL_ALPHA, GL_NONE),
	FORMAT(GL_ALPHA16, GL_ALPHA, GL_UNSIGNED_SHORT),

	FORMAT(GL_LUMINANCE, GL_LUMINANCE, GL_NONE),
	FORMAT(GL_LUMINANCE4, GL_LUMINANCE, GL_NONE),
	FORMAT(GL_LUMINANCE8, GL_LUMINANCE, GL_UNSIGNED_BYTE),
	FORMAT(GL_SLUMINANCE8, GL_LUMINANCE, GL_UNSIGNED_BYTE),
	FORMAT(GL_LUMINANCE12, GL_LUMINANCE, GL_NONE),
	FORMAT(GL_LUMINANCE16, GL_LUMINANCE, GL_UNSIGNED_SHORT),

	FORMAT(GL_LUMINANCE_ALPHA, GL_LUMINANCE_ALPHA, GL_NONE),
	FORMAT(GL_LUMINANCE4_ALPHA4, GL_LUMINANCE_ALPHA, GL_NONE),
	FORMAT(GL_LUMINANCE6_ALPHA2, GL_LUMINANCE_ALPHA, GL_NONE),
	FORMAT(GL_LUMINANCE8_ALPHA8, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE),
	FORMAT(GL_SLUMINANCE8_ALPHA8, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE),
	FORMAT(GL_LUMINANCE12_ALPHA4, GL_LUMINANCE_ALPHA, GL_NONE),
	FORMAT(GL_LUMINANCE12_ALPHA12, GL_LUMINANCE_ALPHA, GL_NONE),
	FORMAT(GL_LUMINANCE16_ALPHA16, GL_LUMINANCE_ALPHA, GL_UNSIGNED_SHORT),
#undef FORMAT
};

#define ARRAY_LENGTH(arr) (sizeof(arr) / sizeof(*arr))

static struct texture_format *
find_format(const char *name)
{
	int i;

	for (i = 0; i < ARRAY_LENGTH(formats); ++i)
		if (strcmp(name, formats[i].name) == 0)
			return &formats[i];

	return NULL;
}

static const GLenum gl_formats[] = {
	GL_RED,
	GL_GREEN,
	GL_BLUE,
	GL_ALPHA,
	GL_RG,
	GL_RGB,
	GL_BGR,
	GL_RGBA,
	GL_BGRA,
	GL_ABGR_EXT,
//	GL_INTENSITY,
	GL_LUMINANCE,
	GL_LUMINANCE_ALPHA,
};

static const GLenum gl_types[] = {
	GL_UNSIGNED_BYTE_3_3_2,
	GL_UNSIGNED_BYTE_2_3_3_REV,
	GL_UNSIGNED_SHORT_5_6_5,
	GL_UNSIGNED_SHORT_5_6_5_REV,
	GL_UNSIGNED_SHORT_4_4_4_4,
	GL_UNSIGNED_SHORT_4_4_4_4_REV,
	GL_UNSIGNED_SHORT_5_5_5_1,
	GL_UNSIGNED_SHORT_1_5_5_5_REV,
	GL_UNSIGNED_INT_10_10_10_2,
	GL_UNSIGNED_INT_2_10_10_10_REV,
	GL_UNSIGNED_INT_8_8_8_8,
	GL_UNSIGNED_INT_8_8_8_8_REV,
	GL_BYTE,
	GL_UNSIGNED_BYTE,
	GL_SHORT,
	GL_UNSIGNED_SHORT,
	GL_FLOAT,
	GL_INT,
	GL_UNSIGNED_INT,
};

static bool
valid_combination(GLenum format, GLenum type)
{
	switch (type) {
	case GL_UNSIGNED_BYTE_3_3_2:
	case GL_UNSIGNED_BYTE_2_3_3_REV:
	case GL_UNSIGNED_SHORT_5_6_5:
	case GL_UNSIGNED_SHORT_5_6_5_REV:
		return format == GL_RGB;
	case GL_UNSIGNED_SHORT_4_4_4_4:
	case GL_UNSIGNED_SHORT_4_4_4_4_REV:
	case GL_UNSIGNED_SHORT_5_5_5_1:
	case GL_UNSIGNED_SHORT_1_5_5_5_REV:
	case GL_UNSIGNED_INT_10_10_10_2:
	case GL_UNSIGNED_INT_2_10_10_10_REV:
	case GL_UNSIGNED_INT_8_8_8_8:
	case GL_UNSIGNED_INT_8_8_8_8_REV:
		return format == GL_RGBA || format == GL_BGRA;
	default:
		return true;
	}
}

static float
un_to_float(unsigned char bits, unsigned int color)
{
	unsigned int max = ~0u >> (sizeof(max) * 8 - bits);
	return (float)color / (float)max;
}

static float
sn_to_float(unsigned char bits, int color)
{
	int max = ~(~0 << (bits-1));
	if (color < -max)
		color = -max;
	return (float)color / (float)max;
}

static float
srgb_to_linear(float s)
{
	if (s > 0.0405)
		return pow((s + 0.055) / 1.055, 2.4);
	else
		return s / 12.92;
}

#define UNPACK(val, len, off) ((val) >> off) & ~(~0ul << len)

static void
to_float(void *data, int num_chan, GLenum type, float *out)
{
	int i;

	switch (type) {
	case GL_UNSIGNED_BYTE_3_3_2:
		assert(num_chan == 3);
		out[0] = un_to_float(3, UNPACK(*(GLubyte *)data, 3, 5));
		out[1] = un_to_float(3, UNPACK(*(GLubyte *)data, 3, 2));
		out[2] = un_to_float(2, UNPACK(*(GLubyte *)data, 2, 0));
		break;
	case GL_UNSIGNED_BYTE_2_3_3_REV:
		assert(num_chan == 3);
		out[0] = un_to_float(3, UNPACK(*(GLubyte *)data, 3, 0));
		out[1] = un_to_float(3, UNPACK(*(GLubyte *)data, 3, 3));
		out[2] = un_to_float(2, UNPACK(*(GLubyte *)data, 2, 6));
		break;
	case GL_UNSIGNED_SHORT_5_6_5:
		assert(num_chan == 3);
		out[0] = un_to_float(5, UNPACK(*(GLushort *)data, 5, 11));
		out[1] = un_to_float(6, UNPACK(*(GLushort *)data, 6, 5));
		out[2] = un_to_float(5, UNPACK(*(GLushort *)data, 5, 0));
		break;
	case GL_UNSIGNED_SHORT_5_6_5_REV:
		assert(num_chan == 3);
		out[0] = un_to_float(5, UNPACK(*(GLushort *)data, 5, 0));
		out[1] = un_to_float(6, UNPACK(*(GLushort *)data, 6, 5));
		out[2] = un_to_float(5, UNPACK(*(GLushort *)data, 5, 11));
		break;
	case GL_UNSIGNED_SHORT_4_4_4_4:
		assert(num_chan == 4);
		out[0] = un_to_float(4, UNPACK(*(GLushort *)data, 4, 12));
		out[1] = un_to_float(4, UNPACK(*(GLushort *)data, 4, 8));
		out[2] = un_to_float(4, UNPACK(*(GLushort *)data, 4, 4));
		out[3] = un_to_float(4, UNPACK(*(GLushort *)data, 4, 0));
		break;
	case GL_UNSIGNED_SHORT_4_4_4_4_REV:
		assert(num_chan == 4);
		out[0] = un_to_float(4, UNPACK(*(GLushort *)data, 4, 0));
		out[1] = un_to_float(4, UNPACK(*(GLushort *)data, 4, 4));
		out[2] = un_to_float(4, UNPACK(*(GLushort *)data, 4, 8));
		out[3] = un_to_float(4, UNPACK(*(GLushort *)data, 4, 12));
		break;
	case GL_UNSIGNED_SHORT_5_5_5_1:
		assert(num_chan == 4);
		out[0] = un_to_float(5, UNPACK(*(GLushort *)data, 5, 11));
		out[1] = un_to_float(5, UNPACK(*(GLushort *)data, 5, 6));
		out[2] = un_to_float(5, UNPACK(*(GLushort *)data, 5, 1));
		out[3] = un_to_float(1, UNPACK(*(GLushort *)data, 1, 0));
		break;
	case GL_UNSIGNED_SHORT_1_5_5_5_REV:
		assert(num_chan == 4);
		out[0] = un_to_float(5, UNPACK(*(GLushort *)data, 5, 0));
		out[1] = un_to_float(5, UNPACK(*(GLushort *)data, 5, 5));
		out[2] = un_to_float(5, UNPACK(*(GLushort *)data, 5, 10));
		out[3] = un_to_float(1, UNPACK(*(GLushort *)data, 1, 15));
		break;
	case GL_UNSIGNED_INT_10_10_10_2:
		assert(num_chan == 4);
		out[0] = un_to_float(10, UNPACK(*(GLuint *)data, 10, 22));
		out[1] = un_to_float(10, UNPACK(*(GLuint *)data, 10, 12));
		out[2] = un_to_float(10, UNPACK(*(GLuint *)data, 10, 2));
		out[3] = un_to_float(2, UNPACK(*(GLuint *)data, 2, 0));
		break;
	case GL_UNSIGNED_INT_2_10_10_10_REV:
		assert(num_chan == 4);
		out[0] = un_to_float(10, UNPACK(*(GLuint *)data, 10, 0));
		out[1] = un_to_float(10, UNPACK(*(GLuint *)data, 10, 10));
		out[2] = un_to_float(10, UNPACK(*(GLuint *)data, 10, 20));
		out[3] = un_to_float(2, UNPACK(*(GLuint *)data, 2, 30));
		break;
	case GL_UNSIGNED_INT_8_8_8_8:
		assert(num_chan == 4);
		out[0] = un_to_float(8, UNPACK(*(GLuint *)data, 8, 24));
		out[1] = un_to_float(8, UNPACK(*(GLuint *)data, 8, 16));
		out[2] = un_to_float(8, UNPACK(*(GLuint *)data, 8, 8));
		out[3] = un_to_float(8, UNPACK(*(GLuint *)data, 8, 0));
		break;
	case GL_UNSIGNED_INT_8_8_8_8_REV:
		assert(num_chan == 4);
		out[0] = un_to_float(8, UNPACK(*(GLuint *)data, 8, 0));
		out[1] = un_to_float(8, UNPACK(*(GLuint *)data, 8, 8));
		out[2] = un_to_float(8, UNPACK(*(GLuint *)data, 8, 16));
		out[3] = un_to_float(8, UNPACK(*(GLuint *)data, 8, 24));
		break;
	case GL_BYTE:
		for (i = 0; i < num_chan; ++i)
			out[i] = sn_to_float(8, ((GLbyte *)data)[i]);
		break;
	case GL_UNSIGNED_BYTE:
		for (i = 0; i < num_chan; ++i)
			out[i] = un_to_float(8, ((GLubyte *)data)[i]);
		break;
	case GL_SHORT:
		for (i = 0; i < num_chan; ++i)
			out[i] = sn_to_float(16, ((GLshort *)data)[i]);
		break;
	case GL_UNSIGNED_SHORT:
		for (i = 0; i < num_chan; ++i)
			out[i] = un_to_float(16, ((GLushort *)data)[i]);
		break;
	case GL_FLOAT:
		for (i = 0; i < num_chan; ++i)
			out[i] = ((float *)data)[i];
		break;
	case GL_INT:
		for (i = 0; i < num_chan; ++i)
			out[i] = sn_to_float(32, ((GLint *)data)[i]);
		break;
	case GL_UNSIGNED_INT:
		for (i = 0; i < num_chan; ++i)
			out[i] = un_to_float(32, ((GLuint *)data)[i]);
		break;
	default:
		assert(!"Invalid type");
	}
}

static bool
is_format_signed(GLenum format)
{
	switch (format) {
	case GL_R8_SNORM:
	case GL_R16_SNORM:
	case GL_R32F:
	case GL_RG8_SNORM:
	case GL_RG16_SNORM:
	case GL_RG32F:
	case GL_RGB8_SNORM:
	case GL_RGB16_SNORM:
	case GL_RGB32F:
	case GL_RGBA8_SNORM:
	case GL_RGBA16_SNORM:
	case GL_RGBA32F:
		return true;
	default:
		return false;
	}
}

static bool
is_format_srgb(GLenum format)
{
	switch (format) {
	case GL_SRGB:
	case GL_SRGB8:
	case GL_SRGB_ALPHA:
	case GL_SRGB8_ALPHA8:
	case GL_SLUMINANCE:
	case GL_SLUMINANCE8:
	case GL_SLUMINANCE_ALPHA:
	case GL_SLUMINANCE8_ALPHA8:
		return true;
	default:
		return false;
	}
}

static int
num_channels(GLenum format)
{
	switch (format) {
	case GL_RED:
	case GL_GREEN:
	case GL_BLUE:
	case GL_ALPHA:
	case GL_INTENSITY:
	case GL_LUMINANCE:
		return 1;
	case GL_RG:
	case GL_LUMINANCE_ALPHA:
		return 2;
	case GL_RGB:
	case GL_BGR:
		return 3;
	case GL_RGBA:
	case GL_BGRA:
	case GL_ABGR_EXT:
		return 4;
	default:
		assert(!"Invalid format");
		return 0;
	}
}

static int
bytes_per_pixel(GLenum format, GLenum type)
{
	int channels = num_channels(format);

	switch (type) {
	case GL_UNSIGNED_BYTE_3_3_2:
	case GL_UNSIGNED_BYTE_2_3_3_REV:
		assert(channels == 3);
		return 1;
	case GL_UNSIGNED_SHORT_5_6_5:
	case GL_UNSIGNED_SHORT_5_6_5_REV:
		assert(channels == 3);
		return 2;
	case GL_UNSIGNED_SHORT_4_4_4_4:
	case GL_UNSIGNED_SHORT_4_4_4_4_REV:
	case GL_UNSIGNED_SHORT_5_5_5_1:
	case GL_UNSIGNED_SHORT_1_5_5_5_REV:
		assert(channels == 4);
		return 2;
	case GL_UNSIGNED_INT_10_10_10_2:
	case GL_UNSIGNED_INT_2_10_10_10_REV:
	case GL_UNSIGNED_INT_8_8_8_8:
	case GL_UNSIGNED_INT_8_8_8_8_REV:
		assert(channels == 4);
		return 4;
	case GL_BYTE:
	case GL_UNSIGNED_BYTE:
		return channels;
	case GL_SHORT:
	case GL_UNSIGNED_SHORT:
		return channels * 2;
	case GL_FLOAT:
	case GL_INT:
	case GL_UNSIGNED_INT:
		return channels * 4;
	default:
		assert(!"Invalid type");
		return 0;
	}
}

static const char *frag_shader_unsigned_src =
"uniform sampler2D tex; \n"
"void main() \n"
"{ \n"
"	gl_FragColor = texture2D(tex, gl_TexCoord[0].xy);\n"
"} \n";

static const char *frag_shader_signed_src =
"uniform sampler2D tex; \n"
"void main() \n"
"{ \n"
"	gl_FragColor = 0.5 + 0.5 * texture2D(tex, gl_TexCoord[0].xy);\n"
"} \n";

int texture_size = 31;
struct texture_format *format = NULL;
GLuint unsigned_prog, signed_prog;
void *rand_data;
float tolerance[4];
bool benchmark = false;

void
piglit_init(int argc, char **argv)
{
	int i, seed = 0;

	for (i = 1; i < argc; ++i) {
		if (sscanf(argv[i], "--seed=%d", &seed) > 0) {
			srand(seed);
		} else if (strcmp(argv[i], "--benchmark") == 0) {
			benchmark = true;
			texture_size = 128;
		} else if (i == argc - 1) {
			format = find_format(argv[i]);
			break;
		}
	}

	if (argc < 1) {
		printf("usage: texstore-colors [--seed=seed] [--benchmark] format");
		exit(1);
	}

	piglit_require_extension("GL_EXT_texture_integer");

	assert(format);

	signed_prog = piglit_build_simple_program(NULL, frag_shader_signed_src);
	unsigned_prog = piglit_build_simple_program(NULL, frag_shader_unsigned_src);

	srand(seed);
	rand_data = malloc(texture_size * texture_size * 128);
	for (i = 0; i < texture_size * texture_size * 128; ++i)
		((GLubyte *)rand_data)[i] = rand();

	if (is_format_srgb(format->internal_format)) {
		/* We loose a little precision in the high numbers */
		tolerance[0] = 0.02;
		tolerance[1] = 0.02;
		tolerance[2] = 0.02;
		tolerance[3] = 0.02;
	} else {
		tolerance[0] = 0.01;
		tolerance[1] = 0.01;
		tolerance[2] = 0.01;
		tolerance[3] = 0.01;
	}

	if (format->internal_format == GL_R11F_G11F_B10F) {
		tolerance[0] = 0.3;
		tolerance[1] = 0.3;
		tolerance[2] = 0.3;
	}

	switch (format->data_type) {
	case GL_UNSIGNED_BYTE_3_3_2:
	case GL_UNSIGNED_BYTE_2_3_3_REV:
		tolerance[0] = 0.3;
		tolerance[1] = 0.3;
		tolerance[2] = 0.3;
		break;
	case GL_UNSIGNED_SHORT_5_5_5_1:
	case GL_UNSIGNED_SHORT_1_5_5_5_REV:
		tolerance[3] = 0.6;
		/* Fall through. */
	case GL_UNSIGNED_SHORT_5_6_5:
	case GL_UNSIGNED_SHORT_5_6_5_REV:
		tolerance[0] = 0.05;
		tolerance[1] = 0.05;
		tolerance[2] = 0.05;
		break;
	case GL_UNSIGNED_SHORT_4_4_4_4:
	case GL_UNSIGNED_SHORT_4_4_4_4_REV:
		tolerance[0] = 0.1;
		tolerance[1] = 0.1;
		tolerance[2] = 0.1;
		tolerance[3] = 0.1;
		break;
	case GL_UNSIGNED_INT_10_10_10_2:
	case GL_UNSIGNED_INT_2_10_10_10_REV:
		tolerance[3] = 0.3;
		break;
	}

	/*
	 * The tolerance lowering above only works for formats which have
	 * explicit data types associated with them and even then it's fishy
	 * for some.
	 * The default sort of assumes at least 7 bits which doesn't make
	 * much sense in any case (for the specific formats with more bits).
	 * But just fix the cases which cannot pass (unless the driver encodes
	 * them with more bits).
	 */
	switch (format->internal_format) {
	case GL_RGB4:
		tolerance[0] = 0.1;
		tolerance[1] = 0.1;
		tolerance[2] = 0.1;
		tolerance[3] = 0.1;
		break;
	case GL_RGB5:
		tolerance[0] = 0.05;
		tolerance[1] = 0.05;
		tolerance[2] = 0.05;
		break;
	case GL_LUMINANCE4_ALPHA4:
		tolerance[0] = 0.1;
		tolerance[1] = 0.1;
		tolerance[2] = 0.1;
		tolerance[3] = 0.1;
		break;
	case GL_LUMINANCE6_ALPHA2: /* broken but everybody uses 8+8 bits */
	case GL_LUMINANCE4: /* broken but presumably noone uses just 4 bits */
	case GL_ALPHA4: /* broken but presumably noone uses just 4 bits */
	case GL_RGBA2: /* broken (4444) but everybody uses more bits anyway */
	default:
		break;
	}
}

void
to_expected(GLenum test_format, GLenum test_type, void *up_raw, float *expected)
{
	float up_rgba[4];
	int num_chan = num_channels(test_format);

	to_float(up_raw, num_chan, test_type, up_rgba);

	expected[0] = 0.0f;
	expected[1] = 0.0f;
	expected[2] = 0.0f;
	expected[3] = 1.0f;

	switch (test_format) {
	case GL_RED:
		expected[0] = up_rgba[0];
		break;
	case GL_GREEN:
		expected[1] = up_rgba[0];
		break;
	case GL_BLUE:
		expected[2] = up_rgba[0];
		break;
	case GL_ALPHA:
		expected[3] = up_rgba[0];
		break;
	case GL_RG:
		expected[0] = up_rgba[0];
		expected[1] = up_rgba[1];
		break;
	case GL_RGBA:
		expected[3] = up_rgba[3];
	case GL_RGB:
		expected[0] = up_rgba[0];
		expected[1] = up_rgba[1];
		expected[2] = up_rgba[2];
		break;
	case GL_BGRA:
		expected[3] = up_rgba[3];
	case GL_BGR:
		expected[0] = up_rgba[2];
		expected[1] = up_rgba[1];
		expected[2] = up_rgba[0];
		break;
	case GL_ABGR_EXT:
		expected[0] = up_rgba[3];
		expected[1] = up_rgba[2];
		expected[2] = up_rgba[1];
		expected[3] = up_rgba[0];
		break;
	case GL_INTENSITY:
		expected[0] = up_rgba[0];
		expected[1] = up_rgba[0];
		expected[2] = up_rgba[0];
		expected[3] = up_rgba[0];
		break;
	case GL_LUMINANCE_ALPHA:
		expected[3] = up_rgba[1];
	case GL_LUMINANCE:
		expected[0] = up_rgba[0];
		expected[1] = up_rgba[0];
		expected[2] = up_rgba[0];
		break;
	default:
		assert(!"Invalid color format");
	}

	switch (format->format) {
	case GL_RED:
	case GL_RED_INTEGER:
		expected[1] = 0.0f;
	case GL_RG:
	case GL_RG_INTEGER:
		expected[2] = 0.0f;
	case GL_RGB:
	case GL_RGB_INTEGER:
		expected[3] = 1.0f;
		break;
	case GL_RGBA:
	case GL_RGBA_INTEGER:
		break;
	case GL_ALPHA:
		expected[0] = 0.0f;
		expected[1] = 0.0f;
		expected[2] = 0.0f;
		break;
	case GL_LUMINANCE:
		expected[3] = 1.0f;
	case GL_LUMINANCE_ALPHA:
		expected[1] = expected[0];
		expected[2] = expected[0];
		break;
	default:
		assert(!"Invalid color format");
	}

	if (!is_format_signed(format->internal_format)) {
		if (expected[0] < 0.0f)
			expected[0] = 0.0f;
		if (expected[1] < 0.0f)
			expected[1] = 0.0f;
		if (expected[2] < 0.0f)
			expected[2] = 0.0f;
		if (expected[3] < 0.0f)
			expected[3] = 0.0f;
	}

	if (is_format_srgb(format->internal_format)) {
		expected[0] = srgb_to_linear(expected[0]);
		expected[1] = srgb_to_linear(expected[1]);
		expected[2] = srgb_to_linear(expected[2]);
	}
}

enum piglit_result
run_test(GLenum test_format, GLenum test_type, float *time_out)
{
	bool pass = true;
	int64_t time;
	GLuint tex;
	int i, Bpp, channels;
	float *tmp, *expected, *observed;
	void *data;

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	channels = num_channels(test_format);
	Bpp = bytes_per_pixel(test_format, test_type);

	if (test_type == GL_FLOAT) {
		/* Sanatize so we don't get invalid floating point values */
		tmp = malloc(texture_size * texture_size * channels * sizeof(float));
		for (i = 0; i < texture_size * texture_size * channels; ++i)
			tmp[i] = sn_to_float(32, ((GLint *)rand_data)[i]);
		data = tmp;
	} else {
		tmp = NULL;
		data = rand_data;
	}

	expected = malloc(texture_size * texture_size * 4 * sizeof(float));
	for (i = 0; i < texture_size * texture_size; ++i)
		to_expected(test_format, test_type, (GLubyte *)data + (i * Bpp),
			    expected + 4 * i);

	if (benchmark) {
		time = piglit_time_get_nano();
		for (i = 0; i < BENCHMARK_ITERATIONS; ++i)
			glTexImage2D(GL_TEXTURE_2D, 0, format->internal_format,
				     texture_size, texture_size, 0,
				     test_format, test_type, data);
		time = piglit_time_get_nano() - time;
		*time_out = (double)time / (double)(BENCHMARK_ITERATIONS*1000);
	} else {
		glTexImage2D(GL_TEXTURE_2D, 0, format->internal_format,
			     texture_size, texture_size, 0,
			     test_format, test_type, data);
	}
	pass &= piglit_check_gl_error(GL_NO_ERROR);

	if (is_format_signed(format->internal_format)) {
		glUseProgram(signed_prog);

		for (i = 0; i < texture_size * texture_size * 4; ++i)
			expected[i] = 0.5 + 0.5 * expected[i];
	} else {
		glUseProgram(unsigned_prog);
	}

	piglit_draw_rect_tex(0, 0, texture_size, texture_size, 0, 0, 1, 1);

	observed = malloc(texture_size * texture_size * 4 * sizeof(float));
	glReadPixels(0, 0, texture_size, texture_size,
		     GL_RGBA, GL_FLOAT, observed);
	pass &= piglit_check_gl_error(GL_NO_ERROR);

	pass &= piglit_compare_images_color(0, 0, texture_size, texture_size, 4,
					    tolerance, expected, observed);

	free(observed);
	free(expected);
	free(tmp);

	piglit_report_subtest_result(pass ? PIGLIT_PASS : PIGLIT_FAIL,
				     "%s texture with %s and %s",
				     piglit_get_gl_enum_name(format->internal_format),
				     piglit_get_gl_enum_name(test_format),
				     piglit_get_gl_enum_name(test_type));

	glDeleteTextures(1, &tex);

	return pass;
}

bool
test_exact()
{
	int i, Bpp, channels;
	float *tmp_float;
	GLubyte *data, *observed;
	GLint tex_width, tex_height;
	bool pass = true;

	if (format->data_type == GL_NONE) {
		piglit_report_subtest_result(PIGLIT_SKIP,
					     "Exact upload-download of %s",
					     piglit_get_gl_enum_name(format->internal_format));
		return true;
	}

	channels = num_channels(format->format);
	Bpp = bytes_per_pixel(format->format, format->data_type);

	if (format->data_type == GL_FLOAT) {
		/* Sanatize so we don't get invalid floating point values */
		tmp_float = malloc(texture_size * texture_size *
				   channels * sizeof(float));
		for (i = 0; i < texture_size * texture_size * channels; ++i)
			tmp_float[i] = sn_to_float(32, ((GLint *)rand_data)[i]);
		data = (GLubyte *)tmp_float;
	} else {
		tmp_float = NULL;
		data = rand_data;
	}

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, format->internal_format,
		     texture_size, texture_size, 0, format->format,
		     format->data_type, data);
	pass &= piglit_check_gl_error(GL_NO_ERROR);

	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &tex_width);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &tex_height);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	observed = malloc(tex_width * tex_height * Bpp);

	glGetTexImage(GL_TEXTURE_2D, 0, format->format, format->data_type,
		      observed);
	pass &= piglit_check_gl_error(GL_NO_ERROR);

	for (i = 0; i < texture_size; ++i)
		pass &= memcmp(&data[i * texture_size * Bpp],
			       &observed[i * tex_width * Bpp],
			       texture_size * Bpp) == 0;

	free(observed);
	free(tmp_float);

	piglit_report_subtest_result(pass ? PIGLIT_PASS : PIGLIT_FAIL,
				     "Exact upload-download of %s",
				     piglit_get_gl_enum_name(format->internal_format));

	return pass;
}

enum piglit_result
piglit_display(void)
{
	bool warn = false, pass = true;
	GLuint rb, fbo;
	int i, j;
	float times[ARRAY_LENGTH(gl_formats)][ARRAY_LENGTH(gl_types)];

	glGenRenderbuffersEXT(1, &rb);
	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, rb);
	glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT,
				 GL_RGBA, texture_size, texture_size);
	pass &= piglit_check_gl_error(GL_NO_ERROR);

	glGenFramebuffersEXT(1, &fbo);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
	glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT,
				     GL_COLOR_ATTACHMENT0_EXT,
				     GL_RENDERBUFFER_EXT, rb);
	pass &= piglit_check_gl_error(GL_NO_ERROR);

	/* Set up basic GL stuff */
	glEnable(GL_TEXTURE_2D);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	warn = !test_exact();

	for (i = 0; i < ARRAY_LENGTH(gl_formats); ++i) {
		for (j = 0; j < ARRAY_LENGTH(gl_types); ++j) {
			if (!valid_combination(gl_formats[i], gl_types[j]))
				continue;

			pass &= run_test(gl_formats[i], gl_types[j],
					 &times[i][j]);
		}
	}

	glDisable(GL_TEXTURE_2D);
	glDeleteFramebuffers(1, &fbo);
	glDeleteRenderbuffers(1, &rb);

	if (benchmark) {
		fprintf(stdout, "internalFormat, format, type, time (us/call)\n");
		for (i = 0; i < ARRAY_LENGTH(gl_formats); ++i) {
			for (j = 0; j < ARRAY_LENGTH(gl_types); ++j) {
				if (!valid_combination(gl_formats[i], gl_types[j]))
					continue;

				fprintf(stdout, "%s, %s, %s, %.3f\n",
					piglit_get_gl_enum_name(format->internal_format),
					piglit_get_gl_enum_name(gl_formats[i]),
					piglit_get_gl_enum_name(gl_types[j]),
					times[i][j]);
			}
		}
	}

	if (pass) {
		return warn ? PIGLIT_WARN : PIGLIT_PASS;
	} else {
		return PIGLIT_FAIL;
	}
}
