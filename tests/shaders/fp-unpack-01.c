/*
 * Copyright © 2009 Intel Corporation
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

/**
 * \file fp-unpack-01.c
 * Validate the four unpack instructions in GL_NV_fragment_program_option.
 *
 * \author Ian Romanick <ian.d.romanick@intel.com>
 */

#include "piglit-util-gl.h"

/* There are 128 possible values.  These values a distributed into 3 color
 * components.  Ensure that all of the values are seen at least once.
 */
#define TEST_COLS  ((128 / 3) + 1)

/* One for the reference square and each of the 4 unpack instructions
 */
#define TEST_ROWS  5

#define BOX_SIZE   16

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = (((BOX_SIZE+1)*TEST_COLS)+1);
	config.window_height = (((BOX_SIZE+1)*TEST_ROWS)+1);
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static char shader_source[64 * 1024];
static GLfloat colors[TEST_COLS][4];

union uif {
	float f;
	unsigned int ui;
};

static const GLenum types[4] = {
	GL_BYTE,
	GL_UNSIGNED_BYTE,
	GL_UNSIGNED_SHORT,
	GL_HALF_FLOAT
};


static const char *const opcodes[4] = {
	"UP4B", "UP4UB", "UP2US", "UP2H"
};


/**
 * Source for the fragment program to render the reference box.
 */
static const char reference_shader_source[] =
	"!!ARBfp1.0\n"
	"MOV	result.color, program.env[0];\n"
	"END"
	;

/**
 * \name Handles to fragment programs.
 */
/*@{*/
static GLint reference_prog;
static GLint progs[ARRAY_SIZE(types)];
/*@}*/


void
generate_shader(GLenum type)
{
	unsigned len;
	char *swiz1;
	char *swiz2 = NULL;
	char *inst;

	len = sprintf(& shader_source[0],
		       "!!ARBfp1.0\n"
		       "OPTION	NV_fragment_program;\n"
		       "TEMP	R0;\n"
		       "\n");

	switch (type) {
	case GL_HALF_FLOAT:
		inst = "UP2H";
		swiz1 = "xy";
		swiz2 = "zw";
		break;
	case GL_UNSIGNED_SHORT:
		inst = "UP2US";
		swiz1 = "xy";
		swiz2 = "zw";
		break;
	case GL_UNSIGNED_BYTE:
		inst = "UP4UB";
		swiz1 = "xyzw";
		break;
	case GL_BYTE:
	default:
		inst = "UP4B";
		swiz1 = "xyzw";
		break;
	}

	len += sprintf(& shader_source[len],
		       "\n"
		       "# Unpack the data in fragment.color into four\n"
		       "# components of color data.\n"
		       "%s	R0.%s, program.env[0].x;\n",
		       inst, swiz1);

	if (swiz2 != NULL) {
		len += sprintf(& shader_source[len],
			       "%s	R0.%s, program.env[0].y;\n",
			       inst, swiz2);
	}

	len += sprintf(& shader_source[len],
		       "MOV	result.color, R0;\n"
		       "END\n");
}


/* Largest magnitued positive half-precision float value.
 */
#define HALF_MAX 65504.0

static GLushort
float_to_half(float f)
{
	union uif bits;
	unsigned sign;
	unsigned exponent;
	unsigned mantissa;


	/* Clamp the value to the range of values representable by a
	 * half precision float.
	 */
	bits.f = CLAMP(f, -HALF_MAX, HALF_MAX);

	sign = bits.ui & (1U << 31);
	sign >>= 16;

	/* Round denorms to zero, but keep the sign.
	 */
	exponent = bits.ui & (0x0ff << 23);
	if (exponent == 0) {
		return sign;
	}

	exponent >>= 23;
	exponent += -(127 - 15);
	exponent <<= 10;

	/* Instead of just truncating bits of the mantissa, round the value.
	 */
	mantissa = bits.ui & ((1U << 23) - 1);
	mantissa += (1U << (23 - 10)) >> 1;
	mantissa >>= (23 - 10);

	return (sign | exponent | mantissa);
}


