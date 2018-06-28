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
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

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

char *dummy_outputs[4] = {"", "", "", ""};
char *vs_outputs[4] = {"", "", "", ""};
char *gs_outputs[4] = {"", "", "", ""};
char *gs_inputs_outputs[4] = {"", "", "", ""};
char *tcs_outputs[4] = {"", "", "", ""};
char *tcs_inputs_outputs[4] = {"", "", "", ""};
char *tes_outputs[4] = {"", "", "", ""};
char *tes_inputs_outputs[4] = {"", "", "", ""};

const char * tests[7] = {"vs and fs", "gs-out and fs", "vs, gs and fs",
			 "tes-out and fs", "tcs-out, tes and fs",
			 "vs, tcs, tes and fs", NULL };

static const char *dummy_vs_source =
	"void main()\n"
	"{\n"
	"	gl_Position = gl_Vertex;\n"
	"}\n";

static const char *fs_source =
	"uniform bool draw_secondary;\n"
	"void main()\n"
	"{\n"
	"	if (draw_secondary)\n"
	"		gl_FragColor = gl_SecondaryColor;\n"
	"	else\n"
	"		gl_FragColor = gl_Color;\n"
	"}\n";

static bool
probe_colors()
{
	bool pass = true;
	int x1 = 0, y1 = 0;
	int w = piglit_width / 2, h = piglit_height / 2;
	int x2 = piglit_width - w, y2 = piglit_height - h;

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

	return pass;
}

static bool
test_prog(unsigned prog, const char *test_name, bool use_patches)
{
	glUseProgram(prog);
	draw_secondary_loc = glGetUniformLocation(prog, "draw_secondary");
	assert(draw_secondary_loc != -1);

	if (enabled)
		glEnable(GL_VERTEX_PROGRAM_TWO_SIDE);

	/* Draw */
	glClearColor(0.5, 0.5, 0.5, 0.5);
	glClear(GL_COLOR_BUFFER_BIT);

	glUniform1i(draw_secondary_loc, false);
	piglit_draw_rect_custom(-1,  0,  1, 1, use_patches, 1); /* top left */
	piglit_draw_rect_custom( 1,  0, -1, 1, use_patches, 1); /* top right */

	glUniform1i(draw_secondary_loc, true);
	piglit_draw_rect_custom(-1, -1,  1, 1, use_patches, 1); /* bot left */
	piglit_draw_rect_custom( 1, -1, -1, 1, use_patches, 1); /* bot right */

	/* probe and report result */
	bool pass = probe_colors();
	piglit_report_subtest_result(pass ? PIGLIT_PASS : PIGLIT_FAIL, "%s",
				     test_name);

	return pass;
}

static void
setup_vs_output(char **out, const char *name, float *values)
{
	(void)!asprintf(out,
		 "	%s = vec4(%f, %f, %f, %f);\n",
		 name,
		 values[0],
		 values[1],
		 values[2],
		 values[3]);
}

static void
setup_gs_vars(char **in_out, char **out, const char *name, float *values)
{
	(void)!asprintf(in_out, "	%s = gl_in[i].%s;\n", name, name);
	(void)!asprintf(out, "	%s = vec4(%f, %f, %f, %f);\n",
			name,
			values[0],
			values[1],
			values[2],
			values[3]);
}

static void
setup_tcs_vars(char **in_out, char **out, const char *name, float *values)
{
	(void)!asprintf(in_out, "	gl_out[gl_InvocationID].%s = gl_in[gl_InvocationID].%s;\n", name, name);
	(void)!asprintf(out, "	gl_out[gl_InvocationID].%s = vec4(%f, %f, %f, %f);\n",
			name,
			values[0],
			values[1],
			values[2],
			values[3]);
}

static void
setup_tes_vars(char **in_out, char **out, const char *name, float *values)
{
	(void)!asprintf(in_out, "	INTERP_QUAD(gl_in[0].%s, %s);\n", name, name);
	(void)!asprintf(out, "	INTERP_QUAD(vec4(%f, %f, %f, %f), %s);\n",
			values[0],
			values[1],
			values[2],
			values[3],
			name);
}

