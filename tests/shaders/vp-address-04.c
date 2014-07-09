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
 * \file vp-address-04.c
 * Validate vectored address registers with various constant offsets.
 *
 * This is something of a combination of vp-address-02 and vp-address-03.
 * GL_NV_vertex_program2_option requires at least two address registers.  Base
 * GL_ARB_vertex_program implementations can also support more than one, but
 * only one is required.
 *
 * \author Ian Romanick <ian.d.romanick@intel.com>
 */

#include "piglit-util-gl.h"

static const GLfloat attrib[] = {
	0.0, 0.0,
	0.0, 1.0,
	0.0, 2.0,
	0.0, -1.0,
	0.0, -2.0,

	1.0, 0.0,
	1.0, 1.0,
	1.0, 2.0,
	1.0, -1.0,
	1.0, -2.0,

	2.0, 0.0,
	2.0, 1.0,
	2.0, 2.0,
	2.0, -1.0,
	2.0, -2.0,

	-1.0, 0.0,
	-1.0, 1.0,
	-1.0, 2.0,
	-1.0, -1.0,
	-1.0, -2.0,

	-2.0, 0.0,
	-2.0, 1.0,
	-2.0, 2.0,
	-2.0, -1.0,
	-2.0, -2.0,
};

#define TEST_ROWS  16
#define TEST_COLS  (ARRAY_SIZE(attrib) / 2)
#define BOX_SIZE   16

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = (((BOX_SIZE+1)*TEST_COLS)+1);
	config.window_height = (((BOX_SIZE+1)*TEST_ROWS)+1);
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static const char vertex_source_template[] =
	"!!ARBvp1.0\n"
	"OPTION	NV_vertex_program2;\n"
	"PARAM	colors[] = { program.env[0..3] };\n"
	"ADDRESS	A0, A1;\n"
	"\n"
	"ARL	A0, vertex.attrib[1];\n"
	"ARL	A1, vertex.attrib[2];\n"
	"ADD	result.color, colors[A0.%c %c %u], colors[A1.%c %c %u];\n"
	PIGLIT_VERTEX_PROGRAM_MVP_TRANSFORM
	"END\n"
	;


/**
 * \name Handles to programs.
 */
/*@{*/
static GLint progs[TEST_COLS * TEST_ROWS];
/*@}*/


static void generate_shader_source(char *source, size_t source_len,
    unsigned component_mask, const GLfloat *attr);


static void
set_attribute(unsigned component, GLuint index, GLfloat value)
{
	GLfloat v[4];

	/* Set all of the components to something invalid.  Then set one
	 * component to the desried, valid value.
	 */
	v[0] = -value;
	v[1] = -value;
	v[2] = -value;
	v[3] = -value;
	v[component] = value;

	glVertexAttrib4fvARB(index, v);
}


enum piglit_result
piglit_display(void)
{
	static const GLfloat color[4] = { 0.0, 0.5, 0.0, 0.5 };
	static const GLfloat good_color[4] = { 0.0, 1.0, 0.0, 1.0 };
	static const GLfloat bad_color[4] = { 1.0, 0.0, 0.0, 1.0 };
	enum piglit_result result = PIGLIT_PASS;
	unsigned i;
	unsigned j;

	glClear(GL_COLOR_BUFFER_BIT);

	glProgramEnvParameter4fvARB(GL_VERTEX_PROGRAM_ARB, 0, bad_color);
	glProgramEnvParameter4fvARB(GL_VERTEX_PROGRAM_ARB, 1, color);
	glProgramEnvParameter4fvARB(GL_VERTEX_PROGRAM_ARB, 2, bad_color);
	glProgramEnvParameter4fvARB(GL_VERTEX_PROGRAM_ARB, 3, bad_color);

	for (i = 0; i < TEST_ROWS; i++) {
		const int y = 1 + (i * (BOX_SIZE + 1));

		for (j = 0; j < TEST_COLS; j++) {
			const int x = 1 + (j * (BOX_SIZE + 1));
			const unsigned idx = (i * TEST_COLS) + j;

			glBindProgramARB(GL_VERTEX_PROGRAM_ARB, progs[idx]);

			set_attribute( i       & 0x03, 1, attrib[(j * 2) + 0]);
			set_attribute((i >> 2) & 0x03, 2, attrib[(j * 2) + 1]);

			piglit_draw_rect(x, y, BOX_SIZE, BOX_SIZE);

			if (!piglit_probe_pixel_rgb(x + (BOX_SIZE / 2),
						    y + (BOX_SIZE / 2),
						    good_color)) {
				if (! piglit_automatic) {
					char src[1024];
					generate_shader_source(src,
							       sizeof(src),
							       i,
							       & attrib[j * 2]);

					printf("shader %u failed with "
					       "attributes %.1f, %.1f:\n%s\n",
					       idx,
					       attrib[(j * 2) + 0],
					       attrib[(j * 2) + 1],
					       src);
				}

				result = PIGLIT_FAIL;
			}
		}
	}

	piglit_present_results();
	return result;
}


