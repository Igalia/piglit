/*
 * Copyright © 2013 Intel Corporation
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

#include "piglit-util-gl.h"
#include "parser_utils.h"

static void parse_file(const char *filename);

struct test_vector {
	const char *name;
	int minimum;
};

struct test_vector tests[500];
unsigned num_tests = 0;

int required_glsl_version = 0;
char *required_glsl_version_string = NULL;
GLenum shader_type = 0;

/**
 * List of extensions required by the current test set.
 */
char *required_extensions[32];
unsigned num_required_extensions = 0;

/**
 * Array of extension enables for the shader code
 *
 * For each used entry in \c required_extensions, there is text in
 * this string of the form "#extension ...: require\n".
 */
#define MAX_EXTENSION_ENABLE_LINE_LEN 80
char extension_enables[ARRAY_SIZE(required_extensions)
		       * MAX_EXTENSION_ENABLE_LINE_LEN];
unsigned extension_enables_len = 0;

static const char *const uniform_template =
	"uniform float f[%s %s %d ? 1 : -1];\n"
	;

static const char *const vertex_shader_body =
	"void main() { gl_Position = vec4(f[0]); }\n"
	;

static const char *const geometry_shader_body =
	"layout(points) in;\n"
	"layout(points, max_vertices = 1) out;\n"
	"void main() { gl_Position = vec4(f[0]); EmitVertex(); }\n"
	;

static const char *const compute_shader_body =
	"layout(local_size_x = 1) in;\n"
	"void main() { }\n"
	;

/* The __VERSION__ stuff is to work-around gl_FragColor not existing in GLSL
 * ES 3.00.
 */
static const char *const fragment_shader_body =
	"#if __VERSION__ >= 300\n"
	"out vec4 color;\n"
	"#define gl_FragColor color\n"
	"#endif\n"
	"void main() { gl_FragColor = vec4(f[0]); }\n"
	;


PIGLIT_GL_TEST_CONFIG_BEGIN

	parse_file(argv[1]);

	switch (required_glsl_version) {
	case 100:
		config.supports_gl_compat_version = 10;
		config.supports_gl_es_version = 20;
		break;
	case 300:
		config.supports_gl_compat_version = 10;
		config.supports_gl_es_version = 30;
		break;
	default: {
		const unsigned int gl_version
			= required_gl_version_from_glsl_version(required_glsl_version);
		config.supports_gl_compat_version = gl_version;
		if (gl_version < 31)
			config.supports_gl_core_version = 0;
		else
			config.supports_gl_core_version = gl_version;
		break;
	}
	}

        config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
        /* UNREACHED */
        return PIGLIT_FAIL;
}


/**
 * Comparison function for qsort of test_vector list
 */
static int
compar(const void *_a, const void *_b)
{
	const struct test_vector *a = (const struct test_vector *) _a;
	const struct test_vector *b = (const struct test_vector *) _b;

	return strcmp(a->name, b->name);
}

static GLenum
parse_shader_type(const char *line, ptrdiff_t len)
{
	int i;
	static struct {
		const char *name;
		GLenum type;
	} shader_types[] = {
		{ "GL_VERTEX_SHADER", GL_VERTEX_SHADER },
		{ "GL_GEOMETRY_SHADER", GL_GEOMETRY_SHADER },
		{ "GL_FRAGMENT_SHADER", GL_FRAGMENT_SHADER },
		{ "GL_COMPUTE_SHADER", GL_COMPUTE_SHADER },
	};

	for (i = 0; i < ARRAY_SIZE(shader_types); i++) {
		if (len == strlen(shader_types[i].name) &&
		    strncmp(shader_types[i].name, line, 3) == 0) {
			return shader_types[i].type;
		}
	}

	return 0;
}

/**
 * Parse the file of values to test, fill in test vector list.
 */