static void
create_gs_source(char **gs_source, char **builtins)
{
	(void)!asprintf(gs_source,
		"#version 150 compatibility\n"
		"layout(triangles) in;\n"
		"layout(triangle_strip, max_vertices = 3) out;\n"
		"\n"
		"void main()\n"
		"{\n"
		"	for (int i = 0; i < 3; i++) {\n"
		"		gl_Position = gl_in[i].gl_Position;\n"
		"		%s%s%s%s\n"
		"		EmitVertex();\n"
		"	}\n"
		"}\n",
		builtins[0],
		builtins[1],
		builtins[2],
		builtins[3]);
}

static void
create_tess_source(char **tcs_source, char **tcs_builtins,
                   char **tes_source, char **tes_builtins)
{
	(void)!asprintf(tcs_source,
		"#version 150 compatibility\n"
		"#extension GL_ARB_tessellation_shader: require\n"
		"layout(vertices = 4) out;\n"
		"\n"
		"void main()\n"
		"{\n"
		"	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;\n"
		"	gl_TessLevelOuter = float[4](1.0, 1.0, 1.0, 1.0);\n"
		"	gl_TessLevelInner = float[2](1.0, 1.0);\n"
		"	%s%s%s%s\n"
		"}\n",
		tcs_builtins[0],
		tcs_builtins[1],
		tcs_builtins[2],
		tcs_builtins[3]);

	(void)!asprintf(tes_source,
		"#version 150 compatibility\n"
		"#extension GL_ARB_tessellation_shader: require\n"
		"layout(quads) in;\n"
		"\n"
		"#define INTERP_QUAD(INi, OUT) do { \\\n"
		"	vec4 v[4]; \\\n"
		"	for (int i = 0; i < 4; i++) v[i] = INi; \\\n"
		"		OUT = mix(mix(v[0], v[1], gl_TessCoord[0]), mix(v[2], v[3], \\\n"
		"			  gl_TessCoord[0]), gl_TessCoord[1]); \\\n"
		"} while(false);\n"
		"\n"
		"void main()\n"
		"{\n"
		"	INTERP_QUAD(gl_in[i].gl_Position, gl_Position);\n"
		"	%s%s%s%s\n"
		"}\n",
		tes_builtins[0],
		tes_builtins[1],
		tes_builtins[2],
		tes_builtins[3]);
}

