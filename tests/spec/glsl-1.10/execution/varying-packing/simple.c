/*
 * Copyright © 2012 Intel Corporation
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
 * \file simple.c
 *
 * This file checks that simple cases of varying packing work
 * correctly.  Specifically, it tests that for each basic type allowed
 * in varyings, it is possible to create a shader with the maximum
 * possible number of that type of varying (determined by the
 * implementation's reported value of GL_MAX_VARYING_FLOATS).  If the
 * size of the basic type being tested does not evenly divide
 * GL_MAX_VARYING_FLOATS, the remaining varyings are taken up by
 * individual floats.
 *
 * The test may be run in two modes: "array" mode, in which the test
 * uses a single varying whose type is an array (e.g. mat3[7]), and
 * "separate" mode, in which the test uses separate individual
 * varyings of the given type.
 *
 * The test operates by first querying the implementation's value of
 * GL_MAX_VARYING_FLOATS, then creating a vertex and fragment shader
 * that use up all possible varying components.  The vertex shader
 * fills the varying components with consecutive integer values (where
 * the starting value is determined by a uniform), and the fragment
 * shader checks that all of the varying components were received
 * correctly.  The shaders are compiled and run, to ensure that the
 * implementation not only claims to be able to pack the varyings, but
 * actually packs them correctly too.
 *
 * For example, on an implementation where GL_MAX_VARYING_FLOATS is
 * 64, when testing the mat3 type in "array" mode, the vertex shader
 * looks like this:
 *
 *   #version 110
 *   uniform int i;
 *   varying mat3 var0[7];
 *   varying float var1;
 *
 *   void main()
 *   {
 *     gl_Position = gl_Vertex;
 *     var0[0][0][0] = float(i + 0);
 *     var0[0][0][1] = float(i + 1);
 *     var0[0][0][2] = float(i + 2);
 *     var0[0][1][0] = float(i + 3);
 *     ...
 *     var0[6][2][1] = float(i + 61);
 *     var0[6][2][2] = float(i + 62);
 *     var1 = float(i + 63);
 *   }
 *
 * And the fragment shader looks like this:
 *
 *   #version 110
 *   uniform int i;
 *   varying mat3 var0[7];
 *   varying float var1;
 *   
 *   void main()
 *   {
 *     bool failed = false;
 *     if (var0[0][0][0] != float(i + 0))
 *       failed = true;
 *     if (var0[0][0][1] != float(i + 1))
 *       failed = true;
 *     ...
 *     if (var0[6][2][2] != float(i + 62))
 *       failed = true;
 *     if (var1 != float(i + 63))
 *       failed = true;
 *     if (failed)
 *       gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
 *     else
 *       gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);
 *   }
 */
#include "piglit-util-gl.h"

static void
parse_args(int argc, char *argv[], struct piglit_gl_test_config *config);

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;
	parse_args(argc, argv, &config);

PIGLIT_GL_TEST_CONFIG_END

static GLuint prog;
static GLint i_location;

enum base_type
{
	BASE_TYPE_FLOAT,
	BASE_TYPE_UINT,
	BASE_TYPE_INT,
	BASE_TYPE_DOUBLE,
};

static const char *
get_base_type_name(enum base_type t)
{
	switch (t) {
	case BASE_TYPE_FLOAT: return "float";
	case BASE_TYPE_UINT:  return "uint";
	case BASE_TYPE_INT:   return "int";
	case BASE_TYPE_DOUBLE:return "double";
	default:              return "???";
	}
}

struct type_desc
{
	const char *name;
	enum base_type base;
	unsigned num_cols;
	unsigned num_rows;
	unsigned glsl_version_required;
};

