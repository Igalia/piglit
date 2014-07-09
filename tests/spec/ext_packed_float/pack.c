/*
 * Copyright © 2011 Intel Corporation
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

/** @file pack.c
 *
 * Tests packing of floating point values to GL_EXT_packed_float's
 * GL_UNSIGNED_INT_10F_11F_11F_REV format.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

/* Any maximum e with m != 0 is NAN */

#define PACK(r, g, b) 	((b << 22) | (g << 11) | (r))
#define GET_R(p)	((p) & 0x7ff)
#define GET_G(p)	(((p) >> 11) & 0x7ff)
#define GET_B(p)	(((p) >> 22) & 0x3ff)

/*     "An unsigned 11-bit floating-point number has no sign bit, a
 *      5-bit exponent (E), and a 6-bit mantissa (M).  The value of an
 *      unsigned 11-bit floating-point number (represented as an
 *      11-bit unsigned integer N) is determined by the following:
 *
 *          0.0,                      if E == 0 and M == 0,
 *          2^-14 * (M / 64),         if E == 0 and M != 0,
 *          2^(E-15) * (1 + M/64),    if 0 < E < 31,
 *          INF,                      if E == 31 and M == 0, or
 *          NaN,                      if E == 31 and M != 0,
 *
 *      where
 *
 *          E = floor(N / 64), and
 *          M = N mod 64.
 *
 *      Implementations are also allowed to use any of the following
 *      alternative encodings:
 *
 *          0.0,                      if E == 0 and M != 0
 *          2^(E-15) * (1 + M/64)     if E == 31 and M == 0
 *          2^(E-15) * (1 + M/64)     if E == 31 and M != 0"
 */
#define F11(e, m)	((e) << 6 | (m))

/*     "An unsigned 10-bit floating-point number has no sign bit, a
 *      5-bit exponent (E), and a 5-bit mantissa (M).  The value of an
 *      unsigned 10-bit floating-point number (represented as an
 *      10-bit unsigned integer N) is determined by the following:
 *
 *          0.0,                      if E == 0 and M == 0,
 *          2^-14 * (M / 32),         if E == 0 and M != 0,
 *          2^(E-15) * (1 + M/32),    if 0 < E < 31,
 *          INF,                      if E == 31 and M == 0, or
 *          NaN,                      if E == 31 and M != 0,
 *
 *      where
 *
 *          E = floor(N / 32), and
 *          M = N mod 32."
 */
#define F10(e, m)	((e) << 5 | (m))

enum piglit_result
piglit_display(void)
{
	/* UNREACHED */
	return PIGLIT_FAIL;
}

#define VALUE(r, g, b, p_r, p_g, p_b) { {r, g, b}, PACK(p_r, p_g, p_b) }

struct {
	float in;
	uint32_t f10;
	uint32_t f11;
} values[] = {
	{ 1.0,		F10(15, 0),	F11(15, 0) },
	{ -1.0,		F10(0, 0),	F11(0, 0) },

	/*    "Likewise, finite positive values greater than 65024
	 *     (the maximum finite representable unsigned 11-bit
	 *     floating-point value) are converted to 65024.
	 *
	 * ...
	 *
	 *     Likewise, finite positive values greater than 64512
	 *     (the maximum finite representable unsigned 10-bit
	 *     floating-point value) are converted to 64512"
	 */
	{ 1000000,	F10(30, 31),	F11(30, 63) },
	{ 65025,	F10(30, 31),	F11(30, 63) },
	{ 64513,	F10(30, 31),	F11(30, 62) },

	/*    "Additionally: negative infinity is converted to zero;
	 *     positive infinity is converted to positive infinity;
	 *     and both positive and negative NaN are converted to
	 *     positive NaN."
	 */
	{ INFINITY,	F10(31, 0),	F11(31, 0) },
	{ -INFINITY,	F10(0, 0),	F11(0, 0) },
	{ NAN,		F10(31, 1),	F11(31, 1) },
	{ -NAN,		F10(31, 1),	F11(31, 1) },
};