enum piglit_result
piglit_display(void)
{
	char *vs_source;
	char *gs_source;
	char *gs_source2;
	char *tcs_source;
	char *tes_source;
	bool pass;

	(void)!asprintf(&vs_source,
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
	pass = test_prog(prog, tests[0], false);

	if (piglit_get_gl_version() >= 32) {
		/* Test the gs outputs only */
		create_gs_source(&gs_source, gs_outputs);
		prog = piglit_build_simple_program_multiple_shaders(
			GL_VERTEX_SHADER, dummy_vs_source,
			GL_GEOMETRY_SHADER, gs_source,
			GL_FRAGMENT_SHADER, fs_source,
			0);

		pass = pass && test_prog(prog, tests[1], false);

		/* Test both the gs outputs and inputs */
		create_gs_source(&gs_source2, gs_inputs_outputs);
		prog = piglit_build_simple_program_multiple_shaders(
			GL_VERTEX_SHADER, vs_source,
			GL_GEOMETRY_SHADER, gs_source2,
			GL_FRAGMENT_SHADER, fs_source,
			0);

		pass = pass && test_prog(prog, tests[2], false);

		if (piglit_is_extension_supported("GL_ARB_tessellation_shader")) {
			/* Test tes outputs only */
			create_tess_source(&tcs_source, dummy_outputs,
					   &tes_source, tes_outputs);
			prog = piglit_build_simple_program_multiple_shaders(
				GL_VERTEX_SHADER, dummy_vs_source,
				GL_TESS_CONTROL_SHADER, tcs_source,
				GL_TESS_EVALUATION_SHADER, tes_source,
				GL_FRAGMENT_SHADER, fs_source,
				0);
			pass = pass && test_prog(prog, tests[3], true);
			free(tcs_source);
			free(tes_source);

			/* Test tcs outputs and tes inputs/outputs */
			create_tess_source(&tcs_source, tcs_outputs,
					   &tes_source, tes_inputs_outputs);
			prog = piglit_build_simple_program_multiple_shaders(
				GL_VERTEX_SHADER, dummy_vs_source,
				GL_TESS_CONTROL_SHADER, tcs_source,
				GL_TESS_EVALUATION_SHADER, tes_source,
				GL_FRAGMENT_SHADER, fs_source,
				0);
			pass = pass && test_prog(prog, tests[4], true);
			free(tcs_source);
			free(tes_source);

			/* Test tcs inputs/outputs and tes inputs/outputs */
			create_tess_source(&tcs_source, tcs_inputs_outputs,
					   &tes_source, tes_inputs_outputs);
			prog = piglit_build_simple_program_multiple_shaders(
				GL_VERTEX_SHADER, vs_source,
				GL_TESS_CONTROL_SHADER, tcs_source,
				GL_TESS_EVALUATION_SHADER, tes_source,
				GL_FRAGMENT_SHADER, fs_source,
				0);
			pass = pass && test_prog(prog, tests[5], true);
			free(tcs_source);
			free(tes_source);
		} else {
			piglit_report_subtest_result(PIGLIT_SKIP, "%s", tests[3]);
			piglit_report_subtest_result(PIGLIT_SKIP, "%s", tests[4]);
			piglit_report_subtest_result(PIGLIT_SKIP, "%s", tests[5]);
		}
	} else {
		piglit_report_subtest_result(PIGLIT_SKIP, "%s", tests[1]);
		piglit_report_subtest_result(PIGLIT_SKIP, "%s", tests[2]);
		piglit_report_subtest_result(PIGLIT_SKIP, "%s", tests[3]);
		piglit_report_subtest_result(PIGLIT_SKIP, "%s", tests[4]);
		piglit_report_subtest_result(PIGLIT_SKIP, "%s", tests[5]);
	}

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	piglit_require_GLSL();
	piglit_require_gl_version(20);

	printf("Window quadrants show:\n");
	printf("+-------------------------+------------------------+\n");
	printf("| front gl_Color          | back gl_Color          |\n");
	printf("+-------------------------+------------------------+\n");
	printf("| front gl_SecondaryColor | back gl_SecondaryColor |\n");
	printf("+-------------------------+------------------------+\n");

	for (unsigned i = 1; i < argc; i++) {
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

	piglit_register_subtests(tests);

	if (front) {
		setup_vs_output(&vs_outputs[0], "gl_FrontColor", frontcolor);
		setup_gs_vars(&gs_inputs_outputs[0], &gs_outputs[0], "gl_FrontColor", frontcolor);
		setup_tcs_vars(&tcs_inputs_outputs[0], &tcs_outputs[0], "gl_FrontColor", frontcolor);
		setup_tes_vars(&tes_inputs_outputs[0], &tes_outputs[0], "gl_FrontColor", frontcolor);
	}
	if (back) {
		setup_vs_output(&vs_outputs[1], "gl_BackColor", backcolor);
		setup_gs_vars(&gs_inputs_outputs[1], &gs_outputs[1], "gl_BackColor", backcolor);
		setup_tcs_vars(&tcs_inputs_outputs[1], &tcs_outputs[1], "gl_BackColor", backcolor);
		setup_tes_vars(&tes_inputs_outputs[1], &tes_outputs[1], "gl_BackColor", backcolor);
	}
	if (front2) {
		setup_vs_output(&vs_outputs[2], "gl_FrontSecondaryColor", secondary_frontcolor);
		setup_gs_vars(&gs_inputs_outputs[2], &gs_outputs[2], "gl_FrontSecondaryColor", secondary_frontcolor);
		setup_tcs_vars(&tcs_inputs_outputs[2], &tcs_outputs[2], "gl_FrontSecondaryColor", secondary_frontcolor);
		setup_tes_vars(&tes_inputs_outputs[2], &tes_outputs[2], "gl_FrontSecondaryColor", secondary_frontcolor);
	}
	if (back2) {
		setup_vs_output(&vs_outputs[3], "gl_BackSecondaryColor", secondary_backcolor);
		setup_gs_vars(&gs_inputs_outputs[3], &gs_outputs[3], "gl_BackSecondaryColor", secondary_backcolor);
		setup_tcs_vars(&tcs_inputs_outputs[3], &tcs_outputs[3], "gl_BackSecondaryColor", secondary_backcolor);
		setup_tes_vars(&tes_inputs_outputs[3], &tes_outputs[3], "gl_BackSecondaryColor", secondary_backcolor);
	}
}
