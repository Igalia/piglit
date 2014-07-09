/*
 * Copyright © 2009 Intel Corporation
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
 *
 * Authors:
 *    Eric Anholt <eric@anholt.net>
 *
 */

/** @file glslparsertest.c
 *
 * Tests that compiling (but not linking or drawing with) a given
 * shader either succeeds or fails as expected.
 */

#include <errno.h>

#include "piglit-util-gl.h"

static unsigned parse_glsl_version_number(const char *str);
static int process_options(int argc, char **argv);

PIGLIT_GL_TEST_CONFIG_BEGIN

	argc = process_options(argc, argv);
	if (argc > 3) {
		const unsigned int int_version
			= parse_glsl_version_number(argv[3]);
		switch (int_version) {
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
				= required_gl_version_from_glsl_version(int_version);
			config.supports_gl_compat_version = gl_version;
			if (gl_version < 31)
				config.supports_gl_core_version = 0;
			else
				config.supports_gl_core_version = gl_version;
		}
			break;
		}
	} else {
		config.supports_gl_compat_version = 10;
		config.supports_gl_es_version = 20;
	}

	config.window_width = 200;
	config.window_height = 100;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

static char *filename;
static int expected_pass;
static int gl_version_times_10 = 0;
static int check_link = 0;
static unsigned requested_version = 110;
static bool test_requires_geometry_shader4 = false;

static GLint
get_shader_compile_status(GLuint shader)
{
	GLint status;

#if defined PIGLIT_USE_OPENGL
	if (gl_version_times_10 >= 20) {
		glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	} else {
		glGetObjectParameterivARB(shader, GL_OBJECT_COMPILE_STATUS_ARB, &status);
	}
#elif defined PIGLIT_USE_OPENGL_ES2
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
#else
#	error
#endif

	return status;
}

static GLsizei
get_shader_info_log_length(GLuint shader)
{
	GLsizei length;

#if defined PIGLIT_USE_OPENGL
	if (gl_version_times_10 >= 20) {
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
	} else {
		glGetObjectParameterivARB(shader, GL_OBJECT_INFO_LOG_LENGTH_ARB, &length);
	}
#elif defined PIGLIT_USE_OPENGL_ES2
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
#else
#	error
#endif

	return length;
}

static char*
get_shader_name(GLenum type)
{
	switch(type) {
	case GL_VERTEX_SHADER:
		return "vertex";
	case GL_TESS_CONTROL_SHADER:
		return "tessellation control";
	case GL_TESS_EVALUATION_SHADER:
		return "tessellation evaluation";
	case GL_GEOMETRY_SHADER:
		return "geometry";
	case GL_FRAGMENT_SHADER:
		return "fragment";
	case GL_COMPUTE_SHADER:
		return "compute";
	default:
		fprintf(stderr, "Unexpected type in get_shader_name()\n");
		piglit_report_result(PIGLIT_FAIL);
	}
	return NULL;
}

/**
 * Attach a dumy shader of the given type.
 */
static void
attach_dummy_shader(GLuint shader_prog, GLenum type)
{
	const char *shader_template;
	char shader_text[128];
	GLint shader;

	switch (type) {
	case GL_VERTEX_SHADER:
		shader_template =
			"#version %d %s\n"
			"void main() { gl_Position = vec4(0.0); }";
		break;
	case GL_FRAGMENT_SHADER:
		shader_template =
			"#version %d %s\n"
			"void main() { }";
		break;
	default:
		shader_template = NULL;
		printf("No dummy shader available for this shader type\n");
		piglit_report_result(PIGLIT_FAIL);
		break;
	}

	sprintf(shader_text,
		shader_template,
		requested_version,
		(requested_version == 300) ? "es" : "");
	shader = piglit_compile_shader_text(type, shader_text);
	glAttachShader(shader_prog, shader);
}


/**
 * GLES requires both vertex and fragment shaders to be present in
 * order to link.  From section 2.10.3 (Program Objects) of the GLES 2.0 spec:
 *
 *   "Linking will also fail ... if program does not contain both a
 *   vertex shader and a fragment shader ..."
 *
 * So compile a dummy shader of type complementary to "type" and
 * attach it to shader_prog.
 */
static void
attach_complementary_shader(GLuint shader_prog, GLenum type)
{
	switch (type) {
	case GL_FRAGMENT_SHADER:
		attach_dummy_shader(shader_prog, GL_VERTEX_SHADER);
		break;
	case GL_VERTEX_SHADER:
		attach_dummy_shader(shader_prog, GL_FRAGMENT_SHADER);
		break;
	default:
		fprintf(stderr,
			"Unexpected type in attach_complementary_shader()");
		piglit_report_result(PIGLIT_FAIL);
	}
}

