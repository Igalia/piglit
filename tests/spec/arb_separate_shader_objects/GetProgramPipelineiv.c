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

	config.supports_gl_compat_version = 20;
	config.supports_gl_core_version = 31;

PIGLIT_GL_TEST_CONFIG_END

static bool pass;

enum piglit_result
piglit_display(void)
{
	/* UNREACHED */
	return PIGLIT_FAIL;
}

static GLbitfield
stage2bitfield(GLint stage)
{
	switch (stage) {
	case GL_VERTEX_SHADER: return GL_VERTEX_SHADER_BIT;
	case GL_FRAGMENT_SHADER: return GL_FRAGMENT_SHADER_BIT;
	case GL_GEOMETRY_SHADER: return GL_GEOMETRY_SHADER_BIT;
	case GL_TESS_CONTROL_SHADER: return GL_TESS_CONTROL_SHADER_BIT;
	case GL_TESS_EVALUATION_SHADER:return GL_TESS_EVALUATION_SHADER_BIT;
	case GL_COMPUTE_SHADER: return GL_COMPUTE_SHADER_BIT;
	default:
		assert(!"Should not get here.");
		return 0;
	}
}

static void
check_stage(GLint pipe, GLint expected, GLint stage, bool supported)
{
	GLint param = 0;
	glGetProgramPipelineiv(pipe, stage, &param);

	if (!supported) {
		pass = piglit_check_gl_error(GL_INVALID_ENUM) && pass;
	} else if (param != expected) {
		fprintf(stderr, "Failed to get program of stage %s.\n",
			piglit_get_gl_enum_name(stage));
		pass = false;
	}
}

static void
use_stage_and_check(GLint pipe, GLint program, GLint stage, bool supported)
{
	printf("Attach program (%d) to stage (%s). Expected to be "
	       "supported: %s\n",
	       program,
	       piglit_get_gl_enum_name(stage),
	       supported ? "yes" : "no");

	glUseProgramStages(pipe, stage2bitfield(stage), program);
	if (!supported) {
		pass = piglit_check_gl_error(GL_INVALID_VALUE) && pass;
	} else {
		pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
	}

	check_stage(pipe, program, stage, supported);
}

