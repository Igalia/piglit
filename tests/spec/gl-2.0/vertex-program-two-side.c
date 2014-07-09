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

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static GLint prog;

static bool enabled = false;
static bool front = false;
static bool back = false;
static bool front2 = false;
static bool back2 = false;

static float frontcolor[4] = {0.0, 0.5, 0.0, 0.0};
static float backcolor[4] = {0.0, 0.0, 0.5, 0.0};
static float secondary_frontcolor[4] = {0.0, 0.25, 0.0, 0.0};
static float secondary_backcolor[4] = {0.0, 0.0, 0.25, 0.0};
static int draw_secondary_loc;

static const char *fs_source =
	"uniform bool draw_secondary;\n"
	"void main()\n"
	"{\n"
	"	if (draw_secondary)\n"
	"		gl_FragColor = gl_SecondaryColor;\n"
	"	else\n"
	"		gl_FragColor = gl_Color;\n"
	"}\n";

enum piglit_result
piglit_display(void)
{
	int x1 = 0, y1 = 0;
	int w = piglit_width / 2, h = piglit_height / 2;
	int x2 = piglit_width - w, y2 = piglit_height - h;
	bool pass = true;

	glClearColor(0.5, 0.5, 0.5, 0.5);
	glClear(GL_COLOR_BUFFER_BIT);

	glUniform1i(draw_secondary_loc, false);
	piglit_draw_rect(-1,  0,  1, 1); /* top left */
	piglit_draw_rect( 1,  0, -1, 1); /* top right */

	glUniform1i(draw_secondary_loc, true);
	piglit_draw_rect(-1, -1,  1, 1); /* bot left */
	piglit_draw_rect( 1, -1, -1, 1); /* bot right */

	if (front) {
		pass = pass && piglit_probe_rect_rgba(x1, y2, w, h,
						      frontcolor);
	}

	if (front2) {
		pass = pass && piglit_probe_rect_rgba(x1, y1, w, h,
						      secondary_frontcolor);
	}

	if (enabled) {
		/* Two-sided: Get the back color/secondarycolor. */
		if (back) {
			pass = pass && piglit_probe_rect_rgba(x2, y2, w, h,
							      backcolor);
		}
		if (back2) {
			pass = pass && piglit_probe_rect_rgba(x2, y1, w, h,
							      secondary_backcolor);
		}
	} else {
		/* Non-two-sided: Get the front color/secondarycolor. */
		if (front) {
			pass = pass && piglit_probe_rect_rgba(x2, y2, w, h,
							      frontcolor);
		}
		if (front2) {
			pass = pass && piglit_probe_rect_rgba(x2, y1, w, h,
							      secondary_frontcolor);
		}
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
	char *vs_outputs[4] = {"", "", "", ""};
	char *vs_source;
	int i;

	piglit_require_GLSL();

	piglit_require_gl_version(20);

	printf("Window quadrants show:\n");
	printf("+-------------------------+------------------------+\n");
	printf("| front gl_Color          | back gl_Color          |\n");
	printf("+-------------------------+------------------------+\n");
	printf("| front gl_SecondaryColor | back gl_SecondaryColor |\n");
	printf("+-------------------------+------------------------+\n");

	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "enabled") == 0) {
			enabled = true;
		} else if (strcmp(argv[i], "front") == 0) {
			front = true;
		} else if (strcmp(argv[i], "back") == 0) {
			back = true;
		} else if (strcmp(argv[i], "front2") == 0) {
			front2 = true;
		} else if (strcmp(argv[i], "back2") == 0) {
			back2 = true;
		} else {
			fprintf(stderr, "unknown argument %s\n", argv[i]);
		}
	}

	if (front)
		setup_output(&vs_outputs[0], "gl_FrontColor", frontcolor);
	if (back)
		setup_output(&vs_outputs[1], "gl_BackColor", backcolor);
	if (front2)
		setup_output(&vs_outputs[2], "gl_FrontSecondaryColor", secondary_frontcolor);
	if (back2)
		setup_output(&vs_outputs[3], "gl_BackSecondaryColor", secondary_backcolor);

	asprintf(&vs_source,
		 "void main()\n"
		 "{\n"
		 "	gl_Position = gl_Vertex;\n"
		 "%s%s%s%s"
		 "}\n",
		 vs_outputs[0],
		 vs_outputs[1],
		 vs_outputs[2],
		 vs_outputs[3]);

	prog = piglit_build_simple_program(vs_source, fs_source);
	glUseProgram(prog);
	draw_secondary_loc = glGetUniformLocation(prog, "draw_secondary");
	assert(draw_secondary_loc != -1);

	if (enabled)
		glEnable(GL_VERTEX_PROGRAM_TWO_SIDE);
}
