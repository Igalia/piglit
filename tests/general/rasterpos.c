/*
 * Copyright (c) 2019 Advanced Micro Devices, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NON-INFRINGEMENT.  IN NO EVENT SHALL AUTHORS AND/OR THEIR SUPPLIERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.window_width = 200;
	config.window_height = 200;
	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_DEPTH;

PIGLIT_GL_TEST_CONFIG_END

/* Not tested:
 *   GL_CURRENT_RASTER_DISTANCE
 *   GL_CURRENT_RASTER_INDEX
 */

enum {
	POS,
	COLOR0,
	COLOR1,
	TEXCOORD0,
	TEXCOORD1,
	TEXCOORD2,
	TEXCOORD3,
	TEXCOORD4,
	TEXCOORD5,
	TEXCOORD6,
	TEXCOORD7,
};

struct {
	const char *name;
	GLenum glenum;
	unsigned texunit;
} map[] = {
	{"pos", GL_CURRENT_RASTER_POSITION},
	{"color0", GL_CURRENT_RASTER_COLOR},
	{"color1", GL_CURRENT_RASTER_SECONDARY_COLOR},
	{"texcoord0", GL_CURRENT_RASTER_TEXTURE_COORDS, 0},
	{"texcoord1", GL_CURRENT_RASTER_TEXTURE_COORDS, 1},
	{"texcoord2", GL_CURRENT_RASTER_TEXTURE_COORDS, 2},
	{"texcoord3", GL_CURRENT_RASTER_TEXTURE_COORDS, 3},
	{"texcoord4", GL_CURRENT_RASTER_TEXTURE_COORDS, 4},
	{"texcoord5", GL_CURRENT_RASTER_TEXTURE_COORDS, 5},
	{"texcoord6", GL_CURRENT_RASTER_TEXTURE_COORDS, 6},
	{"texcoord7", GL_CURRENT_RASTER_TEXTURE_COORDS, 7},
};

struct raster_pos {
	union {
		float vec[ARRAY_SIZE(map)][4];
		float flt[ARRAY_SIZE(map) * 4];
	};
	GLboolean valid;
};

static void
query_raster(struct raster_pos *r)
{
	for (unsigned i = 0; i < ARRAY_SIZE(map); i++) {
		glActiveTexture(GL_TEXTURE0 + map[i].texunit);
		glGetFloatv(map[i].glenum, r->vec[i]);
	}
	glGetBooleanv(GL_CURRENT_RASTER_POSITION_VALID, &r->valid);
	piglit_check_gl_error(0);
}

static void
print_raster(const struct raster_pos *r)
{
	for (unsigned i = 0; i < ARRAY_SIZE(map); i++) {
		printf("%s = %.3f, %.3f, %.3f, %.3f\n", map[i].name,
		       r->vec[i][0], r->vec[i][1], r->vec[i][2], r->vec[i][3]);
	}
	printf("valid = %u\n", r->valid);
}

static bool
compare_float(const float *probed, const float *expected, unsigned count,
	      const char *name)
{
	for (unsigned i = 0; i < count; ++i) {
		if (fabsf(probed[i] - expected[i]) > 0.0001) {
			printf("Probe %s\n", name);
			printf("  Expected: ");
			for (unsigned j = 0; j < count; j++) {
				printf("%s%f%s", j > 0 ? ", " : "",
				       expected[j], j + 1 == count ? "\n" : "");
			}
			printf("  Observed: ");
			for (unsigned j = 0; j < count; j++) {
				printf("%s%f%s", j > 0 ? ", " : "",
				       probed[j], j + 1 == count ? "\n" : "");
			}
			return false;
		}
	}
	return true;
}

