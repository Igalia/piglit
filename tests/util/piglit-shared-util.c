/*
 * Copyright (c) The Piglit project 2007
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
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.  IN NO EVENT SHALL
 * VA LINUX SYSTEM, IBM AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#if defined(_MSC_VER)
#include <windows.h>
#endif

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#include "piglit-util.h"

void piglit_get_gl_version(bool *es, float* version)
{
	/* Version of OpenGL API. */
	bool es_local;
	int major;
	int minor;
	const size_t buffer_size = 32;
	char buffer[buffer_size];

	const char *version_string;
	int c; /* scanf count */

	version_string = (const char*) glGetString(GL_VERSION);
	es_local = strncmp("OpenGL ES ", version_string, 10) == 0;
	if (es_local) {
		c = sscanf(version_string,
		           "OpenGL ES %i.%i",
		           &major,
		           &minor);
	} else {
		c = sscanf(version_string,
		           "%i.%i",
		           &major,
		           &minor);
	}
	assert(c == 2);
	memset(buffer, 0, buffer_size * sizeof(char));
	sprintf(buffer, "%i.%i", major, minor);

	/* Write outputs. */
	if (es != NULL)
		*es = es_local;
	if (version != NULL)
		*version = strtof(buffer, NULL);
}

bool piglit_is_extension_supported(const char *name)
{
	char *extensions;
	bool found = false;
	char *i;

	assert(name != NULL);
	extensions = strdup((const char*) glGetString(GL_EXTENSIONS));
	for (i = strtok(extensions, " "); i != NULL; i = strtok(NULL, " ")) {
		if (strcmp(name, i) == 0) {
			found = true;
			break;
		}
	}
	free(extensions);
	return found;
}

/* These texture coordinates should have 1 or -1 in the major axis selecting
 * the face, and a nearly-1-or-negative-1 value in the other two coordinates
 * which will be used to produce the s,t values used to sample that face's
 * image.
 */
GLfloat cube_face_texcoords[6][4][3] = {
	{ /* GL_TEXTURE_CUBE_MAP_POSITIVE_X */
		{1.0,  0.99,  0.99},
		{1.0,  0.99, -0.99},
		{1.0, -0.99, -0.99},
		{1.0, -0.99,  0.99},
	},
	{ /* GL_TEXTURE_CUBE_MAP_POSITIVE_Y */
		{-0.99, 1.0, -0.99},
		{ 0.99, 1.0, -0.99},
		{ 0.99, 1.0,  0.99},
		{-0.99, 1.0,  0.99},
	},
	{ /* GL_TEXTURE_CUBE_MAP_POSITIVE_Z */
		{-0.99,  0.99, 1.0},
		{-0.99, -0.99, 1.0},
		{ 0.99, -0.99, 1.0},
		{ 0.99,  0.99, 1.0},
	},
	{ /* GL_TEXTURE_CUBE_MAP_NEGATIVE_X */
		{-1.0,  0.99, -0.99},
		{-1.0,  0.99,  0.99},
		{-1.0, -0.99,  0.99},
		{-1.0, -0.99, -0.99},
	},
	{ /* GL_TEXTURE_CUBE_MAP_NEGATIVE_Y */
		{-0.99, -1.0,  0.99},
		{-0.99, -1.0, -0.99},
		{ 0.99, -1.0, -0.99},
		{ 0.99, -1.0,  0.99},
	},
	{ /* GL_TEXTURE_CUBE_MAP_NEGATIVE_Z */
		{ 0.99,  0.99, -1.0},
		{-0.99,  0.99, -1.0},
		{-0.99, -0.99, -1.0},
		{ 0.99, -0.99, -1.0},
	},
};

const char *cube_face_names[6] = {
	"POSITIVE_X",
	"POSITIVE_Y",
	"POSITIVE_Z",
	"NEGATIVE_X",
	"NEGATIVE_Y",
	"NEGATIVE_Z",
};

const GLenum cube_face_targets[6] = {
	GL_TEXTURE_CUBE_MAP_POSITIVE_X,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
};

/** Returns the line in the program string given the character position. */
int FindLine(const char *program, int position)
{
	int i, line = 1;
	for (i = 0; i < position; i++) {
		if (program[i] == '0')
			return -1; /* unknown line */
		if (program[i] == '\n')
			line++;
	}
	return line;
}

void
piglit_report_result(enum piglit_result result)
{
	fflush(stderr);

	if (result == PIGLIT_SUCCESS) {
		printf("PIGLIT: {'result': 'pass' }\n");
		fflush(stdout);
		exit(0);
	} else if (result == PIGLIT_SKIP) {
		printf("PIGLIT: {'result': 'skip' }\n");
		fflush(stdout);
		exit(0);
	} else if (result == PIGLIT_WARN) {
		printf("PIGLIT: {'result': 'warn' }\n");
		fflush(stdout);
		exit(0);
	} else {
		printf("PIGLIT: {'result': 'fail' }\n");
		fflush(stdout);
		exit(1);
	}
}

