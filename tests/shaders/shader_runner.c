/*
 * Copyright © 2010 Intel Corporation
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

#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#include "piglit-util.h"
#include "piglit-util-gl.h"
#include "piglit-vbo.h"
#include "piglit-framework-gl/piglit_gl_framework.h"
#include "piglit-subprocess.h"

#include "shader_runner_gles_workarounds.h"
#include "parser_utils.h"

#include "shader_runner_vs_passthrough_spv.h"

#define DEFAULT_WINDOW_WIDTH 250
#define DEFAULT_WINDOW_HEIGHT 250

static struct piglit_gl_test_config current_config;

static void
get_required_config(const char *script_name, bool spirv,
		    struct piglit_gl_test_config *config);
static GLenum
decode_drawing_mode(const char *mode_str);

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.window_width = DEFAULT_WINDOW_WIDTH;
	config.window_height = DEFAULT_WINDOW_HEIGHT;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

	/* By default SPIR-V mode is false. It will not be enabled
	 * unless the script includes SPIRV YES or SPIRV ONLY lines at
	 * [require] section, so it will be handled later.
	 */
	if (argc > 1) {
		get_required_config(argv[1], false, &config);
	} else {
		config.supports_gl_compat_version = 10;
	}

	current_config = config;

PIGLIT_GL_TEST_CONFIG_END

static const char passthrough_vertex_shader_source[] =
	"#if __VERSION__ >= 130\n"
	"in vec4 piglit_vertex;\n"
	"#else\n"
	"attribute vec4 piglit_vertex;\n"
	"#endif\n"
	"void main() { gl_Position = piglit_vertex; }\n"
	;

struct component_version {
	enum version_tag {
		VERSION_GL,
		VERSION_GLSL,
	} _tag;

	bool core;
	bool compat;
	bool es;
	unsigned num;
	char _string[100];
};

#define ENUM_STRING(e) { #e, e }

extern float piglit_tolerance[4];

static int test_num = 1;
static struct component_version gl_version;
static struct component_version glsl_version;
static struct component_version glsl_req_version;
static int gl_max_vertex_output_components;
static int gl_max_fragment_uniform_components;
static int gl_max_vertex_uniform_components;
static int gl_max_vertex_attribs;
static int gl_max_varying_components;
static int gl_max_clip_planes;

static const char *test_start = NULL;
static unsigned test_start_line_num = 0;

static GLuint vertex_shaders[256];
static unsigned num_vertex_shaders = 0;
static GLuint tess_ctrl_shaders[256];
static unsigned num_tess_ctrl_shaders = 0;
static GLuint tess_eval_shaders[256];
static unsigned num_tess_eval_shaders = 0;
static GLuint geometry_shaders[256];
static unsigned num_geometry_shaders = 0;
static GLuint fragment_shaders[256];
static unsigned num_fragment_shaders = 0;
static GLuint compute_shaders[256];
static unsigned num_compute_shaders = 0;
static int num_uniform_blocks;
static GLuint *uniform_block_bos;
static GLenum geometry_layout_input_type = GL_TRIANGLES;
static GLenum geometry_layout_output_type = GL_TRIANGLE_STRIP;
static GLint geometry_layout_vertices_out = 0;
static GLuint atomics_bos[8];
static GLuint ssbo[32];

#define SHADER_TYPES 6
static GLuint *subuniform_locations[SHADER_TYPES];
static int num_subuniform_locations[SHADER_TYPES];
static char *shader_string;
static GLint shader_string_size;
static const char *vertex_data_start = NULL;
static const char *vertex_data_end = NULL;
static GLuint prog;
static GLuint sso_vertex_prog;
static GLuint sso_tess_control_prog;
static GLuint sso_tess_eval_prog;
static GLuint sso_geometry_prog;
static GLuint sso_fragment_prog;
static GLuint sso_compute_prog;
static GLuint pipeline = 0;
static size_t num_vbo_rows = 0;
static bool vbo_present = false;
static bool link_ok = false;
static bool prog_in_use = false;
static bool sso_in_use = false;
static bool glsl_in_use = false;
static bool force_glsl = false;
static bool spirv_in_use = false;
static bool spirv_replaces_glsl = false;
static GLchar *prog_err_info = NULL;
static GLuint vao = 0;
static GLuint draw_fbo, read_fbo;
static GLint render_width, render_height;
static GLint read_width, read_height;

static bool report_subtests = false;

struct specialization_list {
	size_t buffer_size;
	size_t n_entries;
	GLuint *indices;
	union { GLuint u; GLfloat f; } *values;
};

static struct specialization_list
specializations[SHADER_TYPES];

static struct texture_binding {
	GLuint obj;
	unsigned width;
	unsigned height;
	unsigned layers;
} texture_bindings[32];

static struct resident_handle {
	GLuint64 handle;
	bool is_tex;
} resident_handles[32];

static void
clear_texture_binding(unsigned idx)
{
	REQUIRE(idx < ARRAY_SIZE(texture_bindings),
		"Invalid texture index %d\n", idx);

	if (texture_bindings[idx].obj) {
		glDeleteTextures(1, &texture_bindings[idx].obj);
		texture_bindings[idx].obj = 0;
	}
}

static void
set_texture_binding(unsigned idx, GLuint obj, unsigned w, unsigned h, unsigned l)
{
	clear_texture_binding(idx);

	REQUIRE(idx < ARRAY_SIZE(texture_bindings),
		"Invalid texture index %d\n", idx);
	texture_bindings[idx].obj = obj;
	texture_bindings[idx].width = w;
	texture_bindings[idx].height = h;
	texture_bindings[idx].layers = l;
}

static const struct texture_binding *
get_texture_binding(unsigned idx)
{
	REQUIRE(idx < ARRAY_SIZE(texture_bindings),
		"Invalid texture index %d\n", idx);
	REQUIRE(texture_bindings[idx].obj,
		"No texture bound at %d\n", idx);
	return &texture_bindings[idx];
}

static void
clear_resident_handle(unsigned idx)
{
	REQUIRE(idx < ARRAY_SIZE(resident_handles),
		"Invalid resident handle index %d\n", idx);

	if (resident_handles[idx].handle) {
		GLuint64 handle = resident_handles[idx].handle;
		if (resident_handles[idx].is_tex) {
			if (glIsTextureHandleResidentARB(handle))
				glMakeTextureHandleNonResidentARB(handle);
		} else {
			if (glIsImageHandleResidentARB(handle))
				glMakeImageHandleNonResidentARB(handle);
		}
		resident_handles[idx].handle = 0;
	}
}

static void
set_resident_handle(unsigned idx, GLuint64 handle, bool is_tex)
{
	clear_resident_handle(idx);

	REQUIRE(idx < ARRAY_SIZE(resident_handles),
		"Invalid resident handle index %d\n", idx);
	resident_handles[idx].handle = handle;
	resident_handles[idx].is_tex = is_tex;
}

static const struct resident_handle *
get_resident_handle(unsigned idx)
{
	REQUIRE(idx < ARRAY_SIZE(resident_handles),
		"Invalid resident handle index %d\n", idx);
	REQUIRE(resident_handles[idx].handle,
		"No resident handle at %d\n", idx);
	return &resident_handles[idx];
}

enum states {
	none = 0,
	requirements,
	vertex_shader,
	vertex_shader_passthrough,
	vertex_shader_spirv,
	vertex_shader_specializations,
	vertex_program,
	tess_ctrl_shader,
	tess_ctrl_shader_spirv,
	tess_ctrl_shader_specializations,
	tess_eval_shader,
	tess_eval_shader_spirv,
	tess_eval_shader_specializations,
	geometry_shader,
	geometry_shader_spirv,
	geometry_shader_specializations,
	geometry_layout,
	fragment_shader,
	fragment_shader_spirv,
	fragment_shader_specializations,
	fragment_program,
	compute_shader,
	compute_shader_spirv,
	compute_shader_specializations,
	vertex_data,
	test,
};

static const struct string_to_enum all_types[] = {
	ENUM_STRING(GL_FLOAT),
	ENUM_STRING(GL_FLOAT_VEC2),
	ENUM_STRING(GL_FLOAT_VEC3),
	ENUM_STRING(GL_FLOAT_VEC4),
	ENUM_STRING(GL_DOUBLE),
	ENUM_STRING(GL_DOUBLE_VEC2),
	ENUM_STRING(GL_DOUBLE_VEC3),
	ENUM_STRING(GL_DOUBLE_VEC4),
	ENUM_STRING(GL_INT),
	ENUM_STRING(GL_INT_VEC2),
	ENUM_STRING(GL_INT_VEC3),
	ENUM_STRING(GL_INT_VEC4),
	ENUM_STRING(GL_UNSIGNED_INT),
	ENUM_STRING(GL_UNSIGNED_INT_VEC2),
	ENUM_STRING(GL_UNSIGNED_INT_VEC3),
	ENUM_STRING(GL_UNSIGNED_INT_VEC4),
	ENUM_STRING(GL_INT64_ARB),
	ENUM_STRING(GL_INT64_VEC2_ARB),
	ENUM_STRING(GL_INT64_VEC3_ARB),
	ENUM_STRING(GL_INT64_VEC4_ARB),
	ENUM_STRING(GL_UNSIGNED_INT64_ARB),
	ENUM_STRING(GL_UNSIGNED_INT64_VEC2_ARB),
	ENUM_STRING(GL_UNSIGNED_INT64_VEC3_ARB),
	ENUM_STRING(GL_UNSIGNED_INT64_VEC4_ARB),
	ENUM_STRING(GL_BOOL),
	ENUM_STRING(GL_BOOL_VEC2),
	ENUM_STRING(GL_BOOL_VEC3),
	ENUM_STRING(GL_BOOL_VEC4),
	ENUM_STRING(GL_FLOAT_MAT2),
	ENUM_STRING(GL_FLOAT_MAT3),
	ENUM_STRING(GL_FLOAT_MAT4),
	ENUM_STRING(GL_FLOAT_MAT2x3),
	ENUM_STRING(GL_FLOAT_MAT2x4),
	ENUM_STRING(GL_FLOAT_MAT3x2),
	ENUM_STRING(GL_FLOAT_MAT3x4),
	ENUM_STRING(GL_FLOAT_MAT4x2),
	ENUM_STRING(GL_FLOAT_MAT4x3),
	ENUM_STRING(GL_DOUBLE_MAT2),
	ENUM_STRING(GL_DOUBLE_MAT3),
	ENUM_STRING(GL_DOUBLE_MAT4),
	ENUM_STRING(GL_DOUBLE_MAT2x3),
	ENUM_STRING(GL_DOUBLE_MAT2x4),
	ENUM_STRING(GL_DOUBLE_MAT3x2),
	ENUM_STRING(GL_DOUBLE_MAT3x4),
	ENUM_STRING(GL_DOUBLE_MAT4x2),
	ENUM_STRING(GL_DOUBLE_MAT4x3),
	ENUM_STRING(GL_SAMPLER_1D),
	ENUM_STRING(GL_SAMPLER_2D),
	ENUM_STRING(GL_SAMPLER_3D),
	ENUM_STRING(GL_SAMPLER_CUBE),
	ENUM_STRING(GL_SAMPLER_1D_SHADOW),
	ENUM_STRING(GL_SAMPLER_2D_SHADOW),
	ENUM_STRING(GL_SAMPLER_1D_ARRAY),
	ENUM_STRING(GL_SAMPLER_2D_ARRAY),
	ENUM_STRING(GL_SAMPLER_1D_ARRAY_SHADOW),
	ENUM_STRING(GL_SAMPLER_2D_ARRAY_SHADOW),
	ENUM_STRING(GL_SAMPLER_2D_MULTISAMPLE),
	ENUM_STRING(GL_SAMPLER_2D_MULTISAMPLE_ARRAY),
	ENUM_STRING(GL_SAMPLER_CUBE_SHADOW),
	ENUM_STRING(GL_SAMPLER_BUFFER),
	ENUM_STRING(GL_SAMPLER_2D_RECT),
	ENUM_STRING(GL_SAMPLER_2D_RECT_SHADOW),
	ENUM_STRING(GL_INT_SAMPLER_1D),
	ENUM_STRING(GL_INT_SAMPLER_2D),
	ENUM_STRING(GL_INT_SAMPLER_3D),
	ENUM_STRING(GL_INT_SAMPLER_CUBE),
	ENUM_STRING(GL_INT_SAMPLER_1D_ARRAY),
	ENUM_STRING(GL_INT_SAMPLER_2D_ARRAY),
	ENUM_STRING(GL_INT_SAMPLER_2D_MULTISAMPLE),
	ENUM_STRING(GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY),
	ENUM_STRING(GL_INT_SAMPLER_BUFFER),
	ENUM_STRING(GL_INT_SAMPLER_2D_RECT),
	ENUM_STRING(GL_UNSIGNED_INT_SAMPLER_1D),
	ENUM_STRING(GL_UNSIGNED_INT_SAMPLER_2D),
	ENUM_STRING(GL_UNSIGNED_INT_SAMPLER_3D),
	ENUM_STRING(GL_UNSIGNED_INT_SAMPLER_CUBE),
	ENUM_STRING(GL_UNSIGNED_INT_SAMPLER_1D_ARRAY),
	ENUM_STRING(GL_UNSIGNED_INT_SAMPLER_2D_ARRAY),
	ENUM_STRING(GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE),
	ENUM_STRING(GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY),
	ENUM_STRING(GL_UNSIGNED_INT_SAMPLER_BUFFER),
	ENUM_STRING(GL_UNSIGNED_INT_SAMPLER_2D_RECT),
	{ NULL, 0 }
};

static bool
compare(float ref, float value, enum comparison cmp);

static bool
compare_uint(GLuint ref, GLuint value, enum comparison cmp);

static void
version_init(struct component_version *v, enum version_tag tag, bool core, bool compat, bool es, unsigned num)
{
	assert(tag == VERSION_GL || tag == VERSION_GLSL);

	v->_tag = tag;
	v->core = core;
	v->compat = compat;
	v->es = es;
	v->num = num;
	v->_string[0] = 0;
}

static void
version_copy(struct component_version *dest, struct component_version *src)
{
	memcpy(dest, src, sizeof(*dest));
}

static bool
version_compare(struct component_version *a, struct component_version *b, enum comparison cmp)
{
	assert(a->_tag == b->_tag);

	if (a->es != b->es)
		return false;

	return compare(a->num, b->num, cmp);
}

/**
 * Get the version string.
 */
static const char*
version_string(struct component_version *v)
{
	if (v->_string[0])
		return v->_string;

	switch (v->_tag) {
	case VERSION_GL:
		snprintf(v->_string, sizeof(v->_string) - 1, "GL%s %d.%d",
		         v->es ? " ES" : "",
		         v->num / 10, v->num % 10);
		break;
	case VERSION_GLSL:
		snprintf(v->_string, sizeof(v->_string) - 1, "GLSL%s %d.%d",
		         v->es ? " ES" : "",
		         v->num / 100, v->num % 100);
		break;
	default:
		assert(false);
		break;
	}

	return v->_string;
}


static const char *
target_to_short_name(GLenum target)
{
	switch (target) {
	case GL_VERTEX_SHADER:
		return "VS";
	case GL_FRAGMENT_SHADER:
		return "FS";
	case GL_TESS_CONTROL_SHADER:
		return "TCS";
	case GL_TESS_EVALUATION_SHADER:
		return "TES";
	case GL_GEOMETRY_SHADER:
		return "GS";
	case GL_COMPUTE_SHADER:
		return "CS";
	default:
		return "???";
	}
}


static enum piglit_result
compile_glsl(GLenum target)
{
	GLuint shader = glCreateShader(target);
	GLint ok;

	if (spirv_in_use) {
		printf("Cannot mix SPIRV and non-SPIRV shaders\n");
		return PIGLIT_FAIL;
	}

	glsl_in_use = true;

	switch (target) {
	case GL_VERTEX_SHADER:
		if (piglit_get_gl_version() < 20 &&
		    !(piglit_is_extension_supported("GL_ARB_shader_objects") &&
		      piglit_is_extension_supported("GL_ARB_vertex_shader")))
			return PIGLIT_SKIP;
		break;
	case GL_FRAGMENT_SHADER:
		if (piglit_get_gl_version() < 20 &&
		    !(piglit_is_extension_supported("GL_ARB_shader_objects") &&
		      piglit_is_extension_supported("GL_ARB_fragment_shader")))
			return PIGLIT_SKIP;
		break;
	case GL_TESS_CONTROL_SHADER:
	case GL_TESS_EVALUATION_SHADER:
		if (gl_version.num < (gl_version.es ? 32 : 40))
			if (!piglit_is_extension_supported(gl_version.es ?
							   "GL_OES_tessellation_shader" :
							   "GL_ARB_tessellation_shader"))
				return PIGLIT_SKIP;
		break;
	case GL_GEOMETRY_SHADER:
		if (gl_version.num < 32)
			if (!piglit_is_extension_supported(gl_version.es ?
							   "GL_OES_geometry_shader" :
							   "GL_ARB_geometry_shader4"))
				return PIGLIT_SKIP;
		break;
	case GL_COMPUTE_SHADER:
		if (gl_version.num < (gl_version.es ? 31 : 43))
			if (!piglit_is_extension_supported("GL_ARB_compute_shader"))
				return PIGLIT_SKIP;
		break;
	}

	if (!glsl_req_version.num) {
		printf("GLSL version requirement missing\n");
		return PIGLIT_FAIL;
	}

	if (!strstr(shader_string, "#version ")) {
		char *shader_strings[2];
		char version_string[100];
		GLint shader_string_sizes[2];

		/* Add a #version directive based on the GLSL requirement. */
		sprintf(version_string, "#version %d", glsl_req_version.num);
		if (glsl_req_version.es && glsl_req_version.num != 100) {
			strcat(version_string, " es");
		}
		strcat(version_string, "\n");
		shader_strings[0] = version_string;
		shader_string_sizes[0] = strlen(version_string);
		shader_strings[1] = shader_string;
		shader_string_sizes[1] = shader_string_size;

		glShaderSource(shader, 2,
				    (const GLchar **) shader_strings,
				    shader_string_sizes);

	} else {
		glShaderSource(shader, 1,
				    (const GLchar **) &shader_string,
				    &shader_string_size);
	}

	glCompileShader(shader);

	glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);

	if (!ok) {
		GLchar *info;
		GLint size;

		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &size);
		info = malloc(size);

		glGetShaderInfoLog(shader, size, NULL, info);

		fprintf(stderr, "Failed to compile %s: %s\n",
			target_to_short_name(target),
			info);

		free(info);
		return PIGLIT_FAIL;
	}

	switch (target) {
	case GL_VERTEX_SHADER:
		vertex_shaders[num_vertex_shaders] = shader;
		num_vertex_shaders++;
		break;
	case GL_TESS_CONTROL_SHADER:
		tess_ctrl_shaders[num_tess_ctrl_shaders] = shader;
		num_tess_ctrl_shaders++;
		break;
	case GL_TESS_EVALUATION_SHADER:
		tess_eval_shaders[num_tess_eval_shaders] = shader;
		num_tess_eval_shaders++;
		break;
	case GL_GEOMETRY_SHADER:
		geometry_shaders[num_geometry_shaders] = shader;
		num_geometry_shaders++;
		break;
	case GL_FRAGMENT_SHADER:
		fragment_shaders[num_fragment_shaders] = shader;
		num_fragment_shaders++;
		break;
	case GL_COMPUTE_SHADER:
		compute_shaders[num_compute_shaders] = shader;
		num_compute_shaders++;
		break;
	}
	return PIGLIT_PASS;
}


static enum piglit_result
compile_and_bind_program(GLenum target, const char *start, int len)
{
	GLuint prog;
	char *source;

	switch (target) {
	case GL_VERTEX_PROGRAM_ARB:
		if (!piglit_is_extension_supported("GL_ARB_vertex_program"))
			return PIGLIT_SKIP;
		break;
	case GL_FRAGMENT_PROGRAM_ARB:
		if (!piglit_is_extension_supported("GL_ARB_fragment_program"))
			return PIGLIT_SKIP;
		break;
	}

	source = malloc(len + 1);
	memcpy(source, start, len);
	source[len] = 0;
	prog = piglit_compile_program(target, source);

	glEnable(target);
	glBindProgramARB(target, prog);
	link_ok = true;
	prog_in_use = true;

	return PIGLIT_PASS;
}