static bool
verify_raster_pos(const struct raster_pos *expected)
{
	bool pass = true;
	struct raster_pos r;

	query_raster(&r);

	for (unsigned i = 0; i < ARRAY_SIZE(map); i++) {
		pass &= compare_float(r.vec[i], expected->vec[i], 4,
				      map[i].name);
	}
	if (r.valid != expected->valid) {
		printf("Probe 'valid'\n");
		printf("  Expected: %u\n", expected->valid);
		printf("  Observed: %u\n", r.valid);
		pass = false;
	}
	return pass;
}

static void
init_raster_pos(struct raster_pos *r)
{
	memset(r, 0, sizeof(*r));
	for (unsigned i = 0; i < ARRAY_SIZE(r->flt); i++)
		r->flt[i] = 0.01 * (i + 1);
	r->vec[POS][3] = 1;
	r->vec[COLOR1][3] = 1; /* the secondary color doesn't have W */
	r->valid = true;
}

static void
set_raster_pos(const struct raster_pos *r)
{
	glColor4fv(r->vec[COLOR0]);
	glSecondaryColor3fv(r->vec[COLOR1]);
	for (unsigned i = 0; i < 8; i++)
		glMultiTexCoord4fv(GL_TEXTURE0 + i, r->vec[TEXCOORD0 + i]);
	glRasterPos4fv(r->vec[POS]);
}


static void
set_raster_pos_fixed_func(const struct raster_pos *r)
{
	set_raster_pos(r);
}

static void
set_raster_pos_arb_vp(const struct raster_pos *r)
{
	char source[2048];

	snprintf(source, sizeof(source),
		 "!!ARBvp1.0\n"
		 "MOV result.position, {%f, %f, %f, %f};\n"
		 "MOV result.color, {%f, %f, %f, %f};\n"
		 "MOV result.color.secondary, {%f, %f, %f, %f};\n"
		 "MOV result.texcoord[0], {%f, %f, %f, %f};\n"
		 "MOV result.texcoord[1], {%f, %f, %f, %f};\n"
		 "MOV result.texcoord[2], {%f, %f, %f, %f};\n"
		 "MOV result.texcoord[3], {%f, %f, %f, %f};\n"
		 "MOV result.texcoord[4], {%f, %f, %f, %f};\n"
		 "MOV result.texcoord[5], {%f, %f, %f, %f};\n"
		 "MOV result.texcoord[6], {%f, %f, %f, %f};\n"
		 "MOV result.texcoord[7], {%f, %f, %f, %f};\n"
		 "END\n",
		 r->flt[0], r->flt[1], r->flt[2], r->flt[3],
		r->flt[4], r->flt[5], r->flt[6], r->flt[7],
		r->flt[8], r->flt[9], r->flt[10], r->flt[11],
		r->flt[12], r->flt[13], r->flt[14], r->flt[15],
		r->flt[16], r->flt[17], r->flt[18], r->flt[19],
		r->flt[20], r->flt[21], r->flt[22], r->flt[23],
		r->flt[24], r->flt[25], r->flt[26], r->flt[27],
		r->flt[28], r->flt[29], r->flt[30], r->flt[31],
		r->flt[32], r->flt[33], r->flt[34], r->flt[35],
		r->flt[36], r->flt[37], r->flt[38], r->flt[39],
		r->flt[40], r->flt[41], r->flt[42], r->flt[43]);

	GLuint vp = piglit_compile_program(GL_VERTEX_PROGRAM_ARB, source);
	assert(vp);
	glEnable(GL_VERTEX_PROGRAM_ARB);
	glBindProgramARB(GL_VERTEX_PROGRAM_ARB, vp);

	struct raster_pos dummy_input = {0};
	set_raster_pos(&dummy_input);

	glBindProgramARB(GL_VERTEX_PROGRAM_ARB, 0);
	glDisable(GL_VERTEX_PROGRAM_ARB);
	glDeleteProgramsARB(1, &vp);
}