void
parse_file(const char *filename)
{
	unsigned text_size;
	char *text = piglit_load_text_file(filename, &text_size);
	char *line = text;
	int count;
	int major;
	int minor;
	char *end_of_line;
	ptrdiff_t len;

	/* The format of the test file is:
	 *
	 * major.minor
	 * GL_VERTEX_SHADER|GL_GEOMETRY_SHADER|GL_FRAGMENT_SHADER|GL_COMPUTE_SHADER
	 * GL_ARB_some_extension
	 * gl_MaxFoo 8
	 * gl_MaxBar 16
	 * gl_MinAsdf -2
	 */

	/* Process the version requirement.
	 */
	count = sscanf(line, "%d.%d", &major, &minor);
	if (count != 2) {
		fprintf(stderr, "Parse error in version line.\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	required_glsl_version = major * 100 + minor;

	/* Skip to the next line.
	 */
	line = strchrnul(line, '\n');
	if (line[0] != '\0')
		line++;

	end_of_line = strchrnul(line, '\n');
	len = end_of_line - line;
	assert(end_of_line[0] == '\n' || end_of_line[0] == '\0');

	/* Process the shader type.
	 */
	shader_type = parse_shader_type(line, len);
	if (shader_type != 0) {
		/* Advance to the next input line.
		 */
		line = end_of_line;
		if (line[0] == '\n')
			line++;
	}

	/* Process the list of required extensions.
	 */
	while (strncmp("GL_", line, 3) == 0) {
		end_of_line = strchrnul(line, '\n');
		len = end_of_line - line;

		assert(end_of_line[0] == '\n' || end_of_line[0] == '\0');

		if (num_required_extensions >= ARRAY_SIZE(required_extensions)) {
			fprintf(stderr, "Too many required extensions!\n");
			piglit_report_result(PIGLIT_FAIL);
		}

		/* Copy the new extension to the list.
		 */
		required_extensions[num_required_extensions] =
			strndup(line, len);
		num_required_extensions++;

		/* Advance to the next input line.
		 */
		line = end_of_line;
		if (line[0] == '\n')
			line++;
	}

	while (line[0] != '\0') {
		char *endptr;

		line = (char *) eat_whitespace(line);

		if (string_match("gl_Max", line) != 0
		    && string_match("gl_Min", line) != 0) {
			char bad_name[80];

			strcpy_to_space(bad_name, line);
			fprintf(stderr,
				"Invalid built-in constant name \"%s\".\n",
				bad_name);
			piglit_report_result(PIGLIT_FAIL);
		}

		tests[num_tests].name = line;

		line = (char *) eat_text(line);
		line[0] = '\0';
		line++;

		line = (char *) eat_whitespace(line);

		tests[num_tests].minimum = strtol(line, &endptr, 0);
		if (endptr == line) {
			char bad_number[80];

			strcpy_to_space(bad_number, line);

			fprintf(stderr,
				"Invalid built-in constant value \"%s\".\n",
				bad_number);
			piglit_report_result(PIGLIT_FAIL);
		}
		line = endptr;

		num_tests++;

		/* Skip to the next line.
		 */
		line = strchrnul(line, '\n');
		if (line[0] != '\0')
			line++;
	}

	/* After parsing the full list of values to test, sort the list by
	 * variable name.  This ensures that the piglit results will be
	 * generated in a consistent order... no matter what happens in the
	 * control file.
	 */
	qsort(tests, num_tests, sizeof(tests[0]), compar);
}

static bool
check_compile_status(const char *name, GLuint sh)
{
	GLint ok;

        glGetShaderiv(sh, GL_COMPILE_STATUS, &ok);
	if (!ok) {
		GLchar *info;
		GLint size;

		glGetShaderiv(sh, GL_INFO_LOG_LENGTH, &size);
		info = malloc(size);
		glGetShaderInfoLog(sh, size, NULL, info);

		fprintf(stderr,
			"Failed to compile shader %s: %s\n",
			name, info);

		free(info);
	}

	return !!ok;
}

static GLuint
create_shader(GLenum type)
{
	if (shader_type != 0 && shader_type != type)
		return 0;
	if (type == GL_GEOMETRY_SHADER &&
	    (required_glsl_version < 150 || required_glsl_version == 300))
		return 0;
	/* Only create compute shaders when explicitly requested
	 */
	if (type == GL_COMPUTE_SHADER && shader_type != type)
		return 0;
	return glCreateShader(type);
}

void
piglit_init(int argc, char **argv)
{
	bool pass = true;
	char uniform[80];
	char *version_string = NULL;
	unsigned i;

	const char *shader_source[3];

	GLuint test_vs;
	GLuint test_gs;
	GLuint test_fs;
	GLuint test_cs;

	bool is_es;
	int major;
	int minor;
	int glsl_version;

	piglit_get_glsl_version(&is_es, &major, &minor);
	glsl_version = major * 100 + minor;
	if (glsl_version < required_glsl_version)
		piglit_report_result(PIGLIT_SKIP);

	/* Process the list of required extensions.  While doing this,
	 * generate the GLSL code that will enable those extensions in the
	 * shaders.
	 */
	for (i = 0; i < num_required_extensions; i++) {
		int len;

		if (!piglit_is_extension_supported(required_extensions[i])) {
			printf("%s not supported\n", required_extensions[i]);
			piglit_report_result(PIGLIT_SKIP);
		}

		if ((extension_enables_len + MAX_EXTENSION_ENABLE_LINE_LEN)
		    >= sizeof(extension_enables)) {
			printf("Extension enables too long.\n");
			piglit_report_result(PIGLIT_FAIL);
		}

		len = snprintf(&extension_enables[extension_enables_len],
			       MAX_EXTENSION_ENABLE_LINE_LEN,
			       "#extension %s: require\n",
			       required_extensions[i]);

		/* After the last use of the extension string, free it.
		 */
		free(required_extensions[i]);

		if (len <= 0) {
			printf("Extension enable snprintf failed.\n");
			piglit_report_result(PIGLIT_FAIL);
		}

		extension_enables_len += len;
	}

	/* Generate the version declaration that will be used by all of the
	 * shaders in the test run.
	 */
	asprintf(&version_string,
		 "#version %d %s\n"
		 "%s"
		 "#ifdef GL_ES\n"
		 "precision mediump float;\n"
		 "#endif\n",
		 required_glsl_version,
		 required_glsl_version == 300 ? "es" : "",
		 extension_enables);

	/* Create the shaders that will be used for the real part of the test.
	 */
	test_vs = create_shader(GL_VERTEX_SHADER);
	test_gs = create_shader(GL_GEOMETRY_SHADER);
	test_fs = create_shader(GL_FRAGMENT_SHADER);
	test_cs = create_shader(GL_COMPUTE_SHADER);

	for (i = 0; i < num_tests; i++) {
		bool subtest_pass = true;
		const char *comparitor =
			string_match("gl_Min", tests[i].name) ? "<=" : ">=";

		/* Generate the uniform declaration for the test.  This will
		 * be shared by all shader stages.
		 */
		snprintf(uniform, sizeof(uniform),
			 uniform_template,
			 tests[i].name, comparitor, tests[i].minimum);

		/* Try to compile the vertex shader.
		 */
		if (test_vs != 0) {
			shader_source[0] = version_string;
			shader_source[1] = uniform;
			shader_source[2] = vertex_shader_body;

			glShaderSource(test_vs, 3, shader_source, NULL);
			glCompileShader(test_vs);

			subtest_pass = check_compile_status(tests[i].name, test_vs)
				&& subtest_pass;
		}

		/* Try to compile the geometry shader.
		 */
		if (test_gs != 0) {
			shader_source[0] = version_string;
			shader_source[1] = uniform;
			shader_source[2] = geometry_shader_body;

			glShaderSource(test_gs, 3, shader_source, NULL);
			glCompileShader(test_gs);

			subtest_pass = check_compile_status(tests[i].name, test_gs)
				&& subtest_pass;
		}

		/* Try to compile the fragment shader.
		 */
		if (test_fs != 0) {
			shader_source[0] = version_string;
			shader_source[1] = uniform;
			shader_source[2] = fragment_shader_body;

			glShaderSource(test_fs, 3, shader_source, NULL);
			glCompileShader(test_fs);

			subtest_pass = check_compile_status(tests[i].name, test_fs)
				&& subtest_pass;
		}

		/* Try to compile the compute shader.
		 */
		if (test_cs != 0) {
			shader_source[0] = version_string;
			shader_source[1] = uniform;
			shader_source[2] = compute_shader_body;

			glShaderSource(test_cs, 3, shader_source, NULL);
			glCompileShader(test_cs);

			subtest_pass = check_compile_status(tests[i].name, test_cs)
				&& subtest_pass;
		}

		/* If both compilation phases passed, try to link the shaders
		 * together.
		 */
		if (subtest_pass) {
			GLuint prog = glCreateProgram();

			if (test_vs != 0)
				glAttachShader(prog, test_vs);
			if (test_gs != 0)
				glAttachShader(prog, test_gs);
			if (test_fs != 0)
				glAttachShader(prog, test_fs);
			if (test_cs != 0)
				glAttachShader(prog, test_cs);

			glLinkProgram(prog);
			subtest_pass = !!piglit_link_check_status(prog);

			glDeleteProgram(prog);
		}

		piglit_report_subtest_result(subtest_pass ? PIGLIT_PASS : PIGLIT_FAIL,
					     "%s", tests[i].name);

		pass = subtest_pass && pass;
	}

	free(version_string);
	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