static enum piglit_result
load_and_specialize_spirv(GLenum target,
			  const char *binary, unsigned size)
{
	if (glsl_in_use) {
		printf("Cannot mix SPIR-V and non-SPIR-V shaders\n");
		return PIGLIT_FAIL;
	}

	spirv_in_use = true;

	GLuint shader = glCreateShader(target);

	glShaderBinary(1, &shader, GL_SHADER_BINARY_FORMAT_SPIR_V_ARB,
		       binary, size);

	const struct specialization_list *specs;

	switch (target) {
	case GL_VERTEX_SHADER:
		specs = specializations + 0;
		break;
	case GL_TESS_CONTROL_SHADER:
		specs = specializations + 1;
		break;
	case GL_TESS_EVALUATION_SHADER:
		specs = specializations + 2;
		break;
	case GL_GEOMETRY_SHADER:
		specs = specializations + 3;
		break;
	case GL_FRAGMENT_SHADER:
		specs = specializations + 4;
		break;
	case GL_COMPUTE_SHADER:
		specs = specializations + 5;
		break;
	default:
		assert(!"Should not get here.");
	}

	glSpecializeShaderARB(shader,
			      "main",
			      specs->n_entries,
			      specs->indices,
			      &specs->values[0].u);

	GLint ok;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);

	if (!ok) {
		GLchar *info;
		GLint size;

		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &size);
		info = malloc(size);

		glGetShaderInfoLog(shader, size, NULL, info);

		printf("Failed to specialize %s: %s\n",
		       target_to_short_name(target), info);

		free(info);
		return PIGLIT_FAIL;
	}

	switch (target) {
	case GL_VERTEX_SHADER:
		vertex_shaders[num_vertex_shaders] = shader;
		num_vertex_shaders++;
		break;
	case GL_TESS_CONTROL_SHADER:
		tess_ctrl_shaders[num_tess_ctrl_shaders] = shader;
		num_tess_ctrl_shaders++;
		break;
	case GL_TESS_EVALUATION_SHADER:
		tess_eval_shaders[num_tess_eval_shaders] = shader;
		num_tess_eval_shaders++;
		break;
	case GL_GEOMETRY_SHADER:
		geometry_shaders[num_geometry_shaders] = shader;
		num_geometry_shaders++;
		break;
	case GL_FRAGMENT_SHADER:
		fragment_shaders[num_fragment_shaders] = shader;
		num_fragment_shaders++;
		break;
	case GL_COMPUTE_SHADER:
		compute_shaders[num_compute_shaders] = shader;
		num_compute_shaders++;
		break;
	}

	return PIGLIT_PASS;
}

static enum piglit_result
assemble_spirv(GLenum target)
{
	if (!piglit_is_extension_supported("GL_ARB_gl_spirv")) {
		return PIGLIT_SKIP;
	}

	char *arguments[] = {
		getenv("PIGLIT_SPIRV_AS_BINARY"),
		"-o", "-",
		NULL
	};

	if (arguments[0] == NULL)
		arguments[0] = "spirv-as";

	/* Strip comments from the source */
	char *stripped_source = malloc(shader_string_size);
	char *p = stripped_source;
	bool at_start_of_line = true;

	for (const char *in = shader_string;
	     in < shader_string + shader_string_size;
	     in++) {
		if (*in == '#' && at_start_of_line) {
			const char *end;
			end = memchr(in,
				     '\n',
				     shader_string + shader_string_size - in);
			if (end == NULL)
				break;
			in = end;
		} else {
			at_start_of_line = *in == '\n';
			*(p++) = *in;
		}
	}

	uint8_t *binary_source;
	size_t binary_source_length;
	bool res = piglit_subprocess(arguments,
				     p - stripped_source,
				     (const uint8_t *)
				     stripped_source,
				     &binary_source_length,
				     &binary_source);

	free(stripped_source);

	if (!res) {
		fprintf(stderr, "spirv-as failed\n");
		return PIGLIT_FAIL;
	}

	enum piglit_result ret;

	ret = load_and_specialize_spirv(target,
					(const char *)
					binary_source,
					binary_source_length);

	free(binary_source);

	return ret;
}

static enum piglit_result
link_sso(GLenum target)
{
	GLint ok;

	glLinkProgram(prog);

	glGetProgramiv(prog, GL_LINK_STATUS, &ok);
	if (ok) {
		link_ok = true;
	} else {
		GLint size;

		glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &size);
		prog_err_info = malloc(size);

		glGetProgramInfoLog(prog, size, NULL, prog_err_info);

		fprintf(stderr, "SSO glLinkProgram(%s) failed: %s\n",
			target_to_short_name(target),
			prog_err_info);

		free(prog_err_info);
		return PIGLIT_FAIL;
	}

	switch (target) {
	case GL_VERTEX_SHADER:
		sso_vertex_prog = prog;
		glUseProgramStages(pipeline, GL_VERTEX_SHADER_BIT, prog);
		break;
	case GL_TESS_CONTROL_SHADER:
		sso_tess_control_prog = prog;
		glUseProgramStages(pipeline, GL_TESS_CONTROL_SHADER_BIT, prog);
		break;
	case GL_TESS_EVALUATION_SHADER:
		sso_tess_eval_prog = prog;
		glUseProgramStages(pipeline, GL_TESS_EVALUATION_SHADER_BIT, prog);
		break;
	case GL_GEOMETRY_SHADER:
		sso_geometry_prog = prog;
		glUseProgramStages(pipeline, GL_GEOMETRY_SHADER_BIT, prog);
		break;
	case GL_FRAGMENT_SHADER:
		sso_fragment_prog = prog;
		glUseProgramStages(pipeline, GL_FRAGMENT_SHADER_BIT, prog);
		break;
	case GL_COMPUTE_SHADER:
		sso_compute_prog = prog;
		glUseProgramStages(pipeline, GL_COMPUTE_SHADER_BIT, prog);
		break;
	}
	return PIGLIT_PASS;
}

/**
 * Compare two values given a specified comparison operator
 */
static bool
compare(float ref, float value, enum comparison cmp)
{
	switch (cmp) {
	case equal:         return value == ref;
	case not_equal:     return value != ref;
	case less:          return value <  ref;
	case greater_equal: return value >= ref;
	case greater:       return value >  ref;
	case less_equal:    return value <= ref;
	}

	assert(!"Should not get here.");
	return false;
}

static bool
compare_uint(GLuint ref, GLuint value, enum comparison cmp)
{
	switch (cmp) {
	case equal:         return value == ref;
	case not_equal:     return value != ref;
	case less:          return value <  ref;
	case greater_equal: return value >= ref;
	case greater:       return value >  ref;
	case less_equal:    return value <= ref;
	}

	assert(!"Should not get here.");
	return false;
}

/**
 * Get the string representation of a comparison operator
 */
static const char *
comparison_string(enum comparison cmp)
{
	switch (cmp) {
	case equal:         return "==";
	case not_equal:     return "!=";
	case less:          return "<";
	case greater_equal: return ">=";
	case greater:       return ">";
	case less_equal:    return "<=";
	}

	assert(!"Should not get here.");
	return false;
}

/**
 * " ES" before the comparison operator indicates the version
 * pertains to GL ES.
 */
static void
parse_version_comparison(const char *line, enum comparison *cmp,
			 struct component_version *v, enum version_tag tag)
{
	unsigned major;
	unsigned minor;
	unsigned full_num;
	const bool core = parse_str(line, "CORE", &line);
	const bool compat = parse_str(line, "COMPAT", &line);
	const bool es = parse_str(line, "ES", &line);

	REQUIRE(parse_comparison_op(line, cmp, &line),
		"Invalid comparison operation at: %s\n", line);

	REQUIRE(parse_uint(line, &major, &line) &&
		parse_str(line, ".", &line) &&
		parse_uint(line, &minor, &line),
		"Invalid version string: %s\n", line);

	parse_whitespace(line, &line);
	if (*line != '\n') {
		printf("Unexpected characters following version comparison\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	/* This hack is so that we can tell the difference between GL versions
	 * and GLSL versions.  All GL versions look like 3.2, and we want the
	 * integer to be 32.  All GLSL versions look like 1.40, and we want
	 * the integer to be 140.
	 */
	if (tag == VERSION_GLSL) {
		full_num = (major * 100) + minor;
	} else {
		full_num = (major * 10) + minor;
	}

	version_init(v, tag, core, compat, es, full_num);
}

#define KNOWN_GL_SPV_MAPPING 3

static const char* table[KNOWN_GL_SPV_MAPPING][2] =
{{ "GL_AMD_shader_trinary_minmax", "SPV_AMD_shader_trinary_minmax"},
 {"GL_ARB_shader_group_vote", "SPV_KHR_subgroup_vote"},
 {"GL_ARB_shader_ballot", "SPV_KHR_shader_ballot"}};

/*
 * Returns the SPIR-V extension that defines the equivalent
 * funcionality provided by the OpenGL extension @gl_name
 */
static const char *
check_spv_extension_equivalent(const char *gl_name)
{
        unsigned int i;

        for (i = 0 ; i < KNOWN_GL_SPV_MAPPING; i++)
                if (strcmp(gl_name, table[i][0]) == 0)
                        return table[i][1];

	return NULL;
}


/* Perhaps we could move this to piglit-util. For now it is only used
 * on shader_runner
 */
static const char **spirv_extensions = NULL;

static void
initialize_spv_extensions(void)
{
	int loop, num_spir_v_extensions;

	if (spirv_extensions != NULL)
		return;

	glGetIntegerv(GL_NUM_SPIR_V_EXTENSIONS, &num_spir_v_extensions);
	spirv_extensions = malloc (sizeof(char*) * (num_spir_v_extensions + 1));
	assert (spirv_extensions != NULL);

	for (loop = 0; loop < num_spir_v_extensions; loop++) {
		spirv_extensions[loop] = (const char*) glGetStringi(GL_SPIR_V_EXTENSIONS, loop);
	}

	spirv_extensions[loop] = NULL;
}

static bool
spv_is_extension_supported(const char *name)
{
	initialize_spv_extensions();
	return piglit_is_extension_in_array(spirv_extensions, name);
}

/*
 * Wrapper for extension checking.  If using a GLSL shader, this
 * method is equivalent to call piglit_is_extension_supported.
 *
 * But with SPIR-V shaders we can't just check for the OpenGL
 * extension, but also for the SPV equivalent if using a SPIR-V
 * binary, as it is not mandatory to support the SPV extension even if
 * the equivalent OpenGL one is.
 */
static bool
shader_runner_is_extension_supported(const char *name)
{
	bool result = true;
	if (spirv_replaces_glsl) {
		const char *spv_name = check_spv_extension_equivalent(name);

		if (spv_name != NULL)
			result = result && spv_is_extension_supported(spv_name);
	}

	return result && piglit_is_extension_supported(name);
}

/**
 * Parse and check a line from the requirement section of the test
 */
static enum piglit_result
process_requirement(const char *line)
{
	char buffer[4096];
	static const struct {
		const char *name;
		int *val;
		const char *desc;
	} getint_limits[] = {
		{
			"GL_MAX_VERTEX_OUTPUT_COMPONENTS",
			&gl_max_vertex_output_components,
			"vertex output components",
		},
		{
			"GL_MAX_FRAGMENT_UNIFORM_COMPONENTS",
			&gl_max_fragment_uniform_components,
			"fragment uniform components",
		},
		{
			"GL_MAX_VERTEX_UNIFORM_COMPONENTS",
			&gl_max_vertex_uniform_components,
			"vertex uniform components",
		},
		{
			"GL_MAX_VERTEX_ATTRIBS",
			&gl_max_vertex_attribs,
			"vertex attribs",
		},
		{
			"GL_MAX_VARYING_COMPONENTS",
			&gl_max_varying_components,
			"varying components",
		},
	};
	unsigned i;

	/* The INT keyword in the requirements section causes
	 * shader_runner to read the specified integer value and
	 * processes the given requirement.
	 */
	if (parse_str(line, "INT ", &line)) {
		enum comparison cmp;
		int comparison_value, gl_int_value;
		unsigned int_enum;

		REQUIRE(parse_enum_gl(line, &int_enum, &line),
			"Invalid comparison enum at: %s\n", line);
		REQUIRE(parse_comparison_op(line, &cmp, &line),
			"Invalid comparison operation at: %s\n", line);
		REQUIRE(parse_int(line, &comparison_value, &line),
			"Invalid comparison value at: %s\n", line);

		glGetIntegerv(int_enum, &gl_int_value);
		if (!piglit_check_gl_error(GL_NO_ERROR)) {
			fprintf(stderr, "Error reading %s\n",
				piglit_get_gl_enum_name(int_enum));
			return PIGLIT_FAIL;
		}

		if (!compare(comparison_value, gl_int_value, cmp)) {
			printf("Test requires %s %s %i.  "
			       "The driver supports %i.\n",
			       piglit_get_gl_enum_name(int_enum),
			       comparison_string(cmp),
			       comparison_value,
			       gl_int_value);
			return PIGLIT_SKIP;
		}

		return PIGLIT_PASS;
	}

	/* There are five types of requirements that a test can currently
	 * have:
	 *
	 *    * Require that some GL extension be supported
	 *    * Require some particular versions of GL
	 *    * Require some particular versions of GLSL
	 *    * Require some particular number of uniform components
	 *    * Require shaders be built as separate shader objects
	 *
	 * The tests for GL and GLSL versions can be equal, not equal,
	 * less, less-or-equal, greater, or greater-or-equal.  Extension tests
	 * can also require that a particular extension not be supported by
	 * prepending ! to the extension name.
	 */
	for (i = 0; i < ARRAY_SIZE(getint_limits); i++) {
		enum comparison cmp;
		int maxcomp;

		if (!parse_str(line, getint_limits[i].name, &line))
			continue;

		REQUIRE(parse_comparison_op(line, &cmp, &line),
			"Invalid comparison operation at: %s\n", line);

		maxcomp = atoi(line);
		if (!compare(maxcomp, *getint_limits[i].val, cmp)) {
			printf("Test requires %s %s %i.  "
			       "The driver supports %i.\n",
			       getint_limits[i].desc,
			       comparison_string(cmp),
			       maxcomp,
			       *getint_limits[i].val);
			return PIGLIT_SKIP;
		}
		return PIGLIT_PASS;
	}

	if (parse_str(line, "GL_", NULL) &&
	    parse_word_copy(line, buffer, sizeof(buffer), &line)) {
		if (!shader_runner_is_extension_supported(buffer))
			return PIGLIT_SKIP;
	} else if (parse_str(line, "!", &line) &&
		   parse_str(line, "GL_", NULL) &&
		   parse_word_copy(line, buffer, sizeof(buffer), &line)) {
		if (shader_runner_is_extension_supported(buffer))
			return PIGLIT_SKIP;
	} else if (parse_str(line, "GLSL", &line)) {
		enum comparison cmp;

		parse_version_comparison(line, &cmp, &glsl_req_version,
		                         VERSION_GLSL);

		/* We only allow >= because we potentially use the
		 * version number to insert a #version directive. */
		if (cmp != greater_equal) {
			printf("Unsupported GLSL version comparison\n");
			return PIGLIT_FAIL;
		}

		if (!version_compare(&glsl_req_version, &glsl_version, cmp)) {
			printf("Test requires %s %s.  "
			       "Actual version %s.\n",
			       comparison_string(cmp),
			       version_string(&glsl_req_version),
			       version_string(&glsl_version));
			return PIGLIT_SKIP;
		}
	} else if (parse_str(line, "GL", &line)) {
		enum comparison cmp;
		struct component_version gl_req_version;

		parse_version_comparison(line, &cmp, &gl_req_version,
		                         VERSION_GL);

		if (!version_compare(&gl_req_version, &gl_version, cmp)) {
			printf("Test requires %s %s.  "
			       "Actual version is %s.\n",
			       comparison_string(cmp),
			       version_string(&gl_req_version),
			       version_string(&gl_version));
			return PIGLIT_SKIP;
		}
	} else if (parse_str(line, "rlimit", &line)) {
		unsigned lim;

		REQUIRE(parse_uint(line, &lim, &line),
			"Invalid rlimit argument at: %s\n", line);

		piglit_set_rlimit(lim);
	}  else if (parse_str(line, "SSO", &line) &&
		    parse_str(line, "ENABLED", NULL)) {
		const char *const ext_name = gl_version.es
			? "GL_EXT_separate_shader_objects"
			: "GL_ARB_separate_shader_objects";
		const unsigned min_version = gl_version.es
			? 31 : 41;

		if (gl_version.num < min_version)
			piglit_require_extension(ext_name);

		sso_in_use = true;
		glGenProgramPipelines(1, &pipeline);
	} else if (parse_str(line, "SPIRV", &line)) {
		spirv_replaces_glsl = !force_glsl;

		if (parse_str(line, "ONLY", NULL)) {
			if (force_glsl) {
				printf("This shader is not compatible with GLSL\n");
				return PIGLIT_SKIP;
			}
		} else if (parse_str(line, "YES", NULL)) {
			/* Empty. Everything already set. Just parsing
			 * correct options
			 */
		} else {
			printf("Unknown SPIRV line in [require]\n");
			return PIGLIT_FAIL;
		}
	}
	return PIGLIT_PASS;
}


/**
 * Process a line from the [geometry layout] section of a test
 */
static void
process_geometry_layout(const char *line)
{
	char s[32];
	int x;

	parse_whitespace(line, &line);

	if (line[0] == '\0' || line[0] == '\n') {
		return;
	} else if (sscanf(line, "input type %31s", s) == 1) {
		geometry_layout_input_type = decode_drawing_mode(s);
	} else if (sscanf(line, "output type %31s", s) == 1) {
		geometry_layout_output_type = decode_drawing_mode(s);
	} else if (sscanf(line, "vertices out %d", &x) == 1) {
		geometry_layout_vertices_out = x;
	} else {
		printf("Could not parse geometry layout line: %s\n", line);
		piglit_report_result(PIGLIT_FAIL);
	}
}

static enum piglit_result
leave_state(enum states state, const char *line, const char *script_name)
{
	switch (state) {
	case none:
		break;

	case requirements:
		if (spirv_replaces_glsl) {
			printf("Running on SPIR-V mode\n");
		}
		break;

	case vertex_shader:
		if (spirv_replaces_glsl)
			break;
		shader_string_size = line - shader_string;
		return compile_glsl(GL_VERTEX_SHADER);

	case vertex_shader_passthrough:
		if (spirv_replaces_glsl) {
			shader_string = (char *) passthrough_vertex_shader_source_spv;
			shader_string_size = strlen(passthrough_vertex_shader_source_spv);

			return assemble_spirv(GL_VERTEX_SHADER);
		}
		return compile_glsl(GL_VERTEX_SHADER);

	case vertex_program:
		return compile_and_bind_program(GL_VERTEX_PROGRAM_ARB,
						shader_string,
						line - shader_string);

	case vertex_shader_spirv:
		if (!spirv_replaces_glsl)
			break;
		shader_string_size = line - shader_string;
		return assemble_spirv(GL_VERTEX_SHADER);

	case vertex_shader_specializations:
		break;

	case tess_ctrl_shader:
		if (spirv_replaces_glsl)
			break;
		shader_string_size = line - shader_string;
		return compile_glsl(GL_TESS_CONTROL_SHADER);

	case tess_ctrl_shader_spirv:
		if (!spirv_replaces_glsl)
			break;
		shader_string_size = line - shader_string;
		return assemble_spirv(GL_TESS_CONTROL_SHADER);

	case tess_ctrl_shader_specializations:
		break;

	case tess_eval_shader:
		if (spirv_replaces_glsl)
			break;
		shader_string_size = line - shader_string;
		return compile_glsl(GL_TESS_EVALUATION_SHADER);

	case tess_eval_shader_spirv:
		if (!spirv_replaces_glsl)
			break;
		shader_string_size = line - shader_string;
		return assemble_spirv(GL_TESS_EVALUATION_SHADER);

	case tess_eval_shader_specializations:
		break;

	case geometry_shader:
		if (spirv_replaces_glsl)
			break;
		shader_string_size = line - shader_string;
		return compile_glsl(GL_GEOMETRY_SHADER);

	case geometry_shader_spirv:
		if (!spirv_replaces_glsl)
			break;
		shader_string_size = line - shader_string;
		return assemble_spirv(GL_GEOMETRY_SHADER);

	case geometry_shader_specializations:
		break;

	case geometry_layout:
		break;

	case fragment_shader:
		if (spirv_replaces_glsl)
			break;
		shader_string_size = line - shader_string;
		return compile_glsl(GL_FRAGMENT_SHADER);

	case fragment_program:
		return compile_and_bind_program(GL_FRAGMENT_PROGRAM_ARB,
						shader_string,
						line - shader_string);
		break;

	case fragment_shader_spirv:
		if (!spirv_replaces_glsl)
			break;
		shader_string_size = line - shader_string;
		return assemble_spirv(GL_FRAGMENT_SHADER);

	case fragment_shader_specializations:
		break;

	case compute_shader:
		if (spirv_replaces_glsl)
			break;
		shader_string_size = line - shader_string;
		return compile_glsl(GL_COMPUTE_SHADER);

	case compute_shader_spirv:
		if (!spirv_replaces_glsl)
			break;
		shader_string_size = line - shader_string;
		return assemble_spirv(GL_COMPUTE_SHADER);

	case compute_shader_specializations:
		break;

	case vertex_data:
		vertex_data_end = line;
		break;

	case test:
		break;

	default:
		assert(!"Not yet supported.");
	}
	return PIGLIT_PASS;
}


static enum piglit_result
process_shader(GLenum target, unsigned num_shaders, GLuint *shaders)
{
	if (num_shaders == 0)
		return PIGLIT_PASS;

	if (sso_in_use) {
		prog = glCreateProgram();
		glProgramParameteri(prog, GL_PROGRAM_SEPARABLE, GL_TRUE);
	}

	for (unsigned i = 0; i < num_shaders; i++) {
		glAttachShader(prog, shaders[i]);
	}

#ifdef PIGLIT_USE_OPENGL
	if (geometry_layout_input_type != GL_TRIANGLES) {
		glProgramParameteriARB(prog, GL_GEOMETRY_INPUT_TYPE_ARB,
				       geometry_layout_input_type);
	}
	if (geometry_layout_output_type != GL_TRIANGLE_STRIP) {
		glProgramParameteriARB(prog, GL_GEOMETRY_OUTPUT_TYPE_ARB,
				       geometry_layout_output_type);
	}
	if (geometry_layout_vertices_out != 0) {
		glProgramParameteriARB(prog, GL_GEOMETRY_VERTICES_OUT_ARB,
				       geometry_layout_vertices_out);
	}
#endif

	/* If the shaders reference piglit_vertex or piglit_tex, bind
	 * them to some fixed attribute locations so they can be used
	 * with piglit_draw_rect_tex() in GLES.
	 */
	glBindAttribLocation(prog, PIGLIT_ATTRIB_POS, "piglit_vertex");
	glBindAttribLocation(prog, PIGLIT_ATTRIB_TEX, "piglit_texcoord");

	if (sso_in_use) {
		return link_sso(target);
	}
	return PIGLIT_PASS;
}


static enum piglit_result
link_and_use_shaders(void)
{
	enum piglit_result result;
	unsigned i;
	GLenum err;
	GLint ok;

	if ((num_vertex_shaders == 0)
	    && (num_fragment_shaders == 0)
	    && (num_tess_ctrl_shaders == 0)
	    && (num_tess_eval_shaders == 0)
	    && (num_geometry_shaders == 0)
	    && (num_compute_shaders == 0))
		return PIGLIT_PASS;

	if (!sso_in_use)
		prog = glCreateProgram();

	result = process_shader(GL_VERTEX_SHADER, num_vertex_shaders, vertex_shaders);
	if (result != PIGLIT_PASS)
		goto cleanup;
	result = process_shader(GL_TESS_CONTROL_SHADER, num_tess_ctrl_shaders, tess_ctrl_shaders);
	if (result != PIGLIT_PASS)
		goto cleanup;
	result = process_shader(GL_TESS_EVALUATION_SHADER, num_tess_eval_shaders, tess_eval_shaders);
	if (result != PIGLIT_PASS)
		goto cleanup;
	result = process_shader(GL_GEOMETRY_SHADER, num_geometry_shaders, geometry_shaders);
	if (result != PIGLIT_PASS)
		goto cleanup;
	result = process_shader(GL_FRAGMENT_SHADER, num_fragment_shaders, fragment_shaders);
	if (result != PIGLIT_PASS)
		goto cleanup;
	result = process_shader(GL_COMPUTE_SHADER, num_compute_shaders, compute_shaders);
	if (result != PIGLIT_PASS)
		goto cleanup;

	if (!sso_in_use)
		glLinkProgram(prog);

	if (!sso_in_use) {
		glGetProgramiv(prog, GL_LINK_STATUS, &ok);
		if (ok) {
			link_ok = true;
		} else {
			GLint size;

			glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &size);
			prog_err_info = malloc(size);

			glGetProgramInfoLog(prog, size, NULL, prog_err_info);

			result = PIGLIT_PASS;
			goto cleanup;
		}

		glUseProgram(prog);
	}

	err = glGetError();
	if (!err) {
		prog_in_use = true;
	} else {
		GLint size;

		glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &size);
		prog_err_info = malloc(size);

		glGetProgramInfoLog(prog, size, NULL, prog_err_info);
	}

cleanup:
	for (i = 0; i < num_vertex_shaders; i++) {
		glDeleteShader(vertex_shaders[i]);
	}
	num_vertex_shaders = 0;