static void
set_raster_pos_glsl(const struct raster_pos *r, GLenum next_shader, bool sso)
{
	char vs_source[2048];
	unsigned glsl_version = next_shader == GL_TESS_EVALUATION_SHADER ? 400 :
				next_shader == GL_GEOMETRY_SHADER ? 150 : 110;

	snprintf(vs_source, sizeof(vs_source),
		 "#version %u%s\n"
		 "void main() {\n"
		 "  gl_Position = vec4(%f, %f, %f, %f);\n"
		 "  gl_FrontColor = vec4(%f, %f, %f, %f);\n"
		 "  gl_FrontSecondaryColor = vec4(%f, %f, %f, %f);\n"
		 "  gl_TexCoord[0] = vec4(%f, %f, %f, %f);\n"
		 "  gl_TexCoord[1] = vec4(%f, %f, %f, %f);\n"
		 "  gl_TexCoord[2] = vec4(%f, %f, %f, %f);\n"
		 "  gl_TexCoord[3] = vec4(%f, %f, %f, %f);\n"
		 "  gl_TexCoord[4] = vec4(%f, %f, %f, %f);\n"
		 "  gl_TexCoord[5] = vec4(%f, %f, %f, %f);\n"
		 "  gl_TexCoord[6] = vec4(%f, %f, %f, %f);\n"
		 "  gl_TexCoord[7] = vec4(%f, %f, %f, %f);\n"
		 "}\n",
		 glsl_version, glsl_version >= 150 ? " compatibility" : "",
		 r->flt[0], r->flt[1], r->flt[2], r->flt[3],
		r->flt[4], r->flt[5], r->flt[6], r->flt[7],
		r->flt[8], r->flt[9], r->flt[10], r->flt[11],
		r->flt[12], r->flt[13], r->flt[14], r->flt[15],
		r->flt[16], r->flt[17], r->flt[18], r->flt[19],
		r->flt[20], r->flt[21], r->flt[22], r->flt[23],
		r->flt[24], r->flt[25], r->flt[26], r->flt[27],
		r->flt[28], r->flt[29], r->flt[30], r->flt[31],
		r->flt[32], r->flt[33], r->flt[34], r->flt[35],
		r->flt[36], r->flt[37], r->flt[38], r->flt[39],
		r->flt[40], r->flt[41], r->flt[42], r->flt[43]);

	/* Tessellation shaders should not be run on the raster position. */
	/* Emit a point that will be clipped. */
	const char *tes_source =
		"#version 400 compatibility\n"
		"layout(quads) in;\n"
		"void main() { gl_Position = vec4(-2.0, -2.0, -2.0, 1.0); }\n";

	/* Geometry shaders should not be run on the raster position. */
	/* Don't emit any primitives. */
	const char *gs_source =
		"#version 150 compatibility\n"
		"layout(triangles) in;\n"
		"layout(triangle_strip, max_vertices = 0) out;\n"
		"void main() {}\n";

	const char *next_source = next_shader == GL_TESS_EVALUATION_SHADER ? tes_source :
				  next_shader == GL_GEOMETRY_SHADER ? gs_source : NULL;

	GLuint prog = 0, prog2 = 0, pipeline = 0;
	if (sso) {
		const char *vs = vs_source;
		prog = glCreateShaderProgramv(GL_VERTEX_SHADER, 1,
					      (const GLchar **)&vs);
		prog2 = glCreateShaderProgramv(next_shader, 1,
					       (const GLchar **)&next_source);

		glGenProgramPipelines(1, &pipeline);
		glUseProgramStages(pipeline, GL_VERTEX_SHADER_BIT, prog);
		glUseProgramStages(pipeline,
				   next_shader == GL_GEOMETRY_SHADER ?
					   GL_GEOMETRY_SHADER_BIT :
					   GL_TESS_EVALUATION_SHADER_BIT,
				   prog2);
		glBindProgramPipeline(pipeline);
	} else {
		if (next_source) {
			prog = piglit_build_simple_program_multiple_shaders(
				       GL_VERTEX_SHADER, vs_source,
				       next_shader, next_source);
		} else {
			prog = piglit_build_simple_program(vs_source, NULL);
		}
		assert(prog);
		glUseProgram(prog);
	}

	struct raster_pos dummy_input = {0};
	set_raster_pos(&dummy_input);

	if (sso) {
		glBindProgramPipeline(0);
		glDeleteProgramPipelines(1, &pipeline);
		glDeleteProgram(prog);
		glDeleteProgram(prog2);
	} else {
		glUseProgram(0);
		glDeleteProgram(prog);
	}
}

