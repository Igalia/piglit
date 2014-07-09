/*
 * Copyright (c) 2009 Nicolai Hähnle
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
 *
 * Authors:
 *    Nicolai Hähnle <nhaehnle@gmail.com>
 *
 */

/**
 * \file
 * Whereas fp-indirections tests that the native indirection limits are
 * reported essentially correctly, this test actually exercises multiple
 * indirection counts up to the reported native limit.
 */

#include "piglit-util-gl.h"

#define TEXTURE_SIZE 32 /* Note: Hardcoded dependencies in texture_init and texture_follow */

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = TEXTURE_SIZE;
	config.window_height = TEXTURE_SIZE;
	config.window_visual = PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

unsigned int max_samples;
unsigned char * texture_data;
unsigned char * texture_data_as_rgba;
GLuint texture_objects[3];


static void texture_init(void)
{
	unsigned int x, y, z;
	unsigned char *p;
	unsigned char *q;

	srand(0x12345678); /* we want repeatable test runs */

	texture_data = malloc(TEXTURE_SIZE * TEXTURE_SIZE * TEXTURE_SIZE * 3);
	texture_data_as_rgba = malloc(TEXTURE_SIZE * TEXTURE_SIZE * TEXTURE_SIZE * 4);

	p = texture_data;
	q = texture_data_as_rgba;
	for(z = 0; z < TEXTURE_SIZE; ++z) {
		for(y = 0; y < TEXTURE_SIZE; ++y) {
			for(x = 0; x < TEXTURE_SIZE; ++x) {
				unsigned int r = rand();
				p[0] = r & 31;
				p[1] = (r >> 5) & 31;
				p[2] = (r >> 10) & 31;

				q[0] = p[0] * 8 + 4;
				q[1] = p[1] * 8 + 4;
				q[2] = p[2] * 8 + 4;
				q[3] = 0xff;

				p += 3;
				q += 4;
			}
		}
	}

	glGenTextures(3, texture_objects);

	glBindTexture(GL_TEXTURE_1D, texture_objects[0]);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, TEXTURE_SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture_data_as_rgba);

	glBindTexture(GL_TEXTURE_2D, texture_objects[1]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TEXTURE_SIZE, TEXTURE_SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture_data_as_rgba);

	glBindTexture(GL_TEXTURE_3D, texture_objects[2]);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB, TEXTURE_SIZE, TEXTURE_SIZE, TEXTURE_SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture_data_as_rgba);
}

static void texture_follow(
		unsigned int dim,
		unsigned int x, unsigned int y, unsigned int z,
		unsigned int hops,
		float * expected)
{
	unsigned int i;

	for(i = 0; i < hops; ++i) {
		unsigned char * p;

		if (dim < 3)
			z = 0;
		if (dim < 2)
			y = 0;

		p = texture_data + z*TEXTURE_SIZE*TEXTURE_SIZE*3 + y*TEXTURE_SIZE*3 + x*3;
		x = p[0];
		y = p[1];
		z = p[2];
	}

	expected[0] = (x + 0.5) / 32.0;
	expected[1] = (y + 0.5) / 32.0;
	expected[2] = (z + 0.5) / 32.0;

	if (!hops)
		expected[2] = 0.0;
}

static const char program_Head[] =
	"!!ARBfp1.0\n"
	"TEMP r;\n"
	;

static const char program_TEX[] =
	"TEX %s, %s, texture[0], %iD;\n";

static const char program_MOV[] =
	"MOV %s, %s;\n";

static const char program_Tail[] =
	"END\n";

static const char program_Input[] = "fragment.texcoord[0]";
static const char program_Output[] = "result.color";

static enum piglit_result test(unsigned int dim, unsigned int samples)
{
	char * program_text = malloc(sizeof(program_Head) +
	                             (samples + 1)*sizeof(program_TEX) +
	                             sizeof(program_Tail) +
	                             sizeof(program_Input) + sizeof(program_Output));
	char buf[128];
	GLuint program_object;
	unsigned int draw_height;
	unsigned int x, y;

	strcpy(program_text, program_Head);
	if (!samples) {
		snprintf(buf, sizeof(buf), program_MOV, program_Output, program_Input);
		strcat(program_text, buf);
	} else {
		const char * input = program_Input;
		unsigned int i;
		for(i = 1; i <= samples; ++i) {
			const char * output = "r";
			if (i == samples)
				output = program_Output;
			snprintf(buf, sizeof(buf), program_TEX, output, input, dim);
			strcat(program_text, buf);
			input = output;
		}
	}
	strcat(program_text, program_Tail);

	program_object = piglit_compile_program(GL_FRAGMENT_PROGRAM_ARB, program_text);

	glEnable(GL_FRAGMENT_PROGRAM_ARB);
	glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, program_object);

	draw_height = TEXTURE_SIZE;
	piglit_draw_rect_tex(0, 0, TEXTURE_SIZE, draw_height, 0, 0, 1, 1);

	glDisable(GL_FRAGMENT_PROGRAM_ARB);
	glDeleteProgramsARB(1, &program_object);

	for(y = 0; y < draw_height; ++y) {
		for(x = 0; x < TEXTURE_SIZE; ++x) {
			float expected[3];
			texture_follow(dim, x, y, 0, samples, expected);
			if (!piglit_probe_pixel_rgb(x, y, expected)) {
				fprintf(stderr, "Failure in dim = %i, samples = %i\n", dim, samples);
				return PIGLIT_FAIL;
			}
		}
	}

	return PIGLIT_PASS;
}

enum piglit_result
piglit_display(void)
{
	enum piglit_result result;
	unsigned int dim;
	unsigned int samples;

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);
	glClear(GL_COLOR_BUFFER_BIT);

	for(dim = 1; dim <= 3; ++dim) {
		samples = 0;
		for(;;) {
			result = test(dim, samples);
			if (result != PIGLIT_PASS)
				return result;

			if (samples < 8) {
				samples++;
			} else if (samples < max_samples) {
				samples *= 2;
				if (samples > max_samples)
					samples = max_samples;
			} else {
				break;
			}
		}
	}

	return PIGLIT_PASS;
}


void piglit_init(int argc, char ** argv)
{
	GLint max_native_tex_instructions;
	GLint max_native_tex_indirections;

	piglit_require_fragment_program();

	glGetProgramivARB(GL_FRAGMENT_PROGRAM_ARB,
			  GL_MAX_PROGRAM_NATIVE_TEX_INSTRUCTIONS_ARB,
			  &max_native_tex_instructions);
	glGetProgramivARB(GL_FRAGMENT_PROGRAM_ARB,
			  GL_MAX_PROGRAM_NATIVE_TEX_INDIRECTIONS_ARB,
			  &max_native_tex_indirections);

	printf("Max TEX instructions / TEX indirections: %i / %i\n",
	       max_native_tex_instructions,
	       max_native_tex_indirections);

	max_samples = max_native_tex_indirections;
	if (max_samples > max_native_tex_instructions) {
		/* ARB_fragment_program, issue 24:
		 * For implementations with no restrictions on the number of indirections,
		 * the maximum indirection count will equal the maximum texture instruction
		 * count.
		 */
		fprintf(stderr, "Violation of ARB_fragment_program issue 24: TEX indirections > TEX instructions\n");
		max_samples = max_native_tex_instructions;
	}

	if (max_samples > 1024)
		max_samples = 1024;

	texture_init();
}