	for (i = 0; i < num_tess_ctrl_shaders; i++) {
		glDeleteShader(tess_ctrl_shaders[i]);
	}
	num_tess_ctrl_shaders = 0;

	for (i = 0; i < num_tess_eval_shaders; i++) {
		glDeleteShader(tess_eval_shaders[i]);
	}
	num_tess_eval_shaders = 0;

	for (i = 0; i < num_geometry_shaders; i++) {
		glDeleteShader(geometry_shaders[i]);
	}
	num_geometry_shaders = 0;

	for (i = 0; i < num_fragment_shaders; i++) {
		glDeleteShader(fragment_shaders[i]);
	}
	num_fragment_shaders = 0;

	for (i = 0; i < num_compute_shaders; i++) {
		glDeleteShader(compute_shaders[i]);
	}
	num_compute_shaders = 0;

	return result;
}

static enum piglit_result
process_specialization(enum states state, const char *line)
{
	const char *end = strchrnul(line, '\n');
	const char *next;
	enum { TYPE_FLOAT, TYPE_UINT } type;

	while (line < end && isspace(*line))
		line++;

	if (line >= end || *line == '#')
		return PIGLIT_PASS;

	if (parse_str(line, "uint", &next))
		type = TYPE_UINT;
	else if (parse_str(line, "float", &next))
		type = TYPE_FLOAT;
	else
		goto invalid;

	struct specialization_list *list;

	switch (state) {
	case vertex_shader_specializations:
		list = specializations + 0;
		break;
	case tess_ctrl_shader_specializations:
		list = specializations + 1;
		break;
	case tess_eval_shader_specializations:
		list = specializations + 2;
		break;
	case geometry_shader_specializations:
		list = specializations + 3;
		break;
	case fragment_shader_specializations:
		list = specializations + 4;
		break;
	case compute_shader_specializations:
		list = specializations + 5;
		break;
	default:
		assert(!"Should not get here.");
	}

	if (list->n_entries >= list->buffer_size) {
		if (list->buffer_size == 0)
			list->buffer_size = 1;
		else
			list->buffer_size *= 2;
		list->indices = realloc(list->indices,
					(sizeof list->indices[0]) *
					list->buffer_size);
		list->values = realloc(list->values,
				       (sizeof list->values[0]) *
				       list->buffer_size);
	}

	if (parse_uints(next, list->indices + list->n_entries, 1, &next) != 1)
		goto invalid;

	switch (type) {
	case TYPE_UINT:
		if (parse_uints(next,
				&list->values[list->n_entries].u,
				1,
				&next) != 1)
			goto invalid;
		break;
	case TYPE_FLOAT:
		if (parse_floats(next,
				 &list->values[list->n_entries].f,
				 1,
				 &next) != 1)
			goto invalid;
		break;
	}

	list->n_entries++;

	return PIGLIT_PASS;

 invalid:
	fprintf(stderr, "Invalid specialization line\n");
	return PIGLIT_FAIL;
}

static enum piglit_result
process_test_script(const char *script_name)
{
	unsigned text_size;
	unsigned line_num;
	char *text = piglit_load_text_file(script_name, &text_size);
	enum states state = none;
	const char *line = text;
	enum piglit_result result;

	if (line == NULL) {
		printf("could not read file \"%s\"\n", script_name);
		return PIGLIT_FAIL;
	}

	line_num = 1;

	while (line[0] != '\0') {
		if (line[0] == '[') {
			result = leave_state(state, line, script_name);
			if (result != PIGLIT_PASS)
				return result;

			if (parse_str(line, "[require]", NULL)) {
				state = requirements;
			} else if (parse_str(line, "[vertex shader]", NULL)) {
				state = vertex_shader;
				shader_string = NULL;
			} else if (parse_str(line, "[vertex program]", NULL)) {
				state = vertex_program;
				shader_string = NULL;
			} else if (parse_str(line, "[vertex shader passthrough]", NULL)) {
				state = vertex_shader_passthrough;
				shader_string =
					(char *) passthrough_vertex_shader_source;
				shader_string_size = strlen(shader_string);
			} else if (parse_str(line, "[vertex shader spirv]", NULL)) {
				state = vertex_shader_spirv;
				shader_string = NULL;
			} else if (parse_str(line, "[vertex shader specializations]", NULL)) {
				state = vertex_shader_specializations;
			} else if (parse_str(line, "[tessellation control shader]", NULL)) {
				state = tess_ctrl_shader;
				shader_string = NULL;
			} else if (parse_str(line, "[tessellation control shader spirv]", NULL)) {
				state = tess_ctrl_shader_spirv;
				shader_string = NULL;
			} else if (parse_str(line, "[tessellation control shader specializations]", NULL)) {
				state = tess_ctrl_shader_specializations;
			} else if (parse_str(line, "[tessellation evaluation shader]", NULL)) {
				state = tess_eval_shader;
				shader_string = NULL;
			} else if (parse_str(line, "[tessellation evaluation shader spirv]", NULL)) {
				state = tess_eval_shader_spirv;
				shader_string = NULL;
			} else if (parse_str(line, "[tessellation evaluation shader specializations]", NULL)) {
				state = tess_eval_shader_specializations;
			} else if (parse_str(line, "[geometry shader]", NULL)) {
				state = geometry_shader;
				shader_string = NULL;
			} else if (parse_str(line, "[geometry shader specializations]", NULL)) {
				state = geometry_shader_specializations;
			} else if (parse_str(line, "[geometry shader spirv]", NULL)) {
				state = geometry_shader_spirv;
				shader_string = NULL;
			} else if (parse_str(line, "[geometry shader specializations]", NULL)) {
				state = geometry_shader_specializations;
			} else if (parse_str(line, "[geometry layout]", NULL)) {
				state = geometry_layout;
				shader_string = NULL;
			} else if (parse_str(line, "[fragment shader]", NULL)) {
				state = fragment_shader;
				shader_string = NULL;
			} else if (parse_str(line, "[fragment program]", NULL)) {
				state = fragment_program;
				shader_string = NULL;
			} else if (parse_str(line, "[fragment shader specializations]", NULL)) {
				state = fragment_shader_specializations;
			} else if (parse_str(line, "[fragment shader spirv]", NULL)) {
				state = fragment_shader_spirv;
				shader_string = NULL;
			} else if (parse_str(line, "[fragment shader specializations]", NULL)) {
				state = fragment_shader_specializations;
			} else if (parse_str(line, "[compute shader]", NULL)) {
				state = compute_shader;
				shader_string = NULL;
			} else if (parse_str(line, "[compute shader spirv]", NULL)) {
				state = compute_shader_spirv;
				shader_string = NULL;
			} else if (parse_str(line, "[compute shader specializations]", NULL)) {
				state = compute_shader_specializations;
			} else if (parse_str(line, "[vertex data]", NULL)) {
				state = vertex_data;
				vertex_data_start = NULL;
			} else if (parse_str(line, "[test]", NULL)) {
				test_start = strchrnul(line, '\n');
				test_start_line_num = line_num + 1;
				if (test_start[0] != '\0')
					test_start++;
				return PIGLIT_PASS;
			} else {
				fprintf(stderr,
					"Unknown section in test script.  "
					"Perhaps missing closing ']'?\n");
				return PIGLIT_FAIL;
			}
		} else {
			switch (state) {
			case none:
			case vertex_shader_passthrough:
				break;

			case requirements:
				result = process_requirement(line);
				if (result != PIGLIT_PASS)
					return result;
				break;

			case geometry_layout:
				process_geometry_layout(line);
				break;

			case vertex_shader:
			case vertex_program:
			case tess_ctrl_shader:
			case tess_eval_shader:
			case geometry_shader:
			case fragment_shader:
			case fragment_program:
			case compute_shader:
			case vertex_shader_spirv:
			case tess_ctrl_shader_spirv:
			case tess_eval_shader_spirv:
			case geometry_shader_spirv:
			case fragment_shader_spirv:
			case compute_shader_spirv:
				if (shader_string == NULL)
					shader_string = (char *) line;
				break;

			case vertex_shader_specializations:
			case tess_ctrl_shader_specializations:
			case tess_eval_shader_specializations:
			case geometry_shader_specializations:
			case fragment_shader_specializations:
			case compute_shader_specializations: {
				enum piglit_result result =
					process_specialization(state, line);
				if (result != PIGLIT_PASS)
					return result;
				break;
			}

			case vertex_data:
				if (vertex_data_start == NULL)
					vertex_data_start = line;
				break;

			case test:
				break;
			}
		}

		line = strchrnul(line, '\n');
		if (line[0] != '\0')
			line++;

		line_num++;
	}

	return leave_state(state, line, script_name);
}

struct requirement_parse_results {
	bool found_gl;
	bool found_glsl;
	bool found_size;
	bool found_depthbuffer;
	struct component_version gl_version;
	struct component_version glsl_version;
	unsigned size[2];
};