static void
set_raster_pos_glsl_vs(const struct raster_pos *r)
{
	set_raster_pos_glsl(r, 0, false);
}

static void
set_raster_pos_glsl_vs_tes_linked(const struct raster_pos *r)
{
	set_raster_pos_glsl(r, GL_TESS_EVALUATION_SHADER, false);
}

static void
set_raster_pos_glsl_vs_gs_linked(const struct raster_pos *r)
{
	set_raster_pos_glsl(r, GL_GEOMETRY_SHADER, false);
}

static void
set_raster_pos_glsl_vs_tes_sso(const struct raster_pos *r)
{
	set_raster_pos_glsl(r, GL_TESS_EVALUATION_SHADER, true);
}

static void
set_raster_pos_glsl_vs_gs_sso(const struct raster_pos *r)
{
	set_raster_pos_glsl(r, GL_GEOMETRY_SHADER, true);
}

static void
set_raster_pos_glsl_vs_uniforms(const struct raster_pos *r)
{
	char vs_source[2048];

	snprintf(vs_source, sizeof(vs_source),
		 "#version 110\n"
		 /* Add a dummy fog uniform to test the case when drivers use
		  * the std430 packing for uniforms internally.
		  */
		 "uniform float fog;\n"
		 "uniform vec4 vec[11];\n"
		 "void main() {\n"
		 "  gl_Position = vec[0];\n"
		 "  gl_FrontColor = vec[1];\n"
		 "  gl_FrontSecondaryColor = vec[2];\n"
		 "  gl_TexCoord[0] = vec[3];\n"
		 "  gl_TexCoord[1] = vec[4];\n"
		 "  gl_TexCoord[2] = vec[5];\n"
		 "  gl_TexCoord[3] = vec[6];\n"
		 "  gl_TexCoord[4] = vec[7];\n"
		 "  gl_TexCoord[5] = vec[8];\n"
		 "  gl_TexCoord[6] = vec[9];\n"
		 "  gl_TexCoord[7] = vec[10];\n"
		 "  gl_FogFragCoord = fog;\n"
		 "}\n");

	GLuint prog = piglit_build_simple_program(vs_source, NULL);
	assert(prog);
	glUseProgram(prog);
	glUniform1f(glGetUniformLocation(prog, "fog"), 0);
	glUniform4fv(glGetUniformLocation(prog, "vec"), 11, r->flt);

	struct raster_pos dummy_input = {0};
	set_raster_pos(&dummy_input);

	glUseProgram(0);
	glDeleteProgram(prog);
}

static void
set_raster_pos_from_buffer(const struct raster_pos *r,
			   const char *vs_source, GLenum buffer_target,
			   bool image)
{
	GLuint bo, tex;
	glGenBuffers(1, &bo);
	glBindBuffer(buffer_target, bo);
	glBufferData(buffer_target, sizeof(r->flt), r->flt, GL_STATIC_DRAW);

	glGenTextures(1, &tex);

	if (buffer_target == GL_TEXTURE_BUFFER) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_BUFFER, tex);
		glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, bo);
		if (image) {
			glBindImageTexture(0, tex, 0, false, 0, GL_READ_ONLY,
					   GL_RGBA32F);
		}
	} else {
		glBindBufferBase(buffer_target, 0, bo);
	}

	GLuint prog = piglit_build_simple_program(vs_source, NULL);
	assert(prog);
	glUseProgram(prog);

	struct raster_pos dummy_input = {0};
	set_raster_pos(&dummy_input);

	glUseProgram(0);
	glDeleteProgram(prog);
	glBindBuffer(buffer_target, 0);
	glDeleteBuffers(1, &bo);
	glDeleteTextures(1, &tex);
}

