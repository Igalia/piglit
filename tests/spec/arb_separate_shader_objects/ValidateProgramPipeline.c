/*
 * Copyright © 2013 Gregory Hainaut <gregory.hainaut@gmail.com>
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

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 21;
	config.supports_gl_core_version = 31;

PIGLIT_GL_TEST_CONFIG_END

static const char vs_source_template[] =
	"#version %d\n"
	"\n"
	"#if __VERSION__ > 140\n"
	"out gl_PerVertex {\n"
	"    vec4 gl_Position;\n"
	"    float gl_PointSize;\n"
	"    float gl_ClipDistance[];\n"
	"};\n"
	"\n"
	"in vec4 position;\n"
	"#else\n"
	"attribute vec4 position;\n"
	"#endif\n"
	"\n"
	"void main()\n"
	"{\n"
	"    gl_Position = position;\n"
	"}\n"
	;
static const char fs_source_template[] =
	"#version %d\n"
	"\n"
	"#if __VERSION__ > 140\n"
	"out vec4 color;\n"
	"#else\n"
	"#define color gl_FragColor\n"
	"#endif\n"
	"\n"
	"void main()\n"
	"{\n"
	"    color = vec4(0.0, 1.0, 0.0, 0.0);\n"
	"}\n"
	;
static const char gs_source_template[] =
	"#version %d\n"
	"\n"
	"in gl_PerVertex {\n"
	"    vec4 gl_Position;\n"
	"    float gl_PointSize;\n"
	"    float gl_ClipDistance[];\n"
	"} gl_in[];\n"
	"\n"
	"out gl_PerVertex {\n"
	"    vec4 gl_Position;\n"
	"    float gl_PointSize;\n"
	"    float gl_ClipDistance[];\n"
	"};\n"
	"\n"
	"layout(triangles) in;\n"
	"layout(triangle_strip, max_vertices = 3) out;\n"
	"void main()\n"
	"{\n"
	"    for(int i = 0; i < gl_in.length(); i++) {\n"
	"        gl_Position = gl_in[i].gl_Position;\n"
	"        EmitVertex();\n"
	"    }\n"
	"    EndPrimitive();\n"
	"}\n"
	;
static const char tc_source_template[] =
	"#version %d\n"
	"#extension GL_ARB_tessellation_shader: enable\n"
	"\n"
	"in gl_PerVertex {\n"
	"    vec4 gl_Position;\n"
	"    float gl_PointSize;\n"
	"    float gl_ClipDistance[];\n"
	"} gl_in[];\n"
	"\n"
	"out gl_PerVertex {\n"
	"    vec4 gl_Position;\n"
	"    float gl_PointSize;\n"
	"    float gl_ClipDistance[];\n"
	"} gl_out[];\n"
	"\n"
	"layout(vertices = 3) out;\n"
	"void main()\n"
	"{\n"
	"    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;\n"
	"    gl_TessLevelOuter[0] = 1.0;\n"
	"    gl_TessLevelOuter[1] = 1.0;\n"
	"    gl_TessLevelOuter[2] = 1.0;\n"
	"    gl_TessLevelInner[0] = 1.0;\n"
	"    gl_TessLevelInner[1] = 1.0;\n"
	"}\n"
	;
static const char te_source_template[] =
	"#version %d\n"
	"#extension GL_ARB_tessellation_shader: enable\n"
	"\n"
	"in gl_PerVertex {\n"
	"    vec4 gl_Position;\n"
	"    float gl_PointSize;\n"
	"    float gl_ClipDistance[];\n"
	"} gl_in[];\n"
	"\n"
	"out gl_PerVertex {\n"
	"    vec4 gl_Position;\n"
	"    float gl_PointSize;\n"
	"    float gl_ClipDistance[];\n"
	"};\n"
	"\n"
	"layout(triangles, equal_spacing)  in;\n"
	"\n"
	"void main()\n"
	"{\n"
	"    vec4 p0 = gl_in[0].gl_Position;\n"
	"    vec4 p1 = gl_in[1].gl_Position;\n"
	"    vec4 p2 = gl_in[2].gl_Position;\n"
	"\n"
	"    vec3 p = gl_TessCoord.xyz;\n"
	"\n"
	"    gl_Position = p0*p.x + p1*p.y + p2*p.z;\n"
	"}\n"
	;

static bool pass;

enum piglit_result
piglit_display(void)
{
	/* UNREACHED */
	return PIGLIT_FAIL;
}