static void
parse_required_config(struct requirement_parse_results *results,
		      const char *script_name)
{
	unsigned text_size;
	char *text = piglit_load_text_file(script_name, &text_size);
	const char *line = text;
	bool in_requirement_section = false;

	results->found_gl = false;
	results->found_glsl = false;
	results->found_size = false;
	results->found_depthbuffer = false;

	if (line == NULL) {
		printf("could not read file \"%s\"\n", script_name);
		piglit_report_result(PIGLIT_FAIL);
	}

	while (line[0] != '\0') {
		if (line[0] == '[') {
			if (in_requirement_section)
				break;
			else
				in_requirement_section = false;
		}

		if (!in_requirement_section) {
			if (parse_str(line, "[require]", NULL)) {
				in_requirement_section = true;
			}
		} else {
			if (parse_str(line, "GL_", NULL)
			    || parse_str(line, "!GL_", NULL)) {
				/* empty */
			} else if (parse_str(line, "GLSL", &line)) {
				enum comparison cmp;
				struct component_version version;

				parse_version_comparison(line, &cmp,
							 &version, VERSION_GLSL);
				if (cmp == greater_equal) {
					results->found_glsl = true;
					version_copy(&results->glsl_version, &version);
				}
			} else if (parse_str(line, "GL", &line)) {
				enum comparison cmp;
				struct component_version version;

				parse_version_comparison(line, &cmp,
							 &version, VERSION_GL);
				if (cmp == greater_equal
				    || cmp == greater
				    || cmp == equal) {
					results->found_gl = true;
					version_copy(&results->gl_version, &version);
				}
			} else if (parse_str(line, "SIZE", &line)) {
				results->found_size = true;
				parse_uints(line, results->size, 2, NULL);
			} else if (parse_str(line, "depthbuffer", NULL)) {
				results->found_depthbuffer = true;
			}
		}

		line = strchrnul(line, '\n');
		if (line[0] != '\0')
			line++;
	}

	free(text);

	if (!in_requirement_section) {
		printf("[require] section missing\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	if (results->found_glsl && results->glsl_version.es && !results->found_gl) {
		printf("%s", "The test specifies a requirement for GLSL ES, "
		       "but specifies no GL requirement\n.");
		piglit_report_result(PIGLIT_FAIL);
	}
}


static void
choose_required_gl_version(struct requirement_parse_results *parse_results,
                           struct component_version *gl_version)
{
	if (parse_results->found_gl) {
		version_copy(gl_version, &parse_results->gl_version);
	} else {
		assert(!parse_results->found_glsl || !parse_results->glsl_version.es);
		version_init(gl_version, VERSION_GL, false, false, false, 10);
	}

	if (gl_version->es)
		return;

	/* Possibly promote the GL version. */
	if (gl_version->num < required_gl_version_from_glsl_version(
			parse_results->glsl_version.num)) {
		gl_version->num = required_gl_version_from_glsl_version(
			parse_results->glsl_version.num);
	}
}

/**
 * Just determine the GLSL version required by the shader script.
 *
 * This function is a bit of a hack that is, unfortunately necessary.  A test
 * script can require a specific GLSL version or a specific GL version.  To
 * satisfy this requirement, the piglit framework code needs to know about the
 * requirement before creating the context.  However, the requirements section
 * can contain other requirements, such as minimum number of uniforms.
 *
 * The requirements section can't be fully processed until after the context
 * is created, but the context can't be created until after the requirements
 * section is processed.  Do a quick scan over the requirements section to find
 * the GL and GLSL version requirements.  Use these to guide context creation.
 */
static void
get_required_config(const char *script_name, bool spirv,
		    struct piglit_gl_test_config *config)
{
	struct requirement_parse_results parse_results;
	struct component_version required_gl_version;

	parse_required_config(&parse_results, script_name);
	choose_required_gl_version(&parse_results, &required_gl_version);

	if (spirv) {
		required_gl_version.es = false;
		required_gl_version.core = true;
		required_gl_version.num = MAX2(required_gl_version.num, 33);
	}

	if (parse_results.found_size) {
		config->window_width = parse_results.size[0];
		config->window_height = parse_results.size[1];
	}

	if (required_gl_version.es) {
		config->supports_gl_es_version = required_gl_version.num;
	} else if (required_gl_version.num >= 31) {
		if (!required_gl_version.compat)
			config->supports_gl_core_version = required_gl_version.num;
		if (!required_gl_version.core)
			config->supports_gl_compat_version = required_gl_version.num;
	} else {
		config->supports_gl_compat_version = 10;
	}

	if (parse_results.found_depthbuffer) {
		config->window_visual |= PIGLIT_GL_VISUAL_DEPTH;
	}
}

/**
 * Check that the GL implementation supports unsigned uniforms
 * (e.g. through glUniform1ui).  If not, terminate the test with a
 * SKIP.
 */
static void
check_unsigned_support(void)
{
	if (gl_version.num < 30 && !piglit_is_extension_supported("GL_EXT_gpu_shader4"))
		piglit_report_result(PIGLIT_SKIP);
}

/**
 * Check that the GL implementation supports double uniforms
 * (e.g. through glUniform1d).  If not, terminate the test with a
 * SKIP.
 */
static void
check_double_support(void)
{
	if (gl_version.num < 40 && !piglit_is_extension_supported("GL_ARB_gpu_shader_fp64"))
		piglit_report_result(PIGLIT_SKIP);
}

/**
 * Check that the GL implementation supports double uniforms
 * (e.g. through glUniform1d).  If not, terminate the test with a
 * SKIP.
 */
static void
check_int64_support(void)
{
	if (!piglit_is_extension_supported("GL_ARB_gpu_shader_int64"))
		piglit_report_result(PIGLIT_SKIP);
}

/**
 * Check that the GL implementation supports shader subroutines
 * If not, terminate the test with a SKIP.
 */
static void
check_shader_subroutine_support(void)
{
	if (gl_version.num < 40 && !piglit_is_extension_supported("GL_ARB_shader_subroutine"))
		piglit_report_result(PIGLIT_SKIP);
}

/**
 * Check that the GL implementation supports texture handles.
 * If not, terminate the test with a SKIP.
 */
static void
check_texture_handle_support(void)
{
	if (!piglit_is_extension_supported("GL_ARB_bindless_texture"))
		piglit_report_result(PIGLIT_SKIP);
}

/**
 * Handles uploads of UBO uniforms by mapping the buffer and storing
 * the data.  If the uniform is not in a uniform block, returns false.
 */
static bool
set_ubo_uniform(char *name, const char *type, const char *line, int ubo_array_index)
{
	GLuint uniform_index;
	GLint block_index;
	GLint offset;
	GLint array_index = 0;
	char *data;
	float f[16];
	double d[16];
	int ints[16];
	unsigned uints[16];
	uint64_t uint64s[16];
	int64_t int64s[16];
	int name_len = strlen(name);

	if (!num_uniform_blocks)
		return false;

	/* if the uniform is an array, strip the index, as GL
	   prevents non-zero indexes from matching a name */
	if (name[name_len - 1] == ']') {
		int i;

		for (i = name_len - 1; (i > 0) && isdigit(name[i-1]); --i)
			/* empty */;

		array_index = strtol(&name[i], NULL, 0);

		if (i) {
			i--;
			if (name[i] != '[') {
				printf("cannot parse uniform \"%s\"\n", name);
				piglit_report_result(PIGLIT_FAIL);
			}
			name[i] = 0;
		}

	}


	glGetUniformIndices(prog, 1, (const char **)&name, &uniform_index);
	if (uniform_index == GL_INVALID_INDEX) {
		printf("cannot get index of uniform \"%s\"\n", name);
		piglit_report_result(PIGLIT_FAIL);
	}

	glGetActiveUniformsiv(prog, 1, &uniform_index,
			      GL_UNIFORM_BLOCK_INDEX, &block_index);

	if (block_index == -1)
		return false;

	/* if the uniform block is an array, then GetActiveUniformsiv with
	 * UNIFORM_BLOCK_INDEX will have given us the index of the first
	 * element in the array.
	 */
	block_index += ubo_array_index;

	glGetActiveUniformsiv(prog, 1, &uniform_index,
			      GL_UNIFORM_OFFSET, &offset);

	if (name[name_len - 1] == ']') {
		GLint stride;

		glGetActiveUniformsiv(prog, 1, &uniform_index,
				      GL_UNIFORM_ARRAY_STRIDE, &stride);
		offset += stride * array_index;
	}

	glBindBuffer(GL_UNIFORM_BUFFER,
		     uniform_block_bos[block_index]);
	data = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
	data += offset;

	if (parse_str(type, "float", NULL)) {
		parse_floats(line, f, 1, NULL);
		memcpy(data, f, sizeof(float));
	} else if (parse_str(type, "int64_t", NULL)) {
		parse_int64s(line, int64s, 1, NULL);
		memcpy(data, int64s, sizeof(int64_t));
	} else if (parse_str(type, "uint64_t", NULL)) {
		parse_uint64s(line, uint64s, 1, NULL);
		memcpy(data, uint64s, sizeof(uint64_t));
	} else if (parse_str(type, "int", NULL)) {
		parse_ints(line, ints, 1, NULL);
		memcpy(data, ints, sizeof(int));
	} else if (parse_str(type, "uint", NULL)) {
		parse_uints(line, uints, 1, NULL);
		memcpy(data, uints, sizeof(int));
	} else if (parse_str(type, "double", NULL)) {
		parse_doubles(line, d, 1, NULL);
		memcpy(data, d, sizeof(double));
	} else if (parse_str(type, "vec", NULL)) {
		int elements = type[3] - '0';
		parse_floats(line, f, elements, NULL);
		memcpy(data, f, elements * sizeof(float));
	} else if (parse_str(type, "ivec", NULL)) {
		int elements = type[4] - '0';
		parse_ints(line, ints, elements, NULL);
		memcpy(data, ints, elements * sizeof(int));
	} else if (parse_str(type, "uvec", NULL)) {
		int elements = type[4] - '0';
		parse_uints(line, uints, elements, NULL);
		memcpy(data, uints, elements * sizeof(unsigned));
	} else if (parse_str(type, "i64vec", NULL)) {
		int elements = type[6] - '0';
		parse_int64s(line, int64s, elements, NULL);
		memcpy(data, int64s, elements * sizeof(int64_t));
	} else if (parse_str(type, "u64vec", NULL)) {
		int elements = type[6] - '0';
		parse_uint64s(line, uint64s, elements, NULL);
		memcpy(data, uint64s, elements * sizeof(uint64_t));
	} else if (parse_str(type, "dvec", NULL)) {
		int elements = type[4] - '0';
		parse_doubles(line, d, elements, NULL);
		memcpy(data, d, elements * sizeof(double));
	} else if (parse_str(type, "mat", NULL)) {
		GLint matrix_stride, row_major;
		int cols = type[3] - '0';
		int rows = type[4] == 'x' ? type[5] - '0' : cols;
		int r, c;
		float *matrixdata = (float *)data;

		assert(cols >= 2 && cols <= 4);
		assert(rows >= 2 && rows <= 4);

		parse_floats(line, f, rows * cols, NULL);

		glGetActiveUniformsiv(prog, 1, &uniform_index,
				      GL_UNIFORM_MATRIX_STRIDE, &matrix_stride);
		glGetActiveUniformsiv(prog, 1, &uniform_index,
				      GL_UNIFORM_IS_ROW_MAJOR, &row_major);

		matrix_stride /= sizeof(float);

		/* Expect the data in the .shader_test file to be listed in
		 * column-major order no matter what the layout of the data in
		 * the UBO will be.
		 */
		for (c = 0; c < cols; c++) {
			for (r = 0; r < rows; r++) {
				if (row_major) {
					matrixdata[matrix_stride * r + c] =
						f[c * rows + r];
				} else {
					matrixdata[matrix_stride * c + r] =
						f[c * rows + r];
				}
			}
		}
	} else if (parse_str(type, "dmat", NULL)) {
		GLint matrix_stride, row_major;
		int cols = type[4] - '0';
		int rows = type[5] == 'x' ? type[6] - '0' : cols;
		int r, c;
		double *matrixdata = (double *)data;

		assert(cols >= 2 && cols <= 4);
		assert(rows >= 2 && rows <= 4);

		parse_doubles(line, d, rows * cols, NULL);

		glGetActiveUniformsiv(prog, 1, &uniform_index,
				      GL_UNIFORM_MATRIX_STRIDE, &matrix_stride);
		glGetActiveUniformsiv(prog, 1, &uniform_index,
				      GL_UNIFORM_IS_ROW_MAJOR, &row_major);

		matrix_stride /= sizeof(double);

		/* Expect the data in the .shader_test file to be listed in
		 * column-major order no matter what the layout of the data in
		 * the UBO will be.
		 */
		for (c = 0; c < cols; c++) {
			for (r = 0; r < rows; r++) {
				if (row_major) {
					matrixdata[matrix_stride * r + c] =
						d[c * rows + r];
				} else {
					matrixdata[matrix_stride * c + r] =
						d[c * rows + r];
				}
			}
		}
	} else if (parse_str(type, "handle", NULL)) {
		check_unsigned_support();
		check_texture_handle_support();
		parse_uints(line, uints, 1, NULL);
		GLuint64 handle = get_resident_handle(uints[0])->handle;
		memcpy(data, &handle, sizeof(uint64_t));
	} else {
		printf("unknown uniform type \"%s\" for \"%s\"\n", type, name);
		piglit_report_result(PIGLIT_FAIL);
	}

	glUnmapBuffer(GL_UNIFORM_BUFFER);

	return true;
}

static void
set_uniform(const char *line, int ubo_array_index)
{
	char name[512], type[512];
	float f[16];
	double d[16];
	int ints[16];
	unsigned uints[16];
	int64_t int64s[16];
	uint64_t uint64s[16];
	GLint loc;

	REQUIRE(parse_word_copy(line, type, sizeof(type), &line) &&
		parse_word_copy(line, name, sizeof(name), &line),
		"Invalid set uniform command at: %s\n", line);

	if (isdigit(name[0])) {
		loc = strtol(name, NULL, 0);
	} else {
		GLuint prog;

		if (set_ubo_uniform(name, type, line, ubo_array_index))
			return;

		glGetIntegerv(GL_CURRENT_PROGRAM, (GLint *) &prog);
		loc = glGetUniformLocation(prog, name);
		if (loc < 0) {
			printf("cannot get location of uniform \"%s\"\n",
			       name);
			piglit_report_result(PIGLIT_FAIL);
		}
        }

	if (parse_str(type, "float", NULL)) {
		parse_floats(line, f, 1, NULL);
		glUniform1fv(loc, 1, f);
		return;
	} else if (parse_str(type, "int64_t", NULL)) {
		check_int64_support();
		parse_int64s(line, int64s, 1, NULL);
		glUniform1i64vARB(loc, 1, int64s);
		return;
	} else if (parse_str(type, "uint64_t", NULL)) {
		check_int64_support();
		parse_uint64s(line, uint64s, 1, NULL);
		glUniform1ui64vARB(loc, 1, uint64s);
		return;
	} else if (parse_str(type, "int", NULL)) {
		parse_ints(line, ints, 1, NULL);
		glUniform1iv(loc, 1, ints);
		return;
	} else if (parse_str(type, "uint", NULL)) {
		check_unsigned_support();
		parse_uints(line, uints, 1, NULL);
		glUniform1uiv(loc, 1, uints);
		return;
	} else if (parse_str(type, "double", NULL)) {
		check_double_support();
		parse_doubles(line, d, 1, NULL);
		glUniform1dv(loc, 1, d);
		return;
	} else if (parse_str(type, "vec", NULL)) {
		switch (type[3]) {
		case '2':
			parse_floats(line, f, 2, NULL);
			glUniform2fv(loc, 1, f);
			return;
		case '3':
			parse_floats(line, f, 3, NULL);
			glUniform3fv(loc, 1, f);
			return;
		case '4':
			parse_floats(line, f, 4, NULL);
			glUniform4fv(loc, 1, f);
			return;
		}
	} else if (parse_str(type, "ivec", NULL)) {
		switch (type[4]) {
		case '2':
			parse_ints(line, ints, 2, NULL);
			glUniform2iv(loc, 1, ints);
			return;
		case '3':
			parse_ints(line, ints, 3, NULL);
			glUniform3iv(loc, 1, ints);
			return;
		case '4':
			parse_ints(line, ints, 4, NULL);
			glUniform4iv(loc, 1, ints);
			return;
		}
	} else if (parse_str(type, "uvec", NULL)) {
		check_unsigned_support();
		switch (type[4]) {
		case '2':
			parse_uints(line, uints, 2, NULL);
			glUniform2uiv(loc, 1, uints);
			return;
		case '3':
			parse_uints(line, uints, 3, NULL);
			glUniform3uiv(loc, 1, uints);
			return;
		case '4':
			parse_uints(line, uints, 4, NULL);
			glUniform4uiv(loc, 1, uints);
			return;
		}
	} else if (parse_str(type, "dvec", NULL)) {
		check_double_support();
		switch (type[4]) {
		case '2':
			parse_doubles(line, d, 2, NULL);
			glUniform2dv(loc, 1, d);
			return;
		case '3':
			parse_doubles(line, d, 3, NULL);
			glUniform3dv(loc, 1, d);
			return;
		case '4':
			parse_doubles(line, d, 4, NULL);
			glUniform4dv(loc, 1, d);
			return;
		}
	} else if (parse_str(type, "i64vec", NULL)) {
		check_int64_support();
		switch (type[6]) {
		case '2':
			parse_int64s(line, int64s, 2, NULL);
			glUniform2i64vARB(loc, 1, int64s);
			return;
		case '3':
			parse_int64s(line, int64s, 3, NULL);
			glUniform3i64vARB(loc, 1, int64s);
			return;
		case '4':
			parse_int64s(line, int64s, 4, NULL);
			glUniform4i64vARB(loc, 1, int64s);
			return;
		}
	} else if (parse_str(type, "u64vec", NULL)) {
		check_int64_support();
		switch (type[6]) {
		case '2':
			parse_uint64s(line, uint64s, 2, NULL);
			glUniform2ui64vARB(loc, 1, uint64s);
			return;
		case '3':
			parse_uint64s(line, uint64s, 3, NULL);
			glUniform3ui64vARB(loc, 1, uint64s);
			return;
		case '4':
			parse_uint64s(line, uint64s, 4, NULL);
			glUniform4ui64vARB(loc, 1, uint64s);
			return;
		}
	} else if (parse_str(type, "mat", NULL) && type[3] != '\0') {
		char cols = type[3];
		char rows = type[4] == 'x' ? type[5] : cols;
		switch (cols) {
		case '2':
			switch (rows) {
			case '2':
				parse_floats(line, f, 4, NULL);
				glUniformMatrix2fv(loc, 1, GL_FALSE, f);
				return;
			case '3':
				parse_floats(line, f, 6, NULL);
				glUniformMatrix2x3fv(loc, 1, GL_FALSE, f);
				return;
			case '4':
				parse_floats(line, f, 8, NULL);
				glUniformMatrix2x4fv(loc, 1, GL_FALSE, f);
				return;
			}
		case '3':
			switch (rows) {
			case '2':
				parse_floats(line, f, 6, NULL);
				glUniformMatrix3x2fv(loc, 1, GL_FALSE, f);
				return;
			case '3':
				parse_floats(line, f, 9, NULL);
				glUniformMatrix3fv(loc, 1, GL_FALSE, f);
				return;
			case '4':
				parse_floats(line, f, 12, NULL);
				glUniformMatrix3x4fv(loc, 1, GL_FALSE, f);
				return;
			}
		case '4':
			switch (rows) {
			case '2':
				parse_floats(line, f, 8, NULL);
				glUniformMatrix4x2fv(loc, 1, GL_FALSE, f);
				return;
			case '3':
				parse_floats(line, f, 12, NULL);
				glUniformMatrix4x3fv(loc, 1, GL_FALSE, f);
				return;
			case '4':
				parse_floats(line, f, 16, NULL);
				glUniformMatrix4fv(loc, 1, GL_FALSE, f);
				return;
			}
		}
	} else if (parse_str(type, "dmat", NULL) && type[4] != '\0') {
		char cols = type[4];
		char rows = type[5] == 'x' ? type[6] : cols;
		switch (cols) {
		case '2':
			switch (rows) {
			case '2':
				parse_doubles(line, d, 4, NULL);
				glUniformMatrix2dv(loc, 1, GL_FALSE, d);
				return;
			case '3':
				parse_doubles(line, d, 6, NULL);
				glUniformMatrix2x3dv(loc, 1, GL_FALSE, d);
				return;
			case '4':
				parse_doubles(line, d, 8, NULL);
				glUniformMatrix2x4dv(loc, 1, GL_FALSE, d);
				return;
			}
		case '3':
			switch (rows) {
			case '2':
				parse_doubles(line, d, 6, NULL);
				glUniformMatrix3x2dv(loc, 1, GL_FALSE, d);
				return;
			case '3':
				parse_doubles(line, d, 9, NULL);
				glUniformMatrix3dv(loc, 1, GL_FALSE, d);
				return;
			case '4':
				parse_doubles(line, d, 12, NULL);
				glUniformMatrix3x4dv(loc, 1, GL_FALSE, d);
				return;
			}
		case '4':
			switch (rows) {
			case '2':
				parse_doubles(line, d, 8, NULL);
				glUniformMatrix4x2dv(loc, 1, GL_FALSE, d);
				return;
			case '3':
				parse_doubles(line, d, 12, NULL);
				glUniformMatrix4x3dv(loc, 1, GL_FALSE, d);
				return;
			case '4':
				parse_doubles(line, d, 16, NULL);
				glUniformMatrix4dv(loc, 1, GL_FALSE, d);
				return;
			}
		}
	} else if (parse_str(type, "handle", NULL)) {
		check_unsigned_support();
		check_texture_handle_support();
		parse_uints(line, uints, 1, NULL);
		glUniformHandleui64ARB(loc, get_resident_handle(uints[0])->handle);
		return;
	}

	printf("unknown uniform type \"%s\"\n", type);
	piglit_report_result(PIGLIT_FAIL);

	return;
}

static void
set_vertex_attrib(const char *line)
{
	char name[512], type[512];
	uint32_t uints[16];
	GLint loc;

	REQUIRE(parse_word_copy(line, type, sizeof(type), &line) &&
		parse_word_copy(line, name, sizeof(name), &line),
		"Invalid set vertex attrib command at: %s\n", line);

	if (isdigit(name[0])) {
		loc = strtol(name, NULL, 0);
	} else {
		GLuint prog;

		glGetIntegerv(GL_CURRENT_PROGRAM, (GLint *) &prog);
		loc = glGetAttribLocation(prog, name);
		if (loc < 0) {
			printf("cannot get location of vertex attrib \"%s\"\n",
			       name);
			piglit_report_result(PIGLIT_FAIL);
		}
        }

	if (parse_str(type, "handle", NULL)) {
		check_unsigned_support();
		check_texture_handle_support();
		parse_uints(line, uints, 1, NULL);
		glVertexAttribL1ui64ARB(loc, get_resident_handle(uints[0])->handle);
		return;
	}

	printf("unknown vertex attrib type \"%s\"\n", type);
	printf("use [vertex data] instead if possible\n");
	piglit_report_result(PIGLIT_FAIL);

	return;
}

static GLenum lookup_shader_type(GLuint idx)
{
	switch (idx) {
	case 0:
		return GL_VERTEX_SHADER;
	case 1:
		return GL_FRAGMENT_SHADER;
	case 2:
		return GL_GEOMETRY_SHADER;
	case 3:
		return GL_TESS_CONTROL_SHADER;
	case 4:
		return GL_TESS_EVALUATION_SHADER;
	case 5:
		return GL_COMPUTE_SHADER;
	default:
		return 0;
	}
}

static GLenum get_shader_from_string(const char *name, int *idx)
{
	if (parse_str(name, "GL_VERTEX_SHADER", NULL)) {
		*idx = 0;
		return GL_VERTEX_SHADER;
	}
	if (parse_str(name, "GL_FRAGMENT_SHADER", NULL)) {
		*idx = 1;
		return GL_FRAGMENT_SHADER;
	}
	if (parse_str(name, "GL_GEOMETRY_SHADER", NULL)) {
		*idx = 2;
		return GL_GEOMETRY_SHADER;
	}
	if (parse_str(name, "GL_TESS_CONTROL_SHADER", NULL)) {
		*idx = 3;
		return GL_TESS_CONTROL_SHADER;
	}
	if (parse_str(name, "GL_TESS_EVALUATION_SHADER", NULL)) {
		*idx = 4;
		return GL_TESS_EVALUATION_SHADER;
	}
	if (parse_str(name, "GL_COMPUTE_SHADER", NULL)) {
		*idx = 5;
		return GL_COMPUTE_SHADER;
	}
	return 0;
}

static void
free_subroutine_uniforms(void)
{
	int sidx;
	for (sidx = 0; sidx < 4; sidx++) {
		free(subuniform_locations[sidx]);
		subuniform_locations[sidx] = NULL;
	}
}

static void
program_subroutine_uniforms(void)
{
	int sidx;
	int stype;

	for (sidx = 0; sidx < 4; sidx++) {

		if (num_subuniform_locations[sidx] == 0)
			continue;

		stype = lookup_shader_type(sidx);
		if (!stype)
			continue;

		glUniformSubroutinesuiv(stype, num_subuniform_locations[sidx], subuniform_locations[sidx]);
	}
}

static void
set_subroutine_uniform(const char *line)
{
	GLuint prog;
	char type[512];
	char name[512];
	char subname[512];
	GLint loc;
	GLuint idx;
	GLenum ptype = 0;
	int sidx = 0;

	REQUIRE(parse_word_copy(line, type, sizeof(type), &line) &&
		parse_word_copy(line, name, sizeof(name), &line) &&
		parse_word_copy(line, subname, sizeof(subname), &line),
		"Invalid set subroutine uniform command at: %s\n", line);

	ptype = get_shader_from_string(type, &sidx);
	if (ptype == 0) {
		printf("illegal type in subroutine uniform\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	glGetIntegerv(GL_CURRENT_PROGRAM, (GLint *) &prog);

	if (num_subuniform_locations[sidx] == 0) {
		glGetProgramStageiv(prog, ptype, GL_ACTIVE_SUBROUTINE_UNIFORM_LOCATIONS,
				    &num_subuniform_locations[sidx]);

		if (num_subuniform_locations[sidx] == 0) {
			printf("illegal subroutine uniform specified\n");
			piglit_report_result(PIGLIT_FAIL);
		}

		subuniform_locations[sidx] = calloc(num_subuniform_locations[sidx], sizeof(GLuint));
		if (!subuniform_locations[sidx])
			piglit_report_result(PIGLIT_FAIL);
	}

	loc = glGetSubroutineUniformLocation(prog, ptype, name);
	if (loc < 0) {
		printf("cannot get location of subroutine uniform \"%s\"\n",
		       name);
		piglit_report_result(PIGLIT_FAIL);
	}

	idx = glGetSubroutineIndex(prog, ptype, subname);
	if (idx == GL_INVALID_INDEX) {
		printf("cannot get index of subroutine uniform \"%s\"\n",
		       subname);
		piglit_report_result(PIGLIT_FAIL);
	}

	subuniform_locations[sidx][loc] = idx;
	return;
}

/**
 * Query a uniform using glGetActiveUniformsiv
 *
 * Format of the command:
 *
 *     active uniform uniform_name GL_PNAME_ENUM integer
 *
 * or
 *
 *     active uniform uniform_name GL_PNAME_ENUM GL_TYPE_ENUM
 */
static void
active_uniform(const char *line)
{
	static const struct string_to_enum all_pnames[] = {
		ENUM_STRING(GL_UNIFORM_TYPE),
		ENUM_STRING(GL_UNIFORM_SIZE),
		ENUM_STRING(GL_UNIFORM_NAME_LENGTH),
		ENUM_STRING(GL_UNIFORM_BLOCK_INDEX),
		ENUM_STRING(GL_UNIFORM_OFFSET),
		ENUM_STRING(GL_UNIFORM_ARRAY_STRIDE),
		ENUM_STRING(GL_UNIFORM_MATRIX_STRIDE),
		ENUM_STRING(GL_UNIFORM_IS_ROW_MAJOR),
		ENUM_STRING(GL_UNIFORM_ATOMIC_COUNTER_BUFFER_INDEX),
		{ NULL, 0 }
	};

	const char *rest = line;
	char name[512];
	char name_buf[512];
	unsigned pname;
	int expected;
	int i;
	int num_active_uniforms;

	REQUIRE(parse_word_copy(rest, name, sizeof(name), &rest),
		"Bad uniform name at: %s\n", line);

	REQUIRE(parse_enum_tab(all_pnames, rest, &pname, &rest),
		"Bad glGetUniformsiv pname at: %s\n", line);

	REQUIRE(parse_enum_tab(all_types, rest, (unsigned *)&expected, &rest) ||
		parse_int(rest, &expected, &rest),
		"Bad expected value at: %s\n", line);

	glGetProgramiv(prog, GL_ACTIVE_UNIFORMS, &num_active_uniforms);
	for (i = 0; i < num_active_uniforms; i++) {
		GLint got;
		GLint size;
		GLenum type;
		GLsizei name_len;
		bool pass = true;

		glGetActiveUniform(prog, i, sizeof(name_buf), &name_len,
				   &size, &type, name_buf);

		if (!piglit_check_gl_error(GL_NO_ERROR)) {
			fprintf(stderr, "glGetActiveUniform error\n");
			piglit_report_result(PIGLIT_FAIL);
		}

		if (strcmp(name, name_buf) != 0)
			continue;

		/* If the requested pname is one of the values that
		 * glGetActiveUniform happens to return, check the value
		 * returned by that function too.
		 */
		switch (pname) {
		case GL_UNIFORM_TYPE:
			got = (GLint) type;
			break;

		case GL_UNIFORM_SIZE:
			got = size;
			break;

		case GL_UNIFORM_NAME_LENGTH:
			got = name_len;
			break;

		default:
			/* This ensures the check below will pass when the
			 * requested enum is not one of the values already
			 * returned by glGetActiveUniform.
			 */
			got = expected;
			break;
		}

		if (got != expected) {
			fprintf(stderr,
				"glGetActiveUniform(%s, %s): "
				"expected %d (0x%04x), got %d (0x%04x)\n",
				name, piglit_get_gl_enum_name(pname),
				expected, expected, got, got);
			pass = false;
		}

		/* Set 'got' to some value in case glGetActiveUniformsiv
		 * doesn't write to it.  That should only be able to occur
		 * when the function raises a GL error, but "should" is kind
		 * of a funny word.
		 */
		got = ~expected;
		glGetActiveUniformsiv(prog, 1, (GLuint *) &i, pname, &got);

		if (!piglit_check_gl_error(GL_NO_ERROR)) {
			fprintf(stderr, "glGetActiveUniformsiv error\n");
			piglit_report_result(PIGLIT_FAIL);
		}

		if (got != expected) {
			fprintf(stderr,
				"glGetActiveUniformsiv(%s, %s): "
				"expected %d, got %d\n",
				name, piglit_get_gl_enum_name(pname),
				expected, got);
			pass = false;
		}

		if (!pass)
			piglit_report_result(PIGLIT_FAIL);

		return;
	}

	fprintf(stderr, "No active uniform named \"%s\"\n", name);
	piglit_report_result(PIGLIT_FAIL);
	return;
}

/**
 * Query an active resource using ARB_program_interface_query functions
 *
 * Format of the command:
 *
 *  verify program_interface_query GL_INTERFACE_TYPE_ENUM name GL_PNAME_ENUM integer
 *
 * or
 *
 *  verify program_interface_query GL_INTERFACE_TYPE_ENUM name GL_PNAME_ENUM GL_TYPE_ENUM
 */
static void
active_program_interface(const char *line)
{
	static const struct string_to_enum all_props[] = {
		ENUM_STRING(GL_TYPE),
		ENUM_STRING(GL_ARRAY_SIZE),
		ENUM_STRING(GL_NAME_LENGTH),
		ENUM_STRING(GL_BLOCK_INDEX),
		ENUM_STRING(GL_OFFSET),
		ENUM_STRING(GL_ARRAY_STRIDE),
		ENUM_STRING(GL_MATRIX_STRIDE),
		ENUM_STRING(GL_IS_ROW_MAJOR),
		ENUM_STRING(GL_ATOMIC_COUNTER_BUFFER_INDEX),
		ENUM_STRING(GL_BUFFER_BINDING),
		ENUM_STRING(GL_BUFFER_DATA_SIZE),
		ENUM_STRING(GL_NUM_ACTIVE_VARIABLES),
		ENUM_STRING(GL_REFERENCED_BY_VERTEX_SHADER),
		ENUM_STRING(GL_REFERENCED_BY_TESS_CONTROL_SHADER),
		ENUM_STRING(GL_REFERENCED_BY_TESS_EVALUATION_SHADER),
		ENUM_STRING(GL_REFERENCED_BY_GEOMETRY_SHADER),
		ENUM_STRING(GL_REFERENCED_BY_FRAGMENT_SHADER),
		ENUM_STRING(GL_REFERENCED_BY_COMPUTE_SHADER),
		ENUM_STRING(GL_TOP_LEVEL_ARRAY_SIZE),
		ENUM_STRING(GL_TOP_LEVEL_ARRAY_STRIDE),
		ENUM_STRING(GL_LOCATION),
		ENUM_STRING(GL_LOCATION_INDEX),
		ENUM_STRING(GL_LOCATION_COMPONENT),
		ENUM_STRING(GL_IS_PER_PATCH),
		ENUM_STRING(GL_NUM_COMPATIBLE_SUBROUTINES),
		ENUM_STRING(GL_COMPATIBLE_SUBROUTINES),
		{ NULL, 0 }
	};

	static const struct string_to_enum all_program_interface[] = {
		ENUM_STRING(GL_UNIFORM),
		ENUM_STRING(GL_UNIFORM_BLOCK),
		ENUM_STRING(GL_PROGRAM_INPUT),
		ENUM_STRING(GL_PROGRAM_OUTPUT),
		ENUM_STRING(GL_BUFFER_VARIABLE),
		ENUM_STRING(GL_SHADER_STORAGE_BLOCK),
		ENUM_STRING(GL_ATOMIC_COUNTER_BUFFER),
		ENUM_STRING(GL_VERTEX_SUBROUTINE),
		ENUM_STRING(GL_TESS_CONTROL_SUBROUTINE),
		ENUM_STRING(GL_TESS_EVALUATION_SUBROUTINE),
		ENUM_STRING(GL_GEOMETRY_SUBROUTINE),
		ENUM_STRING(GL_FRAGMENT_SUBROUTINE),
		ENUM_STRING(GL_COMPUTE_SUBROUTINE),
		ENUM_STRING(GL_VERTEX_SUBROUTINE_UNIFORM),
		ENUM_STRING(GL_TESS_CONTROL_SUBROUTINE_UNIFORM),
		ENUM_STRING(GL_TESS_EVALUATION_SUBROUTINE_UNIFORM),
		ENUM_STRING(GL_GEOMETRY_SUBROUTINE_UNIFORM),
		ENUM_STRING(GL_FRAGMENT_SUBROUTINE_UNIFORM),
		ENUM_STRING(GL_COMPUTE_SUBROUTINE_UNIFORM),
		ENUM_STRING(GL_TRANSFORM_FEEDBACK_VARYING),
		{ NULL, 0 }
	};

	char name[512];
	char name_buf[512];
	unsigned prop, interface_type;
	int expected;
	int i;
	int num_active_buffers;

	if (!piglit_is_extension_supported("GL_ARB_program_interface_query") &&
	    piglit_get_gl_version() < 43) {
		fprintf(stderr,
			"GL_ARB_program_interface_query not supported or "
			"OpenGL version < 4.3\n");
		return;
	}

	REQUIRE(parse_enum_tab(all_program_interface, line,
			       &interface_type, &line),
		"Bad program interface at: %s\n", line);
	REQUIRE(parse_word_copy(line, name, sizeof(name), &line),
		"Bad program resource name at: %s\n", line);
	REQUIRE(parse_enum_tab(all_props, line, &prop, &line),
		"Bad glGetProgramResourceiv pname at: %s\n", line);
	REQUIRE(parse_enum_tab(all_types, line, (unsigned *)&expected, &line) ||
		parse_int(line, &expected, &line),
		"Bad expected value at: %s\n", line);

	glGetProgramInterfaceiv(prog, interface_type,
				GL_ACTIVE_RESOURCES, &num_active_buffers);
	for (i = 0; i < num_active_buffers; i++) {
		GLint got;
		GLint length;
		GLsizei name_len;
		bool pass = true;

		glGetProgramResourceName(prog, interface_type,
					 i, 512, &name_len, name_buf);

		if (!piglit_check_gl_error(GL_NO_ERROR)) {
			fprintf(stderr, "glGetProgramResourceName error\n");
			piglit_report_result(PIGLIT_FAIL);
		}

		if (strcmp(name, name_buf) != 0)
			continue;

		if (prop == GL_NAME_LENGTH && name_len != expected) {
			fprintf(stderr,
				"glGetProgramResourceName(%s, %s): "
				"expected %d (0x%04x), got %d (0x%04x)\n",
				name, piglit_get_gl_enum_name(prop),
				expected, expected, name_len, name_len);
			pass = false;
		}

		/* Set 'got' to some value in case glGetActiveUniformsiv
		 * doesn't write to it.  That should only be able to occur
		 * when the function raises a GL error, but "should" is kind
		 * of a funny word.
		 */
		got = ~expected;
		glGetProgramResourceiv(prog, interface_type,
				       i, 1, &prop, 1,
				       &length, &got);

		if (!piglit_check_gl_error(GL_NO_ERROR)) {
			fprintf(stderr, "glGetProgramResourceiv error\n");
			piglit_report_result(PIGLIT_FAIL);
		}

		if (got != expected) {
			fprintf(stderr,
				"glGetProgramResourceiv(%s, %s): "
				"expected %d, got %d\n",
				name, piglit_get_gl_enum_name(prop),
				expected, got);
			pass = false;
		}

		if (!pass)
			piglit_report_result(PIGLIT_FAIL);

		return;
	}

	fprintf(stderr, "No active resource named \"%s\"\n", name);
	piglit_report_result(PIGLIT_FAIL);
	return;
}

static void
set_parameter(const char *line)
{
	float f[4];
	int index, count;
	char type[1024];

	count = sscanf(line, "%s %d (%f , %f , %f , %f)",
		       type, &index, &f[0], &f[1], &f[2], &f[3]);
	if (count != 6) {
		fprintf(stderr, "Couldn't parse parameter command:\n%s\n", line);
		piglit_report_result(PIGLIT_FAIL);
	}

	if (parse_str(type, "env_vp", NULL)) {
		glProgramEnvParameter4fvARB(GL_VERTEX_PROGRAM_ARB, index, f);
	} else if (parse_str(type, "local_vp", NULL)) {
		glProgramLocalParameter4fvARB(GL_VERTEX_PROGRAM_ARB, index, f);
	} else if (parse_str(type, "env_fp", NULL)) {
		glProgramEnvParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, index, f);
	} else if (parse_str(type, "local_fp", NULL)) {
		glProgramLocalParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, index, f);
	} else {
		fprintf(stderr, "Unknown parameter type `%s'\n", type);
		piglit_report_result(PIGLIT_FAIL);
	}
}

static void
set_patch_parameter(const char *line)
{
#ifdef PIGLIT_USE_OPENGL
	float f[4];
	int i, count;
	const char *const line0 = line;

	if (gl_version.num < 40)
		piglit_require_extension("GL_ARB_tessellation_shader");

	if (parse_str(line, "vertices ", &line)) {
		count = sscanf(line, "%d", &i);
		if (count != 1) {
			fprintf(stderr, "Couldn't parse patch parameter command:\n%s\n", line0);
			piglit_report_result(PIGLIT_FAIL);
		}
		glPatchParameteri(GL_PATCH_VERTICES, i);
	} else if (parse_str(line, "default level outer ", &line)) {
		count = sscanf(line, "%f %f %f %f", &f[0], &f[1], &f[2], &f[3]);
		if (count != 4) {
			fprintf(stderr, "Couldn't parse patch parameter command:\n%s\n", line0);
			piglit_report_result(PIGLIT_FAIL);
		}
		glPatchParameterfv(GL_PATCH_DEFAULT_OUTER_LEVEL, f);
	} else if (parse_str(line, "default level inner ", &line)) {
		count = sscanf(line, "%f %f", &f[0], &f[1]);
		if (count != 2) {
			fprintf(stderr, "Couldn't parse patch parameter command:\n%s\n", line0);
			piglit_report_result(PIGLIT_FAIL);
		}
		glPatchParameterfv(GL_PATCH_DEFAULT_INNER_LEVEL, f);
	} else {
		fprintf(stderr, "Couldn't parse patch parameter command:\n%s\n", line);
		piglit_report_result(PIGLIT_FAIL);
	}
#else
	printf("patch parameters are only available in desktop GL\n");
	piglit_report_result(PIGLIT_SKIP);
#endif
}

static void
set_provoking_vertex(const char *line)
{
	if (parse_str(line, "first", NULL)) {
		glProvokingVertexEXT(GL_FIRST_VERTEX_CONVENTION_EXT);
	} else if (parse_str(line, "last", NULL)) {
		glProvokingVertexEXT(GL_LAST_VERTEX_CONVENTION_EXT);
	} else {
		fprintf(stderr, "Unknown provoking vertex parameter `%s'\n", line);
		piglit_report_result(PIGLIT_FAIL);
	}
}

static const struct string_to_enum enable_table[] = {
	{ "GL_CLIP_PLANE0", GL_CLIP_PLANE0 },
	{ "GL_CLIP_PLANE1", GL_CLIP_PLANE1 },
	{ "GL_CLIP_PLANE2", GL_CLIP_PLANE2 },
	{ "GL_CLIP_PLANE3", GL_CLIP_PLANE3 },
	{ "GL_CLIP_PLANE4", GL_CLIP_PLANE4 },
	{ "GL_CLIP_PLANE5", GL_CLIP_PLANE5 },
	{ "GL_CLIP_PLANE6", GL_CLIP_PLANE0+6 },
	{ "GL_CLIP_PLANE7", GL_CLIP_PLANE0+7 },
	{ "GL_VERTEX_PROGRAM_TWO_SIDE", GL_VERTEX_PROGRAM_TWO_SIDE },
	{ "GL_PROGRAM_POINT_SIZE", GL_PROGRAM_POINT_SIZE },
	{ "GL_DEPTH_TEST", GL_DEPTH_TEST },
	{ NULL, 0 }
};

static void
do_enable_disable(const char *line, bool enable_flag)
{
	GLenum value;
	REQUIRE(parse_enum_tab(enable_table, line, &value, NULL),
		"Bad enable/disable enum at: %s\n", line);

	if (enable_flag)
		glEnable(value);
	else
		glDisable(value);
}

static const struct string_to_enum hint_target_table[] = {
	ENUM_STRING(GL_LINE_SMOOTH_HINT),
	ENUM_STRING(GL_POLYGON_SMOOTH_HINT),
	ENUM_STRING(GL_TEXTURE_COMPRESSION_HINT),
	ENUM_STRING(GL_FRAGMENT_SHADER_DERIVATIVE_HINT),
	{ NULL, 0 }
};

static const struct string_to_enum hint_param_table[] = {
	ENUM_STRING(GL_FASTEST),
	ENUM_STRING(GL_NICEST),
	ENUM_STRING(GL_DONT_CARE),
	{ NULL, 0 }
};

static void do_hint(const char *line)
{
	unsigned target, param;
	REQUIRE(parse_enum_tab(hint_target_table, line, &target, &line),
		"Bad hint target at: %s\n", line);
	REQUIRE(parse_enum_tab(hint_param_table, line, &param, &line),
		"Bad hint param at: %s\n", line);

	glHint(target, param);
}

static void
draw_instanced_rect(int primcount, float x, float y, float w, float h)
{
	float verts[4][4];

	piglit_require_extension("GL_ARB_draw_instanced");

	verts[0][0] = x;
	verts[0][1] = y;
	verts[0][2] = 0.0;
	verts[0][3] = 1.0;
	verts[1][0] = x + w;
	verts[1][1] = y;
	verts[1][2] = 0.0;
	verts[1][3] = 1.0;
	verts[2][0] = x;
	verts[2][1] = y + h;
	verts[2][2] = 0.0;
	verts[2][3] = 1.0;
	verts[3][0] = x + w;
	verts[3][1] = y + h;
	verts[3][2] = 0.0;
	verts[3][3] = 1.0;

	glVertexPointer(4, GL_FLOAT, 0, verts);
	glEnableClientState(GL_VERTEX_ARRAY);

	glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, primcount);

	glDisableClientState(GL_VERTEX_ARRAY);
}