static void
set_raster_pos_glsl_vs_ubo(const struct raster_pos *r)
{
	char vs_source[2048];

	snprintf(vs_source, sizeof(vs_source),
		 "#version 140\n"
		 "uniform block {\n"
		 "  vec4 vec[11];\n"
		 "};\n"
		 "void main() {\n"
		 "  gl_Position = vec[0];\n"
		 "  gl_FrontColor = vec[1];\n"
		 "  gl_FrontSecondaryColor = vec[2];\n"
		 "  gl_TexCoord[0] = vec[3];\n"
		 "  gl_TexCoord[1] = vec[4];\n"
		 "  gl_TexCoord[2] = vec[5];\n"
		 "  gl_TexCoord[3] = vec[6];\n"
		 "  gl_TexCoord[4] = vec[7];\n"
		 "  gl_TexCoord[5] = vec[8];\n"
		 "  gl_TexCoord[6] = vec[9];\n"
		 "  gl_TexCoord[7] = vec[10];\n"
		 "}\n");

	set_raster_pos_from_buffer(r, vs_source, GL_UNIFORM_BUFFER, false);
}

static void
set_raster_pos_glsl_vs_ssbo(const struct raster_pos *r)
{
	char vs_source[2048];

	snprintf(vs_source, sizeof(vs_source),
		 "#version 430 compatibility\n"
		 "layout(binding = 0) buffer ssbo {\n"
		 "  vec4 vec[11];\n"
		 "};\n"
		 "void main() {\n"
		 "  gl_Position = vec[0];\n"
		 "  gl_FrontColor = vec[1];\n"
		 "  gl_FrontSecondaryColor = vec[2];\n"
		 "  gl_TexCoord[0] = vec[3];\n"
		 "  gl_TexCoord[1] = vec[4];\n"
		 "  gl_TexCoord[2] = vec[5];\n"
		 "  gl_TexCoord[3] = vec[6];\n"
		 "  gl_TexCoord[4] = vec[7];\n"
		 "  gl_TexCoord[5] = vec[8];\n"
		 "  gl_TexCoord[6] = vec[9];\n"
		 "  gl_TexCoord[7] = vec[10];\n"
		 "}\n");

	set_raster_pos_from_buffer(r, vs_source, GL_SHADER_STORAGE_BUFFER, false);
}

static void
set_raster_pos_glsl_vs_tbo(const struct raster_pos *r)
{
	char vs_source[2048];

	snprintf(vs_source, sizeof(vs_source),
		 "#version 140\n"
		 "uniform samplerBuffer bo;\n"
		 "void main() {\n"
		 "  gl_Position = texelFetch(bo, 0);\n"
		 "  gl_FrontColor = texelFetch(bo, 1);\n"
		 "  gl_FrontSecondaryColor = texelFetch(bo, 2);\n"
		 "  gl_TexCoord[0] = texelFetch(bo, 3);\n"
		 "  gl_TexCoord[1] = texelFetch(bo, 4);\n"
		 "  gl_TexCoord[2] = texelFetch(bo, 5);\n"
		 "  gl_TexCoord[3] = texelFetch(bo, 6);\n"
		 "  gl_TexCoord[4] = texelFetch(bo, 7);\n"
		 "  gl_TexCoord[5] = texelFetch(bo, 8);\n"
		 "  gl_TexCoord[6] = texelFetch(bo, 9);\n"
		 "  gl_TexCoord[7] = texelFetch(bo, 10);\n"
		 "}\n");

	set_raster_pos_from_buffer(r, vs_source, GL_TEXTURE_BUFFER, false);
}

