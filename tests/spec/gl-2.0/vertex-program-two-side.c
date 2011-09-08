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

/** @file vertex-program-two-side.c
 *
 * Tests two-sided lighting behavior.
 *
 * From the GL 2.1 spec, page 63 (page 77 of the PDF):
 *
 *     "Additionally, vertex shaders can operate in two-sided color
 *      mode. When a vertex shader is active, front and back colors
 *      can be computed by the vertex shader and written to the
 *      gl_FrontColor, gl_BackColor, gl_FrontSecondaryColor and
 *      gl_BackSecondaryColor outputs. If VERTEX PROGRAM TWO SIDE is
 *      enabled, the GL chooses between front and back colors, as
 *      described below. Otherwise, the front color output is always
 *      selected. Two-sided color mode is enabled and disabled by
 *      calling Enable or Disable with the symbolic value VERTEX
 *      PROGRAM TWO SIDE."
 *
 * This appears to override the text in the GLSL 1.10 spec which
 * implies that two-sided behavior always occurs.
 */

#define _GNU_SOURCE
#include "piglit-util.h"

int piglit_width = 100, piglit_height = 100;
int piglit_window_mode = GLUT_RGB | GLUT_ALPHA | GLUT_DOUBLE;

static GLint prog;

static bool primary = true;
static bool secondary = true;
static bool enabled = true;
static bool front = true;
static bool back = true;
static float frontcolor[4] = {0.0, 0.5, 0.0, 0.0};
static float backcolor[4] = {0.0, 0.0, 0.5, 0.0};
static float secondary_frontcolor[4] = {0.0, 0.25, 0.0, 0.0};
static float secondary_backcolor[4] = {0.0, 0.0, 0.25, 0.0};

static const char *fs_source_primary =
	"void main()\n"
	"{\n"
	"	gl_FragColor = gl_Color;\n"
	"}\n";

static const char *fs_source_secondary =
	"void main()\n"
	"{\n"
	"	gl_FragColor = gl_SecondaryColor;\n"
	"}\n";

static const char *fs_source_both =
	"void main()\n"
	"{\n"
	"	gl_FragColor = gl_Color + gl_SecondaryColor;\n"
	"}\n";

void
add(float *values, float *a)
{
	int i;

	for (i = 0; i < 4; i++)
		values[i] += a[i];
}

void
get_expected(float *values, bool drew_front)
{
	int i;

	for (i = 0; i < 4; i++)
		values[i] = 0.0;

	if (drew_front || !enabled) {
		if (primary)
			add(values, frontcolor);
		if (secondary)
			add(values, secondary_frontcolor);
	}

	if (!drew_front && enabled) {
		if (primary)
			add(values, backcolor);
		if (secondary)
			add(values, secondary_backcolor);
	}
}

enum piglit_result
piglit_display(void)
{
	int front_x = 10;
	int front_y = 10;
	int front_w = piglit_width / 2 - 20;
	int front_h = piglit_height - 20;
	int back_x = piglit_width - 10;
	int back_y = 10;
	int back_w = -(front_w);
	int back_h = piglit_height - 20;
	bool pass = true;
	float expected[4];

	piglit_ortho_projection(piglit_width, piglit_height, false);

	glClearColor(0.5, 0.5, 0.5, 0.5);
	glClear(GL_COLOR_BUFFER_BIT);

	if (front) {
		piglit_draw_rect(front_x, front_y, front_w, front_h);
		get_expected(expected, true);
		pass = pass && piglit_probe_rect_rgba(front_x, front_y,
						      front_w, front_h,
						      expected);
	}

	if (back || !enabled) {
		piglit_draw_rect(back_x, back_y, back_w, back_h);
		get_expected(expected, false);
		pass = pass && piglit_probe_rect_rgba(back_x + back_w, back_y,
						      -back_w, back_h,
						      expected);
	}

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

static void
setup_output(char **out, const char *name, float *values)
{
	asprintf(out,
		 "	%s = vec4(%f, %f, %f, %f);\n",
		 name,
		 values[0],
		 values[1],
		 values[2],
		 values[3]);
}

void
piglit_init(int argc, char **argv)
{
	GLint vs, fs;
	char *vs_outputs[4] = {"", "", "", ""};
	char *vs_source;
	const char *fs_source = fs_source_both;
	int i;

	piglit_require_GLSL();

	if (!GLEW_VERSION_2_0) {
		printf("Requires OpenGL 2.0\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "disabled") == 0) {
			enabled = false;
		} else if (strcmp(argv[i], "front") == 0) {
			back = false;
		} else if (strcmp(argv[i], "back") == 0) {
			front = false;
		} else if (strcmp(argv[i], "primary") == 0) {
			secondary = false;
			fs_source = fs_source_primary;
		} else if (strcmp(argv[i], "secondary") == 0) {
			primary = false;
			fs_source = fs_source_secondary;
		} else {
			fprintf(stderr, "unknown argument %s\n", argv[i]);
		}
	}

	assert(enabled || front);

	if (front && primary)
		setup_output(&vs_outputs[0], "gl_FrontColor", frontcolor);
	if (back && primary)
		setup_output(&vs_outputs[1], "gl_BackColor", backcolor);
	if (front && secondary)
		setup_output(&vs_outputs[2], "gl_FrontSecondaryColor", secondary_frontcolor);
	if (back && secondary)
		setup_output(&vs_outputs[3], "gl_BackSecondaryColor", secondary_backcolor);

	asprintf(&vs_source,
		 "void main()\n"
		 "{\n"
		 "	gl_Position = ftransform();\n"
		 "%s%s%s%s"
		 "}\n",
		 vs_outputs[0],
		 vs_outputs[1],
		 vs_outputs[2],
		 vs_outputs[3]);

	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vs_source);
	fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, fs_source);
	prog = piglit_link_simple_program(vs, fs);

	if (!prog || !vs || !fs) {
		printf("VS source:\n%s", vs_source);
		printf("FS source:\n%s", fs_source);
		piglit_report_result(PIGLIT_FAIL);
	}

	glUseProgram(prog);

	if (enabled)
		glEnable(GL_VERTEX_PROGRAM_TWO_SIDE);
}