struct type_desc int_type     = { "int",     BASE_TYPE_INT,    1, 1, 130 };
struct type_desc uint_type    = { "uint",    BASE_TYPE_UINT,   1, 1, 130 };
struct type_desc float_type   = { "float",   BASE_TYPE_FLOAT,  1, 1, 110 };
struct type_desc double_type  = { "double",  BASE_TYPE_DOUBLE, 1, 1, 150 };
struct type_desc vec2_type    = { "vec2",    BASE_TYPE_FLOAT,  1, 2, 110 };
struct type_desc vec3_type    = { "vec3",    BASE_TYPE_FLOAT,  1, 3, 110 };
struct type_desc vec4_type    = { "vec4",    BASE_TYPE_FLOAT,  1, 4, 110 };
struct type_desc ivec2_type   = { "ivec2",   BASE_TYPE_INT,    1, 2, 130 };
struct type_desc ivec3_type   = { "ivec3",   BASE_TYPE_INT,    1, 3, 130 };
struct type_desc ivec4_type   = { "ivec4",   BASE_TYPE_INT,    1, 4, 130 };
struct type_desc uvec2_type   = { "uvec2",   BASE_TYPE_UINT,   1, 2, 130 };
struct type_desc uvec3_type   = { "uvec3",   BASE_TYPE_UINT,   1, 3, 130 };
struct type_desc uvec4_type   = { "uvec4",   BASE_TYPE_UINT,   1, 4, 130 };
struct type_desc dvec2_type   = { "dvec2",   BASE_TYPE_DOUBLE, 1, 2, 150 };
struct type_desc dvec3_type   = { "dvec3",   BASE_TYPE_DOUBLE, 1, 3, 150 };
struct type_desc dvec4_type   = { "dvec4",   BASE_TYPE_DOUBLE, 1, 4, 150 };
struct type_desc mat2_type    = { "mat2",    BASE_TYPE_FLOAT,  2, 2, 110 };
struct type_desc mat3_type    = { "mat3",    BASE_TYPE_FLOAT,  3, 3, 110 };
struct type_desc mat4_type    = { "mat4",    BASE_TYPE_FLOAT,  4, 4, 110 };
struct type_desc mat2x3_type  = { "mat2x3",  BASE_TYPE_FLOAT,  2, 3, 120 };
struct type_desc mat2x4_type  = { "mat2x4",  BASE_TYPE_FLOAT,  2, 4, 120 };
struct type_desc mat3x2_type  = { "mat3x2",  BASE_TYPE_FLOAT,  3, 2, 120 };
struct type_desc mat3x4_type  = { "mat3x4",  BASE_TYPE_FLOAT,  3, 4, 120 };
struct type_desc mat4x2_type  = { "mat4x2",  BASE_TYPE_FLOAT,  4, 2, 120 };
struct type_desc mat4x3_type  = { "mat4x3",  BASE_TYPE_FLOAT,  4, 3, 120 };
struct type_desc dmat2_type   = { "dmat2",   BASE_TYPE_DOUBLE, 2, 2, 150 };
struct type_desc dmat3_type   = { "dmat3",   BASE_TYPE_DOUBLE, 3, 3, 150 };
struct type_desc dmat4_type   = { "dmat4",   BASE_TYPE_DOUBLE, 4, 4, 150 };
struct type_desc dmat2x3_type = { "dmat2x3", BASE_TYPE_DOUBLE, 2, 3, 150 };
struct type_desc dmat2x4_type = { "dmat2x4", BASE_TYPE_DOUBLE, 2, 4, 150 };
struct type_desc dmat3x2_type = { "dmat3x2", BASE_TYPE_DOUBLE, 3, 2, 150 };
struct type_desc dmat3x4_type = { "dmat3x4", BASE_TYPE_DOUBLE, 3, 4, 150 };
struct type_desc dmat4x2_type = { "dmat4x2", BASE_TYPE_DOUBLE, 4, 2, 150 };
struct type_desc dmat4x3_type = { "dmat4x3", BASE_TYPE_DOUBLE, 4, 3, 150 };

const struct type_desc *all_types[] = {
	&int_type,
	&uint_type,
	&float_type,
	&double_type,
	&vec2_type,
	&vec3_type,
	&vec4_type,
	&ivec2_type,
	&ivec3_type,
	&ivec4_type,
	&uvec2_type,
	&uvec3_type,
	&uvec4_type,
	&dvec2_type,
	&dvec3_type,
	&dvec4_type,
	&mat2_type,
	&mat3_type,
	&mat4_type,
	&mat2x3_type,
	&mat2x4_type,
	&mat3x2_type,
	&mat3x4_type,
	&mat4x2_type,
	&mat4x3_type,
	&dmat2_type,
	&dmat3_type,
	&dmat4_type,
	&dmat2x3_type,
	&dmat2x4_type,
	&dmat3x2_type,
	&dmat3x4_type,
	&dmat4x2_type,
	&dmat4x3_type,
	NULL,
};

