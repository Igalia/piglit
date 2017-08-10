/*
 * Copyright © 2016 Intel Corporation
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/** @file fs-non-uniform-control-flow-ubo.c
 *
 * it checks that uniform block reads work correctly when they are
 * under non-uniform control flow.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.window_width = 62;
	config.window_height = 62;
	config.supports_gl_compat_version = 32;
	config.supports_gl_core_version = 32;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;
PIGLIT_GL_TEST_CONFIG_END

#define UBO_SIZE 12

static const char vs_pass_thru_text[] =
	"#version 130\n"
	"\n"
	"in vec4 piglit_vertex;\n"
	"void main() {\n"
	"	gl_Position = piglit_vertex;\n"
	"}\n";

static const char fs_source[] =
	"#version 150\n"
	"#extension GL_ARB_gpu_shader_fp64 : require\n"
	"#extension GL_ARB_shading_language_420pack : require\n"
	"\n"
	"out vec4 color;\n"
	"\n"
	"layout(binding=2) uniform ubo {\n"
	"        dvec2 color2[];\n"
	"};\n"
	"\n"
	"void main() {\n"
	"        int cx = int(gl_FragCoord.x) / 31;\n"
	"        int cy = int(gl_FragCoord.y) / 31;\n"
	"        dvec2 rg;\n"
	"        if ((cx + cy) % 2 == 0) {\n"
	"                rg = color2[0];\n"
	"        } else {\n"
	"                rg = color2[1];\n"
	"        }\n"
	"        color = vec4(rg, 0, 1);\n"
	"}\n";

GLuint prog, fbo;

void
piglit_init(int argc, char **argv)
{
	GLuint buffer;
	double ubo_values[UBO_SIZE] = {1, 0, 0, 1,
				       0, 0, 0, 0,
				       0, 0, 0, 0};

	piglit_require_extension("GL_ARB_gpu_shader_fp64");
	piglit_require_extension("GL_ARB_shading_language_420pack");

	piglit_require_GLSL_version(150);

	prog = piglit_build_simple_program(vs_pass_thru_text, fs_source);

	glUseProgram(prog);

	glClearColor(0, 0, 0, 1);

	glGenBuffers(1, &buffer);
	glBindBufferBase(GL_UNIFORM_BUFFER, 2, buffer);
	glBufferData(GL_UNIFORM_BUFFER, UBO_SIZE*sizeof(GLdouble),
				&ubo_values[0], GL_DYNAMIC_DRAW);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);
}

enum piglit_result piglit_display(void)
{
	bool pass = true;
	const int num_pixels = piglit_width * piglit_height;
	float *srcPixels = malloc(num_pixels * 4 * sizeof(float));
	float expected[4];
	int i, j;

	glViewport(0, 0, piglit_width, piglit_height);
	glUseProgram(prog);
	glClear(GL_COLOR_BUFFER_BIT);

	piglit_draw_rect(-1, -1, 2, 2);

	glReadPixels(0, 0, piglit_width, piglit_height,
		     GL_RGBA, GL_FLOAT, srcPixels);

	/* Verify */
	for (i = 0; i < piglit_height; i++) {
		for (j = 0; j < piglit_width; j++) {
			int cx = j / 31;
			int cy = i / 31;
			int pos = (i * piglit_width + j) * 4;
			if ((cx + cy) % 2 != 0) {
				expected[0] = 0.0;
				expected[1] = 1.0;
				expected[2] = 0.0;
				expected[3] = 1.0;
			} else {
				expected[0] = 1.0;
				expected[1] = 0.0;
				expected[2] = 0.0;
				expected[3] = 1.0;
			}

			pass = piglit_compare_pixels(j, i, expected,
						     srcPixels + pos,
						     piglit_tolerance,
						     4) && pass;
		}
	}
	piglit_present_results();
	free(srcPixels);

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