static void
validate_pipe(GLuint pipe, bool expected, const char *test_name)
{
	GLint status;

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	status = piglit_program_pipeline_check_status_quiet(pipe);

	if (status != expected) {
		fprintf(stderr,
			"Wrong pipeline validation status. Got %d, but "
			"expected %d\n",
			status, expected);
		piglit_report_subtest_result(PIGLIT_FAIL, "%s", test_name);
		pass = false;
	} else {
		piglit_report_subtest_result(PIGLIT_PASS, "%s", test_name);
	}
}

static void
build_and_validate_pipe(GLuint pipe, bool expected, const char *test_name,
			GLbitfield bit_a, GLuint prog_a,
			GLbitfield bit_b, GLuint prog_b,
			GLbitfield bit_c, GLuint prog_c,
			GLbitfield bit_d, GLuint prog_d,
			GLbitfield bit_e, GLuint prog_e)
{
	if (!piglit_automatic)
		printf("%s\n", test_name);

	if (bit_a != GL_ALL_SHADER_BITS)
		glUseProgramStages(pipe, GL_ALL_SHADER_BITS, 0);

	if (bit_a != 0)
		glUseProgramStages(pipe, bit_a, prog_a);

	if (bit_b != 0)
		glUseProgramStages(pipe, bit_b, prog_b);

	if (bit_c != 0)
		glUseProgramStages(pipe, bit_c, prog_c);

	if (bit_d != 0)
		glUseProgramStages(pipe, bit_d, prog_d);

	if (bit_e != 0)
		glUseProgramStages(pipe, bit_e, prog_e);

	validate_pipe(pipe, expected, test_name);

	if (!piglit_automatic)
		printf("\n");
}

static GLuint
create_prog(GLint sh1, GLint sh2)
{
	GLint p = 0;

	p = glCreateProgram();
	glProgramParameteri(p, GL_PROGRAM_SEPARABLE, GL_TRUE);
	if (sh1)
		glAttachShader(p, sh1);
	if (sh2)
		glAttachShader(p, sh2);
	glLinkProgram(p);

	pass = piglit_link_check_status(p) && pass;

	return p;
}