void
pack(float *packed, const float *color, GLenum type)
{
	unsigned *p = (unsigned *) packed;
	GLubyte ub[4];
	GLushort us[4];
	unsigned i;

	packed[0] = 0.0f;
	packed[1] = 0.0f;
	packed[2] = 0.0f;
	packed[3] = 1.0f;

	switch (type) {
	case GL_HALF_FLOAT:
		for (i = 0; i < 4; i++)
			us[i] = float_to_half(color[i]);

		p[0] = (us[0]) | (us[1] << 16);
		p[1] = (us[2]) | (us[3] << 16);
		break;

	case GL_UNSIGNED_SHORT:
		for (i = 0; i < 4; i++) {
			const float tmp = CLAMP(color[i], 0.0, 1.0);
			us[i] = (GLushort) round(65535.0 * tmp);
		}

		p[0] = (us[0]) | (us[1] << 16);
		p[1] = (us[2]) | (us[3] << 16);
		break;

	case GL_UNSIGNED_BYTE:
		for (i = 0; i < 4; i++) {
			const float tmp = CLAMP(color[i], 0.0, 1.0);
			ub[i] = (GLubyte) round(255.0 * tmp);
		}

		p[0] = (ub[0]) | (ub[1] << 8) | (ub[2] << 16) | (ub[3] << 24);
		break;

	case GL_BYTE:
		for (i = 0; i < 4; i++) {
			const float tmp =
				CLAMP(color[i], -(128.0 / 127.0), 1.0);
			ub[i] = (GLubyte) round(127.0 * tmp + 128.0);
		}

		p[0] = (ub[0]) | (ub[1] << 8) | (ub[2] << 16) | (ub[3] << 24);
		break;
	}
}


enum piglit_result
piglit_display(void)
{
	unsigned i;
	unsigned j;
	enum piglit_result result = PIGLIT_PASS;


	glClear(GL_COLOR_BUFFER_BIT);
	glEnable(GL_FRAGMENT_PROGRAM_ARB);

	for (i = 0; i < TEST_COLS; i++) {
		const int x = (i * (BOX_SIZE + 1)) + 1;

		glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, reference_prog);
		glProgramEnvParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB,
					    0, colors[i]);
		piglit_draw_rect(x, 1, BOX_SIZE, BOX_SIZE);

		for (j = 0; j < ARRAY_SIZE(types); j++) {
			const int y = ((j + 1) * (BOX_SIZE + 1)) + 1;
			GLfloat v[4];

			pack(v, colors[i], types[j]);
			glProgramEnvParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB,
						    0, v);

			glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, progs[j]);
			piglit_draw_rect(x, y, BOX_SIZE, BOX_SIZE);

			if (!piglit_probe_pixel_rgb(x + (BOX_SIZE / 2),
						    y + (BOX_SIZE / 2),
						    colors[i])) {
				if (!piglit_automatic)
                                        printf("%s failed on color { %f %f %f %f }\n",
					       opcodes[j],
					       colors[i][0], colors[i][1],
					       colors[i][2], colors[i][3]);

				result = PIGLIT_FAIL;
			}
		}
	}

	piglit_present_results();
	return result;
}


/**
 * Shuffle values in-place using Fisher–Yates shuffle.
 */
void shuffle(float *values, unsigned count)
{
	srand(0xCAFEBEEF);
	for (/* empty */; count > 1; count--) {
		int32_t idx;
		float tmp;

		/* Generate a random index within the unshuffled portion of the
		 * array.
		 */
		idx = rand();
		idx = idx % count;

		/* Exchange the randomly selected index and the list unshuffled
		 * element in the array.
		 */
		tmp = values[idx];
		values[idx] = values[count - 1];
		values[count - 1] = tmp;
        }
}


void
piglit_init(int argc, char **argv)
{
	unsigned i;
	float v[TEST_COLS * 3 ];

	(void) argc;
	(void) argv;

	piglit_require_fragment_program();
	piglit_require_extension("GL_NV_fragment_program_option");
	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	reference_prog = piglit_compile_program(GL_FRAGMENT_PROGRAM_ARB,
						reference_shader_source);

	glClearColor(1.0, 1.0, 1.0, 1.0);

	for (i = 0; i < ARRAY_SIZE(types); i++) {
		generate_shader(types[i]);
		progs[i] = piglit_compile_program(GL_FRAGMENT_PROGRAM_ARB,
						  shader_source);
	}


	/* Generate the possible color values.
	 */
	for (i = 0; i <= 127; i++) {
		v[i] = ((float) i) / 127.0;
	}

	for (/* empty */; i < ARRAY_SIZE(v); i++) {
		v[i] = 0.5;
	}


	/* Shuffle the values into random order.  Generate the color data
	 * used by the tests from the shuffled values.
	 */
	shuffle(v, 128);
	for (i = 0; i < TEST_COLS; i++) {
		assert((i * 3) + 2 < ARRAY_SIZE(v));

		colors[i][0] = v[(i * 3) + 0];
		colors[i][1] = v[(i * 3) + 1];
		colors[i][2] = v[(i * 3) + 2];
		colors[i][3] = 1.0;
	}
}