static void
set_raster_pos_glsl_vs_image_bo(const struct raster_pos *r)
{
	char vs_source[2048];

	snprintf(vs_source, sizeof(vs_source),
		 "#version 420 compatibility\n"
		 "layout(rgba32f) uniform imageBuffer bo;\n"
		 "void main() {\n"
		 "  gl_Position = imageLoad(bo, 0);\n"
		 "  gl_FrontColor = imageLoad(bo, 1);\n"
		 "  gl_FrontSecondaryColor = imageLoad(bo, 2);\n"
		 "  gl_TexCoord[0] = imageLoad(bo, 3);\n"
		 "  gl_TexCoord[1] = imageLoad(bo, 4);\n"
		 "  gl_TexCoord[2] = imageLoad(bo, 5);\n"
		 "  gl_TexCoord[3] = imageLoad(bo, 6);\n"
		 "  gl_TexCoord[4] = imageLoad(bo, 7);\n"
		 "  gl_TexCoord[5] = imageLoad(bo, 8);\n"
		 "  gl_TexCoord[6] = imageLoad(bo, 9);\n"
		 "  gl_TexCoord[7] = imageLoad(bo, 10);\n"
		 "}\n");

	set_raster_pos_from_buffer(r, vs_source, GL_TEXTURE_BUFFER, true);
}

static void
set_raster_pos_glsl_vs_tex(const struct raster_pos *r, const char *vs_source,
			   bool image)
{
	GLuint tex;
	glGenTextures(1, &tex);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_1D, tex);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	static char zeros[22 * sizeof(float) * 4];
	/* The shader reads level 1, so initialize all other levels to zeros. */
	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA32F, 22, 0, GL_RGBA, GL_FLOAT,
		     zeros);
	glTexImage1D(GL_TEXTURE_1D, 1, GL_RGBA32F, 11, 0, GL_RGBA, GL_FLOAT,
		     r->flt);
	glTexImage1D(GL_TEXTURE_1D, 2, GL_RGBA32F, 5, 0, GL_RGBA, GL_FLOAT,
		     zeros);
	glTexImage1D(GL_TEXTURE_1D, 3, GL_RGBA32F, 2, 0, GL_RGBA, GL_FLOAT,
		     zeros);
	glTexImage1D(GL_TEXTURE_1D, 4, GL_RGBA32F, 1, 0, GL_RGBA, GL_FLOAT,
		     zeros);
	if (image) {
		glBindImageTexture(0, tex, 1, false, 0, GL_READ_ONLY,
				   GL_RGBA32F);
	}

	GLuint prog = piglit_build_simple_program(vs_source, NULL);
	assert(prog);
	glUseProgram(prog);

	struct raster_pos dummy_input = {0};
	set_raster_pos(&dummy_input);

	glUseProgram(0);
	glDeleteProgram(prog);
	glBindTexture(GL_TEXTURE_1D, 0);
	glDeleteTextures(1, &tex);
}

static void
set_raster_pos_glsl_vs_image1D(const struct raster_pos *r)
{
	char vs_source[2048];

	snprintf(vs_source, sizeof(vs_source),
		 "#version 420 compatibility\n"
		 "layout(rgba32f) uniform image1D tex;\n"
		 "void main() {\n"
		 "  gl_Position = imageLoad(tex, 0);\n"
		 "  gl_FrontColor = imageLoad(tex, 1);\n"
		 "  gl_FrontSecondaryColor = imageLoad(tex, 2);\n"
		 "  gl_TexCoord[0] = imageLoad(tex, 3);\n"
		 "  gl_TexCoord[1] = imageLoad(tex, 4);\n"
		 "  gl_TexCoord[2] = imageLoad(tex, 5);\n"
		 "  gl_TexCoord[3] = imageLoad(tex, 6);\n"
		 "  gl_TexCoord[4] = imageLoad(tex, 7);\n"
		 "  gl_TexCoord[5] = imageLoad(tex, 8);\n"
		 "  gl_TexCoord[6] = imageLoad(tex, 9);\n"
		 "  gl_TexCoord[7] = imageLoad(tex, 10);\n"
		 "}\n");

	set_raster_pos_glsl_vs_tex(r, vs_source, true);
}