/**
 * Type used to communicate to get_shader() the set of varyings to
 * test.
 */
struct varying_desc
{
	const struct type_desc *type;
	unsigned array_elems;
};

/**
 * Generate a vertex or fragment shader to test the given set of
 * varyings.
 */
static GLint
get_shader(bool is_vs, unsigned glsl_version, int num_varyings,
	   struct varying_desc *varyings)
{
	GLuint shader;
	char *full_text = malloc(1000 * 100 + num_varyings);
	char *text = full_text;
	unsigned i, j, k, l;
	const char *varying_keyword;
	unsigned offset = 0;
	GLenum shader_type = is_vs ? GL_VERTEX_SHADER : GL_FRAGMENT_SHADER;
	bool fp64 = false;

	if (glsl_version >= 130) {
		if (is_vs)
			varying_keyword = "out";
		else
			varying_keyword = "in";
	} else {
		varying_keyword = "varying";
	}

	text += sprintf(text, "#version %u\n", glsl_version);
	for (i = 0; i < num_varyings; ++i) {
		const char *opt_flat_keyword = "";
		if (!fp64 && varyings[i].type->base == BASE_TYPE_DOUBLE) {
			text += sprintf(text, "#extension GL_ARB_gpu_shader_fp64: enable\n");
			fp64 = true;
		}
		if (varyings[i].type->base != BASE_TYPE_FLOAT)
			opt_flat_keyword = "flat ";
		if (varyings[i].array_elems != 0) {
			text += sprintf(text, "%s%s %s var%03u[%u];\n",
					opt_flat_keyword, varying_keyword,
					varyings[i].type->name, i,
					varyings[i].array_elems);
		} else {
			text += sprintf(text, "%s%s %s var%03u;\n",
					opt_flat_keyword, varying_keyword,
					varyings[i].type->name, i);
		}
	}
	if (glsl_version >= 150 && is_vs) {
		text += sprintf(text, "in vec4 piglit_vertex;\n");
		text += sprintf(text, "#define gl_Vertex piglit_vertex\n");
	}
	text += sprintf(text, "uniform int i;\n");
	text += sprintf(text,
			"\n"
			"void main()\n"
			"{\n");
	if (is_vs)
		text += sprintf(text, "  gl_Position = gl_Vertex;\n");
	else
		text += sprintf(text, "  bool failed = false;\n");
	for (i = 0; i < num_varyings; ++i) {
		unsigned array_loop_bound = varyings[i].array_elems;
		const char *base_type_name
			= get_base_type_name(varyings[i].type->base);
		if (array_loop_bound == 0)
			array_loop_bound = 1;
		for (j = 0; j < array_loop_bound; ++j) {
			for (k = 0; k < varyings[i].type->num_cols; ++k) {
				for (l = 0; l < varyings[i].type->num_rows; ++l) {
					text += sprintf(text, "  ");
					if (!is_vs)
						text += sprintf(text, "failed = failed || ");
					text += sprintf(text, "var%03u", i);
					if (varyings[i].array_elems)
						text += sprintf(text, "[%u]", j);
					if (varyings[i].type->num_cols > 1)
						text += sprintf(text, "[%u]", k);
					if (varyings[i].type->num_rows > 1)
						text += sprintf(text, "[%u]", l);
					if (is_vs)
						text += sprintf(text, " = ");
					else
						text += sprintf(text, " != ");
					text += sprintf(text, "%s(i + %u);\n",
							base_type_name,
							offset++);
				}
			}
		}
	}
	if (!is_vs) {
		text += sprintf(text,
				"  if (failed)\n"
				"    gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
				"  else\n"
				"    gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);\n");
	}
	text += sprintf(text, "}\n");

	shader = piglit_compile_shader_text(shader_type, full_text);

	free(full_text);

	return shader;
}

/**
 * Choose the set of varyings necessary to properly run the given test
 * configuration, given the implementation's reported value of
 * max_varying_floats.
 */