void
piglit_init(int argc, char **argv)
{
	GLint vs, fs, gs, tes, tcs;
	GLuint pipe;
	GLint prog_vs, prog_fs, prog_gs = 0, prog_tcs = 0, prog_tes = 0, prog_tess;
	GLint prog_vs_fs, prog_vs_gs = 0;
	GLint separable;
	int version;

	char *vs_source, *fs_source, *gs_source, *te_source, *tc_source;

	const bool has_tess = piglit_get_gl_version() >= 40
		|| piglit_is_extension_supported("GL_ARB_tessellation_shader");
	const bool has_geo =
		piglit_get_gl_version() >= 32;

	piglit_require_extension("GL_ARB_separate_shader_objects");

	if (piglit_get_gl_version() >= 40)
		version = 400;
	else if (piglit_get_gl_version() >= 32)
		version = 150;
	else
		version = 120;

	pass = true;

	/* create the shader program */
	asprintf(&vs_source, vs_source_template, version);
	asprintf(&fs_source, fs_source_template, version);
	asprintf(&gs_source, gs_source_template, version);
	asprintf(&te_source, te_source_template, version);
	asprintf(&tc_source, tc_source_template, version);

	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vs_source);
	fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, fs_source);
	pass = (fs != 0) && (vs != 0) && pass;
	if (has_geo) {
		gs = piglit_compile_shader_text(GL_GEOMETRY_SHADER, gs_source);
		pass = (gs != 0) && pass;
	}
	if (has_tess) {
		tes = piglit_compile_shader_text(GL_TESS_EVALUATION_SHADER,
						 te_source);
		tcs = piglit_compile_shader_text(GL_TESS_CONTROL_SHADER,
						 tc_source);
		pass = (tes != 0) && (tcs != 0) && pass;
	}

	free(vs_source);
	free(fs_source);
	free(gs_source);
	free(te_source);
	free(tc_source);

	prog_vs = create_prog(vs, 0);
	prog_fs = create_prog(fs, 0);
	prog_vs_fs = create_prog(vs, fs);
	if (has_geo) {
		prog_gs = create_prog(gs, 0);
		prog_vs_gs = create_prog(vs, gs);
	}
	if (has_tess) {
		prog_tcs = create_prog(tcs, 0);
		prog_tes = create_prog(tes, 0);
		prog_tess = create_prog(tcs, tes);
	}

	/* Setup or compilation failure. Stop here */
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
	if (!pass) {
		piglit_report_result(PIGLIT_FAIL);
		return;
	}

	if (!piglit_automatic) {
		if (has_tess && has_geo) {
			printf("INFO: ALL stages supported: running all "
			       "test\n");
		} else {
			if (!has_tess)
				printf("INFO: GL_ARB_tessellation_shader / "
				       "OpenGL 4.0 not supported: tessellation "
				       "test disabled\n");
			if (!has_geo)
				printf("INFO: OpenGL 3.2 not "
				       "supported: geometry test disabled\n");
		}
	}

	/* Create the pipeline */
	glGenProgramPipelines(1, &pipe);

	build_and_validate_pipe(pipe, true, 
				"VS/FS program, single glUseProgramStages "
				"call",
				GL_ALL_SHADER_BITS, prog_vs_fs,
				0, 0,
				0, 0,
				0, 0,
				0, 0);

	build_and_validate_pipe(pipe, true, 
				"VS/FS program, multiple glUseProgramStages "
				"calls",
				GL_FRAGMENT_SHADER_BIT, prog_vs_fs,
				GL_VERTEX_SHADER_BIT, prog_vs_fs,
				0, 0,
				0, 0,
				0, 0);

	build_and_validate_pipe(pipe, true, 
				"program per pipeline stage",
				GL_VERTEX_SHADER_BIT, prog_vs,
				GL_FRAGMENT_SHADER_BIT, prog_fs,
				(has_geo) ? GL_GEOMETRY_SHADER_BIT : 0,
				prog_gs,
				(has_tess) ? GL_TESS_CONTROL_SHADER_BIT : 0,
				prog_tcs,
				(has_tess) ? GL_TESS_EVALUATION_SHADER_BIT : 0,
				prog_tes);

	/* Section 2.11.11 (Shader Execution), subpart "Validation" of the
	 * OpenGL 4.1 spec says:
	 *
	 *     "If the current set of active program objects cannot be
	 *     executed, no primitives are processed and the error
	 *     INVALID_OPERATION will be generated.  This error is generated
	 *     by any command that transfers vertices to the GL if:
	 *
	 *     ...
	 *
	 *     - One program object is active for at least two shader stages
	 *       and a second program is active for a shader stage between two
	 *       stages for which the first program was active."
	 */
	if (has_geo)
		build_and_validate_pipe(pipe, false, 
					"GS splitting a VS/FS pipeline",
					GL_ALL_SHADER_BITS, prog_vs_fs,
					GL_GEOMETRY_SHADER_BIT, prog_gs,
					0, 0,
					0, 0,
					0, 0);
	else
		piglit_report_subtest_result(PIGLIT_SKIP,
					     "GS splitting a VS/FS pipeline");

	if (has_tess)
		build_and_validate_pipe(pipe, false, 
					"TCS splitting a VS/GS pipeline",
					GL_ALL_SHADER_BITS, prog_vs_gs,
					GL_TESS_CONTROL_SHADER_BIT, prog_tcs,
					0, 0,
					0, 0,
					0, 0);
	else
		piglit_report_subtest_result(PIGLIT_SKIP,
					     "TCS splitting a VS/GS pipeline");

	if (has_tess)
		build_and_validate_pipe(pipe, false, 
					"TES splitting a VS/GS program",
					GL_ALL_SHADER_BITS, prog_vs_gs,
					GL_FRAGMENT_SHADER_BIT, prog_fs,
					GL_TESS_EVALUATION_SHADER_BIT, prog_tes,
					0, 0,
					0, 0);
	else
		piglit_report_subtest_result(PIGLIT_SKIP,
					     "TES splitting a VS/GS program");

	/* Section 2.11.11 (Shader Execution), subpart "Validation" of the
	 * OpenGL 4.1 spec says:
	 *
	 *     "If the current set of active program objects cannot be
	 *     executed, no primitives are processed and the error
	 *     INVALID_OPERATION will be generated.  This error is generated
	 *     by any command that transfers vertices to the GL if:
	 *
	 *     ...
	 *
	 *     - There is an active program for tessellation control,
	 *       tessellation evaluation, or geometry stages with
	 *       corresponding executable shader, but there is no active
	 *       program with executable vertex shader."
	 */
	if (has_geo)
		build_and_validate_pipe(pipe, false, 
					"GS without VS",
					GL_FRAGMENT_SHADER_BIT, prog_fs,
					GL_GEOMETRY_SHADER_BIT, prog_gs,
					0, 0,
					0, 0,
					0, 0);
	else
		piglit_report_subtest_result(PIGLIT_SKIP, "GS without VS");

	if (has_tess)
		build_and_validate_pipe(pipe, false, 
					"TES/TCS without VS",
					GL_ALL_SHADER_BITS, prog_tess,
					GL_FRAGMENT_SHADER_BIT, prog_fs,
					0, 0,
					0, 0,
					0, 0);
	else
		piglit_report_subtest_result(PIGLIT_SKIP, "TES/TCS without VS");

	/* Section 2.11.11 (Shader Execution), subpart "Validation" of the
	 * OpenGL 4.1 spec says:
	 *
	 *     "If the current set of active program objects cannot be
	 *     executed, no primitives are processed and the error
	 *     INVALID_OPERATION will be generated.  This error is generated
	 *     by any command that transfers vertices to the GL if:
	 *
	 *     - A program object is active for at least one, but not all of
	 *       the shader stages that were present when the program was
	 *       linked."
	 */
	build_and_validate_pipe(pipe, false, 
				"Only VS from a VS/FS program",
				GL_FRAGMENT_SHADER_BIT, prog_fs,
				GL_VERTEX_SHADER_BIT, prog_vs_fs,
				0, 0,
				0, 0,
				0, 0);

	if (has_geo)
		build_and_validate_pipe(pipe, false, 
					"Only GS from a VS/GS program",
					GL_FRAGMENT_SHADER_BIT, prog_fs,
					GL_GEOMETRY_SHADER_BIT, prog_vs_gs,
					GL_VERTEX_SHADER_BIT, prog_vs,
					0, 0,
					0, 0);
	else
		piglit_report_subtest_result(PIGLIT_SKIP,
					     "Only GS from a VS/GS program");

	if (has_tess)
		build_and_validate_pipe(pipe, false, 
					"Only TES from TES/TCS program",
					GL_FRAGMENT_SHADER_BIT, prog_fs,
					GL_TESS_EVALUATION_SHADER_BIT, prog_tess,
					GL_VERTEX_SHADER_BIT, prog_vs,
					0, 0,
					0, 0);
	else
		piglit_report_subtest_result(PIGLIT_SKIP,
					     "Only TES from TES/TCS program");

	/* Section 2.11.11 (Shader Execution), subpart "Validation" of the
	 * OpenGL 4.1 spec says:
	 *
	 *     "If the current set of active program objects cannot be
	 *     executed, no primitives are processed and the error
	 *     INVALID_OPERATION will be generated.  This error is generated
	 *     by any command that transfers vertices to the GL if:
	 *
	 *     ...
	 *
	 *     - There is no current unified program object and the current
	 *       program pipeline object includes a program object that was
	 *       relinked since being applied to the pipeline object via
	 *       UseProgramStages with the PROGRAM_SEPARABLE parameter set to
	 *       FALSE."
	 */
	build_and_validate_pipe(pipe, true, 
				"Relink attached VS without "
				"GL_PROGRAM_SEPARABLE (sanity pre-test)",
				GL_FRAGMENT_SHADER_BIT, prog_fs,
				GL_VERTEX_SHADER_BIT, prog_vs,
				0, 0,
				0, 0,
				0, 0);

	glGetProgramiv(prog_vs, GL_PROGRAM_SEPARABLE, &separable);
	if (!separable) {
		printf("Error: %d was not a separable program\n", prog_vs);
		pass = false;
	}

	glProgramParameteri(prog_vs, GL_PROGRAM_SEPARABLE, GL_FALSE);

	glGetProgramiv(prog_vs, GL_PROGRAM_SEPARABLE, &separable);

	/* NOTE: This check /may/ need to be moved after the call to
	 * glLinkProgram.  There has been some discussion as to whether this
	 * is supposed to be "latched" state.
	 */
	if (separable) {
		printf("Error: fail to remove separable flags of program %d\n",
		       prog_vs);
		pass = false;
	}

	glLinkProgram(prog_vs);
	pass = piglit_link_check_status(prog_vs) && pass;
	validate_pipe(pipe, GL_FALSE,
		      "Relink attached VS without GL_PROGRAM_SEPARABLE");

	piglit_present_results();

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