static void
set_raster_pos_glsl_vs_tex1D(const struct raster_pos *r)
{
	char vs_source[2048];

	snprintf(vs_source, sizeof(vs_source),
		 "#version 130\n"
		 "uniform sampler1D tex;\n"
		 "void main() {\n"
		 "  gl_Position = texelFetch(tex, 0, 1);\n"
		 "  gl_FrontColor = texelFetch(tex, 1, 1);\n"
		 "  gl_FrontSecondaryColor = texelFetch(tex, 2, 1);\n"
		 "  gl_TexCoord[0] = texelFetch(tex, 3, 1);\n"
		 "  gl_TexCoord[1] = texelFetch(tex, 4, 1);\n"
		 "  gl_TexCoord[2] = texelFetch(tex, 5, 1);\n"
		 "  gl_TexCoord[3] = texelFetch(tex, 6, 1);\n"
		 "  gl_TexCoord[4] = texelFetch(tex, 7, 1);\n"
		 "  gl_TexCoord[5] = texelFetch(tex, 8, 1);\n"
		 "  gl_TexCoord[6] = texelFetch(tex, 9, 1);\n"
		 "  gl_TexCoord[7] = texelFetch(tex, 10, 1);\n"
		 "}\n");

	set_raster_pos_glsl_vs_tex(r, vs_source, false);
}

#define T(n) #n, set_raster_pos_##n

struct {
	const char *name;
	void (*set_raster_pos)(const struct raster_pos *);
	unsigned gl_version;
} tests[] = {
	{T(fixed_func), 20},
	{T(arb_vp), 20},
	{T(glsl_vs), 20},

	{T(glsl_vs_uniforms), 20},
	{T(glsl_vs_tex1D), 30},
	{T(glsl_vs_ubo), 31},
	{T(glsl_vs_tbo), 31},
	{T(glsl_vs_image_bo), 42},
	{T(glsl_vs_image1D), 42},
	{T(glsl_vs_ssbo), 43},

	{T(glsl_vs_gs_linked), 32},
	{T(glsl_vs_tes_linked), 40},
	{T(glsl_vs_gs_sso), 41},
	{T(glsl_vs_tes_sso), 41},
};

void
piglit_init(int argc, char **argv)
{
	enum piglit_result result = PIGLIT_PASS;
	piglit_require_gl_version(20);

	(void)print_raster;

	for (unsigned i = 0; i < ARRAY_SIZE(tests); i++) {
		if (piglit_get_gl_version() < tests[i].gl_version) {
			piglit_report_subtest_result(PIGLIT_SKIP, "%s", tests[i].name);
			piglit_merge_result(&result, PIGLIT_SKIP);
			continue;
		}

		struct raster_pos input;
		init_raster_pos(&input);
		tests[i].set_raster_pos(&input);

		/* Transform the raster position to match the expected output. */
		input.vec[POS][0] = (input.vec[POS][0] * 0.5 + 0.5) * piglit_width;
		input.vec[POS][1] = (input.vec[POS][1] * 0.5 + 0.5) * piglit_height;
		input.vec[POS][2] = input.vec[POS][2] * 0.5 + 0.5;

		enum piglit_result r = verify_raster_pos(&input) ? PIGLIT_PASS :
								   PIGLIT_FAIL;
		piglit_report_subtest_result(r, "%s", tests[i].name);
		piglit_merge_result(&result, r);
	}

	piglit_report_result(result);
}

enum piglit_result
piglit_display(void)
{
	/* unreachable */
	return PIGLIT_FAIL;
}