static GLenum
decode_drawing_mode(const char *mode_str)
{
	int i;

	for (i = GL_POINTS; i <= GL_PATCHES; ++i) {
		const char *name = piglit_get_prim_name(i);
		if (0 == strcmp(mode_str, name))
			return i;
	}

	printf("unknown drawing mode \"%s\"\n", mode_str);
	piglit_report_result(PIGLIT_FAIL);

	/* Should not be reached, but return 0 to avoid compiler warning */
	return 0;
}

static void
handle_texparameter(const char *line)
{
	static const struct string_to_enum compare_funcs[] = {
		{ "greater", GL_GREATER },
		{ "gequal", GL_GEQUAL },
		{ "less", GL_LESS },
		{ "lequal", GL_LEQUAL },
		{ "equal", GL_EQUAL },
		{ "notequal", GL_NOTEQUAL },
		{ "never", GL_NEVER },
		{ "always", GL_ALWAYS },
		{ NULL, 0 },
	};
	static const struct string_to_enum depth_modes[] = {
		{ "intensity", GL_INTENSITY },
		{ "luminance", GL_LUMINANCE },
		{ "alpha", GL_ALPHA },
		{ "red", GL_RED }, /* Requires GL 3.0 or GL_ARB_texture_rg */
		{ NULL, 0 },
	};
	static const struct string_to_enum min_filter_modes[] = {
		{ "nearest_mipmap_nearest", GL_NEAREST_MIPMAP_NEAREST },
		{ "linear_mipmap_nearest",  GL_LINEAR_MIPMAP_NEAREST  },
		{ "nearest_mipmap_linear",  GL_NEAREST_MIPMAP_LINEAR  },
		{ "linear_mipmap_linear",   GL_LINEAR_MIPMAP_LINEAR   },
		{ "nearest",                GL_NEAREST                },
		{ "linear",                 GL_LINEAR                 },
		{ NULL, 0 }
	};
	static const struct string_to_enum mag_filter_modes[] = {
		{ "nearest",                GL_NEAREST                },
		{ "linear",                 GL_LINEAR                 },
		{ NULL, 0 }
	};
	static const struct string_to_enum wrap_modes[] = {
		{ "repeat",                 GL_REPEAT                },
		{ "clamp_to_edge",          GL_CLAMP_TO_EDGE         },
		{ "clamp_to_border",        GL_CLAMP_TO_BORDER       },
		{ NULL, 0 }
	};
	static const struct string_to_enum swizzle_modes[] = {
		{ "red", GL_RED },
		{ "green", GL_GREEN },
		{ "blue", GL_BLUE },
		{ "alpha", GL_ALPHA },
		{ NULL, 0 }
	};
	unsigned target = 0;
	GLenum parameter = GL_NONE;
	const char *parameter_name = NULL;
	const struct string_to_enum *strings = NULL;
	unsigned value;

	REQUIRE(parse_tex_target(line, &target, &line),
		"Bad texture target at: %s\n", line);

	if (parse_str(line, "compare_func ", &line)) {
		parameter = GL_TEXTURE_COMPARE_FUNC;
		parameter_name = "compare_func";
		strings = compare_funcs;
	} else if (parse_str(line, "depth_mode ", &line)) {
		parameter = GL_DEPTH_TEXTURE_MODE;
		parameter_name = "depth_mode";
		strings = depth_modes;
	} else if (parse_str(line, "min ", &line)) {
		parameter = GL_TEXTURE_MIN_FILTER;
		parameter_name = "min";
		strings = min_filter_modes;
	} else if (parse_str(line, "mag ", &line)) {
		parameter = GL_TEXTURE_MAG_FILTER;
		parameter_name = "mag";
		strings = mag_filter_modes;
	} else if (parse_str(line, "wrap_s ", &line)) {
		parameter = GL_TEXTURE_WRAP_S;
		parameter_name = "wrap_s";
		strings = wrap_modes;
	} else if (parse_str(line, "wrap_t ", &line)) {
		parameter = GL_TEXTURE_WRAP_T;
		parameter_name = "wrap_t";
		strings = wrap_modes;
	} else if (parse_str(line, "wrap_r ", &line)) {
		parameter = GL_TEXTURE_WRAP_R;
		parameter_name = "wrap_r";
		strings = wrap_modes;
	} else if (parse_str(line, "lod_bias ", &line)) {
#ifdef PIGLIT_USE_OPENGL
		glTexParameterf(target, GL_TEXTURE_LOD_BIAS,
				strtod(line, NULL));
		return;
#else
		printf("lod_bias feature is only available in desktop GL\n");
		piglit_report_result(PIGLIT_SKIP);
#endif
	} else if (parse_str(line, "max_level ", &line)) {
		glTexParameteri(target, GL_TEXTURE_MAX_LEVEL,
				strtol(line, NULL, 10));
		return;
	} else if (parse_str(line, "base_level ", &line)) {
		glTexParameteri(target, GL_TEXTURE_BASE_LEVEL,
				strtol(line, NULL, 10));
		return;
	} else if (parse_str(line, "border_color ", &line)) {
		float bc[4];
		int count;
		count = sscanf(line, "%f %f %f %f", &bc[0], &bc[1], &bc[2], &bc[3]);
		if (count != 4) {
			fprintf(stderr, "Could not parse border_color texture parameter.\n");
			piglit_report_result(PIGLIT_FAIL);
		}
		glTexParameterfv(target, GL_TEXTURE_BORDER_COLOR, bc);
		return;
	} else if (parse_str(line, "swizzle_r ", &line)) {
		parameter = GL_TEXTURE_SWIZZLE_R;
		parameter_name = "swizzle_r";
		strings = swizzle_modes;
	} else {
		fprintf(stderr, "unknown texture parameter in `%s'\n", line);
		piglit_report_result(PIGLIT_FAIL);
	}

	REQUIRE(parse_enum_tab(strings, line, &value, &line),
		"Bad %s at: %s\n", parameter_name, line);

	glTexParameteri(target, parameter, value);
}

