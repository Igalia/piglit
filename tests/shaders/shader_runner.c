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

#define _GNU_SOURCE
#if defined(_MSC_VER)
#define bool BOOL
#define true 1
#define false 0
#else
#include <stdbool.h>
#endif
#include <string.h>
#include <ctype.h>
#if defined(_WIN32)
#include <stdlib.h>
#else
#include <libgen.h>
#endif
#include "piglit-util.h"

int piglit_width = 250, piglit_height = 250;
int piglit_window_mode = GLUT_RGB | GLUT_ALPHA | GLUT_DOUBLE;

static float gl_version = 0.0;
static float glsl_version = 0.0;
static int gl_max_fragment_uniform_components;
static int gl_max_vertex_uniform_components;

const char *path = NULL;
const char *test_start = NULL;

GLuint vertex_shaders[256];
unsigned num_vertex_shaders = 0;
GLuint geometry_shaders[256];
unsigned num_geometry_shaders = 0;
GLuint fragment_shaders[256];
unsigned num_fragment_shaders = 0;

/**
 * List of strings loaded from files
 *
 * Some test script sections, such as "[vertex shader file]", can supply shader
 * source code from multiple disk files.  This array stores those strings.
 */
char *shader_strings[256];
GLsizei shader_string_sizes[256];
unsigned num_shader_strings = 0;
GLuint prog;

