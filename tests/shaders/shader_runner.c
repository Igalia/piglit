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

#include "piglit-util-gl.h"
#include "piglit-vbo.h"

#include "shader_runner_gles_workarounds.h"
#include "parser_utils.h"

static void
get_required_config(const char *script_name,
		    struct piglit_gl_test_config *config);
GLenum
decode_drawing_mode(const char *mode_str);

void
get_uints(const char *line, unsigned *uints, unsigned count);

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.window_width = 250;
	config.window_height = 250;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

	if (argc > 1)
		get_required_config(argv[1], &config);
	else
		config.supports_gl_compat_version = 10;

PIGLIT_GL_TEST_CONFIG_END

const char passthrough_vertex_shader_source[] =
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
	bool es;
	unsigned num;
	char _string[100];
};

#define ENUM_STRING(e) { #e, e }

struct string_to_enum {
	const char *name;
	GLenum token;
};

extern float piglit_tolerance[4];

static struct component_version gl_version;
static struct component_version glsl_version;
static struct component_version glsl_req_version;
static int gl_max_fragment_uniform_components;
static int gl_max_vertex_uniform_components;
static int gl_max_varying_components;
static int gl_max_clip_planes;

const char *path = NULL;
const char *test_start = NULL;

GLuint vertex_shaders[256];
unsigned num_vertex_shaders = 0;
GLuint tess_ctrl_shaders[256];
unsigned num_tess_ctrl_shaders = 0;
GLuint tess_eval_shaders[256];
unsigned num_tess_eval_shaders = 0;
GLuint geometry_shaders[256];
unsigned num_geometry_shaders = 0;
GLuint fragment_shaders[256];
unsigned num_fragment_shaders = 0;
GLuint compute_shaders[256];
unsigned num_compute_shaders = 0;
int num_uniform_blocks;
GLuint *uniform_block_bos;
GLenum geometry_layout_input_type = GL_TRIANGLES;
GLenum geometry_layout_output_type = GL_TRIANGLE_STRIP;
GLint geometry_layout_vertices_out = 0;
GLuint atomics_bo = 0;

char *shader_string;
GLint shader_string_size;
const char *vertex_data_start = NULL;
const char *vertex_data_end = NULL;
GLuint prog;
size_t num_vbo_rows = 0;
bool vbo_present = false;
bool link_ok = false;
bool prog_in_use = false;
GLchar *prog_err_info = NULL;
GLuint vao = 0;
GLuint fbo = 0;
GLint render_width, render_height;

enum states {
	none = 0,
	requirements,
	vertex_shader,
	vertex_shader_passthrough,
	vertex_program,
	tess_ctrl_shader,
	tess_eval_shader,
	geometry_shader,
	geometry_layout,
	fragment_shader,
	fragment_program,
	compute_shader,
	vertex_data,
	test,
};


enum comparison {
	equal = 0,
	not_equal,
	less,
	greater_equal,
	greater,
	less_equal
};

GLenum
lookup_enum_string(const struct string_to_enum *table, const char **line,
		   const char *error_desc)
{
	int i;
	*line = eat_whitespace(*line);
	for (i = 0; table[i].name; i++) {
		size_t len = strlen(table[i].name);
		if (strncmp(table[i].name, *line, len) == 0 &&
		    ((*line)[len] == '\0' || isspace((*line)[len]))) {
			*line = eat_whitespace(*line + len);
			return table[i].token;
		}
	}
	fprintf(stderr, "Bad %s at: %s\n", error_desc, *line);
	piglit_report_result(PIGLIT_FAIL);
	return 0;
}

bool
compare(float ref, float value, enum comparison cmp);

static bool
compare_uint(GLuint ref, GLuint value, enum comparison cmp);