static unsigned
choose_varyings(struct varying_desc *varyings,
		const struct type_desc *test_type, GLboolean test_array,
		unsigned max_varying_floats)
{
	unsigned num_varyings = 0;
	unsigned element_size = test_type->base == BASE_TYPE_DOUBLE ? 2 : 1;
	unsigned components_in_test_type
		= test_type->num_cols * test_type->num_rows * element_size;
	unsigned num_test_varyings
		= max_varying_floats / components_in_test_type;
	unsigned num_extra_varyings
		= max_varying_floats
		- num_test_varyings * components_in_test_type;
	unsigned i;
	if (test_array) {
		varyings[num_varyings].type = test_type;
		varyings[num_varyings].array_elems = num_test_varyings;
		++num_varyings;
	} else {
		for (i = 0; i < num_test_varyings; ++i) {
			varyings[num_varyings].type = test_type;
			varyings[num_varyings].array_elems = 0;
			++num_varyings;
		}
	}
	for (i = 0; i < num_extra_varyings; ++i) {
		varyings[num_varyings].type = &float_type;
		varyings[num_varyings].array_elems = 0;
		++num_varyings;
	}

	return num_varyings;
}

void
NORETURN print_usage_and_exit(const char *prog_name)
{
	unsigned i;
	printf("Usage: %s <type> <arrayspec>\n"
	       "  where <type> is one of:\n", prog_name);
	for (i = 0; all_types[i]; ++i)
		printf("    %s\n", all_types[i]->name);
	printf("  and <arrayspec> is one of:\n"
	       "    array: test using an array of the above type\n"
	       "    separate: test using separately declared varyings\n");
	piglit_report_result(PIGLIT_FAIL);
}

static const struct type_desc *test_type;

static void
parse_args(int argc, char *argv[], struct piglit_gl_test_config *config)
{
	unsigned i;

	if (argc < 3)
		print_usage_and_exit(argv[0]);
	for (i = 0; all_types[i]; ++i) {
		if (strcmp(argv[1], all_types[i]->name) == 0)
			break;
	}
	if (all_types[i])
		test_type = all_types[i];
	else
		print_usage_and_exit(argv[0]);

	if (test_type->glsl_version_required <= 110)
		config->supports_gl_compat_version = 20;
	else if (test_type->glsl_version_required <= 120)
		config->supports_gl_compat_version = 21;
	else if (test_type->glsl_version_required <= 130)
		config->supports_gl_compat_version = 30;
	else if (test_type->glsl_version_required <= 150)
		config->supports_gl_core_version = 32;
	else
		piglit_report_result(PIGLIT_FAIL);
}

void
piglit_init(int argc, char **argv)
{
	GLboolean test_array;
	GLint max_varying_floats;
	struct varying_desc *varyings;
	unsigned num_varyings;
	GLuint vs, fs;

	if (argc != 3)
		print_usage_and_exit(argv[0]);

	if (strcmp(argv[2], "array") == 0)
		test_array = GL_TRUE;
	else if (strcmp(argv[2], "separate") == 0)
		test_array = GL_FALSE;
	else
		print_usage_and_exit(argv[0]);

	piglit_require_gl_version(20);
	piglit_require_GLSL_version(test_type->glsl_version_required);
	if (test_type->base == BASE_TYPE_DOUBLE)
		piglit_require_extension("GL_ARB_gpu_shader_fp64");
	glGetIntegerv(GL_MAX_VARYING_FLOATS, &max_varying_floats);

	varyings = malloc(sizeof(*varyings) * max_varying_floats);
	num_varyings = choose_varyings(varyings, test_type,
				       test_array, max_varying_floats);

	vs = get_shader(true, test_type->glsl_version_required,
			num_varyings, varyings);
	fs = get_shader(false, test_type->glsl_version_required,
			num_varyings, varyings);
	prog = piglit_link_simple_program(vs, fs);
	i_location = glGetUniformLocation(prog, "i");
	free(varyings);
}

enum piglit_result
piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	GLuint vao;
	float green[] = { 0.0, 1.0, 0.0, 1.0 };

	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(prog);
	glUniform1i(i_location, 0);
	if (piglit_is_core_profile) {
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
	}
	piglit_draw_rect(-1, -1, 2, 2);
	pass = piglit_probe_rect_rgba(0, 0, piglit_width, piglit_height, green)
		&& pass;

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