static void
test(void)
{
	GLint prog;
	GLint ok;
	GLchar *prog_string;
	FILE *out;
	GLboolean pass;
	GLchar *info;
	GLint size;
	GLenum type;
	char *failing_stage = NULL;

	if (strcmp(filename + strlen(filename) - 4, "frag") == 0)
		type = GL_FRAGMENT_SHADER;
	else if (strcmp(filename + strlen(filename) - 4, "vert") == 0)
		type = GL_VERTEX_SHADER;
#ifdef PIGLIT_USE_OPENGL
	else if (strcmp(filename + strlen(filename) - 4, "tesc") == 0)
		type = GL_TESS_CONTROL_SHADER;
	else if (strcmp(filename + strlen(filename) - 4, "tese") == 0)
		type = GL_TESS_EVALUATION_SHADER;
	else if (strcmp(filename + strlen(filename) - 4, "geom") == 0)
		type = GL_GEOMETRY_SHADER;
	else if (strcmp(filename + strlen(filename) - 4, "comp") == 0)
		type = GL_COMPUTE_SHADER;
#endif
	else {
		type = GL_NONE;
		fprintf(stderr, "Couldn't determine type of program %s\n",
			filename);
		piglit_report_result(PIGLIT_FAIL);
	}

	piglit_require_vertex_shader();
	piglit_require_fragment_shader();

	if (type == GL_TESS_CONTROL_SHADER || type == GL_TESS_EVALUATION_SHADER) {
		if (!piglit_is_extension_supported("GL_ARB_tessellation_shader") &&
		    (piglit_is_gles() || piglit_get_gl_version() < 40)) {
			printf("Test requires GL version 4.0 or "
			       "GL_ARB_tessellation_shader\n");
			piglit_report_result(PIGLIT_SKIP);
		}
	}

	if (type == GL_COMPUTE_SHADER) {
		if (!piglit_is_extension_supported("GL_ARB_compute_shader") &&
		    (piglit_is_gles() || piglit_get_gl_version() < 43)) {
			printf("Test requires GL version 4.3 or "
			       "GL_ARB_compute_shader\n");
			piglit_report_result(PIGLIT_SKIP);
		}
	}

	prog_string = piglit_load_text_file(filename, NULL);
	if (prog_string == NULL) {
		fprintf(stderr, "Couldn't open program %s: %s\n",
			filename, strerror(errno));
		exit(1);
	}

	prog = glCreateShader(type);
	glShaderSource(prog, 1, (const GLchar **)&prog_string, NULL);
	glCompileShader(prog);
	ok = get_shader_compile_status(prog);

	size = get_shader_info_log_length(prog);
	if (size != 0) {
		info = malloc(size);
		glGetShaderInfoLog(prog, size, NULL, info);
	} else {
		info = "(no compiler output)";
	}

	if (!ok) {
		failing_stage = "compile";
	} else {
		/* Try linking the shader if it compiled.  We do this
		 * even if --check-link wasn't specified, to increase
		 * coverage of linker code.
		 */
		GLuint shader_prog;

		shader_prog = glCreateProgram();
		glAttachShader(shader_prog, prog);
		if (requested_version == 100 || requested_version == 300)
			attach_complementary_shader(shader_prog, type);
#if PIGLIT_USE_OPENGL
		if (type == GL_GEOMETRY_SHADER ||
		    type == GL_TESS_CONTROL_SHADER ||
		    type == GL_TESS_EVALUATION_SHADER)
			attach_dummy_shader(shader_prog, GL_VERTEX_SHADER);
		if (test_requires_geometry_shader4) {
			/* The default value of
			 * GL_GEOMETRY_VERTICES_OUT_ARB is zero, which
			 * is useless for testing.  Use a value of 3.
			 */
			glProgramParameteriARB(shader_prog,
					       GL_GEOMETRY_VERTICES_OUT_ARB,
					       3);
		}
#endif
		glLinkProgram(shader_prog);
		if (check_link) {
			ok = piglit_link_check_status_quiet(shader_prog);
			if (!ok) {
				failing_stage = "link";
			}
		}
		glDeleteProgram(shader_prog);
	}

	pass = (expected_pass == ok);

	if (pass)
		out = stdout;
	else
		out = stderr;

	if (!ok) {
		fprintf(out, "Failed to %s %s shader %s: %s\n",
			failing_stage,
			get_shader_name(type),
			filename, info);
		if (expected_pass) {
			printf("Shader source:\n");
			printf("%s\n", prog_string);
		}
	} else {
		fprintf(out, "Successfully %s %s shader %s: %s\n",
			check_link ? "compiled and linked" : "compiled",
			get_shader_name(type),
			filename, info);
		if (!expected_pass) {
			printf("Shader source:\n");
			printf("%s\n", prog_string);
		}
	}

	if (size != 0)
		free(info);
	free(prog_string);
	glDeleteShader(prog);
	piglit_report_result (pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

static void usage(char *name)
{
	printf("%s {options} <filename.frag|filename.vert> <pass|fail> "
	       "{requested GLSL version} {list of required GL extensions}\n", name);
	printf("\nSupported options:\n");
	printf("  --check-link: also detect link failures\n");
	exit(1);
}

/**
 * Process any options and remove them from the argv array.  Return
 * the new argc.
 */
static int
process_options(int argc, char **argv)
{
	int i = 1;
	int new_argc = 1;
	while (i < argc) {
		if (argv[i][0] == '-') {
			if (strcmp(argv[i], "--check-link") == 0)
				check_link = 1;
			else
				usage(argv[0]);
			/* do not retain the option; we've processed it */
			i++;
		} else {
			/* retain the option in the argv array */
			argv[new_argc++] = argv[i++];
		}
	}
	return new_argc;
}

static unsigned
parse_glsl_version_number(const char *str)
{
	unsigned major = 0;
	unsigned minor = 0;

	/* Accept a return value of either 1 or 2 from sscanf(), so
	 * that the version number may be supplied as either "<int>"
	 * or "<int>.<int>".
	 */
	if (sscanf(str, "%u.%u", &major, &minor) == 0) {
		printf("Ill-formed GLSL version number: %s\n", str);
		piglit_report_result(PIGLIT_FAIL);
	}

	return (major * 100) + minor;
}


static unsigned
parse_glsl_version_string(const char *str)
{
	if (piglit_is_gles()) {
		/* In GLSL ES, the string returned by
		 * glGetString(GL_SHADING_LANGUAGE_VERSION) is
		 * prefixed by some text.  Verify that the expected
		 * text is there and skip it before calling
		 * parse_glsl_version_number().
		 */
		const char *expected_prefix = "OpenGL ES GLSL ES ";
		if (strncmp(str, expected_prefix,
			    strlen(expected_prefix)) != 0) {
			printf("Ill-formed GLSL version string: %s\n", str);
			piglit_report_result(PIGLIT_FAIL);
		}
		str += strlen(expected_prefix);
	}
	return parse_glsl_version_number(str);
}


static void
check_version(unsigned glsl_version)
{
	if (!piglit_is_gles()) {
		if (requested_version == 100) {
			piglit_require_extension("GL_ARB_ES2_compatibility");
			return;
		} else if (requested_version == 300) {
			piglit_require_extension("GL_ARB_ES3_compatibility");
			return;
		}
	}

	if (glsl_version < requested_version) {
		fprintf(stderr,
			"GLSL version is %u.%u, but requested version %u.%u is required\n",
			glsl_version / 100, glsl_version % 100,
			requested_version / 100, requested_version % 100);
		piglit_report_result(PIGLIT_SKIP);
	}
}


void
piglit_init(int argc, char**argv)
{
	const char *glsl_version_string;
	unsigned glsl_version = 0;
	int i;

	if (argc < 3)
		usage(argv[0]);

	if (strlen(argv[1]) < 5)
		usage(argv[0]);
	filename = argv[1];

	if (strcmp(argv[2], "pass") == 0)
		expected_pass = 1;
	else if (strcmp(argv[2], "fail") == 0)
		expected_pass = 0;
	else
		usage(argv[0]);

	if (argc > 3)
		requested_version = parse_glsl_version_number(argv[3]);

	gl_version_times_10 = piglit_get_gl_version();

	if (gl_version_times_10 < 20
	    && !piglit_is_extension_supported("GL_ARB_shader_objects")) {
		printf("Requires OpenGL 2.0\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	glsl_version_string = (char *)
		glGetString(GL_SHADING_LANGUAGE_VERSION);

	if (glsl_version_string != NULL)
		glsl_version = parse_glsl_version_string(glsl_version_string);

	check_version(glsl_version);

	for (i = 4; i < argc; i++) {
		if (argv[i][0] == '!') {
			piglit_require_not_extension(argv[i] + 1);
		} else {
			piglit_require_extension(argv[i]);
			if (strstr(argv[i], "geometry_shader4") != NULL)
				test_requires_geometry_shader4 = true;
		}
	}

	test();
}

enum piglit_result
piglit_display(void)
{
	/* Should never be reached */
	return PIGLIT_FAIL;
}