static void
version_init(struct component_version *v, enum version_tag tag, bool core, bool es, unsigned num)
{
	assert(tag == VERSION_GL || tag == VERSION_GLSL);

	v->_tag = tag;
	v->core = core;
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


const char *
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


void
compile_glsl(GLenum target)
{
	GLuint shader = glCreateShader(target);
	GLint ok;

	switch (target) {
	case GL_VERTEX_SHADER:
		piglit_require_vertex_shader();
		break;
	case GL_FRAGMENT_SHADER:
		piglit_require_fragment_shader();
		break;
	case GL_TESS_CONTROL_SHADER:
	case GL_TESS_EVALUATION_SHADER:
		if (gl_version.num < 40)
			piglit_require_extension("GL_ARB_tessellation_shader");
		break;
	case GL_GEOMETRY_SHADER:
		if (gl_version.num < 32)
			piglit_require_extension("GL_ARB_geometry_shader4");
		break;
	case GL_COMPUTE_SHADER:
		if (gl_version.num < 43)
			piglit_require_extension("GL_ARB_compute_shader");
		break;
	}

	if (!glsl_req_version.num) {
		printf("GLSL version requirement missing\n");
		piglit_report_result(PIGLIT_FAIL);
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
		piglit_report_result(PIGLIT_FAIL);
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
}

void
compile_and_bind_program(GLenum target, const char *start, int len)
{
	GLuint prog;
	char *source;

	switch (target) {
	case GL_VERTEX_PROGRAM_ARB:
		piglit_require_extension("GL_ARB_vertex_program");
		break;
	case GL_FRAGMENT_PROGRAM_ARB:
		piglit_require_extension("GL_ARB_fragment_program");
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
}

/**
 * Compare two values given a specified comparison operator
 */
bool
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
const char *
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
 * Parse a binary comparison operator and return the matching token
 */
const char *
process_comparison(const char *src, enum comparison *cmp)
{
	char buf[32];

	switch (src[0]) {
	case '=':
		if (src[1] == '=') {
			*cmp = equal;
			return src + 2;
		}
		break;
	case '<':
		if (src[1] == '=') {
			*cmp = less_equal;
			return src + 2;
		} else {
			*cmp = less;
			return src + 1;
		}
	case '>':
		if (src[1] == '=') {
			*cmp = greater_equal;
			return src + 2;
		} else {
			*cmp = greater;
			return src + 1;
		}
	case '!':
		if (src[1] == '=') {
			*cmp = not_equal;
			return src + 2;
		}
		break;
	}

	strncpy(buf, src, sizeof(buf));
	buf[sizeof(buf) - 1] = '\0';
	printf("invalid comparison in test script:\n%s\n", buf);
	piglit_report_result(PIGLIT_FAIL);

	/* Won't get here. */
	return NULL;
}


/**
 * " ES" before the comparison operator indicates the version
 * pertains to GL ES.
 */
void
parse_version_comparison(const char *line, enum comparison *cmp,
			 struct component_version *v, enum version_tag tag)
{
	unsigned major;
	unsigned minor;
	unsigned full_num;
	bool es = false;
	bool core = false;

	if (string_match(" CORE", line)) {
		core = true;
		line += 5;
	}
	if (string_match(" ES", line)) {
		es = true;
		line += 3;
	}
	line = eat_whitespace(line);
	line = process_comparison(line, cmp);

	line = eat_whitespace(line);
	sscanf(line, "%u.%u", &major, &minor);
	line = eat_text(line);

	line = eat_whitespace(line);
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

	version_init(v, tag, core, es, full_num);
}

/**
 * Parse and check a line from the requirement section of the test
 */
void
process_requirement(const char *line)
{
	char buffer[4096];
	static const struct {
		const char *name;
		int *val;
		const char *desc;
	} getint_limits[] = {
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
			"GL_MAX_VARYING_COMPONENTS",
			&gl_max_varying_components,
			"varying components",
		},
	};
	unsigned i;

	/* There are four types of requirements that a test can currently
	 * have:
	 *
	 *    * Require that some GL extension be supported
	 *    * Require some particular versions of GL
	 *    * Require some particular versions of GLSL
	 *    * Require some particular number of uniform components
	 *
	 * The tests for GL and GLSL versions can be equal, not equal,
	 * less, less-or-equal, greater, or greater-or-equal.  Extension tests
	 * can also require that a particular extension not be supported by
	 * prepending ! to the extension name.
	 */
	for (i = 0; i < ARRAY_SIZE(getint_limits); i++) {
		enum comparison cmp;
		int maxcomp;

		if (!string_match(getint_limits[i].name, line))
			continue;

		line = eat_whitespace(line + strlen(getint_limits[i].name));

		line = process_comparison(line, &cmp);

		maxcomp = atoi(line);
		if (!compare(maxcomp, *getint_limits[i].val, cmp)) {
			printf("Test requires %s %s %i.  "
			       "The driver supports %i.\n",
			       getint_limits[i].desc,
			       comparison_string(cmp),
			       maxcomp,
			       *getint_limits[i].val);
			piglit_report_result(PIGLIT_SKIP);
		}
		return;
	}

	if (string_match("GL_", line)) {
		strcpy_to_space(buffer, line);
		piglit_require_extension(buffer);
	} else if (string_match("!GL_", line)) {
		strcpy_to_space(buffer, line + 1);
		piglit_require_not_extension(buffer);
	} else if (string_match("GLSL", line)) {
		enum comparison cmp;

		parse_version_comparison(line + 4, &cmp, &glsl_req_version,
		                         VERSION_GLSL);

		/* We only allow >= because we potentially use the
		 * version number to insert a #version directive. */
		if (cmp != greater_equal) {
			printf("Unsupported GLSL version comparison\n");
			piglit_report_result(PIGLIT_FAIL);
		}

		if (!version_compare(&glsl_req_version, &glsl_version, cmp)) {
			printf("Test requires %s %s.  "
			       "Actual version %s.\n",
			       comparison_string(cmp),
			       version_string(&glsl_req_version),
			       version_string(&glsl_version));
			piglit_report_result(PIGLIT_SKIP);
		}
	} else if (string_match("GL", line)) {
		enum comparison cmp;
		struct component_version gl_req_version;

		parse_version_comparison(line + 2, &cmp, &gl_req_version,
		                         VERSION_GL);

		if (!version_compare(&gl_req_version, &gl_version, cmp)) {
			printf("Test requires %s %s.  "
			       "Actual version is %s.\n",
			       comparison_string(cmp),
			       version_string(&gl_req_version),
			       version_string(&gl_version));
			piglit_report_result(PIGLIT_SKIP);
		}
	} else if (string_match("rlimit", line)) {
		unsigned long lim;
		char *ptr;

		line = eat_whitespace(line + 6);

		lim = strtoul(line, &ptr, 0);
		if (ptr == line) {
			printf("rlimit requires numeric argument\n");
			piglit_report_result(PIGLIT_FAIL);
		}

		piglit_set_rlimit(lim);
	}
}


/**
 * Process a line from the [geometry layout] section of a test
 */
void
process_geometry_layout(const char *line)
{
	char s[32];
	int x;

	line = eat_whitespace(line);

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


void
leave_state(enum states state, const char *line)
{
	switch (state) {
	case none:
		break;

	case requirements:
		break;

	case vertex_shader:
		shader_string_size = line - shader_string;
		compile_glsl(GL_VERTEX_SHADER);
		break;

	case vertex_shader_passthrough:
		compile_glsl(GL_VERTEX_SHADER);
		break;

	case vertex_program:
		compile_and_bind_program(GL_VERTEX_PROGRAM_ARB,
					 shader_string,
					 line - shader_string);
		break;

	case tess_ctrl_shader:
		shader_string_size = line - shader_string;
		compile_glsl(GL_TESS_CONTROL_SHADER);
		break;

	case tess_eval_shader:
		shader_string_size = line - shader_string;
		compile_glsl(GL_TESS_EVALUATION_SHADER);
		break;

	case geometry_shader:
		shader_string_size = line - shader_string;
		compile_glsl(GL_GEOMETRY_SHADER);
		break;

	case geometry_layout:
		break;

	case fragment_shader:
		shader_string_size = line - shader_string;
		compile_glsl(GL_FRAGMENT_SHADER);
		break;

	case fragment_program:
		compile_and_bind_program(GL_FRAGMENT_PROGRAM_ARB,
					 shader_string,
					 line - shader_string);
		break;

	case compute_shader:
		shader_string_size = line - shader_string;
		compile_glsl(GL_COMPUTE_SHADER);
		break;

	case vertex_data:
		vertex_data_end = line;
		break;

	case test:
		break;

	default:
		assert(!"Not yet supported.");
	}
}


void
link_and_use_shaders(void)
{
	unsigned i;
	GLenum err;
	GLint ok;

	if ((num_vertex_shaders == 0)
	    && (num_fragment_shaders == 0)
	    && (num_tess_ctrl_shaders == 0)
	    && (num_tess_eval_shaders == 0)
	    && (num_geometry_shaders == 0)
	    && (num_compute_shaders == 0))
		return;

	prog = glCreateProgram();

	for (i = 0; i < num_vertex_shaders; i++) {
		glAttachShader(prog, vertex_shaders[i]);
	}

	for (i = 0; i < num_tess_ctrl_shaders; i++) {
		glAttachShader(prog, tess_ctrl_shaders[i]);
	}

	for (i = 0; i < num_tess_eval_shaders; i++) {
		glAttachShader(prog, tess_eval_shaders[i]);
	}

	for (i = 0; i < num_geometry_shaders; i++) {
		glAttachShader(prog, geometry_shaders[i]);
	}

	for (i = 0; i < num_fragment_shaders; i++) {
		glAttachShader(prog, fragment_shaders[i]);
	}

	for (i = 0; i < num_compute_shaders; i++) {
		glAttachShader(prog, compute_shaders[i]);
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

	glLinkProgram(prog);

	for (i = 0; i < num_vertex_shaders; i++) {
		glDeleteShader(vertex_shaders[i]);
	}

	for (i = 0; i < num_tess_ctrl_shaders; i++) {
		glDeleteShader(tess_ctrl_shaders[i]);
	}

	for (i = 0; i < num_tess_eval_shaders; i++) {
		glDeleteShader(tess_eval_shaders[i]);
	}

	for (i = 0; i < num_geometry_shaders; i++) {
		glDeleteShader(geometry_shaders[i]);
	}

	for (i = 0; i < num_fragment_shaders; i++) {
		glDeleteShader(fragment_shaders[i]);
	}

	for (i = 0; i < num_compute_shaders; i++) {
		glDeleteShader(compute_shaders[i]);
	}

	glGetProgramiv(prog, GL_LINK_STATUS, &ok);
	if (ok) {
		link_ok = true;
	} else {
		GLint size;

		glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &size);
		prog_err_info = malloc(size);

		glGetProgramInfoLog(prog, size, NULL, prog_err_info);

		return;
	}

	glUseProgram(prog);

	err = glGetError();
	if (!err) {
		prog_in_use = true;
	} else {
		GLint size;

		glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &size);
		prog_err_info = malloc(size);

		glGetProgramInfoLog(prog, size, NULL, prog_err_info);
	}
}


void
process_test_script(const char *script_name)
{
	unsigned text_size;
	char *text = piglit_load_text_file(script_name, &text_size);
	enum states state = none;
	const char *line = text;

	if (line == NULL) {
		printf("could not read file \"%s\"\n", script_name);
		piglit_report_result(PIGLIT_FAIL);
	}

	while (line[0] != '\0') {
		if (line[0] == '[') {
			leave_state(state, line);

			if (string_match("[require]", line)) {
				state = requirements;
			} else if (string_match("[vertex shader]", line)) {
				state = vertex_shader;
				shader_string = NULL;
			} else if (string_match("[vertex program]", line)) {
				state = vertex_program;
				shader_string = NULL;
			} else if (string_match("[vertex shader passthrough]", line)) {
				state = vertex_shader_passthrough;
				shader_string =
					(char *) passthrough_vertex_shader_source;
				shader_string_size = strlen(shader_string);
			} else if (string_match("[tessellation control shader]", line)) {
				state = tess_ctrl_shader;
				shader_string = NULL;
			} else if (string_match("[tessellation evaluation shader]", line)) {
				state = tess_eval_shader;
				shader_string = NULL;
			} else if (string_match("[geometry shader]", line)) {
				state = geometry_shader;
				shader_string = NULL;
			} else if (string_match("[geometry layout]", line)) {
				state = geometry_layout;
				shader_string = NULL;
			} else if (string_match("[fragment shader]", line)) {
				state = fragment_shader;
				shader_string = NULL;
			} else if (string_match("[fragment program]", line)) {
				state = fragment_program;
				shader_string = NULL;
			} else if (string_match("[compute shader]", line)) {
				state = compute_shader;
				shader_string = NULL;
			} else if (string_match("[vertex data]", line)) {
				state = vertex_data;
				vertex_data_start = NULL;
			} else if (string_match("[test]", line)) {
				test_start = strchrnul(line, '\n');
				if (test_start[0] != '\0')
					test_start++;
				return;
			} else {
				fprintf(stderr,
					"Unknown section in test script.  "
					"Perhaps missing closing ']'?\n");
				piglit_report_result(PIGLIT_FAIL);
			}
		} else {
			switch (state) {
			case none:
			case vertex_shader_passthrough:
				break;

			case requirements:
				process_requirement(line);
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
				if (shader_string == NULL)
					shader_string = (char *) line;
				break;

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
	}

	leave_state(state, line);
}

struct requirement_parse_results {
	bool found_gl;
	bool found_glsl;
	bool found_size;
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
			if (string_match("[require]", line)) {
				in_requirement_section = true;
			}
		} else {
			if (string_match("GL_", line)
			    || string_match("!GL_", line)) {
				/* empty */
			} else if (string_match("GLSL", line)) {
				enum comparison cmp;
				struct component_version version;

				parse_version_comparison(line + 4, &cmp,
							 &version, VERSION_GLSL);
				if (cmp == greater_equal) {
					results->found_glsl = true;
					version_copy(&results->glsl_version, &version);
				}
			} else if (string_match("GL", line)) {
				enum comparison cmp;
				struct component_version version;

				parse_version_comparison(line + 2, &cmp,
							 &version, VERSION_GL);
				if (cmp == greater_equal
				    || cmp == greater
				    || cmp == equal) {
					results->found_gl = true;
					version_copy(&results->gl_version, &version);
				}
			} else if (string_match("SIZE", line)) {
				results->found_size = true;
				get_uints(line+4, results->size, 2);
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
		version_init(gl_version, VERSION_GL, false, false, 10);
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
 * section is processed.  Do a quick can over the requirements section to find
 * the GL and GLSL version requirements.  Use these to guide context creation.
 */
void
get_required_config(const char *script_name,
		    struct piglit_gl_test_config *config)
{
	struct requirement_parse_results parse_results;
	struct component_version required_gl_version;

	parse_required_config(&parse_results, script_name);
	choose_required_gl_version(&parse_results, &required_gl_version);

	if (parse_results.found_size) {
		config->window_width = parse_results.size[0];
		config->window_height = parse_results.size[1];
	}

	if (required_gl_version.es) {
		config->supports_gl_es_version = required_gl_version.num;
	} else if (required_gl_version.num >= 31) {
		config->supports_gl_core_version = required_gl_version.num;
		if (!required_gl_version.core)
			config->supports_gl_compat_version = required_gl_version.num;
	} else {
		config->supports_gl_compat_version = 10;
	}
}

void
get_floats(const char *line, float *f, unsigned count)
{
	unsigned i;

	for (i = 0; i < count; i++) {
		line = eat_whitespace(line);

		if (strncmp(line, "0x", 2) == 0) {
			union {
				uint32_t u;
				float f;
			} x;

			x.u = strtoul(line, (char **) &line, 16);
			f[i] = x.f;
		} else {
			f[i] = strtod_inf(line, (char **) &line);
		}
	}
}

void
get_doubles(const char *line, double *d, unsigned count)
{
	unsigned i;

	for (i = 0; i < count; i++) {
		line = eat_whitespace(line);

		if (strncmp(line, "0x", 2) == 0) {
			union {
				uint64_t u64;
				double d;
			} x;

			x.u64 = strtoull(line, (char **) &line, 16);
			d[i] = x.d;
		} else {
			d[i] = strtod_inf(line, (char **) &line);
		}
	}
}


void
get_ints(const char *line, int *ints, unsigned count)
{
	unsigned i;

	for (i = 0; i < count; i++)
		ints[i] = strtol(line, (char **) &line, 0);
}


void
get_uints(const char *line, unsigned *uints, unsigned count)
{
	unsigned i;

	for (i = 0; i < count; i++)
		uints[i] = strtoul(line, (char **) &line, 0);
}


/**
 * Check that the GL implementation supports unsigned uniforms
 * (e.g. through glUniform1ui).  If not, terminate the test with a
 * SKIP.
 */
void
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
void
check_double_support(void)
{
	if (gl_version.num < 40 && !piglit_is_extension_supported("GL_ARB_gpu_shader_fp64"))
		piglit_report_result(PIGLIT_SKIP);
}

/**
 * Handles uploads of UBO uniforms by mapping the buffer and storing
 * the data.  If the uniform is not in a uniform block, returns false.
 */
bool
set_ubo_uniform(char *name, const char *type, const char *line, int ubo_array_index)
{
	GLuint uniform_index;
	GLint block_index;
	GLint offset;
	GLint array_index;
	char *data;
	float f[16];
	double d[16];
	int ints[16];
	unsigned uints[16];
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

	if (string_match("float", type)) {
		get_floats(line, f, 1);
		memcpy(data, f, sizeof(float));
	} else if (string_match("int", type)) {
		get_ints(line, ints, 1);
		memcpy(data, ints, sizeof(int));
	} else if (string_match("uint", type)) {
		get_uints(line, uints, 1);
		memcpy(data, uints, sizeof(int));
	} else if (string_match("double", type)) {
		get_doubles(line, d, 1);
		memcpy(data, d, sizeof(double));
	} else if (string_match("vec", type)) {
		int elements = type[3] - '0';
		get_floats(line, f, elements);
		memcpy(data, f, elements * sizeof(float));
	} else if (string_match("ivec", type)) {
		int elements = type[4] - '0';
		get_ints(line, ints, elements);
		memcpy(data, ints, elements * sizeof(int));
	} else if (string_match("uvec", type)) {
		int elements = type[4] - '0';
		get_uints(line, uints, elements);
		memcpy(data, uints, elements * sizeof(unsigned));
	} else if (string_match("dvec", type)) {
		int elements = type[4] - '0';
		get_doubles(line, d, elements);
		memcpy(data, d, elements * sizeof(double));
	} else if (string_match("mat", type)) {
		GLint matrix_stride, row_major;
		int cols = type[3] - '0';
		int rows = type[4] == 'x' ? type[5] - '0' : cols;
		int r, c;
		float *matrixdata = (float *)data;

		assert(cols >= 2 && cols <= 4);
		assert(rows >= 2 && rows <= 4);

		get_floats(line, f, rows * cols);

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
	} else if (string_match("dmat", type)) {
		GLint matrix_stride, row_major;
		int cols = type[4] - '0';
		int rows = type[5] == 'x' ? type[6] - '0' : cols;
		int r, c;
		double *matrixdata = (double *)data;

		assert(cols >= 2 && cols <= 4);
		assert(rows >= 2 && rows <= 4);

		get_doubles(line, d, rows * cols);

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
	} else {
		printf("unknown uniform type \"%s\" for \"%s\"\n", type, name);
		piglit_report_result(PIGLIT_FAIL);
	}

	glUnmapBuffer(GL_UNIFORM_BUFFER);

	return true;
}

void
set_uniform(const char *line, int ubo_array_index)
{
	char name[512];
	float f[16];
	double d[16];
	int ints[16];
	unsigned uints[16];
	GLuint prog;
	GLint loc;
	const char *type;

	glGetIntegerv(GL_CURRENT_PROGRAM, (GLint *) &prog);

	type = eat_whitespace(line);
	line = eat_text(type);

	line = strcpy_to_space(name, eat_whitespace(line));

	if (set_ubo_uniform(name, type, line, ubo_array_index))
		return;

	loc = glGetUniformLocation(prog, name);
	if (loc < 0) {
		printf("cannot get location of uniform \"%s\"\n",
		       name);
		piglit_report_result(PIGLIT_FAIL);
	}

	if (string_match("float", type)) {
		get_floats(line, f, 1);
		glUniform1fv(loc, 1, f);
		return;
	} else if (string_match("int", type)) {
		int val = atoi(line);
		glUniform1i(loc, val);
		return;
	} else if (string_match("uint", type)) {
		unsigned val;
		check_unsigned_support();
		val = strtoul(line, NULL, 0);
		glUniform1ui(loc, val);
		return;
	} else if (string_match("double", type)) {
		check_double_support();
		get_doubles(line, d, 1);
		glUniform1dv(loc, 1, d);
		return;
	} else if (string_match("vec", type)) {
		switch (type[3]) {
		case '2':
			get_floats(line, f, 2);
			glUniform2fv(loc, 1, f);
			return;
		case '3':
			get_floats(line, f, 3);
			glUniform3fv(loc, 1, f);
			return;
		case '4':
			get_floats(line, f, 4);
			glUniform4fv(loc, 1, f);
			return;
		}
	} else if (string_match("ivec", type)) {
		switch (type[4]) {
		case '2':
			get_ints(line, ints, 2);
			glUniform2iv(loc, 1, ints);
			return;
		case '3':
			get_ints(line, ints, 3);
			glUniform3iv(loc, 1, ints);
			return;
		case '4':
			get_ints(line, ints, 4);
			glUniform4iv(loc, 1, ints);
			return;
		}
	} else if (string_match("uvec", type)) {
		check_unsigned_support();
		switch (type[4]) {
		case '2':
			get_uints(line, uints, 2);
			glUniform2uiv(loc, 1, uints);
			return;
		case '3':
			get_uints(line, uints, 3);
			glUniform3uiv(loc, 1, uints);
			return;
		case '4':
			get_uints(line, uints, 4);
			glUniform4uiv(loc, 1, uints);
			return;
		}
	} else if (string_match("dvec", type)) {
		check_double_support();
		switch (type[4]) {
		case '2':
			get_doubles(line, d, 2);
			glUniform2dv(loc, 1, d);
			return;
		case '3':
			get_doubles(line, d, 3);
			glUniform3dv(loc, 1, d);
			return;
		case '4':
			get_doubles(line, d, 4);
			glUniform4dv(loc, 1, d);
			return;
		}
	} else if (string_match("mat", type) && type[3] != '\0') {
		char cols = type[3];
		char rows = type[4] == 'x' ? type[5] : cols;
		switch (cols) {
		case '2':
			switch (rows) {
			case '2':
				get_floats(line, f, 4);
				glUniformMatrix2fv(loc, 1, GL_FALSE, f);
				return;
			case '3':
				get_floats(line, f, 6);
				glUniformMatrix2x3fv(loc, 1, GL_FALSE, f);
				return;
			case '4':
				get_floats(line, f, 8);
				glUniformMatrix2x4fv(loc, 1, GL_FALSE, f);
				return;
			}
		case '3':
			switch (rows) {
			case '2':
				get_floats(line, f, 6);
				glUniformMatrix3x2fv(loc, 1, GL_FALSE, f);
				return;
			case '3':
				get_floats(line, f, 9);
				glUniformMatrix3fv(loc, 1, GL_FALSE, f);
				return;
			case '4':
				get_floats(line, f, 12);
				glUniformMatrix3x4fv(loc, 1, GL_FALSE, f);
				return;
			}
		case '4':
			switch (rows) {
			case '2':
				get_floats(line, f, 8);
				glUniformMatrix4x2fv(loc, 1, GL_FALSE, f);
				return;
			case '3':
				get_floats(line, f, 12);
				glUniformMatrix4x3fv(loc, 1, GL_FALSE, f);
				return;
			case '4':
				get_floats(line, f, 16);
				glUniformMatrix4fv(loc, 1, GL_FALSE, f);
				return;
			}
		}
	} else if (string_match("dmat", type) && type[4] != '\0') {
		char cols = type[4];
		char rows = type[5] == 'x' ? type[6] : cols;
		switch (cols) {
		case '2':
			switch (rows) {
			case '2':
				get_doubles(line, d, 4);
				glUniformMatrix2dv(loc, 1, GL_FALSE, d);
				return;
			case '3':
				get_doubles(line, d, 6);
				glUniformMatrix2x3dv(loc, 1, GL_FALSE, d);
				return;
			case '4':
				get_doubles(line, d, 8);
				glUniformMatrix2x4dv(loc, 1, GL_FALSE, d);
				return;
			}
		case '3':
			switch (rows) {
			case '2':
				get_doubles(line, d, 6);
				glUniformMatrix3x2dv(loc, 1, GL_FALSE, d);
				return;
			case '3':
				get_doubles(line, d, 9);
				glUniformMatrix3dv(loc, 1, GL_FALSE, d);
				return;
			case '4':
				get_doubles(line, d, 12);
				glUniformMatrix3x4dv(loc, 1, GL_FALSE, d);
				return;
			}
		case '4':
			switch (rows) {
			case '2':
				get_doubles(line, d, 8);
				glUniformMatrix4x2dv(loc, 1, GL_FALSE, d);
				return;
			case '3':
				get_doubles(line, d, 12);
				glUniformMatrix4x3dv(loc, 1, GL_FALSE, d);
				return;
			case '4':
				get_doubles(line, d, 16);
				glUniformMatrix4dv(loc, 1, GL_FALSE, d);
				return;
			}
		}
	}

	strcpy_to_space(name, type);
	printf("unknown uniform type \"%s\"\n", name);
	piglit_report_result(PIGLIT_FAIL);

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
void
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

	char name[512];
	char name_buf[512];
	char pname_string[512];
	GLenum pname;
	GLint expected;
	int i;
	int num_active_uniforms;

	line = strcpy_to_space(name, eat_whitespace(line));

	strcpy_to_space(pname_string, eat_whitespace(line));
	pname = lookup_enum_string(all_pnames, &line, "glGetUniformsiv pname");

	line = eat_whitespace(line);
	if (isdigit(line[0])) {
		expected = strtol(line, NULL, 0);
	} else {
		expected = lookup_enum_string(all_types, &line, "type enum");
	}

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
				name, pname_string,
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
				name, pname_string,
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

void
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

	if (string_match("env_vp", type)) {
		glProgramEnvParameter4fvARB(GL_VERTEX_PROGRAM_ARB, index, f);
	} else if (string_match("local_vp", type)) {
		glProgramLocalParameter4fvARB(GL_VERTEX_PROGRAM_ARB, index, f);
	} else if (string_match("env_fp", type)) {
		glProgramEnvParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, index, f);
	} else if (string_match("local_fp", type)) {
		glProgramLocalParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, index, f);
	} else {
		fprintf(stderr, "Unknown parameter type `%s'\n", type);
		piglit_report_result(PIGLIT_FAIL);
	}
}

void
set_patch_parameter(const char *line)
{
#ifdef PIGLIT_USE_OPENGL
	float f[4];
	int i, count;
	const char *const line0 = line;

	if (gl_version.num < 40)
		piglit_require_extension("GL_ARB_tessellation_shader");

	if (string_match("vertices ", line)) {
		line += strlen("vertices ");
		count = sscanf(line, "%d", &i);
		if (count != 1) {
			fprintf(stderr, "Couldn't parse patch parameter command:\n%s\n", line0);
			piglit_report_result(PIGLIT_FAIL);
		}
		glPatchParameteri(GL_PATCH_VERTICES, i);
	} else if (string_match("default level outer ", line)) {
		line += strlen("default level outer ");
		count = sscanf(line, "%f %f %f %f", &f[0], &f[1], &f[2], &f[3]);
		if (count != 4) {
			fprintf(stderr, "Couldn't parse patch parameter command:\n%s\n", line0);
			piglit_report_result(PIGLIT_FAIL);
		}
		glPatchParameterfv(GL_PATCH_DEFAULT_OUTER_LEVEL, f);
	} else if (string_match("default level inner ", line)) {
		line += strlen("default level inner ");
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

void
set_provoking_vertex(const char *line)
{
	if (string_match("first", line)) {
		glProvokingVertexEXT(GL_FIRST_VERTEX_CONVENTION_EXT);
	} else if (string_match("last", line)) {
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
	{ NULL, 0 }
};

void
do_enable_disable(const char *line, bool enable_flag)
{
	GLenum value = lookup_enum_string(enable_table, &line,
					  "enable/disable enum");
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

void do_hint(const char *line)
{
	GLenum target = lookup_enum_string(hint_target_table, &line,
					   "hint target");
	GLenum param = lookup_enum_string(hint_param_table, &line,
					  "hint param");
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


GLenum
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
	static const struct string_to_enum texture_target[] = {
		{ "1D",        GL_TEXTURE_1D             },
		{ "2D",        GL_TEXTURE_2D             },
		{ "3D",        GL_TEXTURE_3D             },
		{ "Rect",      GL_TEXTURE_RECTANGLE      },
		{ "Cube",      GL_TEXTURE_CUBE_MAP       },
		{ "1DArray",   GL_TEXTURE_1D_ARRAY       },
		{ "2DArray",   GL_TEXTURE_2D_ARRAY       },
		{ "CubeArray", GL_TEXTURE_CUBE_MAP_ARRAY },
		{ NULL, 0 }
	};

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
	static const struct string_to_enum swizzle_modes[] = {
		{ "red", GL_RED },
		{ "green", GL_GREEN },
		{ "blue", GL_BLUE },
		{ "alpha", GL_ALPHA },
		{ NULL, 0 }
	};
	GLenum target = 0;
	GLenum parameter = GL_NONE;
	const char *parameter_name = NULL;
	const struct string_to_enum *strings = NULL;
	GLenum value;

	target = lookup_enum_string(texture_target, &line, "texture target");

	if (string_match("compare_func ", line)) {
		parameter = GL_TEXTURE_COMPARE_FUNC;
		parameter_name = "compare_func";
		line += strlen("compare_func ");
		strings = compare_funcs;
	} else if (string_match("depth_mode ", line)) {
		parameter = GL_DEPTH_TEXTURE_MODE;
		parameter_name = "depth_mode";
		line += strlen("depth_mode ");
		strings = depth_modes;
	} else if (string_match("min ", line)) {
		parameter = GL_TEXTURE_MIN_FILTER;
		parameter_name = "min";
		line += strlen("min ");
		strings = min_filter_modes;
	} else if (string_match("mag ", line)) {
		parameter = GL_TEXTURE_MAG_FILTER;
		parameter_name = "mag";
		line += strlen("mag ");
		strings = mag_filter_modes;
	} else if (string_match("lod_bias ", line)) {
#ifdef PIGLIT_USE_OPENGL
		line += strlen("lod_bias ");
		glTexParameterf(target, GL_TEXTURE_LOD_BIAS,
				strtod(line, NULL));
		return;
#else
		printf("lod_bias feature is only available in desktop GL\n");
		piglit_report_result(PIGLIT_SKIP);
#endif
	} else if (string_match("max_level ", line)) {
		line += strlen("max_level ");
		glTexParameteri(target, GL_TEXTURE_MAX_LEVEL,
				strtol(line, NULL, 10));
		return;
	} else if (string_match("base_level ", line)) {
		line += strlen("base_level ");
		glTexParameteri(target, GL_TEXTURE_BASE_LEVEL,
				strtol(line, NULL, 10));
		return;
	} else if (string_match("swizzle_r ", line)) {
		parameter = GL_TEXTURE_SWIZZLE_R;
		parameter_name = "swizzle_r";
		line += strlen("swizzle_r ");
		strings = swizzle_modes;
	} else {
		fprintf(stderr, "unknown texture parameter in `%s'\n", line);
		piglit_report_result(PIGLIT_FAIL);
	}

	value = lookup_enum_string(strings, &line, parameter_name);
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

void
program_must_be_in_use(void)
{
	if (!link_ok) {
		fprintf(stderr, "Failed to link:\n%s\n", prog_err_info);
		piglit_report_result(PIGLIT_FAIL);
	} else if (!prog_in_use) {
		fprintf(stderr, "Failed to use program: %s\n", prog_err_info);
		piglit_report_result(PIGLIT_FAIL);
	}

}

void
bind_vao_if_supported()
{
	if (vao == 0 && gl_version.num >= 31) {
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
	}
}

static bool
probe_atomic_counter(GLint counter_num, const char *op, uint32_t value)
{
        uint32_t *p;
	enum comparison cmp;
	bool result;

	process_comparison(op, &cmp);

	glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, atomics_bo);
	p = glMapBufferRange(GL_ATOMIC_COUNTER_BUFFER, counter_num * sizeof(uint32_t),
			     sizeof(uint32_t), GL_MAP_READ_BIT);

        if (!p) {
                printf("Couldn't map atomic counter to verify expected value.\n");
                return false;
        }

	result = compare_uint(value, *p, cmp);

	if (!result) {
		printf("Atomic counter %d test failed: Reference %s Observed\n",
		       counter_num, comparison_string(cmp));
		printf("  Reference: %u\n", value);
		printf("  Observed:  %u\n", *p);
		glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);
		return false;
        }

        glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);
        return true;
}

enum piglit_result
piglit_display(void)
{
	const char *line;
	bool pass = true;
	GLbitfield clear_bits = 0;
	bool link_error_expected = false;
	int ubo_array_index = 0;

	if (test_start == NULL)
		return PIGLIT_PASS;

	line = test_start;
	while (line[0] != '\0') {
		float c[32];
		double d[4];
		int x, y, z, w, h, l, tex, level;
		char s[32];

		line = eat_whitespace(line);

		if (sscanf(line, "atomic counters %d", &x) == 1) {
			GLuint *atomics_buf = calloc(x, sizeof(GLuint));
			glGenBuffers(1, &atomics_bo);
			glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, atomics_bo);
			glBufferData(GL_ATOMIC_COUNTER_BUFFER,
				     sizeof(GLuint) * x,
				     atomics_buf, GL_STATIC_DRAW);
			free(atomics_buf);
		} else if (string_match("clear color", line)) {
			get_floats(line + 11, c, 4);
			glClearColor(c[0], c[1], c[2], c[3]);
			clear_bits |= GL_COLOR_BUFFER_BIT;
		} else if (string_match("clear", line)) {
			glClear(clear_bits);
		} else if (sscanf(line,
				  "clip plane %d %lf %lf %lf %lf",
				  &x, &d[0], &d[1], &d[2], &d[3])) {
			if (x < 0 || x >= gl_max_clip_planes) {
				printf("clip plane id %d out of range\n", x);
				piglit_report_result(PIGLIT_FAIL);
			}
			glClipPlane(GL_CLIP_PLANE0 + x, d);
		} else if (sscanf(line,
				  "compute %d %d %d",
				  &x, &y, &z) == 3) {
			program_must_be_in_use();
			glMemoryBarrier(GL_ALL_BARRIER_BITS);
			glDispatchCompute(x, y, z);
			glMemoryBarrier(GL_ALL_BARRIER_BITS);
		} else if (string_match("draw rect tex", line)) {
			program_must_be_in_use();
			get_floats(line + 13, c, 8);
			piglit_draw_rect_tex(c[0], c[1], c[2], c[3],
					     c[4], c[5], c[6], c[7]);
		} else if (string_match("draw rect ortho", line)) {
			program_must_be_in_use();
			get_floats(line + 15, c, 4);

			piglit_draw_rect(-1.0 + 2.0 * (c[0] / piglit_width),
					 -1.0 + 2.0 * (c[1] / piglit_height),
					 2.0 * (c[2] / piglit_width),
					 2.0 * (c[3] / piglit_height));
		} else if (string_match("draw rect", line)) {
			program_must_be_in_use();
			get_floats(line + 9, c, 4);
			piglit_draw_rect(c[0], c[1], c[2], c[3]);
		} else if (string_match("draw instanced rect", line)) {
			int primcount;

			program_must_be_in_use();
			sscanf(line + 19, "%d %f %f %f %f",
			       &primcount,
			       c + 0, c + 1, c + 2, c + 3);
			draw_instanced_rect(primcount, c[0], c[1], c[2], c[3]);
		} else if (sscanf(line, "draw arrays %31s %d %d", s, &x, &y)) {
			GLenum mode = decode_drawing_mode(s);
			int first = x;
			size_t count = (size_t) y;
			program_must_be_in_use();
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
			glDrawArrays(mode, first, count);
		} else if (string_match("disable", line)) {
			do_enable_disable(line + 7, false);
		} else if (string_match("enable", line)) {
			do_enable_disable(line + 6, true);
		} else if (sscanf(line, "fb tex 2d %d", &tex) == 1) {
			GLenum status;
			GLint tex_num;

			glActiveTexture(GL_TEXTURE0 + tex);
			glGetIntegerv(GL_TEXTURE_BINDING_2D, &tex_num);

			if (fbo == 0) {
				glGenFramebuffers(1, &fbo);
				glBindFramebuffer(GL_FRAMEBUFFER, fbo);
			}

			glFramebufferTexture2D(GL_FRAMEBUFFER,
					       GL_COLOR_ATTACHMENT0,
					       GL_TEXTURE_2D, tex_num, 0);
			if (!piglit_check_gl_error(GL_NO_ERROR)) {
				fprintf(stderr, "glFramebufferTexture2D error\n");
				piglit_report_result(PIGLIT_FAIL);
			}

			status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
			if (status != GL_FRAMEBUFFER_COMPLETE) {
				fprintf(stderr, "incomplete fbo (status 0x%x)\n", status);
				piglit_report_result(PIGLIT_FAIL);
			}

			glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &render_width);
			glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &render_height);
		} else if (sscanf(line, "fb tex layered 2DArray %d", &tex) == 1) {
			GLenum status;
			GLint tex_num;

			glActiveTexture(GL_TEXTURE0 + tex);
			glGetIntegerv(GL_TEXTURE_BINDING_2D_ARRAY, &tex_num);

			if (fbo == 0) {
				glGenFramebuffers(1, &fbo);
				glBindFramebuffer(GL_FRAMEBUFFER, fbo);
			}

			glFramebufferTexture(GL_FRAMEBUFFER,
					     GL_COLOR_ATTACHMENT0,
					     tex_num, 0);
			if (!piglit_check_gl_error(GL_NO_ERROR)) {
				fprintf(stderr, "glFramebufferTexture error\n");
				piglit_report_result(PIGLIT_FAIL);
			}

			status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
			if (status != GL_FRAMEBUFFER_COMPLETE) {
				fprintf(stderr, "incomplete fbo (status 0x%x)\n", status);
				piglit_report_result(PIGLIT_FAIL);
			}

			glGetTexLevelParameteriv(GL_TEXTURE_2D_ARRAY, 0, GL_TEXTURE_WIDTH, &render_width);
			glGetTexLevelParameteriv(GL_TEXTURE_2D_ARRAY, 0, GL_TEXTURE_HEIGHT, &render_height);
		} else if (string_match("frustum", line)) {
			get_floats(line + 7, c, 6);
			piglit_frustum_projection(false, c[0], c[1], c[2],
						  c[3], c[4], c[5]);
		} else if (string_match("hint", line)) {
			do_hint(line + 4);
		} else if (sscanf(line,
				  "image texture %d",
				  &tex) == 1) {
			GLint tex_num;

			glActiveTexture(GL_TEXTURE0 + tex);
			glGetIntegerv(GL_TEXTURE_BINDING_2D, &tex_num);
			glBindImageTexture(tex, tex_num, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8);
		} else if (sscanf(line, "ortho %f %f %f %f",
				  c + 0, c + 1, c + 2, c + 3) == 4) {
			piglit_gen_ortho_projection(c[0], c[1], c[2], c[3],
						    -1, 1, GL_FALSE);
		} else if (string_match("ortho", line)) {
			piglit_ortho_projection(render_width, render_height,
						GL_FALSE);
		} else if (string_match("probe rgba", line)) {
			get_floats(line + 10, c, 6);
			if (!piglit_probe_pixel_rgba((int) c[0], (int) c[1],
						    & c[2])) {
				pass = false;
			}
		} else if (sscanf(line,
				  "probe atomic counter %d %s %d",
				  &x, s, &y) == 3) {
			if (!probe_atomic_counter(x, s, y)) {
				piglit_report_result(PIGLIT_FAIL);
			}
		} else if (sscanf(line,
				  "relative probe rgba ( %f , %f ) "
				  "( %f , %f , %f , %f )",
				  c + 0, c + 1,
				  c + 2, c + 3, c + 4, c + 5) == 6) {
			x = c[0] * render_width;
			y = c[1] * render_height;
			if (x >= render_width)
				x = render_width - 1;
			if (y >= render_height)
				y = render_height - 1;

			if (!piglit_probe_pixel_rgba(x, y, &c[2])) {
				pass = false;
			}
		} else if (string_match("probe rgb", line)) {
			get_floats(line + 9, c, 5);
			if (!piglit_probe_pixel_rgb((int) c[0], (int) c[1],
						    & c[2])) {
				pass = false;
			}
		} else if (sscanf(line,
				  "relative probe rgb ( %f , %f ) "
				  "( %f , %f , %f )",
				  c + 0, c + 1,
				  c + 2, c + 3, c + 4) == 5) {
			x = c[0] * render_width;
			y = c[1] * render_height;
			if (x >= render_width)
				x = render_width - 1;
			if (y >= render_height)
				y = render_height - 1;

			if (!piglit_probe_pixel_rgb(x, y, &c[2])) {
				pass = false;
			}
		} else if (sscanf(line, "probe rect rgba "
				  "( %d , %d , %d , %d ) "
				  "( %f , %f , %f , %f )",
				  &x, &y, &w, &h,
				  c + 0, c + 1, c + 2, c + 3) == 8) {
			if (!piglit_probe_rect_rgba(x, y, w, h, c)) {
				pass = false;
			}
		} else if (sscanf(line, "relative probe rect rgb "
				  "( %f , %f , %f , %f ) "
				  "( %f , %f , %f )",
				  c + 0, c + 1, c + 2, c + 3,
				  c + 4, c + 5, c + 6) == 7) {
			x = c[0] * render_width;
			y = c[1] * render_height;
			w = c[2] * render_width;
			h = c[3] * render_height;

			if (!piglit_probe_rect_rgb(x, y, w, h, &c[4])) {
				pass = false;
			}
		} else if (string_match("probe all rgba", line)) {
			get_floats(line + 14, c, 4);
			pass = pass &&
				piglit_probe_rect_rgba(0, 0, render_width,
						       render_height, c);
		} else if (string_match("probe all rgb", line)) {
			get_floats(line + 13, c, 3);
			pass = pass &&
				piglit_probe_rect_rgb(0, 0, render_width,
						      render_height, c);
		} else if (string_match("tolerance", line)) {
			get_floats(line + strlen("tolerance"), piglit_tolerance, 4);
		} else if (string_match("shade model smooth", line)) {
			glShadeModel(GL_SMOOTH);
		} else if (string_match("shade model flat", line)) {
			glShadeModel(GL_FLAT);
		} else if (sscanf(line,
				  "texture rgbw %d ( %d , %d )",
				  &tex, &w, &h) == 3) {
			glActiveTexture(GL_TEXTURE0 + tex);
			piglit_rgbw_texture(GL_RGBA, w, h, GL_FALSE, GL_FALSE, GL_UNSIGNED_NORMALIZED);
			if (!piglit_is_core_profile)
				glEnable(GL_TEXTURE_2D);
		} else if (sscanf(line, "texture miptree %d", &tex) == 1) {
			glActiveTexture(GL_TEXTURE0 + tex);
			piglit_miptree_texture();
			if (!piglit_is_core_profile)
				glEnable(GL_TEXTURE_2D);
		} else if (sscanf(line,
				  "texture checkerboard %d %d ( %d , %d ) "
				  "( %f , %f , %f , %f ) "
				  "( %f , %f , %f , %f )",
				  &tex, &level, &w, &h,
				  c + 0, c + 1, c + 2, c + 3,
				  c + 4, c + 5, c + 6, c + 7) == 12) {
			glActiveTexture(GL_TEXTURE0 + tex);
			piglit_checkerboard_texture(0, level,
						    w, h,
						    w / 2, h / 2,
						    c + 0, c + 4);
			if (!piglit_is_core_profile)
				glEnable(GL_TEXTURE_2D);
		} else if (sscanf(line,
				  "texture junk 2DArray %d ( %d , %d , %d )",
				  &tex, &w, &h, &l) == 4) {
			GLuint texobj;
			glActiveTexture(GL_TEXTURE0 + tex);
			glGenTextures(1, &texobj);
			glBindTexture(GL_TEXTURE_2D_ARRAY, texobj);
			glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA,
				     w, h, l, 0, GL_RGBA, GL_FLOAT, 0);
		} else if (sscanf(line,
				  "texture rgbw 2DArray %d ( %d , %d , %d )",
				  &tex, &w, &h, &l) == 4) {
			glActiveTexture(GL_TEXTURE0 + tex);
			piglit_array_texture(GL_TEXTURE_2D_ARRAY, GL_RGBA,
                                             w, h, l, GL_FALSE);
		} else if (sscanf(line,
				  "texture rgbw 1DArray %d ( %d , %d )",
				  &tex, &w, &l) == 3) {
			glActiveTexture(GL_TEXTURE0 + tex);
                        h = 1;
			piglit_array_texture(GL_TEXTURE_1D_ARRAY, GL_RGBA,
                                             w, h, l, GL_FALSE);
		} else if (sscanf(line,
				  "texture shadow2D %d ( %d , %d )",
				  &tex, &w, &h) == 3) {
			glActiveTexture(GL_TEXTURE0 + tex);
			piglit_depth_texture(GL_TEXTURE_2D, GL_DEPTH_COMPONENT,
					     w, h, 1, GL_FALSE);
			glTexParameteri(GL_TEXTURE_2D,
					GL_TEXTURE_COMPARE_MODE,
					GL_COMPARE_R_TO_TEXTURE);
			glTexParameteri(GL_TEXTURE_2D,
					GL_TEXTURE_COMPARE_FUNC,
					GL_GREATER);

			if (!piglit_is_core_profile)
				glEnable(GL_TEXTURE_2D);
		} else if (sscanf(line,
				  "texture shadowRect %d ( %d , %d )",
				  &tex, &w, &h) == 3) {
			glActiveTexture(GL_TEXTURE0 + tex);
			piglit_depth_texture(GL_TEXTURE_RECTANGLE, GL_DEPTH_COMPONENT,
					     w, h, 1, GL_FALSE);
			glTexParameteri(GL_TEXTURE_RECTANGLE,
					GL_TEXTURE_COMPARE_MODE,
					GL_COMPARE_R_TO_TEXTURE);
			glTexParameteri(GL_TEXTURE_RECTANGLE,
					GL_TEXTURE_COMPARE_FUNC,
					GL_GREATER);
		} else if (sscanf(line,
				  "texture shadow1D %d ( %d )",
				  &tex, &w) == 2) {
			glActiveTexture(GL_TEXTURE0 + tex);
			piglit_depth_texture(GL_TEXTURE_1D, GL_DEPTH_COMPONENT,
					     w, 1, 1, GL_FALSE);
			glTexParameteri(GL_TEXTURE_1D,
					GL_TEXTURE_COMPARE_MODE,
					GL_COMPARE_R_TO_TEXTURE);
			glTexParameteri(GL_TEXTURE_1D,
					GL_TEXTURE_COMPARE_FUNC,
					GL_GREATER);
		} else if (sscanf(line,
				  "texture shadow1DArray %d ( %d , %d )",
				  &tex, &w, &l) == 3) {
			glActiveTexture(GL_TEXTURE0 + tex);
			piglit_depth_texture(GL_TEXTURE_1D_ARRAY, GL_DEPTH_COMPONENT,
					     w, l, 1, GL_FALSE);
			glTexParameteri(GL_TEXTURE_1D_ARRAY,
					GL_TEXTURE_COMPARE_MODE,
					GL_COMPARE_R_TO_TEXTURE);
			glTexParameteri(GL_TEXTURE_1D_ARRAY,
					GL_TEXTURE_COMPARE_FUNC,
					GL_GREATER);
		} else if (sscanf(line,
				  "texture shadow2DArray %d ( %d , %d , %d )",
				  &tex, &w, &h, &l) == 4) {
			glActiveTexture(GL_TEXTURE0 + tex);
			piglit_depth_texture(GL_TEXTURE_2D_ARRAY, GL_DEPTH_COMPONENT,
					     w, h, l, GL_FALSE);
			glTexParameteri(GL_TEXTURE_2D_ARRAY,
					GL_TEXTURE_COMPARE_MODE,
					GL_COMPARE_R_TO_TEXTURE);
			glTexParameteri(GL_TEXTURE_2D_ARRAY,
					GL_TEXTURE_COMPARE_FUNC,
					GL_GREATER);
		} else if (string_match("texparameter ", line)) {
			handle_texparameter(line + strlen("texparameter "));
		} else if (string_match("uniform", line)) {
			program_must_be_in_use();
			set_uniform(line + 7, ubo_array_index);
		} else if (string_match("parameter ", line)) {
			set_parameter(line + strlen("parameter "));
		} else if (string_match("patch parameter ", line)) {
			set_patch_parameter(line + strlen("patch parameter "));
		} else if (string_match("provoking vertex ", line)) {
			set_provoking_vertex(line + strlen("provoking vertex "));
		} else if (string_match("link error", line)) {
			link_error_expected = true;
			if (link_ok) {
				printf("shader link error expected, but it was successful!\n");
				piglit_report_result(PIGLIT_FAIL);
			}
		} else if (string_match("link success", line)) {
			program_must_be_in_use();
		} else if (string_match("ubo array index ", line)) {
			get_ints(line + strlen("ubo array index "), &ubo_array_index, 1);
		} else if (string_match("active uniform ", line)) {
			active_uniform(line + strlen("active uniform "));
		} else if ((line[0] != '\n') && (line[0] != '\0')
			   && (line[0] != '#')) {
			printf("unknown command \"%s\"\n", line);
			piglit_report_result(PIGLIT_FAIL);
		}

		line = strchrnul(line, '\n');
		if (line[0] != '\0')
			line++;
	}

	if (!link_ok && !link_error_expected) {
		program_must_be_in_use();
	}

	piglit_present_results();

	if (piglit_automatic) {
		/* Free our resources, useful for valgrinding. */
		glDeleteProgram(prog);
		glUseProgram(0);
	}

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


void
piglit_init(int argc, char **argv)
{
	int major;
	int minor;
	bool core = piglit_is_core_profile;
	bool es;

	piglit_require_GLSL();

	version_init(&gl_version, VERSION_GL,
		     core,
	             piglit_is_gles(),
	             piglit_get_gl_version());
	piglit_get_glsl_version(&es, &major, &minor);
	version_init(&glsl_version, VERSION_GLSL, core, es,
	             (major * 100) + minor);

#ifdef PIGLIT_USE_OPENGL
	glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS,
		      &gl_max_fragment_uniform_components);
	glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS,
		      &gl_max_vertex_uniform_components);
	glGetIntegerv(GL_MAX_VARYING_COMPONENTS,
		      &gl_max_varying_components);
	glGetIntegerv(GL_MAX_CLIP_PLANES, &gl_max_clip_planes);
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
	gl_max_clip_planes = 0;
#endif
	if (argc < 2) {
		printf("usage: shader_runner <test.shader_test>\n");
		exit(1);
	}

	process_test_script(argv[1]);
	link_and_use_shaders();
	if (link_ok && vertex_data_start != NULL) {
		program_must_be_in_use();
		bind_vao_if_supported();

		num_vbo_rows = setup_vbo_from_text(prog, vertex_data_start,
						   vertex_data_end);
		vbo_present = true;
	}
	setup_ubos();

	render_width = piglit_width;
	render_height = piglit_height;
}