/* Per-pixel RGB float values. */
static float in[ARRAY_SIZE(values) * 3][3];
/* Per-pixel packed RGB float results */
static uint32_t out[ARRAY_SIZE(values) * 3];
/* Per-pixel packed RGB float values */
static uint32_t expected[ARRAY_SIZE(values) * 3];

static uint32_t *
get_packed_values()
{
	static float outf[ARRAY_SIZE(values) * 3][3];
	GLuint tex;
	int i;

	/* Set up the texture data. */
	for (i = 0; i < ARRAY_SIZE(values); i++) {
		in[i * 3 + 0][0] = values[i].in;
		in[i * 3 + 0][1] = 0.0;
		in[i * 3 + 0][2] = 0.0;
		in[i * 3 + 1][0] = 0.0;
		in[i * 3 + 1][1] = values[i].in;
		in[i * 3 + 1][2] = 0.0;
		in[i * 3 + 2][0] = 0.0;
		in[i * 3 + 2][1] = 0.0;
		in[i * 3 + 2][2] = values[i].in;

		expected[i * 3 + 0] = PACK(values[i].f11, 0, 0);
		expected[i * 3 + 1] = PACK(0, values[i].f11, 0);
		expected[i * 3 + 2] = PACK(0, 0, values[i].f10);
	}

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F,
		     1, ARRAY_SIZE(values) * 3, 0,
		     GL_RGB, GL_FLOAT, in);

	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB,
		      GL_UNSIGNED_INT_10F_11F_11F_REV, out);

	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB,
		      GL_FLOAT, outf);

	glDeleteTextures(1, &tex);

	return out;
}

static bool
equals_11(uint32_t e, uint32_t o)
{
	uint32_t e_exp = (e >> 6) & 0x1f;
	uint32_t e_man = e & 0x3f;
	uint32_t o_exp = (o >> 6) & 0x1f;
	uint32_t o_man = o & 0x3f;

	if (e_exp == 0) {
		/* Implementations are allowed to treat denorms as 0.0 */
		return o_exp == 0;
	} else if (e_exp == 31) {
		if (e_man == 0)
			return o_exp == 31 && o_man == 0;
		else
			return o_exp == 31;
	} else {
		return e == o;
	}
}

static bool
equals_10(uint32_t e, uint32_t o)
{
	uint32_t e_exp = (e >> 5) & 0x1f;
	uint32_t e_man = e & 0x1f;
	uint32_t o_exp = (o >> 5) & 0x1f;
	uint32_t o_man = o & 0x1f;

	if (e_exp == 0) {
		/* Implementations are allowed to treat denorms as 0.0 */
		return o_exp == 0;
	} else if (e_exp == 31) {
		if (e_man == 0)
			return o_exp == 31 && o_man == 0;
		else
			return o_exp == 31;
	} else {
		return e == o;
	}
}


static bool
test_output()
{
	bool pass = true;
	int i;

	for (i = 0; i < ARRAY_SIZE(values) * 3; i++) {
		uint32_t e_r = GET_R(expected[i]);
		uint32_t e_g = GET_G(expected[i]);
		uint32_t e_b = GET_B(expected[i]);
		uint32_t o_r = GET_R(out[i]);
		uint32_t o_g = GET_G(out[i]);
		uint32_t o_b = GET_B(out[i]);

		if (!equals_11(e_r, o_r) ||
		    !equals_11(e_g, o_g) ||
		    !equals_10(e_b, o_b)) {
			printf("Packed float value mismatch:\n");
			printf("  input data: %f, %f, %f\n",
			       in[i][0], in[i][1], in[i][2]);
			printf("  expected: 0x%08x (0x%03x, 0x%03x, 0x%03x)\n",
			       expected[i], e_r, e_g, e_b);
			printf("  observed: 0x%08x (0x%03x, 0x%03x, 0x%03x)\n",
			       out[i], o_r, o_g, o_b);
		}
	}

	return pass;
}

void
piglit_init(int argc, char **argv)
{
	bool pass = true;

	piglit_require_extension("GL_ARB_texture_float");
	piglit_require_extension("GL_EXT_packed_float");
	piglit_require_extension("GL_ARB_texture_non_power_of_two");

	get_packed_values();
	pass = test_output();

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