void
generate_shader_source(char *source, size_t source_len, unsigned component_mask,
		       const GLfloat *attr)
{
	static const char components[] = "xyzw";
	char comp[2];
	int offset[2];
	char direction[2];

	comp[0] = components[component_mask & 0x03];
	comp[1] = components[(component_mask >> 2) & 0x03];

	/* We want the constant offset in the instruction plus
	 * the value read from the attribute to be 1.
	 */
	offset[0] = 1 - (int) attr[0];
	offset[1] = 1 - (int) attr[1];

	if (offset[0] < 0) {
		direction[0] = '-';
		offset[0] = -offset[0];
	} else {
		direction[0] = '+';
	}

	if (offset[1] < 0) {
		direction[1] = '-';
		offset[1] = -offset[1];
	} else {
		direction[1] = '+';
	}

	snprintf(source, source_len, vertex_source_template,
		 comp[0], direction[0], offset[0],
		 comp[1], direction[1], offset[1]);
}


void
piglit_init(int argc, char **argv)
{
	GLint max_address_registers;
	unsigned i;
	unsigned j;

	(void) argc;
	(void) argv;

	piglit_require_vertex_program();
	piglit_require_fragment_program();
	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	glGetProgramivARB(GL_VERTEX_PROGRAM_ARB,
			  GL_MAX_PROGRAM_ADDRESS_REGISTERS_ARB,
			  & max_address_registers);
	if (max_address_registers == 0) {
		/* we have to have at least one address register */
		if (! piglit_automatic)
			printf("GL_MAX_PROGRAM_ADDRESS_REGISTERS_ARB == 0\n");

		piglit_report_result(PIGLIT_FAIL);
	} else 	if (max_address_registers == 1) {
		if (piglit_is_extension_supported("GL_NV_vertex_program2_option")) {
			/* this extension requires two address regs */
			if (! piglit_automatic)
				printf("GL_MAX_PROGRAM_ADDRESS_REGISTERS_ARB "
				       "== 1\n");

			piglit_report_result(PIGLIT_FAIL);
		} else {
			piglit_report_result(PIGLIT_SKIP);
		}
	}

	for (i = 0; i < TEST_ROWS; i++) {
		for (j = 0; j < TEST_COLS; j++) {
			char shader_source[1024];
			const unsigned idx = (i * TEST_COLS) + j;


			generate_shader_source(shader_source,
					       sizeof(shader_source),
					       i,
					       &attrib[2 * j]);
			progs[idx] =
				piglit_compile_program(GL_VERTEX_PROGRAM_ARB,
						       shader_source);
		}
	}

	glEnable(GL_FRAGMENT_PROGRAM_ARB);
	glEnable(GL_VERTEX_PROGRAM_ARB);
	glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, piglit_ARBfp_pass_through);

	glClearColor(0.5, 0.5, 0.5, 1.0);
}