enum states {
	none = 0,
	requirements,
	vertex_shader,
	vertex_shader_file,
	vertex_program,
	geometry_shader,
	geometry_shader_file,
	geometry_program,
	fragment_shader,
	fragment_shader_file,
	fragment_program,
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


void
compile_glsl(GLenum target, bool release_text)
{
	GLuint shader = piglit_CreateShader(target);
	GLint ok;
	unsigned i;

	switch (target) {
	case GL_VERTEX_SHADER:
		piglit_require_vertex_shader();
		break;
	case GL_FRAGMENT_SHADER:
		piglit_require_fragment_shader();
		break;
	case GL_GEOMETRY_SHADER_ARB:
		if (gl_version < 3.2)
			piglit_require_extension("GL_ARB_geometry_shader4");
		break;
	}

	piglit_ShaderSource(shader, num_shader_strings,
			    (const GLchar **) shader_strings,
			    shader_string_sizes);

	piglit_CompileShader(shader);

	piglit_GetShaderiv(shader, GL_COMPILE_STATUS, &ok);

	if (!ok) {
		GLchar *info;
		GLint size;

		piglit_GetShaderiv(shader, GL_INFO_LOG_LENGTH, &size);
		info = malloc(size);

		piglit_GetShaderInfoLog(shader, size, NULL, info);

		fprintf(stderr, "Failed to compile %s: %s\n",
			target == GL_FRAGMENT_SHADER ? "FS" : "VS",
			info);

		free(info);
		piglit_report_result(PIGLIT_FAIL);
	}

	if (release_text) {
		for (i = 0; i < num_shader_strings; i++)
			free(shader_strings[i]);
	}

	switch (target) {
	case GL_VERTEX_SHADER:
		vertex_shaders[num_vertex_shaders] = shader;
		num_vertex_shaders++;
		break;
	case GL_GEOMETRY_SHADER_ARB:
		geometry_shaders[num_geometry_shaders] = shader;
		num_geometry_shaders++;
		break;
	case GL_FRAGMENT_SHADER:
		fragment_shaders[num_fragment_shaders] = shader;
		num_fragment_shaders++;
		break;
	}
}


/**
 * Copy a string until either whitespace or the end of the string
 */
const char *
strcpy_to_space(char *dst, const char *src)
{
	while (!isspace(*src) && (*src != '\0'))
		*(dst++) = *(src++);

	*dst = '\0';
	return src;
}


/**
 * Skip over whitespace upto the end of line
 */
const char *
eat_whitespace(const char *src)
{
	while (isspace(*src) && (*src != '\n'))
		src++;

	return src;
}


/**
 * Skip over non-whitespace upto the end of line
 */
const char *
eat_text(const char *src)
{
	while (!isspace(*src) && (*src != '\0'))
		src++;

	return src;
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
}


void
load_shader_file(const char *line)
{
	GLsizei *const size = &shader_string_sizes[num_shader_strings];
	char buf[256];
	char *text;

	strcpy_to_space(buf, line);

	text = piglit_load_text_file(buf, (unsigned *) size);
	if ((text == NULL) && (path != NULL)) {
		const size_t len = strlen(path);

		memcpy(buf, path, len);
		buf[len] = '/';
		strcpy_to_space(&buf[len + 1], line);

		text = piglit_load_text_file(buf, (unsigned *) size);
	}

	if (text == NULL) {
		strcpy_to_space(buf, line);

		printf("could not load file \"%s\"\n", buf);
		piglit_report_result(PIGLIT_FAIL);
	}

	shader_strings[num_shader_strings] = text;
	num_shader_strings++;
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
 * Parse and check a line from the requirement section of the test
 */
void
process_requirement(const char *line)
{
	char buffer[4096];

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
	if (strncmp("GL_MAX_FRAGMENT_UNIFORM_COMPONENTS", line, 34) == 0) {
		enum comparison cmp;
		int maxcomp;

		line = eat_whitespace(line + 34);

		line = process_comparison(line, &cmp);

		maxcomp = atoi(line);
		if (!compare(maxcomp, gl_max_fragment_uniform_components, cmp)) {
			printf("Test requires max fragment uniform components %s %i.  "
			       "The driver supports %i.\n",
			       comparison_string(cmp),
			       maxcomp,
			       gl_max_fragment_uniform_components);
			piglit_report_result(PIGLIT_SKIP);
		}
	} else if (strncmp("GL_MAX_VERTEX_UNIFORM_COMPONENTS", line, 32) == 0) {
		enum comparison cmp;
		int maxcomp;

		line = eat_whitespace(line + 32);

		line = process_comparison(line, &cmp);

		maxcomp = atoi(line);
		if (!compare(maxcomp, gl_max_vertex_uniform_components, cmp)) {
			printf("Test requires max vertex uniform components %s %i.  "
			       "The driver supports %i.\n",
			       comparison_string(cmp),
			       maxcomp,
			       gl_max_vertex_uniform_components);
			piglit_report_result(PIGLIT_SKIP);
		}
	} else if (strncmp("GL_", line, 3) == 0) {
		strcpy_to_space(buffer, line);
		piglit_require_extension(buffer);
	} else if (strncmp("!GL_", line, 4) == 0) {
		strcpy_to_space(buffer, line + 1);
		piglit_require_not_extension(buffer);
	} else if (strncmp("GLSL", line, 4) == 0) {
		enum comparison cmp;
		float version;

		line = eat_whitespace(line + 4);

		line = process_comparison(line, &cmp);

		version = strtod(line, NULL);
		if (!compare(version, glsl_version, cmp)) {
			printf("Test requires GLSL version %s %.1f.  "
			       "Actual version is %.1f.\n",
			       comparison_string(cmp),
			       version,
			       glsl_version);
			piglit_report_result(PIGLIT_SKIP);
		}
	} else if (strncmp("GL", line, 2) == 0) {
		enum comparison cmp;
		float version;

		line = eat_whitespace(line + 2);

		line = process_comparison(line, &cmp);

		version = strtod(line, NULL);
		if (!compare(version, gl_version, cmp)) {
			printf("Test requires GL version %s %.1f.  "
			       "Actual version is %.1f.\n",
			       comparison_string(cmp),
			       version,
			       gl_version);
			piglit_report_result(PIGLIT_SKIP);
		}
	} else if (strncmp("rlimit", line, 6) == 0) {
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


void
leave_state(enum states state, const char *line)
{
	switch (state) {
	case none:
		break;

	case requirements:
		break;

	case vertex_shader:
		shader_string_sizes[0] = line - shader_strings[0];
		num_shader_strings = 1;
		compile_glsl(GL_VERTEX_SHADER, false);
		break;

	case vertex_shader_file:
		compile_glsl(GL_VERTEX_SHADER, true);
		break;

	case vertex_program:
		break;

	case geometry_shader:
		break;

	case geometry_program:
		break;

	case fragment_shader:
		shader_string_sizes[0] = line - shader_strings[0];
		num_shader_strings = 1;
		compile_glsl(GL_FRAGMENT_SHADER, false);
		break;

	case fragment_shader_file:
		compile_glsl(GL_FRAGMENT_SHADER, true);
		break;

	case fragment_program:
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
	    && (num_geometry_shaders == 0))
		return;

	prog = piglit_CreateProgram();

	for (i = 0; i < num_vertex_shaders; i++) {
		piglit_AttachShader(prog, vertex_shaders[i]);
	}

	for (i = 0; i < num_geometry_shaders; i++) {
		piglit_AttachShader(prog, geometry_shaders[i]);
	}

	for (i = 0; i < num_fragment_shaders; i++) {
		piglit_AttachShader(prog, fragment_shaders[i]);
	}

	piglit_LinkProgram(prog);

	for (i = 0; i < num_vertex_shaders; i++) {
		piglit_DeleteShader(vertex_shaders[i]);
	}

	for (i = 0; i < num_geometry_shaders; i++) {
		piglit_DeleteShader(geometry_shaders[i]);
	}

	for (i = 0; i < num_fragment_shaders; i++) {
		piglit_DeleteShader(fragment_shaders[i]);
	}

	piglit_GetProgramiv(prog, GL_LINK_STATUS, &ok);
	if (!ok) {
		GLchar *info;
		GLint size;

		piglit_GetProgramiv(prog, GL_INFO_LOG_LENGTH, &size);
		info = malloc(size);

		piglit_GetProgramInfoLog(prog, size, NULL, info);

		fprintf(stderr, "Failed to link:\n%s\n",
			info);

		free(info);
		piglit_report_result(PIGLIT_FAIL);
	}

	piglit_UseProgram(prog);

	err = glGetError();
	if (err) {
		GLchar *info;
		GLint size;

		printf("GL error after linking program: 0x%04x\n", err);

		piglit_GetProgramiv(prog, GL_INFO_LOG_LENGTH, &size);
		info = malloc(size);

		piglit_GetProgramInfoLog(prog, size, NULL, info);
		fprintf(stderr, "Info log: %s\n", info);

		piglit_report_result(PIGLIT_FAIL);
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

			if (strncmp(line, "[require]", 9) == 0) {
				state = requirements;
			} else if (strncmp(line, "[vertex shader]", 15) == 0) {
				state = vertex_shader;
				shader_strings[0] = NULL;
			} else if (strncmp(line, "[vertex shader file]", 20) == 0) {
				state = vertex_shader_file;
				shader_strings[0] = NULL;
				num_shader_strings = 0;
			} else if (strncmp(line, "[fragment shader]", 17) == 0) {
				state = fragment_shader;
				shader_strings[0] = NULL;
			} else if (strncmp(line, "[fragment shader file]", 22) == 0) {
				state = fragment_shader_file;
				shader_strings[0] = NULL;
				num_shader_strings = 0;
			} else if (strncmp(line, "[test]", 6) == 0) {
				test_start = strchrnul(line, '\n');
				if (test_start[0] != '\0')
					test_start++;
				return;
			}
		} else {
			switch (state) {
			case none:
				break;

			case requirements:
				process_requirement(line);
				break;

			case vertex_shader:
			case vertex_program:
			case geometry_shader:
			case geometry_program:
			case fragment_shader:
			case fragment_program:
				if (shader_strings[0] == NULL)
					shader_strings[0] = (char *) line;
				break;

			case vertex_shader_file:
			case geometry_shader_file:
			case fragment_shader_file:
				line = eat_whitespace(line);
				if ((line[0] != '\n') && (line[0] != '#'))
				    load_shader_file(line);
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


void
get_floats(const char *line, float *f, unsigned count)
{
	unsigned i;

	for (i = 0; i < count; i++)
		f[i] = strtod(line, (char **) &line);
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


void
set_uniform(const char *line)
{
	char name[512];
	float f[16];
	int ints[16];
	unsigned uints[16];
	GLuint prog;
	GLint loc;
	const char *type;

	glGetIntegerv(GL_CURRENT_PROGRAM, (GLint *) &prog);

	type = eat_whitespace(line);
	line = eat_text(type);

	line = strcpy_to_space(name, eat_whitespace(line));
	loc = piglit_GetUniformLocation(prog, name);
	if (loc < 0) {
		printf("cannot get location of uniform \"%s\"\n",
		       name);
		piglit_report_result(PIGLIT_FAIL);
	}

	if (strncmp("float", type, 5) == 0) {
		get_floats(line, f, 1);
		piglit_Uniform1fv(loc, 1, f);
		return;
	} else if (strncmp("int", type, 3) == 0) {
		int val = atoi(line);
		piglit_Uniform1i(loc, val);
		return;
	} else if (strncmp("uint", type, 4) == 0) {
		unsigned val = strtoul(line, NULL, 0);
		piglit_Uniform1ui(loc, val);
		return;
	} else if (strncmp("vec", type, 3) == 0) {
		switch (type[3]) {
		case '2':
			get_floats(line, f, 2);
			piglit_Uniform2fv(loc, 1, f);
			return;
		case '3':
			get_floats(line, f, 3);
			piglit_Uniform3fv(loc, 1, f);
			return;
		case '4':
			get_floats(line, f, 4);
			piglit_Uniform4fv(loc, 1, f);
			return;
		}
	} else if (strncmp("ivec", type, 4) == 0) {
		switch (type[4]) {
		case '2':
			get_ints(line, ints, 2);
			piglit_Uniform2iv(loc, 1, ints);
			return;
		case '3':
			get_ints(line, ints, 3);
			piglit_Uniform3iv(loc, 1, ints);
			return;
		case '4':
			get_ints(line, ints, 4);
			piglit_Uniform4iv(loc, 1, ints);
			return;
		}
	} else if (strncmp("uvec", type, 4) == 0) {
		switch (type[4]) {
		case '2':
			get_uints(line, uints, 2);
			piglit_Uniform2uiv(loc, 1, uints);
			return;
		case '3':
			get_uints(line, uints, 3);
			piglit_Uniform3uiv(loc, 1, uints);
			return;
		case '4':
			get_uints(line, uints, 4);
			piglit_Uniform4uiv(loc, 1, uints);
			return;
		}
	} else if ((strncmp("mat", type, 3) == 0)
		   && (type[3] != '\0')
		   && (type[4] == 'x')) {
		switch (type[3]) {
		case '2':
			switch (type[5]) {
			case '2':
				get_floats(line, f, 4);
				piglit_UniformMatrix2fv(loc, 1, GL_FALSE, f);
				return;
			case '3':
				get_floats(line, f, 6);
				piglit_UniformMatrix2x3fv(loc, 1, GL_FALSE, f);
				return;
			case '4':
				get_floats(line, f, 8);
				piglit_UniformMatrix2x4fv(loc, 1, GL_FALSE, f);
				return;
			}
		case '3':
			switch (type[5]) {
			case '2':
				get_floats(line, f, 6);
				piglit_UniformMatrix3x2fv(loc, 1, GL_FALSE, f);
				return;
			case '3':
				get_floats(line, f, 9);
				piglit_UniformMatrix3fv(loc, 1, GL_FALSE, f);
				return;
			case '4':
				get_floats(line, f, 12);
				piglit_UniformMatrix3x4fv(loc, 1, GL_FALSE, f);
				return;
			}
		case '4':
			switch (type[5]) {
			case '2':
				get_floats(line, f, 8);
				piglit_UniformMatrix4x2fv(loc, 1, GL_FALSE, f);
				return;
			case '3':
				get_floats(line, f, 12);
				piglit_UniformMatrix4x3fv(loc, 1, GL_FALSE, f);
				return;
			case '4':
				get_floats(line, f, 16);
				piglit_UniformMatrix4fv(loc, 1, GL_FALSE, f);
				return;
			}
		}
	}

	strcpy_to_space(name, type);
	printf("unknown uniform type \"%s\"", name);
	piglit_report_result(PIGLIT_FAIL);

	return;
}

struct enable_table {
	const char *name;
	GLenum value;
} enable_table[] = {
	{ "GL_CLIP_PLANE0", GL_CLIP_PLANE0 },
	{ "GL_CLIP_PLANE1", GL_CLIP_PLANE1 },
	{ "GL_CLIP_PLANE2", GL_CLIP_PLANE2 },
	{ "GL_CLIP_PLANE3", GL_CLIP_PLANE3 },
	{ "GL_CLIP_PLANE4", GL_CLIP_PLANE4 },
	{ "GL_CLIP_PLANE5", GL_CLIP_PLANE5 },
	{ NULL, 0 }
};

void
do_enable_disable(const char *line, bool enable_flag)
{
	char name[512];
	int i;

	strcpy_to_space(name, eat_whitespace(line));
	for (i = 0; enable_table[i].name; ++i) {
		if (0 == strcmp(name, enable_table[i].name)) {
			if (enable_flag) {
				glEnable(enable_table[i].value);
			} else {
				glDisable(enable_table[i].value);
			}
			return;
		}
	}

	printf("unknown enable/disable enum \"%s\"", name);
	piglit_report_result(PIGLIT_FAIL);
}

static GLboolean
string_match(const char *string, const char *line)
{
	return (strncmp(string, line, strlen(string)) == 0);
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
	verts[2][0] = x + w;
	verts[2][1] = y + h;
	verts[2][2] = 0.0;
	verts[2][3] = 1.0;
	verts[3][0] = x;
	verts[3][1] = y + h;
	verts[3][2] = 0.0;
	verts[3][3] = 1.0;

	glVertexPointer(4, GL_FLOAT, 0, verts);
	glEnableClientState(GL_VERTEX_ARRAY);

	glDrawArraysInstancedARB(GL_QUADS, 0, 4, primcount);

	glDisableClientState(GL_VERTEX_ARRAY);
}

enum piglit_result
piglit_display(void)
{
	const char *line;
	bool pass = true;
	GLbitfield clear_bits = 0;

	if (test_start == NULL)
		return PIGLIT_PASS;


	line = test_start;
	while (line[0] != '\0') {
		float c[32];
		double d[4];
		int x, y, w, h, tex, level;

		line = eat_whitespace(line);

		if (string_match("clear color", line)) {
			get_floats(line + 11, c, 4);
			glClearColor(c[0], c[1], c[2], c[3]);
			clear_bits |= GL_COLOR_BUFFER_BIT;
		} else if (string_match("clear", line)) {
			glClear(clear_bits);
		} else if (sscanf(line,
				  "clip plane %d %lf %lf %lf %lf",
				  &x, &d[0], &d[1], &d[2], &d[3])) {
			if (x < 0 || x >= GL_MAX_CLIP_PLANES) {
				printf("clip plane id %d out of range", x);
				piglit_report_result(PIGLIT_FAIL);
			}
			glClipPlane(GL_CLIP_PLANE0 + x, d);
		} else if (string_match("draw rect", line)) {
			get_floats(line + 9, c, 4);
			piglit_draw_rect(c[0], c[1], c[2], c[3]);
		} else if (string_match("draw instanced rect", line)) {
			int primcount;

			sscanf(line + 19, "%d %f %f %f %f",
			       &primcount,
			       c + 0, c + 1, c + 2, c + 3);
			draw_instanced_rect(primcount, c[0], c[1], c[2], c[3]);
		} else if (string_match("disable", line)) {
			do_enable_disable(line + 7, false);
		} else if (string_match("enable", line)) {
			do_enable_disable(line + 6, true);
		} else if (sscanf(line, "ortho %f %f %f %f",
				  c + 0, c + 1, c + 2, c + 3) == 4) {
			piglit_gen_ortho_projection(c[0], c[1], c[2], c[3],
						    -1, 1, GL_FALSE);
		} else if (string_match("ortho", line)) {
			piglit_ortho_projection(piglit_width, piglit_height,
						GL_FALSE);
		} else if (string_match("probe rgba", line)) {
			get_floats(line + 10, c, 6);
			if (!piglit_probe_pixel_rgba((int) c[0], (int) c[1],
						    & c[2])) {
				pass = false;
			}
		} else if (sscanf(line,
				  "relative probe rgba ( %f , %f ) "
				  "( %f , %f , %f , %f )",
				  c + 0, c + 1,
				  c + 2, c + 3, c + 4, c + 5) == 6) {
			x = c[0] * piglit_width;
			y = c[1] * piglit_width;
			if (x >= piglit_width)
				x = piglit_width - 1;
			if (y >= piglit_height)
				y = piglit_height - 1;

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
			x = c[0] * piglit_width;
			y = c[1] * piglit_width;
			if (x >= piglit_width)
				x = piglit_width - 1;
			if (y >= piglit_height)
				y = piglit_height - 1;

			if (!piglit_probe_pixel_rgb(x, y, &c[2])) {
				pass = false;
			}
		} else if (string_match("probe all rgba", line)) {
			get_floats(line + 14, c, 4);
			pass = pass &&
				piglit_probe_rect_rgba(0, 0, piglit_width,
						       piglit_height, c);
		} else if (string_match("probe all rgb", line)) {
			get_floats(line + 13, c, 3);
			pass = pass &&
				piglit_probe_rect_rgb(0, 0, piglit_width,
						      piglit_height, c);
		} else if (sscanf(line,
				  "texture rgbw %d ( %d , %d )",
				  &tex, &w, &h) == 3) {
			glActiveTexture(GL_TEXTURE0 + tex);
			piglit_rgbw_texture(GL_RGBA, w, h, GL_FALSE, GL_FALSE, GL_UNSIGNED_NORMALIZED);
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
			glEnable(GL_TEXTURE_2D);
		} else if (sscanf(line,
				  "texture shadow %d ( %d , %d )",
				  &tex, &w, &h) == 3) {
			glActiveTexture(GL_TEXTURE0 + tex);
			piglit_depth_texture(GL_DEPTH_COMPONENT,
					     w, h, GL_FALSE);
			glTexParameteri(GL_TEXTURE_2D,
					GL_TEXTURE_COMPARE_MODE_ARB,
					GL_COMPARE_R_TO_TEXTURE_ARB);
			glTexParameteri(GL_TEXTURE_2D,
					GL_TEXTURE_COMPARE_FUNC_ARB,
					GL_GREATER);
			glTexParameteri(GL_TEXTURE_2D,
					GL_DEPTH_TEXTURE_MODE_ARB,
					GL_INTENSITY);

			glEnable(GL_TEXTURE_2D);
		} else if (!strncmp(line,
				    "texparameter compare_func greater\n",
				    34)) {
			glTexParameteri(GL_TEXTURE_2D,
					GL_TEXTURE_COMPARE_FUNC_ARB,
					GL_GREATER);
		} else if (!strncmp(line,
				    "texparameter compare_func gequal\n",
				    33)) {
			glTexParameteri(GL_TEXTURE_2D,
					GL_TEXTURE_COMPARE_FUNC_ARB,
					GL_GEQUAL);
		} else if (!strncmp(line,
				    "texparameter compare_func less\n",
				    31)) {
			glTexParameteri(GL_TEXTURE_2D,
					GL_TEXTURE_COMPARE_FUNC_ARB,
					GL_LESS);
		} else if (!strncmp(line,
				    "texparameter compare_func lequal\n",
				    33)) {
			glTexParameteri(GL_TEXTURE_2D,
					GL_TEXTURE_COMPARE_FUNC_ARB,
					GL_LEQUAL);
		} else if (!strncmp(line,
				    "texparameter compare_func equal\n",
				    32)) {
			glTexParameteri(GL_TEXTURE_2D,
					GL_TEXTURE_COMPARE_FUNC_ARB,
					GL_EQUAL);
		} else if (!strncmp(line,
				    "texparameter compare_func notequal\n",
				    35)) {
			glTexParameteri(GL_TEXTURE_2D,
					GL_TEXTURE_COMPARE_FUNC_ARB,
					GL_NOTEQUAL);
		} else if (!strncmp(line,
				    "texparameter compare_func always\n",
				    33)) {
			glTexParameteri(GL_TEXTURE_2D,
					GL_TEXTURE_COMPARE_FUNC_ARB,
					GL_ALWAYS);
		} else if (!strncmp(line,
				    "texparameter compare_func never\n",
				    32)) {
			glTexParameteri(GL_TEXTURE_2D,
					GL_TEXTURE_COMPARE_FUNC_ARB,
					GL_NEVER);
		} else if (!strncmp(line,
			           "texparameter depth_mode red\n",
			            28)) {
			/* Requires GL 3.0 or GL_ARB_texture_rg */
			glTexParameteri(GL_TEXTURE_2D,
				        GL_DEPTH_TEXTURE_MODE_ARB,
				        GL_RED);
		} else if (!strncmp(line,
			           "texparameter depth_mode luminance\n",
			            34)) {
			glTexParameteri(GL_TEXTURE_2D,
				        GL_DEPTH_TEXTURE_MODE_ARB,
				        GL_LUMINANCE);
		} else if (!strncmp(line,
			           "texparameter depth_mode intensity\n",
			            34)) {
			glTexParameteri(GL_TEXTURE_2D,
				        GL_DEPTH_TEXTURE_MODE_ARB,
				        GL_INTENSITY);
		} else if (!strncmp(line,
			           "texparameter depth_mode alpha\n",
			            29)) {
			glTexParameteri(GL_TEXTURE_2D,
				        GL_DEPTH_TEXTURE_MODE_ARB,
				        GL_ALPHA);
		} else if (string_match("uniform", line)) {
			set_uniform(line + 7);
		} else if ((line[0] != '\n') && (line[0] != '\0')
			   && (line[0] != '#')) {
			printf("unknown command \"%s\"", line);
			piglit_report_result(PIGLIT_FAIL);
		}

		line = strchrnul(line, '\n');
		if (line[0] != '\0')
			line++;
	}

	glutSwapBuffers();

	if (piglit_automatic) {
		/* Free our resources, useful for valgrinding. */
		piglit_DeleteProgram(prog);
		piglit_UseProgram(0);
	}

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


void
piglit_init(int argc, char **argv)
{
	const char *glsl_version_string;

	piglit_require_GLSL();

	gl_version = strtod((char *) glGetString(GL_VERSION), NULL);

	glsl_version_string = (char *)
		glGetString(GL_SHADING_LANGUAGE_VERSION);
	glsl_version = (glsl_version_string == NULL)
		? 0.0 : strtod(glsl_version_string, NULL);

	glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS,
		      &gl_max_fragment_uniform_components);
	glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS,
		      &gl_max_vertex_uniform_components);

	if (argc > 2) {
		path = argv[2];
	} else {
#if defined(_WIN32)
		char drive[_MAX_DRIVE];
		char dir[_MAX_DIR];
		char fname[_MAX_FNAME];
		char ext[_MAX_EXT];
		char* scriptpath;
		_splitpath(argv[1], drive, dir, fname, ext);
		scriptpath = malloc(strlen(drive) + strlen(dir) + 1);
		strcpy(scriptpath, drive);
		strcat(scriptpath, dir);
		path = scriptpath;
#else
		/* Because dirname()'s memory handling is unpredictable, we
		 * must copy both its input and ouput. */
		char* scriptpath = strdup(argv[1]);
		path = strdup(dirname(scriptpath));
		free(scriptpath);
#endif
	}

	process_test_script(argv[1]);
	link_and_use_shaders();
}