void
piglit_init(int argc, char **argv)
{
	GLint vs, fs, gs, tcs, tes;
	GLint ver;
	GLuint pipe = 0;
	GLint param = 0;
	char *version = NULL;
	const char *shader_source[2];

	static const char vs_source[] =
		"#if __VERSION__ > 140\n"
		"/* At least some versions of AMD's closed-source driver\n"
		" * contain a bug that requires redeclaration of gl_PerVertex\n"
		" * interface block in core profile shaders.\n"
		" */\n"
		"out gl_PerVertex {\n"
		"    vec4 gl_Position;\n"
		"};\n"
		"\n"
		"in vec4 position;\n"
		"#else\n"
		"varying vec4 position;\n"
		"#endif\n"
		"\n"
		"void main()\n"
		"{\n"
		"    gl_Position = position;\n"
		"}\n";
	static const char fs_source[] =
		"void main()\n"
		"{\n"
		"    gl_FragColor = vec4(0.0, 1.0, 0.0, 0.0);\n"
		"}\n";
	static const char gs_source[] =
		"/* At least some versions of AMD's closed-source driver\n"
		" * contain a bug that requires redeclaration of gl_PerVertex\n"
		" * interface block in core profile shaders.\n"
		" */\n"
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
		"void main() {\n"
		"    for(int i = 0; i < gl_in.length(); i++) {\n"
		"        gl_Position = gl_in[i].gl_Position;\n"
		"        EmitVertex();\n"
		"    }\n"
		"    EndPrimitive();\n"
		"}\n";
	static const char tc_source[] =
		"/* At least some versions of AMD's closed-source driver\n"
		" * contain a bug that requires redeclaration of gl_PerVertex\n"
		" * interface block in core profile shaders.\n"
		" */\n"
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
		"layout(vertices = 3)  out;\n"
		"void main()\n"
		"{\n"
		"    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;\n"
		"    gl_TessLevelOuter[0] = 1.0;\n"
		"    gl_TessLevelOuter[1] = 1.0;\n"
		"    gl_TessLevelOuter[2] = 1.0;\n"
		"    gl_TessLevelInner[0] = 1.0;\n"
		"    gl_TessLevelInner[1] = 1.0;\n"
		"}\n";
	static const char te_source[] =
		"/* At least some versions of AMD's closed-source driver\n"
		" * contain a bug that requires redeclaration of gl_PerVertex\n"
		" * interface block in core profile shaders.\n"
		" */\n"
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
		"layout(triangles, equal_spacing) in;\n"
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
		"}\n";
	const bool has_gs = piglit_get_gl_version() >= 32;
	const bool has_tess = piglit_get_gl_version() >= 40
		|| piglit_is_extension_supported("GL_ARB_tessellation_shader");

	piglit_require_extension("GL_ARB_separate_shader_objects");

	pass = true;

	if (piglit_get_gl_version() >= 43) {
		ver = 430;
	} else if (piglit_get_gl_version() >= 32) {
		ver = 150;
	} else {
		ver = 110;
	}

	asprintf(&version,
		 "#version %d\n"
		 "#extension GL_ARB_separate_shader_objects: enable\n\n",
		 ver);

	shader_source[0] = version;
	if (has_tess) {
		shader_source[1] = tc_source;
		tcs = glCreateShaderProgramv(GL_TESS_CONTROL_SHADER, 2,
					     shader_source);
		pass = piglit_link_check_status(tcs) && pass;

		shader_source[1] = te_source;
		tes = glCreateShaderProgramv(GL_TESS_EVALUATION_SHADER, 2,
					     shader_source);
		pass = piglit_link_check_status(tes) && pass;
	} else {
		tcs = 0;
		tes = 0;
	}

	if (has_gs) {
		shader_source[1] = gs_source;
		gs = glCreateShaderProgramv(GL_GEOMETRY_SHADER, 2,
					    shader_source);
		pass = piglit_link_check_status(gs) && pass;
	} else {
		gs = 0;
	}

	shader_source[1] = fs_source;
	fs = glCreateShaderProgramv(GL_FRAGMENT_SHADER, 2, shader_source);
	pass = piglit_link_check_status(fs) && pass;

	shader_source[1] = vs_source;
	vs = glCreateShaderProgramv(GL_VERTEX_SHADER, 2, shader_source);
	pass = piglit_link_check_status(vs) && pass;

	glGenProgramPipelines(1, &pipe);
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	glActiveShaderProgram(pipe, fs);
	glGetProgramPipelineiv(pipe, GL_ACTIVE_PROGRAM, &param);
	if (param != fs) {
		fprintf(stderr, "Failed to get Active Program.\n");
		pass = false;
	}
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	use_stage_and_check(pipe, vs, GL_VERTEX_SHADER, true);
	use_stage_and_check(pipe, fs, GL_FRAGMENT_SHADER, true);
	use_stage_and_check(pipe, gs, GL_GEOMETRY_SHADER, has_gs);
	use_stage_and_check(pipe, tes, GL_TESS_EVALUATION_SHADER, has_tess);
	use_stage_and_check(pipe, tcs, GL_TESS_CONTROL_SHADER, has_tess);

	glActiveShaderProgram(pipe, vs);
	glGetProgramPipelineiv(pipe, GL_ACTIVE_PROGRAM, &param);
	if (param != vs) {
		fprintf(stderr, "Failed to get Active Program.\n");
		pass = false;
	}
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;


	glUseProgramStages(pipe, GL_ALL_SHADER_BITS, 0);
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	check_stage(pipe, 0, GL_VERTEX_SHADER, true);
	check_stage(pipe, 0, GL_FRAGMENT_SHADER, true);
	check_stage(pipe, 0, GL_GEOMETRY_SHADER, has_gs);
	check_stage(pipe, 0, GL_TESS_EVALUATION_SHADER, has_tess);
	check_stage(pipe, 0, GL_TESS_CONTROL_SHADER, has_tess);

	free(version);

	piglit_present_results();
	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
