/*
 * Copyright © 2014 Intel Corporation
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
 * \file layout-mismatch.c
 *
 * Link multiple shader objects with layout qualifiers and check that linking
 * fails or succeeds if the qualifiers mismatch or match, respectively.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 32;
	config.supports_gl_core_version = 32;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END


static const char *const vs_source =
"#version 150\n"
"void main() { gl_Position = vec4(0.0); }\n";

static const char *const tcs_source_main =
"#version 150\n"
"#extension GL_ARB_tessellation_shader: require\n"
"layout(vertices = 3) out;\n"
"void main() {\n"
"	gl_TessLevelOuter = float[4](1.0, 1.0, 1.0, 1.0);\n"
"	gl_TessLevelInner = float[2](1.0, 1.0);\n"
"}\n";

static const char *const tcs_source_main_no_v =
"#version 150\n"
"#extension GL_ARB_tessellation_shader: require\n"
"void main() {\n"
"	gl_TessLevelOuter = float[4](1.0, 1.0, 1.0, 1.0);\n"
"	gl_TessLevelInner = float[2](1.0, 1.0);\n"
"}\n";

static const char *const tes_source_main =
"#version 150\n"
"#extension GL_ARB_tessellation_shader: require\n"
"layout(triangles) in;\n"
"void main() { gl_Position = vec4(0.0); }\n";

static const char *const tes_source_main_no_pm =
"#version 150\n"
"#extension GL_ARB_tessellation_shader: require\n"
"void main() { gl_Position = vec4(0.0); }\n";

static const char *const fs_source =
"#version 150\n"
"void main() { gl_FragColor = vec4(0.0); }\n";


static const char *const tcs_source_template =
"#version 150\n"
"#extension GL_ARB_tessellation_shader: require\n"
"layout(vertices = %d) out;\n"
"int foo%d(void) { return 1; }\n";

static const char *const tes_source_template =
"#version 150\n"
"#extension GL_ARB_tessellation_shader: require\n"
"layout(%s) in;\n"
"int foo%d(void) { return 1; }\n";


static const char *const prim_mode[] = {
	"triangles", "quads", "isolines"
};
static const char *const spacing[] = {
	"triangles, equal_spacing",
	"triangles, fractional_even_spacing",
	"triangles, fractional_odd_spacing"
};
static const char *const vertex_order[] = {
	"triangles, cw", "triangles, ccw"
};

static char *tes_source1, *tes_source2;
static char *tcs_source1, *tcs_source2;

static bool
test_tcs_layout(const int i, const int j)
{
	unsigned int prog;
	bool link_status;

	sprintf(tcs_source1, tcs_source_template, i, 1);
	sprintf(tcs_source2, tcs_source_template, j, 2);

	prog = piglit_build_simple_program_unlinked_multiple_shaders(
			GL_VERTEX_SHADER, vs_source,
			GL_TESS_CONTROL_SHADER, tcs_source_main_no_v,
			GL_TESS_CONTROL_SHADER, tcs_source1,
			GL_TESS_CONTROL_SHADER, tcs_source2,
			GL_TESS_EVALUATION_SHADER, tes_source_main,
			GL_FRAGMENT_SHADER, fs_source,
			0);
	glLinkProgram(prog);
	link_status = piglit_link_check_status_quiet(prog);
	glDeleteProgram(prog);

	if (link_status && (i != j)) {
		fprintf(stderr, "Program with different vertices "
			"specifications linked succesfully\n");
		return false;
	}
	if (!link_status && (i == j)) {
		fprintf(stderr, "Program with identical vertices "
			"specifications failed to link\n");
		return false;
	}

	return true;
}


static bool
test_tes_layout(const char *const layout1, const char *const layout2,
		const char *const layout_name)
{
	unsigned int prog;
	bool link_status;

	sprintf(tes_source1, tes_source_template, layout1, 1);
	sprintf(tes_source2, tes_source_template, layout2, 2);

	prog = piglit_build_simple_program_unlinked_multiple_shaders(
			GL_VERTEX_SHADER, vs_source,
			GL_TESS_CONTROL_SHADER, tcs_source_main,
			GL_TESS_EVALUATION_SHADER, tes_source_main_no_pm,
			GL_TESS_EVALUATION_SHADER, tes_source1,
			GL_TESS_EVALUATION_SHADER, tes_source2,
			GL_FRAGMENT_SHADER, fs_source,
			0);
	glLinkProgram(prog);
	link_status = piglit_link_check_status_quiet(prog);
	glDeleteProgram(prog);

	if (link_status && (layout1 != layout2)) {
		fprintf(stderr, "Program with different %s "
			"specifications linked succesfully\n", layout_name);
		return false;
	}
	if (!link_status && (layout1 == layout2)) {
		fprintf(stderr, "Program with identical %s "
			"specifications failed to link\n", layout_name);
		return false;
	}

	return true;
}


void
piglit_init(int argc, char **argv)
{
	bool pass = true;
	int i, j;

	piglit_require_extension("GL_ARB_tessellation_shader");

	/* From the ARB_tessellation_shader spec (Section 2.14.2):
	 *
	 *  "Linking will fail if the program object contains objects to form
	 *  a tessellation control shader (see section 2.X.1), and
	 *
	 *  [...]
	 *
	 *    * the output patch vertex count is specified differently in
	 *      multiple
	 *      tessellation control shader objects."
	 */
	tcs_source1 = malloc(strlen(tcs_source_template) + 8);
	tcs_source2 = malloc(strlen(tcs_source_template) + 8);

	for(i = 1; i < 32; i *= 4) {
		for(j = 1; j < 32; j *= 4) {
			pass = test_tcs_layout(i, j) && pass;
		}
	}

	free(tcs_source1);
	free(tcs_source2);

	/* From the ARB_tessellation_shader spec (Section 2.14.2):
	 *
	 *  "Linking will fail if the program object contains objects to form
	 *  a tessellation evaluation shader (see section 2.X.3), and
	 *
	 *  [...]
	 *
	 *    * the tessellation primitive mode, spacing, vertex order, or
	 *      point mode is specified differently in multiple tessellation
	 *      evaluation shader objects."
	 */
	tes_source1 = malloc(strlen(tes_source_template) + 64);
	tes_source2 = malloc(strlen(tes_source_template) + 64);

	for(i = 0; i < ARRAY_SIZE(prim_mode); ++i) {
		for(j = 0; j < ARRAY_SIZE(prim_mode); ++j) {
			pass = test_tes_layout(prim_mode[i], prim_mode[j],
					       "primitive mode") && pass;
		}
	}

	for(i = 0; i < ARRAY_SIZE(spacing); ++i) {
		for(j = 0; j < ARRAY_SIZE(spacing); ++j) {
			pass = test_tes_layout(spacing[i], spacing[j],
					       "vertex spacing") && pass;
		}
	}

	for(i = 0; i < ARRAY_SIZE(vertex_order); ++i) {
		for(j = 0; j < ARRAY_SIZE(vertex_order); ++j) {
			pass = test_tes_layout(vertex_order[i],
					       vertex_order[j],
					       "vertex order") && pass;
		}
	}

	free(tes_source1);
	free(tes_source2);

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}


enum piglit_result
piglit_display(void)
{
	return PIGLIT_PASS;
}

