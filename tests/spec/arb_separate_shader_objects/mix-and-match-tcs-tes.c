/*
 * Copyright © 2015 Intel Corporation
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
 * This program tests SSO pipelines where the TCS and TES are not linked
 * together, but specified in separate shaders.  In particular, this means
 * that the GLSL linker won't know the interface between the TCS and TES.
 *
 * We compile two TCS programs.  Both are largely the same, but the second
 * has extra unused outputs, which means the two pipelines have a different
 * number of per-patch outputs.  At least on i965, this requires a re-layout
 * of the TCS/TES interface.
 *
 * The output is a single green square, but drawn in two halves, each with
 * a different SSO pipeline.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 0;
	config.supports_gl_core_version = 32;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static GLuint pipeline[2];

static const char *vs_code =
	"#version 150\n"
	"#extension GL_ARB_separate_shader_objects: require\n"
	"in vec4 piglit_vertex;"
	"void main()\n"
	"{\n"
	"    gl_Position = piglit_vertex;\n"
	"}\n"
	;

#define TCS(vars, extra_code) \
	"#version 150\n" \
	"#extension GL_ARB_separate_shader_objects: require\n" \
	"#extension GL_ARB_tessellation_shader: require\n" \
	"layout(vertices = 3) out;\n" \
	"\n" \
        vars \
	"\n" \
	"void main()\n" \
	"{\n" \
	"    gl_out[gl_InvocationID].gl_Position = \n" \
	"        gl_in[gl_InvocationID].gl_Position;\n" \
	"    gl_TessLevelOuter = float[4](1.0, 1.0, 1.0, 1.0);\n" \
	"    gl_TessLevelInner = float[2](0.0, 0.0);\n" \
	"    patch_color = vec4(0.0, 1.0, 0.0, 1.0);\n" \
	extra_code \
	"}\n"

static const char *tcs0_code =
	TCS("layout(location = 1) patch out vec4 patch_color;\n", "");
static const char *tcs1_code =
	TCS("layout(location = 0) patch out vec4 foo;\n"
	    "layout(location = 1) patch out vec4 patch_color;\n"
	    "layout(location = 2) patch out vec4 bar;\n",

	    "    foo = vec4(0);\n"
	    "    bar = vec4(0);\n");

static const char *tes_code =
	"#version 150\n"
	"#extension GL_ARB_separate_shader_objects: require\n"
	"#extension GL_ARB_tessellation_shader: require\n"
	"layout(triangles) in;\n"
	"\n"
	"layout(location = 1) patch in vec4 patch_color;\n"
	"layout(location = 0) out vec4 color;\n"
	"\n"
	"void main()\n"
	"{\n"
	"    color = patch_color;\n"
	"    gl_Position = gl_in[0].gl_Position * gl_TessCoord[0]\n"
	"                + gl_in[1].gl_Position * gl_TessCoord[1]\n"
	"                + gl_in[2].gl_Position * gl_TessCoord[2];\n"
	"}\n"
	;

static const char *fs_code =
	"#version 150\n"
	"#extension GL_ARB_separate_shader_objects: require\n"
	"\n"
	"layout(location = 0) in vec4 color;"
	"out vec4 out_color;"
	"\n"
	"void main()\n"
	"{\n"
	"    out_color = color;\n"
	"}\n"
	;

enum piglit_result
piglit_display(void)
{
	static const float expected[] = { 0, 1, 0, 1 };
	bool pass;

	glClearColor(0.1f, 0.1f, 0.1f, 0.1f);
	glClear(GL_COLOR_BUFFER_BIT);

	/* Draw the left half using the first pipeline */
	glBindProgramPipeline(pipeline[0]);
	glViewport(0, 0, piglit_width / 2, piglit_height);
	glDrawArrays(GL_PATCHES, 0, 6);

	/* Draw the right half using the second pipeline */
	glBindProgramPipeline(pipeline[1]);
	glViewport(piglit_width / 2, 0, piglit_width / 2, piglit_height);
	glDrawArrays(GL_PATCHES, 0, 6);

	/* The result should be a green square filling the whole viewport. */
	pass = piglit_probe_rect_rgba(0, 0, piglit_width, piglit_height,
				      expected);

	piglit_present_results();
	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void piglit_init(int argc, char **argv)
{
	GLuint vs_prog, tcs0_prog, tcs1_prog, tes_prog, fs_prog;
	GLuint vao, buf;
	static const float verts[] = {
		-1.0, -1.0,
		 1.0, -1.0,
		-1.0,  1.0,
		-1.0,  1.0,
		 1.0, -1.0,
		 1.0,  1.0
	};

	piglit_require_extension("GL_ARB_separate_shader_objects");
	piglit_require_extension("GL_ARB_tessellation_shader");

	vs_prog = glCreateShaderProgramv(GL_VERTEX_SHADER, 1,
					 (const GLchar *const*)&vs_code);
	piglit_link_check_status(vs_prog);

	tcs0_prog = glCreateShaderProgramv(GL_TESS_CONTROL_SHADER, 1,
					   (const GLchar *const *)&tcs0_code);
	piglit_link_check_status(tcs0_prog);

	tcs1_prog = glCreateShaderProgramv(GL_TESS_CONTROL_SHADER, 1,
					   (const GLchar *const *)&tcs1_code);
	piglit_link_check_status(tcs1_prog);

	tes_prog = glCreateShaderProgramv(GL_TESS_EVALUATION_SHADER, 1,
					  (const GLchar *const *)&tes_code);
	piglit_link_check_status(tes_prog);

	fs_prog = glCreateShaderProgramv(GL_FRAGMENT_SHADER, 1,
					 (const GLchar *const *)&fs_code);
	piglit_link_check_status(fs_prog);

	glGenProgramPipelines(2, pipeline);
	glUseProgramStages(pipeline[0], GL_VERTEX_SHADER_BIT, vs_prog);
	glUseProgramStages(pipeline[0], GL_TESS_CONTROL_SHADER_BIT, tcs0_prog);
	glUseProgramStages(pipeline[0], GL_TESS_EVALUATION_SHADER_BIT, tes_prog);
	glUseProgramStages(pipeline[0], GL_FRAGMENT_SHADER_BIT, fs_prog);
	piglit_program_pipeline_check_status(pipeline[0]);

	glUseProgramStages(pipeline[1], GL_VERTEX_SHADER_BIT, vs_prog);
	glUseProgramStages(pipeline[1], GL_TESS_CONTROL_SHADER_BIT, tcs1_prog);
	glUseProgramStages(pipeline[1], GL_TESS_EVALUATION_SHADER_BIT, tes_prog);
	glUseProgramStages(pipeline[1], GL_FRAGMENT_SHADER_BIT, fs_prog);
	piglit_program_pipeline_check_status(pipeline[1]);

	/* Set up the VAOs/VBOs for drawing rectangles using the approach
	 * from spec/arb_tessellation_shader/execution/sanity.shader_test.
	 */
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(1, &buf);
	glBindBuffer(GL_ARRAY_BUFFER, buf);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
	glVertexAttribPointer(PIGLIT_ATTRIB_POS, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(PIGLIT_ATTRIB_POS);
	glPatchParameteri(GL_PATCH_VERTICES, 3);

	if (!piglit_check_gl_error(0))
		piglit_report_result(PIGLIT_FAIL);
}