/**
 * Convenience function to compile a GLSL shader from a file.
 */
GLuint
piglit_compile_shader(GLenum target, char *filename)
{
	GLuint prog;
	struct stat st;
	int err;
	GLchar *prog_string;
	FILE *f;
	const char *source_dir;
	char filename_with_path[FILENAME_MAX];

	source_dir = getenv("PIGLIT_SOURCE_DIR");
	if (source_dir == NULL) {
		source_dir = SOURCE_DIR;
	}

	snprintf(filename_with_path, FILENAME_MAX - 1,
		 "%s/tests/%s", source_dir, filename);
	filename_with_path[FILENAME_MAX - 1] = 0;

	err = stat(filename_with_path, &st);
	if (err == -1) {
		fprintf(stderr, "Couldn't stat program %s: %s\n", filename, strerror(errno));
		fprintf(stderr, "You can override the source dir by setting the PIGLIT_SOURCE_DIR environment variable.\n");
		exit(1);
	}

	prog_string = malloc(st.st_size + 1);
	if (prog_string == NULL) {
		fprintf(stderr, "malloc\n");
		exit(1);
	}

	f = fopen(filename_with_path, "r");
	if (f == NULL) {
		fprintf(stderr, "Couldn't open program: %s\n", strerror(errno));
		exit(1);
	}
	fread(prog_string, 1, st.st_size, f);
	prog_string[st.st_size] = '\0';
	fclose(f);

	prog = piglit_compile_shader_text(target, prog_string);

	free(prog_string);

	return prog;
}

/**
 * Convenience function to compile a GLSL shader.
 */
GLuint
piglit_compile_shader_text(GLenum target, const char *text)
{
	GLuint prog;
	GLint ok;

	prog = glCreateShader(target);
	glShaderSource(prog, 1, (const GLchar **) &text, NULL);
	glCompileShader(prog);

	glGetShaderiv(prog, GL_COMPILE_STATUS, &ok);

	{
		GLchar *info;
		GLint size;

		glGetShaderiv(prog, GL_INFO_LOG_LENGTH, &size);
		info = malloc(size);

		glGetShaderInfoLog(prog, size, NULL, info);
		if (!ok) {
			fprintf(stderr, "Failed to compile %s: %s\n",
				target == GL_FRAGMENT_SHADER ? "FS" : "VS",
				info);
		}
		else if (0) {
			/* Enable this to get extra compilation info.
			 * Even if there's no compilation errors, the info
			 * log may have some remarks.
			 */
			fprintf(stderr, "Shader compiler warning: %s\n", info);
		}
		free(info);
	}

	return prog;
}

static GLboolean
link_check_status(GLint prog, FILE *output)
{
	GLchar *info = NULL;
	GLint size;
	GLint ok;

	glGetProgramiv(prog, GL_LINK_STATUS, &ok);

	/* Some drivers return a size of 1 for an empty log.  This is the size
	 * of a log that contains only a terminating NUL character.
	 */
	glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &size);
	if (size > 1) {
		info = malloc(size);
		glGetProgramInfoLog(prog, size, NULL, info);
	}

	if (!ok) {
		fprintf(output, "Failed to link: %s\n",
			(info != NULL) ? info : "<empty log>");
	}
	else if (0 && info != NULL) {
		/* Enable this to get extra linking info.
		 * Even if there's no link errors, the info log may
		 * have some remarks.
		 */
		printf("Linker warning: %s\n", info);
	}

	free(info);

	return ok;
}

GLboolean
piglit_link_check_status(GLint prog)
{
	return link_check_status(prog, stderr);
}

/**
 * Check link status
 *
 * Similar to piglit_link_check_status except it logs error messages
 * to standard output instead of standard error.  This is useful for
 * tests that want to produce negative link results.
 *
 * \sa piglit_link_check_status
 */
GLboolean
piglit_link_check_status_quiet(GLint prog)
{
	return link_check_status(prog, stdout);
}


GLint piglit_link_simple_program(GLint vs, GLint fs)
{
	GLint prog;

	prog = glCreateProgram();
	if (fs)
		glAttachShader(prog, fs);
	if (vs)
		glAttachShader(prog, vs);
	glLinkProgram(prog);

	piglit_link_check_status(prog);

	return prog;
}

#ifndef HAVE_STRCHRNUL
char *strchrnul(const char *s, int c)
{
	char *t = strchr(s, c);

	return (t == NULL) ? ((char *) s + strlen(s)) : t;
}
#endif
