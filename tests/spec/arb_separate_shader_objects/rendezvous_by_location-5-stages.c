/*
 * Copyright © 2013 Intel Corporation
 * Copyright © 2015 Advanced Micro Devices, Inc.
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
 * This test uses 5 separate shaders (VS, TCS, TES, GS, FS) and tests whether
 * separate shader objects combined with tessellation and geometry shaders
 * all work together.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 0;
	config.supports_gl_core_version = 32;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static GLuint pipeline;

static const char *vs_code =
	"#version 150\n"
	"#extension GL_ARB_separate_shader_objects: require\n"
	"#extension GL_ARB_explicit_attrib_location: require\n"
	"\n"
	"layout(location = 0) in vec4 piglit_vertex;\n"
	"\n"
	"layout(location = 2) out vec3 a;\n"
	"layout(location = 3) out vec3 b;\n"
	"\n"
	"void main()\n"
	"{\n"
	"    gl_Position = piglit_vertex;\n"
	"    a = vec3(0, 0, 1);\n"
	"    b = vec3(1, 0, 0);\n"
	"}\n"
	;

static const char *tcs_code =
	"#version 150\n"
	"#extension GL_ARB_separate_shader_objects: require\n"
	"#extension GL_ARB_explicit_attrib_location: require\n"
	"#extension GL_ARB_tessellation_shader: require\n"
	"layout(vertices = 4) out;\n"
	"\n"
	"layout(location = 3) in vec3 a[]; /* should get vec3(1, 0, 0) */\n"
	"layout(location = 2) in vec3 b[]; /* should get vec3(0, 0, 1) */\n"
	"\n"
	"layout(location = 3) out vec3 vb[]; /* should write vec3(0, 0, 0.4) */\n"
	"layout(location = 5) out vec3 va[]; /* should write vec3(0.5, 0, 0) */\n"
	"\n"
	"layout(location = 4) patch out vec3 pb; /* should write vec3(0, 0, 0.2) */\n"
	"layout(location = 2) patch out vec3 pa; /* should write vec3(0.3, 0, 0) */\n"
	"\n"
	"void main()\n"
	"{\n"
	"    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;\n"
	"    va[gl_InvocationID] = a[gl_InvocationID] * 0.5;\n"
	"    vb[gl_InvocationID] = b[gl_InvocationID] * 0.4;\n"
	"    pa = a[0] * 0.3;\n"
	"    pb = b[0] * 0.2;\n"
	"    gl_TessLevelOuter = float[4](1.0, 1.0, 1.0, 1.0);\n"
	"    gl_TessLevelInner = float[2](1.0, 1.0);\n"
	"}\n"
	;

static const char *tes_code =
	"#version 150\n"
	"#extension GL_ARB_separate_shader_objects: require\n"
	"#extension GL_ARB_explicit_attrib_location: require\n"
	"#extension GL_ARB_tessellation_shader: require\n"
	"layout(quads, equal_spacing) in;\n"
	"\n"
	"layout(location = 2) patch in vec3 pb; /* should get vec3(0.3, 0, 0) */\n"
	"layout(location = 4) patch in vec3 pa; /* should get vec3(0, 0, 0.2) */\n"
	"\n"
	"layout(location = 3) in vec3 va[]; /* should get vec3(0, 0, 0.4) */\n"
	"layout(location = 5) in vec3 vb[]; /* should get vec3(0.5, 0, 0) */\n"
	"\n"
	"layout(location = 3) out vec3 a; /* should write vec2(0.4, 0, 0.2) */\n"
	"layout(location = 4) out vec3 b; /* should write vec2(0.5, 0, 0.3) */\n"
	"\n"
	"void main()\n"
	"{\n"
	"    vec4 p0 = gl_in[0].gl_Position;\n"
	"    vec4 p1 = gl_in[1].gl_Position;\n"
	"    vec4 p2 = gl_in[2].gl_Position;\n"
	"    vec4 p3 = gl_in[3].gl_Position;\n"
	"    gl_Position = mix(mix(p0, p1, gl_TessCoord.x), \n"
	"                      mix(p2, p3, gl_TessCoord.x), gl_TessCoord.y);\n"
	"    a = vec3(va[0].z, 0, pa.z);\n"
	"    b = vec3(vb[0].x, 0, pb.x);\n"
	"}\n"
	;