static void
setup_ubos(void)
{
	int i;

	if (!piglit_is_extension_supported("GL_ARB_uniform_buffer_object") &&
	    piglit_get_gl_version() < 31) {
		return;
	}

	if (prog == 0) {
		/* probably running an ARB_vertex/fragment_program test */
		return;
	}

	glGetProgramiv(prog, GL_ACTIVE_UNIFORM_BLOCKS, &num_uniform_blocks);
	if (num_uniform_blocks == 0)
		return;

	uniform_block_bos = calloc(num_uniform_blocks, sizeof(GLuint));
	glGenBuffers(num_uniform_blocks, uniform_block_bos);

	for (i = 0; i < num_uniform_blocks; i++) {
		GLint size;

		glGetActiveUniformBlockiv(prog, i, GL_UNIFORM_BLOCK_DATA_SIZE,
					  &size);

		glBindBuffer(GL_UNIFORM_BUFFER, uniform_block_bos[i]);
		glBufferData(GL_UNIFORM_BUFFER, size, NULL, GL_STATIC_DRAW);
		glBindBufferBase(GL_UNIFORM_BUFFER, i, uniform_block_bos[i]);

		glUniformBlockBinding(prog, i, i);
	}
}

static void
teardown_fbos(void)
{
	if (draw_fbo != 0 &&
	    draw_fbo != piglit_winsys_fbo)
		glDeleteFramebuffers(1, &draw_fbo);

	if (read_fbo != 0 &&
	    read_fbo != piglit_winsys_fbo &&
	    read_fbo != draw_fbo)
		glDeleteFramebuffers(1, &read_fbo);

	draw_fbo = 0;
	read_fbo = 0;
}

static void
teardown_ubos(void)
{
	if (num_uniform_blocks == 0) {
		return;
	}

	glDeleteBuffers(num_uniform_blocks, uniform_block_bos);
	free(uniform_block_bos);
	uniform_block_bos = NULL;
	num_uniform_blocks = 0;
}

static void
teardown_atomics(void)
{
	for (unsigned i = 0; i < ARRAY_SIZE(atomics_bos); ++i) {
		if (atomics_bos[i])
			glDeleteBuffers(1, &atomics_bos[i]);
	}
}

static enum piglit_result
program_must_be_in_use(void)
{
	if (!link_ok) {
		fprintf(stderr, "Failed to link:\n%s\n", prog_err_info);
		return PIGLIT_FAIL;
	} else if (!prog_in_use) {
		fprintf(stderr, "Failed to use program: %s\n", prog_err_info);
		return PIGLIT_FAIL;
	}
	return PIGLIT_PASS;
}

static void
bind_vao_if_supported()
{
	if (vao == 0 && piglit_is_core_profile) {
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
	}
}

static enum piglit_result
draw_arrays_common(int first, size_t count)
{
	enum piglit_result result = program_must_be_in_use();
	if (first < 0) {
		printf("draw arrays 'first' must be >= 0\n");
		piglit_report_result(PIGLIT_FAIL);
	} else if (vbo_present &&
			(size_t) first >= num_vbo_rows) {
		printf("draw arrays 'first' must be < %lu\n",
			(unsigned long) num_vbo_rows);
		piglit_report_result(PIGLIT_FAIL);
	}
	if (count <= 0) {
		printf("draw arrays 'count' must be > 0\n");
		piglit_report_result(PIGLIT_FAIL);
	} else if (vbo_present &&
			count > num_vbo_rows - (size_t) first) {
		printf("draw arrays cannot draw beyond %lu\n",
			(unsigned long) num_vbo_rows);
		piglit_report_result(PIGLIT_FAIL);
	}
	bind_vao_if_supported();
	return result;
}

static bool
probe_atomic_counter(unsigned buffer_num, GLint counter_num, const char *op,
		     uint32_t value, bool layout_params)
{
        uint32_t *p;
	uint32_t observed;
	enum comparison cmp;
	bool result;

	REQUIRE(parse_comparison_op(op, &cmp, NULL),
		"Invalid comparison operation at: %s\n", op);

	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomics_bos[buffer_num]);
	p = glMapBufferRange(GL_ATOMIC_COUNTER_BUFFER,
			     layout_params ? counter_num : counter_num * sizeof(uint32_t),
			     sizeof(uint32_t), GL_MAP_READ_BIT);

        if (!p) {
                printf("Couldn't map atomic counter to verify expected value.\n");
                return false;
        }

	observed = *p;
	result = compare_uint(value, observed, cmp);

	if (!result) {
		if (layout_params)
			printf("Atomic counter (binding = %d, offset = %d) test failed: "
			       "Reference %s Observed\n",
			       buffer_num, counter_num, comparison_string(cmp));
		else
			printf("Atomic counter %d test failed: Reference %s Observed\n",
			       counter_num, comparison_string(cmp));
		printf("  Reference: %u\n", value);
		printf("  Observed:  %u\n", observed);
		glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);
		return false;
	}

	glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);
	return true;
}

static bool
probe_ssbo_uint(GLint ssbo_index, GLint ssbo_offset, const char *op, uint32_t value)
{
	uint32_t *p;
	uint32_t observed;
	enum comparison cmp;
	bool result;

	REQUIRE(parse_comparison_op(op, &cmp, NULL),
		"Invalid comparison operation at: %s\n", op);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo[ssbo_index]);
	p = glMapBufferRange(GL_SHADER_STORAGE_BUFFER, ssbo_offset,
			     sizeof(uint32_t), GL_MAP_READ_BIT);

	if (!p) {
		printf("Couldn't map ssbo to verify expected value.\n");
		return false;
	}

	observed = *p;
	result = compare_uint(value, observed, cmp);

	if (!result) {
		printf("SSBO %d test failed: Reference %s Observed\n",
		       ssbo_offset, comparison_string(cmp));
		printf("  Reference: %u\n", value);
		printf("  Observed:  %u\n", observed);
		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
		return false;
	}

	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	return true;
}

