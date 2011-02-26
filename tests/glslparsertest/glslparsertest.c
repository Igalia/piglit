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
#include <sys/stat.h>

#include "piglit-util.h"

#define WIN_WIDTH 200
#define WIN_HEIGHT 100

static char *filename;
static int expected_pass;
static float gl_version = 0;

static GLint
get_shader_compile_status(GLuint shader)
{
	GLint status;

#if defined USE_OPENGL
	if (gl_version >= 2.0) {
		glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	} else {
		glGetObjectParameterivARB(shader, GL_OBJECT_COMPILE_STATUS_ARB, &status);
	}
#elif defined USE_OPENGL_ES2
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

#if defined USE_OPENGL
	if (gl_version >= 2.0) {
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
	} else {
		glGetObjectParameterivARB(shader, GL_OBJECT_INFO_LOG_LENGTH_ARB, &length);
	}
#elif defined USE_OPENGL_ES2
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
#else
#	error
#endif

	return length;
}

static void
test(void)
{
	GLint prog;
	GLint ok;
	struct stat st;
	int err;
	GLchar *prog_string;
	FILE *f;
	FILE *out;
	GLboolean pass;
	GLchar *info;
	GLint size;
	GLenum type;

	if (strcmp(filename + strlen(filename) - 4, "frag") == 0)
		type = GL_FRAGMENT_SHADER;
	else if (strcmp(filename + strlen(filename) - 4, "vert") == 0)
		type = GL_VERTEX_SHADER;
	else {
		fprintf(stderr, "Couldn't determine type of program %s\n",
			filename);
		piglit_report_result(PIGLIT_FAILURE);
		exit(1);
	}

	piglit_require_vertex_shader();
	piglit_require_fragment_shader();

	err = stat(filename, &st);
	if (err == -1) {
		fprintf(stderr, "Couldn't stat program %s: %s\n",
			filename, strerror(errno));
		exit(1);
	}

	prog_string = malloc(st.st_size + 1);
	if (prog_string == NULL) {
		fprintf(stderr, "malloc\n");
		exit(1);
	}

	f = fopen(filename, "r");
	if (f == NULL) {
		fprintf(stderr, "Couldn't open program %s: %s\n",
			filename, strerror(errno));
		exit(1);
	}
	fread(prog_string, 1, st.st_size, f);
	prog_string[st.st_size] = '\0';
	fclose(f);


	prog = piglit_CreateShader(type);
	piglit_ShaderSource(prog, 1, (const GLchar **)&prog_string, NULL);
	piglit_CompileShader(prog);
	ok = get_shader_compile_status(prog);
	pass = (expected_pass == ok);

	if (pass)
		out = stdout;
	else
		out = stderr;

	size = get_shader_info_log_length(prog);
	if (size != 0) {
		info = malloc(size);
		piglit_GetShaderInfoLog(prog, size, NULL, info);
	} else {
		info = "(no compiler output)";
	}

	if (!ok) {
		fprintf(out, "Failed to compile %s shader %s: %s\n",
			type == GL_FRAGMENT_SHADER ? "fragment" : "vertex",
			filename, info);
		if (expected_pass) {
			printf("Shader source:\n");
			printf("%s\n", prog_string);
		}
	} else {
		fprintf(out, "Successfully compiled %s shader %s: %s\n",
			type == GL_FRAGMENT_SHADER ? "fragment" : "vertex",
			filename, info);
		if (!expected_pass) {
			printf("Shader source:\n");
			printf("%s\n", prog_string);
		}
	}

	if (size != 0)
		free(info);
	free(prog_string);
	piglit_DeleteShader(prog);
	piglit_report_result (pass ? PIGLIT_SUCCESS : PIGLIT_FAILURE);
}

static void usage(char *name)
{
	printf("%s <filename.frag|filename.vert> <pass|fail> "
	       "{requested GLSL vesion} {list of required GL extensions}\n", name);
	exit(1);
}

int main(int argc, char**argv)
{
	const char *glsl_version_string;
	float glsl_version;
	float requested_version = 1.10;
	int i;

	piglit_glutInit(argc, argv);
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
		requested_version = strtod(argv[3], NULL);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(WIN_WIDTH, WIN_HEIGHT);
	glutCreateWindow("glslparsertest");
	piglit_get_gl_version(NULL, &gl_version);

	if (gl_version < 2.0
	    && !piglit_is_extension_supported("GL_ARB_shader_objects")) {
		printf("Requires OpenGL 2.0\n");
		piglit_report_result(PIGLIT_SKIP);
		exit(1);
	}

	glsl_version_string = (char *)
		glGetString(GL_SHADING_LANGUAGE_VERSION);
	glsl_version = (glsl_version_string == NULL)
		? 0.0 : strtod(glsl_version_string, NULL);

	if (requested_version == 1.00) {
		piglit_require_extension("GL_ARB_ES2_compatibility");
	} else if (glsl_version < requested_version) {
		fprintf(stderr,
			"GLSL version is %f, but requested version %f is required\n",
			glsl_version,
			requested_version);
		piglit_report_result(PIGLIT_SKIP);
	}

	for (i = 4; i < argc; i++) {
		piglit_require_extension(argv[i]);
	}

	test();
	return 0;
}