static const char *gs_code =
	"#version 150\n"
	"#extension GL_ARB_separate_shader_objects: require\n"
	"#extension GL_ARB_explicit_attrib_location: require\n"
	"layout(triangles) in;\n"
	"layout(triangle_strip, max_vertices = 3) out;\n"
	"\n"
	"layout(location = 4) in vec3 a[]; /* should get vec3(0.5, 0, 0.3) */\n"
	"layout(location = 3) in vec3 b[]; /* should get vec3(0.4, 0, 0.2) */\n"
	"\n"
	"layout(location = 2) out vec3 ga; /* should get vec3(0.675, 0, 0.405) */\n"
	"layout(location = 3) out vec3 gb; /* should get vec3(0.28, 0, 0.14) */\n"
	"\n"
	"void main()\n"
	"{\n"
	"    for (int i = 0; i < 3; i++) {"
	"        gl_Position = gl_in[i].gl_Position;\n"
	"        ga = a[i] * 1.35;\n"
	"        gb = b[i] * 0.7;\n"
	"        EmitVertex();\n"
	"    }\n"
	"}\n"
	;

static const char *fs_code =
	"#version 150\n"
	"#extension GL_ARB_separate_shader_objects: require\n"
	"#extension GL_ARB_explicit_attrib_location: require\n"
	"\n"
	"layout(location = 3) in vec3 ga; /* should get vec3(0.28, 0, 0.14) */\n"
	"layout(location = 2) in vec3 gb; /* should get vec3(0.675, 0, 0.405) */\n"
	"\n"
	"layout(location = 0) out vec4 out_color;\n"
	"\n"
	"void main()\n"
	"{\n"
	"    out_color = vec4(ga.x, gb.x, ga.z, gb.z);\n"
	"}\n"
	;

enum piglit_result
piglit_display(void)
{
	static const float expected[] = {
		0.28, 0.675, 0.14, 0.405
	};
	bool pass;

	glClearColor(0.1f, 0.1f, 0.1f, 0.1f);
	glClear(GL_COLOR_BUFFER_BIT);

	glBindProgramPipeline(pipeline);
	piglit_draw_rect_custom(-1, -1, 2, 2, true, 1);

	pass = piglit_probe_rect_rgba(0, 0, piglit_width, piglit_height,
				      expected);

	piglit_present_results();
	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void piglit_init(int argc, char **argv)
{
	GLuint vs_prog, tcs_prog, tes_prog, gs_prog, fs_prog;

	piglit_require_gl_version(32);
	piglit_require_extension("GL_ARB_separate_shader_objects");
	piglit_require_extension("GL_ARB_explicit_attrib_location");
	piglit_require_extension("GL_ARB_tessellation_shader");

	vs_prog = glCreateShaderProgramv(GL_VERTEX_SHADER, 1,
					 (const GLchar *const*)&vs_code);
	piglit_link_check_status(vs_prog);

	tcs_prog = glCreateShaderProgramv(GL_TESS_CONTROL_SHADER, 1,
					  (const GLchar *const *)&tcs_code);
	piglit_link_check_status(tcs_prog);

	tes_prog = glCreateShaderProgramv(GL_TESS_EVALUATION_SHADER, 1,
					  (const GLchar *const *)&tes_code);
	piglit_link_check_status(tes_prog);

	gs_prog = glCreateShaderProgramv(GL_GEOMETRY_SHADER, 1,
					 (const GLchar *const *)&gs_code);
	piglit_link_check_status(gs_prog);

	fs_prog = glCreateShaderProgramv(GL_FRAGMENT_SHADER, 1,
					 (const GLchar *const *)&fs_code);
	piglit_link_check_status(fs_prog);

	glGenProgramPipelines(1, &pipeline);
	glUseProgramStages(pipeline, GL_VERTEX_SHADER_BIT, vs_prog);
	glUseProgramStages(pipeline, GL_TESS_CONTROL_SHADER_BIT, tcs_prog);
	glUseProgramStages(pipeline, GL_TESS_EVALUATION_SHADER_BIT, tes_prog);
	glUseProgramStages(pipeline, GL_GEOMETRY_SHADER_BIT, gs_prog);
	glUseProgramStages(pipeline, GL_FRAGMENT_SHADER_BIT, fs_prog);
	piglit_program_pipeline_check_status(pipeline);

	if (!piglit_check_gl_error(0))
		piglit_report_result(PIGLIT_FAIL);
}