enum piglit_result
piglit_display(void)
{
	const char *line, *next_line, *rest;
	unsigned line_num;
	enum piglit_result full_result = PIGLIT_PASS;
	GLbitfield clear_bits = 0;
	bool link_error_expected = false;
	int ubo_array_index = 0;

	if (test_start == NULL)
		return PIGLIT_PASS;

	next_line = test_start;
	line_num = test_start_line_num;
	while (next_line[0] != '\0') {
		float c[32];
		double d[4];
		int x, y, z, w, h, l, tex, level;
		unsigned ux, uy, uz;
		char s[300]; // 300 for safety
		enum piglit_result result = PIGLIT_PASS;

		parse_whitespace(next_line, &line);

		next_line = strchrnul(next_line, '\n');

		/* Duplicate the line to make it null terminated */
		line = strndup(line, next_line - line);

		/* If strchrnul found a newline, then skip it */
		if (next_line[0] != '\0')
			next_line++;

		if (line[0] == '\0') {
		} else if (sscanf(line, "active shader program %s", s) == 1) {
			switch (get_shader_from_string(s, &x)) {
			case GL_VERTEX_SHADER:
				glActiveShaderProgram(pipeline, sso_vertex_prog);
			break;
			case GL_TESS_CONTROL_SHADER:
				glActiveShaderProgram(pipeline, sso_tess_control_prog);
			break;
			case GL_TESS_EVALUATION_SHADER:
				glActiveShaderProgram(pipeline, sso_tess_eval_prog);
			break;
			case GL_GEOMETRY_SHADER:
				glActiveShaderProgram(pipeline, sso_geometry_prog);
			break;
			case GL_FRAGMENT_SHADER:
				glActiveShaderProgram(pipeline, sso_fragment_prog);
			break;
			case GL_COMPUTE_SHADER:
				glActiveShaderProgram(pipeline, sso_compute_prog);
			break;
			}
		} else if (sscanf(line, "atomic counter buffer %u %u", &x, &y) == 2) {
			GLuint *atomics_buf = calloc(y, sizeof(GLuint));
			glGenBuffers(1, &atomics_bos[x]);
			glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, x, atomics_bos[x]);
			glBufferData(GL_ATOMIC_COUNTER_BUFFER,
				     sizeof(GLuint) * y, atomics_buf,
				     GL_STATIC_DRAW);
			free(atomics_buf);
		} else if (sscanf(line, "atomic counters %d", &x) == 1) {
			GLuint *atomics_buf = calloc(x, sizeof(GLuint));
			glGenBuffers(1, &atomics_bos[0]);
			glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, atomics_bos[0]);
			glBufferData(GL_ATOMIC_COUNTER_BUFFER,
				     sizeof(GLuint) * x,
				     atomics_buf, GL_STATIC_DRAW);
			free(atomics_buf);
		} else if (sscanf(line, "atomic counter %u %u %u", &x, &y, &z) == 3) {
			glNamedBufferSubData(atomics_bos[x],
					     sizeof(GLuint) * y, sizeof(GLuint),
					     &z);
		} else if (parse_str(line, "clear color ", &rest)) {
			parse_floats(rest, c, 4, NULL);
			glClearColor(c[0], c[1], c[2], c[3]);
			clear_bits |= GL_COLOR_BUFFER_BIT;
		} else if (parse_str(line, "clear depth ", &rest)) {
			parse_floats(rest, c, 1, NULL);
			glClearDepth(c[0]);
			clear_bits |= GL_DEPTH_BUFFER_BIT;
		} else if (parse_str(line, "clear", NULL)) {
			glClear(clear_bits);
		} else if (sscanf(line,
				  "clip plane %d %lf %lf %lf %lf",
				  &x, &d[0], &d[1], &d[2], &d[3]) == 5) {
			if (x < 0 || x >= gl_max_clip_planes) {
				printf("clip plane id %d out of range\n", x);
				piglit_report_result(PIGLIT_FAIL);
			}
			glClipPlane(GL_CLIP_PLANE0 + x, d);
#ifdef PIGLIT_USE_OPENGL
		} else if (parse_str(line, "color ", &rest)) {
			parse_floats(rest, c, 4, NULL);
			assert(!piglit_is_core_profile);
			glColor4fv(c);
#endif
		} else if (sscanf(line,
				  "compute %d %d %d",
				  &x, &y, &z) == 3) {
			result = program_must_be_in_use();
			glMemoryBarrier(GL_ALL_BARRIER_BITS);
			glDispatchCompute(x, y, z);
			glMemoryBarrier(GL_ALL_BARRIER_BITS);
		} else if (sscanf(line,
				  "compute group size %d %d %d %d %d %d",
				  &x, &y, &z, &w, &h, &l) == 6) {
			result = program_must_be_in_use();
			glMemoryBarrier(GL_ALL_BARRIER_BITS);
			glDispatchComputeGroupSizeARB(x, y, z, w, h, l);
			glMemoryBarrier(GL_ALL_BARRIER_BITS);
		} else if (parse_str(line, "draw rect tex ", &rest)) {
			result = program_must_be_in_use();
			program_subroutine_uniforms();
			parse_floats(rest, c, 8, NULL);
			piglit_draw_rect_tex(c[0], c[1], c[2], c[3],
					     c[4], c[5], c[6], c[7]);
		} else if (parse_str(line, "draw rect ortho patch ", &rest)) {
			result = program_must_be_in_use();
			program_subroutine_uniforms();
			parse_floats(rest, c, 4, NULL);

			piglit_draw_rect_custom(-1.0 + 2.0 * (c[0] / piglit_width),
						-1.0 + 2.0 * (c[1] / piglit_height),
						2.0 * (c[2] / piglit_width),
						2.0 * (c[3] / piglit_height), true, 1);
		} else if (parse_str(line, "draw rect ortho ", &rest)) {
			result = program_must_be_in_use();
			program_subroutine_uniforms();
			parse_floats(rest, c, 4, NULL);

			piglit_draw_rect(-1.0 + 2.0 * (c[0] / piglit_width),
					 -1.0 + 2.0 * (c[1] / piglit_height),
					 2.0 * (c[2] / piglit_width),
					 2.0 * (c[3] / piglit_height));
		} else if (parse_str(line, "draw rect patch ", &rest)) {
			result = program_must_be_in_use();
			parse_floats(rest, c, 4, NULL);
			piglit_draw_rect_custom(c[0], c[1], c[2], c[3], true, 1);
		} else if (parse_str(line, "draw rect ", &rest)) {
			result = program_must_be_in_use();
			program_subroutine_uniforms();
			parse_floats(rest, c, 4, NULL);
			piglit_draw_rect(c[0], c[1], c[2], c[3]);
		} else if (parse_str(line, "draw instanced rect ortho patch ", &rest)) {
			int instance_count;

			result = program_must_be_in_use();
			sscanf(rest, "%d %f %f %f %f",
			       &instance_count,
			       c + 0, c + 1, c + 2, c + 3);
			piglit_draw_rect_custom(-1.0 + 2.0 * (c[0] / piglit_width),
						-1.0 + 2.0 * (c[1] / piglit_height),
						2.0 * (c[2] / piglit_width),
						2.0 * (c[3] / piglit_height), true,
						instance_count);
		} else if (parse_str(line, "draw instanced rect ortho ", &rest)) {
			int instance_count;

			result = program_must_be_in_use();
			sscanf(rest, "%d %f %f %f %f",
			       &instance_count,
			       c + 0, c + 1, c + 2, c + 3);
			piglit_draw_rect_custom(-1.0 + 2.0 * (c[0] / piglit_width),
						-1.0 + 2.0 * (c[1] / piglit_height),
						2.0 * (c[2] / piglit_width),
						2.0 * (c[3] / piglit_height), false,
						instance_count);
		} else if (parse_str(line, "draw instanced rect ", &rest)) {
			int primcount;

			result = program_must_be_in_use();
			sscanf(rest, "%d %f %f %f %f",
			       &primcount,
			       c + 0, c + 1, c + 2, c + 3);
			draw_instanced_rect(primcount, c[0], c[1], c[2], c[3]);
		} else if (sscanf(line, "draw arrays instanced %31s %d %d %d", s, &x, &y, &z) == 4) {
			GLenum mode = decode_drawing_mode(s);
			int first = x;
			size_t count = (size_t) y;
			size_t primcount = (size_t) z;
			draw_arrays_common(first, count);
			glDrawArraysInstanced(mode, first, count, primcount);
		} else if (sscanf(line, "draw arrays %31s %d %d", s, &x, &y) == 3) {
			GLenum mode = decode_drawing_mode(s);
			int first = x;
			size_t count = (size_t) y;
			result = draw_arrays_common(first, count);
			glDrawArrays(mode, first, count);
		} else if (parse_str(line, "disable ", &rest)) {
			do_enable_disable(rest, false);
		} else if (parse_str(line, "enable ", &rest)) {
			do_enable_disable(rest, true);
		} else if (sscanf(line, "depthfunc %31s", s) == 1) {
			glDepthFunc(piglit_get_gl_enum_from_name(s));
		} else if (parse_str(line, "fb ", &rest)) {
			const GLenum target =
				parse_str(rest, "draw ", &rest) ? GL_DRAW_FRAMEBUFFER :
				parse_str(rest, "read ", &rest) ? GL_READ_FRAMEBUFFER :
				GL_FRAMEBUFFER;
			GLuint fbo = 0;

			if (parse_str(rest, "tex 2d ", &rest)) {
				GLenum attachments[32];
				unsigned num_attachments = 0;

				glGenFramebuffers(1, &fbo);
				glBindFramebuffer(target, fbo);

				while (parse_int(rest, &tex, &rest)) {
					attachments[num_attachments] =
						GL_COLOR_ATTACHMENT0 + num_attachments;
					glFramebufferTexture2D(
						target, attachments[num_attachments],
						GL_TEXTURE_2D,
						get_texture_binding(tex)->obj, 0);

					if (!piglit_check_gl_error(GL_NO_ERROR)) {
						fprintf(stderr,
							"glFramebufferTexture2D error\n");
						piglit_report_result(PIGLIT_FAIL);
					}

					num_attachments++;
				}

				if (target != GL_READ_FRAMEBUFFER)
					glDrawBuffers(num_attachments, attachments);

				w = get_texture_binding(tex)->width;
				h = get_texture_binding(tex)->height;

			} else if (parse_str(rest, "tex slice ", &rest)) {
				GLenum tex_target;

				REQUIRE(parse_tex_target(rest, &tex_target, &rest) &&
					parse_int(rest, &tex, &rest) &&
					parse_int(rest, &l, &rest) &&
					parse_int(rest, &z, &rest),
					"Framebuffer binding command not "
					"understood at: %s\n", rest);

				const GLuint tex_obj = get_texture_binding(tex)->obj;

				glGenFramebuffers(1, &fbo);
				glBindFramebuffer(target, fbo);

				if (tex_target == GL_TEXTURE_1D) {
					REQUIRE(z == 0,
						"Invalid layer index provided "
						"in command: %s\n", line);
					glFramebufferTexture1D(
						target, GL_COLOR_ATTACHMENT0,
						tex_target, tex_obj, l);

				} else if (tex_target == GL_TEXTURE_2D ||
					   tex_target == GL_TEXTURE_RECTANGLE ||
					   tex_target == GL_TEXTURE_2D_MULTISAMPLE) {
					REQUIRE(z == 0,
						"Invalid layer index provided "
						"in command: %s\n", line);
					glFramebufferTexture2D(
						target, GL_COLOR_ATTACHMENT0,
						tex_target, tex_obj, l);

				} else if (tex_target == GL_TEXTURE_CUBE_MAP) {
					static const GLenum cubemap_targets[] = {
						GL_TEXTURE_CUBE_MAP_POSITIVE_X,
						GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
						GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
						GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
						GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
						GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
					};
					REQUIRE(z < ARRAY_SIZE(cubemap_targets),
						"Invalid layer index provided "
						"in command: %s\n", line);
					tex_target = cubemap_targets[z];

					glFramebufferTexture2D(
						target, GL_COLOR_ATTACHMENT0,
						tex_target, tex_obj, l);

				} else {
					glFramebufferTextureLayer(
						target, GL_COLOR_ATTACHMENT0,
						tex_obj, l, z);
				}

				if (!piglit_check_gl_error(GL_NO_ERROR)) {
					fprintf(stderr, "Error binding texture "
						"attachment for command: %s\n",
						line);
					piglit_report_result(PIGLIT_FAIL);
				}

				w = MAX2(1, get_texture_binding(tex)->width >> l);
				h = MAX2(1, get_texture_binding(tex)->height >> l);

			} else if (sscanf(rest, "tex layered %d", &tex) == 1) {
				glGenFramebuffers(1, &fbo);
				glBindFramebuffer(target, fbo);

				glFramebufferTexture(
					target, GL_COLOR_ATTACHMENT0,
					get_texture_binding(tex)->obj, 0);
				if (!piglit_check_gl_error(GL_NO_ERROR)) {
					fprintf(stderr,
						"glFramebufferTexture error\n");
					piglit_report_result(PIGLIT_FAIL);
				}

				w = get_texture_binding(tex)->width;
				h = get_texture_binding(tex)->height;

			} else if (parse_str(rest, "ms ", &rest)) {
				GLuint rb;
				GLenum format;
				int samples;

				REQUIRE(parse_enum_gl(rest, &format, &rest) &&
					parse_int(rest, &w, &rest) &&
					parse_int(rest, &h, &rest) &&
					parse_int(rest, &samples, &rest),
					"Framebuffer binding command not "
					"understood at: %s\n", rest);

				glGenFramebuffers(1, &fbo);
				glBindFramebuffer(target, fbo);

				glGenRenderbuffers(1, &rb);
				glBindRenderbuffer(GL_RENDERBUFFER, rb);

				glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples,
								 format, w, h);

				glFramebufferRenderbuffer(target,
							  GL_COLOR_ATTACHMENT0,
							  GL_RENDERBUFFER, rb);

				if (!piglit_check_gl_error(GL_NO_ERROR)) {
					fprintf(stderr, "glFramebufferRenderbuffer error\n");
					piglit_report_result(PIGLIT_FAIL);
				}

			} else if (parse_str(rest, "winsys", &rest)) {
				fbo = piglit_winsys_fbo;
				glBindFramebuffer(target, fbo);
				if (!piglit_check_gl_error(GL_NO_ERROR)) {
					fprintf(stderr, "glBindFramebuffer error\n");
					piglit_report_result(PIGLIT_FAIL);
				}

				w = piglit_width;
				h = piglit_height;

			} else {
				fprintf(stderr, "Unknown fb bind subcommand "
					"\"%s\"\n", rest);
				piglit_report_result(PIGLIT_FAIL);
			}

			const GLenum status = glCheckFramebufferStatus(target);
			if (status != GL_FRAMEBUFFER_COMPLETE) {
				fprintf(stderr, "incomplete fbo (status 0x%x)\n",
					status);
				piglit_report_result(PIGLIT_FAIL);
			}

			if (target != GL_READ_FRAMEBUFFER) {
				render_width = w;
				render_height = h;

				/* Delete the previous draw FB in case
				 * it's no longer reachable.
				 */
				if (draw_fbo != 0 &&
				    draw_fbo != piglit_winsys_fbo &&
				    draw_fbo != (target == GL_DRAW_FRAMEBUFFER ?
						 read_fbo : 0))
					glDeleteFramebuffers(1, &draw_fbo);

				draw_fbo = fbo;
			}

			if (target != GL_DRAW_FRAMEBUFFER) {
				read_width = w;
				read_height = h;

				/* Delete the previous read FB in case
				 * it's no longer reachable.
				 */
				if (read_fbo != 0 &&
				    read_fbo != piglit_winsys_fbo &&
				    read_fbo != (target == GL_READ_FRAMEBUFFER ?
						 draw_fbo : 0))
					glDeleteFramebuffers(1, &read_fbo);

				read_fbo = fbo;
			}

		} else if (parse_str(line, "blit ", &rest)) {
			static const struct string_to_enum buffers[] = {
				{ "color", GL_COLOR_BUFFER_BIT },
				{ "depth", GL_DEPTH_BUFFER_BIT },
				{ "stencil", GL_STENCIL_BUFFER_BIT },
				{ NULL }
			};
			static const struct string_to_enum filters[] = {
				{ "linear", GL_LINEAR },
				{ "nearest", GL_NEAREST },
				{ NULL }
			};
			unsigned buffer, filter;

			REQUIRE(parse_enum_tab(buffers, rest, &buffer, &rest) &&
				parse_enum_tab(filters, rest, &filter, &rest),
				"FB blit command not understood at: %s\n",
				rest);

			glBlitFramebuffer(0, 0, read_width, read_height,
					  0, 0, render_width, render_height,
					  buffer, filter);

			if (!piglit_check_gl_error(GL_NO_ERROR)) {
				fprintf(stderr, "glBlitFramebuffer error\n");
				piglit_report_result(PIGLIT_FAIL);
			}

		} else if (parse_str(line, "frustum", &rest)) {
			parse_floats(rest, c, 6, NULL);
			piglit_frustum_projection(false, c[0], c[1], c[2],
						  c[3], c[4], c[5]);
		} else if (parse_str(line, "hint", &rest)) {
			do_hint(rest);
		} else if (sscanf(line,
				  "image texture %d %31s",
				  &tex, s) == 2) {
			const GLenum img_fmt = piglit_get_gl_enum_from_name(s);
			glBindImageTexture(tex, get_texture_binding(tex)->obj, 0,
					   GL_FALSE, 0, GL_READ_WRITE, img_fmt);
		} else if (sscanf(line, "memory barrier %s", s) == 1) {
			glMemoryBarrier(piglit_get_gl_memory_barrier_enum_from_name(s));
		} else if (parse_str(line, "blend barrier", NULL)) {
			glBlendBarrier();
		} else if (sscanf(line, "ortho %f %f %f %f",
				  c + 0, c + 1, c + 2, c + 3) == 4) {
			piglit_gen_ortho_projection(c[0], c[1], c[2], c[3],
						    -1, 1, GL_FALSE);
		} else if (parse_str(line, "ortho", NULL)) {
			piglit_ortho_projection(render_width, render_height,
						GL_FALSE);
		} else if (parse_str(line, "probe rgba ", &rest)) {
			parse_floats(rest, c, 6, NULL);
			if (!piglit_probe_pixel_rgba((int) c[0], (int) c[1],
						    & c[2])) {
				result = PIGLIT_FAIL;
			}
		} else if (parse_str(line, "probe depth ", &rest)) {
			parse_floats(rest, c, 3, NULL);
			if (!piglit_probe_pixel_depth((int) c[0], (int) c[1],
						      c[2])) {
				result = PIGLIT_FAIL;
			}
		} else if (sscanf(line,
				  "probe atomic counter buffer %u %u %s %u",
				  &ux, &uy, s, &uz) == 4) {
			if (!probe_atomic_counter(ux, uy, s, uz, true)) {
				piglit_report_result(PIGLIT_FAIL);
			}
		} else if (sscanf(line,
				  "probe atomic counter %u %s %u",
				  &ux, s, &uy) == 3) {
			if (!probe_atomic_counter(0, ux, s, uy, false)) {
				piglit_report_result(PIGLIT_FAIL);
			}
		} else if (sscanf(line, "probe ssbo uint %d %d %s 0x%x",
				  &x, &y, s, &z) == 4) {
			if (!probe_ssbo_uint(x, y, s, z))
				result = PIGLIT_FAIL;
		} else if (sscanf(line, "probe ssbo uint %d %d %s %d",
				  &x, &y, s, &z) == 4) {
			if (!probe_ssbo_uint(x, y, s, z))
				result = PIGLIT_FAIL;
		} else if (sscanf(line,
				  "relative probe rgba ( %f , %f ) "
				  "( %f , %f , %f , %f )",
				  c + 0, c + 1,
				  c + 2, c + 3, c + 4, c + 5) == 6) {
			x = c[0] * read_width;
			y = c[1] * read_height;
			if (x >= read_width)
				x = read_width - 1;
			if (y >= read_height)
				y = read_height - 1;

			if (!piglit_probe_pixel_rgba(x, y, &c[2])) {
				result = PIGLIT_FAIL;
			}
		} else if (parse_str(line, "probe rgb ", &rest)) {
			parse_floats(rest, c, 5, NULL);
			if (!piglit_probe_pixel_rgb((int) c[0], (int) c[1],
						    & c[2])) {
				result = PIGLIT_FAIL;
			}
		} else if (sscanf(line,
				  "relative probe rgb ( %f , %f ) "
				  "( %f , %f , %f )",
				  c + 0, c + 1,
				  c + 2, c + 3, c + 4) == 5) {
			x = c[0] * read_width;
			y = c[1] * read_height;
			if (x >= read_width)
				x = read_width - 1;
			if (y >= read_height)
				y = read_height - 1;

			if (!piglit_probe_pixel_rgb(x, y, &c[2])) {
				result = PIGLIT_FAIL;
			}
		} else if (sscanf(line, "probe rect rgba "
				  "( %d , %d , %d , %d ) "
				  "( %f , %f , %f , %f )",
				  &x, &y, &w, &h,
				  c + 0, c + 1, c + 2, c + 3) == 8) {
			if (!piglit_probe_rect_rgba(x, y, w, h, c)) {
				result = PIGLIT_FAIL;
			}
		} else if (sscanf(line, "relative probe rect rgb "
				  "( %f , %f , %f , %f ) "
				  "( %f , %f , %f )",
				  c + 0, c + 1, c + 2, c + 3,
				  c + 4, c + 5, c + 6) == 7) {
			x = c[0] * read_width;
			y = c[1] * read_height;
			w = c[2] * read_width;
			h = c[3] * read_height;

			if (!piglit_probe_rect_rgb(x, y, w, h, &c[4])) {
				result = PIGLIT_FAIL;
			}
		} else if (sscanf(line, "relative probe rect rgba "
				  "( %f , %f , %f , %f ) "
				  "( %f , %f , %f , %f )",
				  c + 0, c + 1, c + 2, c + 3,
				  c + 4, c + 5, c + 6, c + 7) == 8) {
			x = c[0] * read_width;
			y = c[1] * read_height;
			w = c[2] * read_width;
			h = c[3] * read_height;

			if (!piglit_probe_rect_rgba(x, y, w, h, &c[4])) {
				result = PIGLIT_FAIL;
			}
		} else if (sscanf(line, "relative probe rect rgba int "
				  "( %f , %f , %f , %f ) "
				  "( %d , %d , %d , %d )",
				  c + 0, c + 1, c + 2, c + 3,
				  &x, &y, &z, &w) == 8) {
			const int expected[] = { x, y, z, w };
			if (!piglit_probe_rect_rgba_int(c[0] * read_width,
							c[1] * read_height,
							c[2] * read_width,
							c[3] * read_height,
							expected))
				result = PIGLIT_FAIL;

		} else if (parse_str(line, "polygon mode ", &rest)) {
			GLenum face, mode;

			REQUIRE(parse_enum_gl(rest, &face, &rest) &&
				parse_enum_gl(rest, &mode, &rest),
				"Polygon mode command not understood at %s\n",
				rest);

			glPolygonMode(face, mode);

			if (!piglit_check_gl_error(GL_NO_ERROR)) {
				fprintf(stderr, "glPolygonMode error\n");
				piglit_report_result(PIGLIT_FAIL);
			}
		} else if (parse_str(line, "probe all rgba ", &rest)) {
			parse_floats(rest, c, 4, NULL);
			if (result != PIGLIT_FAIL &&
			    !piglit_probe_rect_rgba(0, 0, read_width,
						    read_height, c))
				result = PIGLIT_FAIL;
		} else if (parse_str(line, "probe warn all rgba ", &rest)) {
			parse_floats(rest, c, 4, NULL);
			if (result == PIGLIT_PASS &&
			    !piglit_probe_rect_rgba(0, 0, read_width,
						    read_height, c))
				result = PIGLIT_WARN;
		} else if (parse_str(line, "probe all rgb", &rest)) {
			parse_floats(rest, c, 3, NULL);
			if (result != PIGLIT_FAIL &&
			    !piglit_probe_rect_rgb(0, 0, read_width,
						   read_height, c))
				result = PIGLIT_FAIL;
		} else if (parse_str(line, "tolerance", &rest)) {
			parse_floats(rest, piglit_tolerance, 4, NULL);
		} else if (parse_str(line, "shade model smooth", NULL)) {
			glShadeModel(GL_SMOOTH);
		} else if (parse_str(line, "shade model flat", NULL)) {
			glShadeModel(GL_FLAT);
		} else if (sscanf(line, "ssbo %d %d", &x, &y) == 2) {
			GLuint *ssbo_init = calloc(y, 1);
			glGenBuffers(1, &ssbo[x]);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, x, ssbo[x]);
			glBufferData(GL_SHADER_STORAGE_BUFFER, y,
				     ssbo_init, GL_DYNAMIC_DRAW);
			free(ssbo_init);
		} else if (sscanf(line, "ssbo %d subdata float %d %f", &x, &y, &c[0]) == 3) {
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo[x]);
			glBufferSubData(GL_SHADER_STORAGE_BUFFER, y, 4, &c[0]);
		} else if (sscanf(line, "texture rgbw %d ( %d", &tex, &w) == 2) {
			GLenum int_fmt = GL_RGBA;
			int num_scanned =
				sscanf(line,
				       "texture rgbw %d ( %d , %d ) %31s",
				       &tex, &w, &h, s);
			if (num_scanned < 3) {
				fprintf(stderr,
					"invalid texture rgbw command!\n");
				piglit_report_result(PIGLIT_FAIL);
			}

			if (num_scanned >= 4) {
				int_fmt = piglit_get_gl_enum_from_name(s);
			}

			glActiveTexture(GL_TEXTURE0 + tex);
			int handle = piglit_rgbw_texture(
				int_fmt, w, h, GL_FALSE, GL_FALSE,
				(piglit_is_gles() ? GL_UNSIGNED_BYTE :
				 GL_UNSIGNED_NORMALIZED));
			set_texture_binding(tex, handle, w, h, 1);

			if (!piglit_is_core_profile &&
			    !(piglit_is_gles() && piglit_get_gl_version() >= 20))
				glEnable(GL_TEXTURE_2D);

		} else if (sscanf(line, "resident texture %d", &tex) == 1) {
			GLuint64 handle;

			glBindTexture(GL_TEXTURE_2D, 0);

			handle = glGetTextureHandleARB(get_texture_binding(tex)->obj);
			glMakeTextureHandleResidentARB(handle);

			set_resident_handle(tex, handle, true);

			if (!piglit_check_gl_error(GL_NO_ERROR)) {
				fprintf(stderr,
					"glMakeTextureHandleResidentARB error\n");
				piglit_report_result(PIGLIT_FAIL);
			}
		} else if (sscanf(line, "resident image texture %d %31s",
				  &tex, s) == 2) {
			const GLenum img_fmt = piglit_get_gl_enum_from_name(s);
			GLuint64 handle;

			glBindTexture(GL_TEXTURE_2D, 0);

			handle = glGetImageHandleARB(get_texture_binding(tex)->obj,
						     0, GL_FALSE, 0, img_fmt);
			glMakeImageHandleResidentARB(handle, GL_READ_WRITE);

			set_resident_handle(tex, handle, false);

			if (!piglit_check_gl_error(GL_NO_ERROR)) {
				fprintf(stderr,
					"glMakeImageHandleResidentARB error\n");
				piglit_report_result(PIGLIT_FAIL);
			}
		} else if (parse_str(line, "texture integer ", &rest)) {
			GLenum int_fmt;
			int b, a;
			int num_scanned =
				sscanf(rest, "%d ( %d , %d ) ( %d, %d ) %31s",
				       &tex, &w, &h, &b, &a, s);
			if (num_scanned < 6) {
				fprintf(stderr,
					"invalid texture integer command!\n");
				piglit_report_result(PIGLIT_FAIL);
			}

			int_fmt = piglit_get_gl_enum_from_name(s);

			glActiveTexture(GL_TEXTURE0 + tex);
			const GLuint handle =
				piglit_integer_texture(int_fmt, w, h, b, a);
			set_texture_binding(tex, handle, w, h, 1);

		} else if (sscanf(line, "texture miptree %d", &tex) == 1) {
			glActiveTexture(GL_TEXTURE0 + tex);
			const GLuint handle = piglit_miptree_texture();
			set_texture_binding(tex, handle, 8, 8, 1);

			if (!piglit_is_core_profile &&
			    !(piglit_is_gles() && piglit_get_gl_version() >= 20))
				glEnable(GL_TEXTURE_2D);
		} else if (sscanf(line,
				  "texture checkerboard %d %d ( %d , %d ) "
				  "( %f , %f , %f , %f ) "
				  "( %f , %f , %f , %f )",
				  &tex, &level, &w, &h,
				  c + 0, c + 1, c + 2, c + 3,
				  c + 4, c + 5, c + 6, c + 7) == 12) {
			glActiveTexture(GL_TEXTURE0 + tex);
			const GLuint handle = piglit_checkerboard_texture(
				0, level, w, h, w / 2, h / 2, c + 0, c + 4);
			set_texture_binding(tex, handle, w, h, 1);

			if (!piglit_is_core_profile &&
			    !(piglit_is_gles() && piglit_get_gl_version() >= 20))
				glEnable(GL_TEXTURE_2D);
		} else if (sscanf(line,
				  "texture quads %d %d ( %d , %d ) ( %d , %d ) "
				  "( %f , %f , %f , %f ) "
				  "( %f , %f , %f , %f ) "
				  "( %f , %f , %f , %f ) "
				  "( %f , %f , %f , %f )",
				  &tex, &level, &w, &h, &x, &y,
				  c + 0, c + 1, c + 2, c + 3,
				  c + 4, c + 5, c + 6, c + 7,
				  c + 8, c + 9, c + 10, c + 11,
				  c + 12, c + 13, c + 14, c + 15) == 22) {
			glActiveTexture(GL_TEXTURE0 + tex);
			const GLuint handle = piglit_quads_texture(
				0, level, w, h, x, y, c + 0, c + 4, c + 8, c + 12);
			set_texture_binding(tex, handle, w, h, 1);

			if (!piglit_is_core_profile &&
			    !(piglit_is_gles() && piglit_get_gl_version() >= 20))
				glEnable(GL_TEXTURE_2D);
		} else if (sscanf(line,
				  "texture junk 2DArray %d ( %d , %d , %d )",
				  &tex, &w, &h, &l) == 4) {
			GLuint texobj;
			glActiveTexture(GL_TEXTURE0 + tex);
			glGenTextures(1, &texobj);
			glBindTexture(GL_TEXTURE_2D_ARRAY, texobj);
			glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA,
				     w, h, l, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
			set_texture_binding(tex, texobj, w, h, l);

		} else if (parse_str(line, "texture storage ", &rest)) {
			GLenum target, format;
			GLuint tex_obj;
			int d = h = w = 1;

			REQUIRE(parse_int(rest, &tex, &rest) &&
				parse_tex_target(rest, &target, &rest) &&
				parse_enum_gl(rest, &format, &rest) &&
				parse_str(rest, "(", &rest) &&
				parse_int(rest, &l, &rest) &&
				parse_int(rest, &w, &rest),
				"Texture storage command not understood "
				"at: %s\n", rest);

			glActiveTexture(GL_TEXTURE0 + tex);
			glGenTextures(1, &tex_obj);
			glBindTexture(target, tex_obj);

			if (!parse_int(rest, &h, &rest))
				glTexStorage1D(target, l, format, w);
			else if (!parse_int(rest, &d, &rest))
				glTexStorage2D(target, l, format, w, h);
			else
				glTexStorage3D(target, l, format, w, h, d);

			if (!piglit_check_gl_error(GL_NO_ERROR)) {
				fprintf(stderr, "glTexStorage error\n");
				piglit_report_result(PIGLIT_FAIL);
			}

			if (target == GL_TEXTURE_1D_ARRAY)
				set_texture_binding(tex, tex_obj, w, 1, h);
			else
				set_texture_binding(tex, tex_obj, w, h, d);

#ifdef PIGLIT_USE_OPENGL
		} else if (sscanf(line,
				  "texture rgbw 1D %d",
				  &tex) == 1) {
			glActiveTexture(GL_TEXTURE0 + tex);
			const GLuint handle = piglit_rgbw_texture_1d();
			set_texture_binding(tex, handle, 4, 1, 1);
#endif

		} else if (sscanf(line,
				  "texture rgbw 3D %d",
				  &tex) == 1) {
			glActiveTexture(GL_TEXTURE0 + tex);
			const GLuint handle = piglit_rgbw_texture_3d();
			set_texture_binding(tex, handle, 2, 2, 2);

		} else if (sscanf(line,
				  "texture rgbw 2DArray %d ( %d , %d , %d )",
				  &tex, &w, &h, &l) == 4) {
			glActiveTexture(GL_TEXTURE0 + tex);
			const GLuint handle = piglit_array_texture(
				GL_TEXTURE_2D_ARRAY, GL_RGBA, w, h, l, GL_FALSE);
			set_texture_binding(tex, handle, w, h, l);

		} else if (sscanf(line,
				  "texture rgbw 1DArray %d ( %d , %d )",
				  &tex, &w, &l) == 3) {
			glActiveTexture(GL_TEXTURE0 + tex);
                        h = 1;
			const GLuint handle = piglit_array_texture(
				GL_TEXTURE_1D_ARRAY, GL_RGBA, w, h, l, GL_FALSE);
			set_texture_binding(tex, handle, w, 1, l);

		} else if (sscanf(line,
				  "texture shadow2D %d ( %d , %d )",
				  &tex, &w, &h) == 3) {
			glActiveTexture(GL_TEXTURE0 + tex);
			const GLuint handle = piglit_depth_texture(
				GL_TEXTURE_2D, GL_DEPTH_COMPONENT,
				w, h, 1, GL_FALSE);
			glTexParameteri(GL_TEXTURE_2D,
					GL_TEXTURE_COMPARE_MODE,
					GL_COMPARE_R_TO_TEXTURE);
			glTexParameteri(GL_TEXTURE_2D,
					GL_TEXTURE_COMPARE_FUNC,
					GL_GREATER);
			set_texture_binding(tex, handle, w, h, 1);

			if (!piglit_is_core_profile &&
			    !(piglit_is_gles() && piglit_get_gl_version() >= 20))
				glEnable(GL_TEXTURE_2D);
		} else if (sscanf(line,
				  "texture shadowRect %d ( %d , %d )",
				  &tex, &w, &h) == 3) {
			glActiveTexture(GL_TEXTURE0 + tex);
			const GLuint handle = piglit_depth_texture(
				GL_TEXTURE_RECTANGLE, GL_DEPTH_COMPONENT,
				w, h, 1, GL_FALSE);
			glTexParameteri(GL_TEXTURE_RECTANGLE,
					GL_TEXTURE_COMPARE_MODE,
					GL_COMPARE_R_TO_TEXTURE);
			glTexParameteri(GL_TEXTURE_RECTANGLE,
					GL_TEXTURE_COMPARE_FUNC,
					GL_GREATER);
			set_texture_binding(tex, handle, w, h, 1);
		} else if (sscanf(line,
				  "texture shadow1D %d ( %d )",
				  &tex, &w) == 2) {
			glActiveTexture(GL_TEXTURE0 + tex);
			const GLuint handle = piglit_depth_texture(
				GL_TEXTURE_1D, GL_DEPTH_COMPONENT,
				w, 1, 1, GL_FALSE);
			glTexParameteri(GL_TEXTURE_1D,
					GL_TEXTURE_COMPARE_MODE,
					GL_COMPARE_R_TO_TEXTURE);
			glTexParameteri(GL_TEXTURE_1D,
					GL_TEXTURE_COMPARE_FUNC,
					GL_GREATER);
			set_texture_binding(tex, handle, w, 1, 1);
		} else if (sscanf(line,
				  "texture shadow1DArray %d ( %d , %d )",
				  &tex, &w, &l) == 3) {
			glActiveTexture(GL_TEXTURE0 + tex);
			const GLuint handle = piglit_depth_texture(
				GL_TEXTURE_1D_ARRAY, GL_DEPTH_COMPONENT,
				w, l, 1, GL_FALSE);
			glTexParameteri(GL_TEXTURE_1D_ARRAY,
					GL_TEXTURE_COMPARE_MODE,
					GL_COMPARE_R_TO_TEXTURE);
			glTexParameteri(GL_TEXTURE_1D_ARRAY,
					GL_TEXTURE_COMPARE_FUNC,
					GL_GREATER);
			set_texture_binding(tex, handle, w, 1, l);
		} else if (sscanf(line,
				  "texture shadow2DArray %d ( %d , %d , %d )",
				  &tex, &w, &h, &l) == 4) {
			glActiveTexture(GL_TEXTURE0 + tex);
			const GLuint handle = piglit_depth_texture(
				GL_TEXTURE_2D_ARRAY, GL_DEPTH_COMPONENT,
				w, h, l, GL_FALSE);
			glTexParameteri(GL_TEXTURE_2D_ARRAY,
					GL_TEXTURE_COMPARE_MODE,
					GL_COMPARE_R_TO_TEXTURE);
			glTexParameteri(GL_TEXTURE_2D_ARRAY,
					GL_TEXTURE_COMPARE_FUNC,
					GL_GREATER);
			set_texture_binding(tex, handle, w, h, l);
		} else if (sscanf(line, "texcoord %d ( %f , %f , %f , %f )",
		                  &x, c + 0, c + 1, c + 2, c + 3) == 5) {
			glMultiTexCoord4fv(GL_TEXTURE0 + x, c);
		} else if (parse_str(line, "texparameter ", &rest)) {
			handle_texparameter(rest);
		} else if (parse_str(line, "uniform ", &rest)) {
			result = program_must_be_in_use();
			set_uniform(rest, ubo_array_index);
		} else if (parse_str(line, "subuniform ", &rest)) {
			result = program_must_be_in_use();
			check_shader_subroutine_support();
			set_subroutine_uniform(rest);
		} else if (parse_str(line, "parameter ", &rest)) {
			set_parameter(rest);
		} else if (parse_str(line, "patch parameter ", &rest)) {
			set_patch_parameter(rest);
		} else if (parse_str(line, "provoking vertex ", &rest)) {
			set_provoking_vertex(rest);
		} else if (parse_str(line, "link error", &rest)) {
			link_error_expected = true;
			if (link_ok) {
				printf("shader link error expected, but it was successful!\n");
				piglit_report_result(PIGLIT_FAIL);
			} else {
				fprintf(stderr, "Failed to link:\n%s\n", prog_err_info);
			}
		} else if (parse_str(line, "link success", &rest)) {
			result = program_must_be_in_use();
		} else if (parse_str(line, "ubo array index ", &rest)) {
			parse_ints(rest, &ubo_array_index, 1, NULL);
		} else if (parse_str(line, "active uniform ", &rest)) {
			active_uniform(rest);
		} else if (parse_str(line, "verify program_interface_query ", &rest)) {
			active_program_interface(rest);
		} else if (parse_str(line, "vertex attrib ", &rest)) {
			set_vertex_attrib(rest);
		} else if ((line[0] != '\n') && (line[0] != '\0')
			   && (line[0] != '#')) {
			printf("unknown command \"%s\"\n", line);
			piglit_report_result(PIGLIT_FAIL);
		}

		free((void*) line);

		if (result != PIGLIT_PASS) {
			printf("Test failure on line %u\n", line_num);
			full_result = result;
		}

		line_num++;
	}

	if (!link_ok && !link_error_expected) {
		full_result = program_must_be_in_use();
	}

	piglit_present_results();

	if (piglit_automatic) {
		unsigned i;

		/* Free our resources, useful for valgrinding. */
	        free_subroutine_uniforms();

		for (i = 0; i < ARRAY_SIZE(resident_handles); i++)
			clear_resident_handle(i);

		for (i = 0; i < ARRAY_SIZE(texture_bindings); i++)
			clear_texture_binding(i);

		if (prog != 0) {
			glDeleteProgram(prog);
			glUseProgram(0);
		} else {
			if (!sso_in_use)
				glDeleteProgramsARB(1, &prog);
		}

		if (pipeline != 0)
			glDeleteProgramPipelines(1, &pipeline);
	}

	return full_result;
}

static enum piglit_result
init_test(const char *file)
{
	enum piglit_result result;

	result = process_test_script(file);
	if (result != PIGLIT_PASS)
		return result;

	result = link_and_use_shaders();
	if (result != PIGLIT_PASS)
		return result;

	if (sso_in_use)
		glBindProgramPipeline(pipeline);

	if (link_ok && vertex_data_start != NULL) {
		result = program_must_be_in_use();
		if (result != PIGLIT_PASS)
			return result;

		bind_vao_if_supported();

		num_vbo_rows = setup_vbo_from_text(prog, vertex_data_start,
						   vertex_data_end);
		vbo_present = true;
	}
	setup_ubos();
	return PIGLIT_PASS;
}

static void
recreate_gl_context(char *exec_arg, int param_argc, char **param_argv)
{
	int argc = param_argc + 4;
	char **argv = malloc(sizeof(char*) * argc);

	if (!argv) {
		fprintf(stderr, "%s: malloc failed.\n", __func__);
		piglit_report_result(PIGLIT_FAIL);
	}

	argv[0] = exec_arg;
	memcpy(&argv[1], param_argv, param_argc * sizeof(char*));
	argv[argc-3] = "-auto";
	argv[argc-2] = "-fbo";
	argv[argc-1] = "-report-subtests";

	if (gl_fw->destroy)
		gl_fw->destroy(gl_fw);
	gl_fw = NULL;

	exit(main(argc, argv));
}

static bool
validate_current_gl_context(const char *filename)
{
	struct piglit_gl_test_config config = { 0 };

	config.window_width = DEFAULT_WINDOW_WIDTH;
	config.window_height = DEFAULT_WINDOW_HEIGHT;

	get_required_config(filename, spirv_replaces_glsl, &config);

	if (!current_config.supports_gl_compat_version !=
	    !config.supports_gl_compat_version)
		return false;

	if (!current_config.supports_gl_core_version !=
	    !config.supports_gl_core_version)
		return false;

	if (!current_config.supports_gl_es_version !=
	    !config.supports_gl_es_version)
		return false;

	if (current_config.window_width != config.window_width ||
	    current_config.window_height != config.window_height)
		return false;

	if (!(current_config.window_visual & PIGLIT_GL_VISUAL_DEPTH) &&
	    config.window_visual & PIGLIT_GL_VISUAL_DEPTH)
		return false;

	return true;
}

void
piglit_init(int argc, char **argv)
{
	int major;
	int minor;
	bool core = piglit_is_core_profile;
	bool es;
	enum piglit_result result;
	float default_piglit_tolerance[4];

	report_subtests = piglit_strip_arg(&argc, argv, "-report-subtests");
	force_glsl =  piglit_strip_arg(&argc, argv, "-glsl");
	if (argc < 2) {
		printf("usage: shader_runner <test.shader_test>\n");
		exit(1);
	}

	memcpy(default_piglit_tolerance, piglit_tolerance,
	       sizeof(piglit_tolerance));

	piglit_require_GLSL();

	version_init(&gl_version, VERSION_GL,
		     core, !core,
	             piglit_is_gles(),
	             piglit_get_gl_version());
	piglit_get_glsl_version(&es, &major, &minor);
	version_init(&glsl_version, VERSION_GLSL, core, !core, es,
	             (major * 100) + minor);

#ifdef PIGLIT_USE_OPENGL
	if (piglit_get_gl_version() >= 32)
		glGetIntegerv(GL_MAX_VERTEX_OUTPUT_COMPONENTS,
			      &gl_max_vertex_output_components);
	if (piglit_get_gl_version() >= 20 ||
	    piglit_is_extension_supported("GL_ARB_fragment_shader"))
		glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS,
			      &gl_max_fragment_uniform_components);
	if (piglit_get_gl_version() >= 20 ||
	    piglit_is_extension_supported("GL_ARB_vertex_shader"))
		glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS,
			      &gl_max_vertex_uniform_components);
	if (piglit_get_gl_version() >= 20 ||
	    piglit_is_extension_supported("GL_ARB_vertex_shader") ||
	    piglit_is_extension_supported("GL_ARB_geometry_shader4") ||
	    piglit_is_extension_supported("GL_EXT_geometry_shader4"))
		glGetIntegerv(GL_MAX_VARYING_COMPONENTS,
			      &gl_max_varying_components);
#else
	glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_VECTORS,
		      &gl_max_fragment_uniform_components);
	glGetIntegerv(GL_MAX_VERTEX_UNIFORM_VECTORS,
		      &gl_max_vertex_uniform_components);
	glGetIntegerv(GL_MAX_VARYING_VECTORS,
		      &gl_max_varying_components);
	gl_max_fragment_uniform_components *= 4;
	gl_max_vertex_uniform_components *= 4;
	gl_max_varying_components *= 4;
#endif
	glGetIntegerv(GL_MAX_CLIP_PLANES, &gl_max_clip_planes);

	if (gl_version.num >= 20 ||
	    piglit_is_extension_supported("GL_ARB_vertex_shader"))
		glGetIntegerv(GL_MAX_VERTEX_ATTRIBS,
			      &gl_max_vertex_attribs);
	else
		gl_max_vertex_attribs = 16;

	read_width = render_width = piglit_width;
	read_height = render_height = piglit_height;

	/* Automatic mode can run multiple tests per session. */
	if (report_subtests) {
		char testname[4096], *ext;
		int i, j;

		for (i = 1; i < argc; i++) {
			const char *hit, *filename = argv[i];

			memcpy(piglit_tolerance, default_piglit_tolerance,
			       sizeof(piglit_tolerance));

			/* Re-initialize the GL context if a different GL config is required. */
			if (!validate_current_gl_context(filename))
				recreate_gl_context(argv[0], argc - i, argv + i);

			/* Clear global variables to defaults. */
			test_start = NULL;
			assert(num_vertex_shaders == 0);
			assert(num_tess_ctrl_shaders == 0);
			assert(num_tess_eval_shaders == 0);
			assert(num_geometry_shaders == 0);
			assert(num_fragment_shaders == 0);
			assert(num_compute_shaders == 0);
			assert(num_uniform_blocks == 0);
			assert(uniform_block_bos == NULL);
			geometry_layout_input_type = GL_TRIANGLES;
			geometry_layout_output_type = GL_TRIANGLE_STRIP;
			geometry_layout_vertices_out = 0;
			memset(atomics_bos, 0, sizeof(atomics_bos));
			memset(ssbo, 0, sizeof(ssbo));
			for (j = 0; j < ARRAY_SIZE(subuniform_locations); j++)
				assert(subuniform_locations[j] == NULL);
			memset(num_subuniform_locations, 0, sizeof(num_subuniform_locations));
			shader_string = NULL;
			shader_string_size = 0;
			vertex_data_start = NULL;
			vertex_data_end = NULL;
			prog = 0;
			sso_vertex_prog = 0;
			sso_tess_control_prog = 0;
			sso_tess_eval_prog = 0;
			sso_geometry_prog = 0;
			sso_fragment_prog = 0;
			sso_compute_prog = 0;
			num_vbo_rows = 0;
			vbo_present = false;
			link_ok = false;
			prog_in_use = false;
			sso_in_use = false;
			prog_err_info = NULL;
			vao = 0;

			/* Clear GL states to defaults. */
			glClearColor(0, 0, 0, 0);
# if PIGLIT_USE_OPENGL
			glClearDepth(1);
# else
			glClearDepthf(1.0);
# endif
			glBindFramebuffer(GL_FRAMEBUFFER, piglit_winsys_fbo);
			glActiveTexture(GL_TEXTURE0);
			glUseProgram(0);
			glDisable(GL_DEPTH_TEST);
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			for (int k = 0; k < gl_max_clip_planes; k++) {
				glDisable(GL_CLIP_PLANE0 + k);
			}

			if (!(es) && (gl_version.num >= 20 ||
			     piglit_is_extension_supported("GL_ARB_vertex_program")))
				glDisable(GL_PROGRAM_POINT_SIZE);

			for (int i = 0; i < 16; i++)
				glDisableVertexAttribArray(i);

			if (!piglit_is_core_profile && !es) {
				glMatrixMode(GL_PROJECTION);
				glLoadIdentity();
				glMatrixMode(GL_MODELVIEW);
				glLoadIdentity();
				glShadeModel(GL_SMOOTH);
				glDisable(GL_VERTEX_PROGRAM_TWO_SIDE);
			}

			if (piglit_is_extension_supported("GL_ARB_vertex_program")) {
				glDisable(GL_VERTEX_PROGRAM_ARB);
				glBindProgramARB(GL_VERTEX_PROGRAM_ARB, 0);
			}
			if (piglit_is_extension_supported("GL_ARB_fragment_program")) {
				glDisable(GL_FRAGMENT_PROGRAM_ARB);
				glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, 0);
			}
			if (piglit_is_extension_supported("GL_ARB_separate_shader_objects")) {
				if (!pipeline)
					glGenProgramPipelines(1, &pipeline);
				glBindProgramPipeline(0);
			}

			if (piglit_is_extension_supported("GL_EXT_provoking_vertex"))
				glProvokingVertexEXT(GL_LAST_VERTEX_CONVENTION_EXT);

# if PIGLIT_USE_OPENGL
			if (gl_version.num >= 40 ||
			    piglit_is_extension_supported("GL_ARB_tessellation_shader")) {
				static float ones[] = {1, 1, 1, 1};
				glPatchParameteri(GL_PATCH_VERTICES, 3);
				glPatchParameterfv(GL_PATCH_DEFAULT_OUTER_LEVEL, ones);
				glPatchParameterfv(GL_PATCH_DEFAULT_INNER_LEVEL, ones);
			}
# else
			/* Ideally one would use the following code:
			 *
			 * if (gl_version.num >= 32) {
			 *         glPatchParameteri(GL_PATCH_VERTICES, 3);
			 * }
			 *
			 * however, that doesn't work with mesa because those
			 * symbols apparently need to be exported, but that
			 * breaks non-gles builds.
			 *
			 * It seems rather unlikely that an implementation
			 * would have GLES 3.2 support but not
			 * OES_tessellation_shader.
			 */
			if (piglit_is_extension_supported("GL_OES_tessellation_shader")) {
				glPatchParameteriOES(GL_PATCH_VERTICES_OES, 3);
			}
# endif

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			/* Strip the file path. */
			hit = strrchr(filename, PIGLIT_PATH_SEP);
			if (hit)
				strcpy(testname, hit+1);
			else
				strcpy(testname, filename);

			/* Strip the file extension. */
			ext = strstr(testname, ".shader_test");
			if (ext && !ext[12])
				*ext = 0;

			/* Print the name before we start the test, that way if
			 * the test fails we can still resume and know which
			 * test failed */
			printf("PIGLIT TEST: %i - %s\n", test_num, testname);
			fprintf(stderr, "PIGLIT TEST: %i - %s\n", test_num, testname);
			test_num++;

			/* Run the test. */
			result = init_test(filename);

			if (result == PIGLIT_PASS) {
				result = piglit_display();
			}
			/* Use subtest when running with more than one test,
			 * but use regular test result when running with just
			 * one.  This allows the standard process-at-a-time
			 * mode to keep working.
			 */
			if (report_subtests) {
				piglit_report_subtest_result(
					result, "%s", testname);
			} else {
				piglit_report_result(result);
			}

			/* destroy GL objects? */
			teardown_ubos();
			teardown_atomics();
			teardown_fbos();
		}
		exit(0);
	}

	result = init_test(argv[1]);
	if (result != PIGLIT_PASS)
		piglit_report_result(result);
}
