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

#if defined(_WIN32)
#include <windows.h>
#endif

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#include "config.h"
#if defined(HAVE_SYS_TIME_H) && defined(HAVE_SYS_RESOURCE_H) && defined(HAVE_SETRLIMIT)
#include <sys/time.h>
#include <sys/resource.h>
#include <errno.h>
#define USE_SETRLIMIT
#endif

#include "piglit-util.h"

void piglit_glutInit(int argc, char **argv)
{
	glutInit(&argc, argv);

#if defined USE_EGLUT && defined USE_OPENGL
	glutInitAPIMask(GLUT_OPENGL_BIT);
#elif defined USE_EGLUT && defined USE_OPENGL_ES2
	glutInitAPIMask(GLUT_OPENGL_ES2_BIT);
#elif defined USE_EGLUT
#	error
#endif
}

void piglit_get_gl_version(bool *es, int* major, int* minor)
{
	/* Version of OpenGL API. */
	bool es_local;
	int major_local;
	int minor_local;

	const char *version_string;
	int c; /* scanf count */

	version_string = (const char*) glGetString(GL_VERSION);
	es_local = strncmp("OpenGL ES ", version_string, 10) == 0;
	if (es_local) {
		c = sscanf(version_string,
		           "OpenGL ES %i.%i",
		           &major_local,
		           &minor_local);
	} else {
		c = sscanf(version_string,
		           "%i.%i",
		           &major_local,
		           &minor_local);
	}
	assert(c == 2);

	/* Write outputs. */
	if (es != NULL)
		*es = es_local;
	if (major != NULL)
		*major = major_local;
	if (minor != NULL)
		*minor = minor_local;
}

void piglit_get_glsl_version(bool *es, int* major, int* minor)
{
	bool es_local;
	int major_local;
	int minor_local;

	const char *version_string;
	int c; /* scanf count */

	version_string = (const char*) glGetString(GL_SHADING_LANGUAGE_VERSION);
	es_local = strncmp("OpenGL ES", version_string, 10) == 0;
	if (es_local) {
		c = sscanf(version_string,
		           "OpenGL ES GLSL ES %i.%i",
		           &major_local,
		           &minor_local);
	} else {
		c = sscanf(version_string,
		           "%i.%i",
		           &major_local,
		           &minor_local);
	}
	assert(c == 2);

	/* Write outputs. */
	if (es != NULL)
		*es = es_local;
	if (major != NULL)
		*major = major_local;
	if (minor != NULL)
		*minor = minor_local;
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

void piglit_require_extension(const char *name)
{
	if (!piglit_is_extension_supported(name)) {
		printf("Test requires %s\n", name);
		piglit_report_result(PIGLIT_SKIP);
		exit(1);
	}
}

void piglit_require_not_extension(const char *name)
{
	if (piglit_is_extension_supported(name)) {
		piglit_report_result(PIGLIT_SKIP);
		exit(1);
	}
}

const char* piglit_get_gl_error_name(GLenum error)
{
#define CASE(x) case x: return #x; 
    switch (error) {
    CASE(GL_INVALID_ENUM)
    CASE(GL_INVALID_FRAMEBUFFER_OPERATION)
    CASE(GL_INVALID_OPERATION)
    CASE(GL_INVALID_VALUE)
    CASE(GL_NO_ERROR)
    CASE(GL_OUT_OF_MEMORY)
    CASE(GL_STACK_OVERFLOW)
    CASE(GL_STACK_UNDERFLOW)
    default:
        return "(unrecognized error)";
    }
#undef CASE
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

	if (result == PIGLIT_PASS) {
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

/** Return a string name for a shader target enum */
static const char *
shader_name(GLenum target)
{
   switch (target) {
   case GL_VERTEX_SHADER:
      return "vertex";
#if defined USE_OPENGL
   case GL_GEOMETRY_SHADER:
      return "geometry";
#endif
   case GL_FRAGMENT_SHADER:
      return "fragment";
   default:
      assert(!"Unexpected shader target in shader_name()");
   }
}

/**
 * Convenience function to compile a GLSL shader.
 */
GLuint
piglit_compile_shader_text(GLenum target, const char *text)
{
	GLuint prog;
	GLint ok;

	piglit_require_GLSL();

	prog = piglit_CreateShader(target);
	piglit_ShaderSource(prog, 1, (const GLchar **) &text, NULL);
	piglit_CompileShader(prog);

	piglit_GetShaderiv(prog, GL_COMPILE_STATUS, &ok);

	{
		GLchar *info;
		GLint size;

		piglit_GetShaderiv(prog, GL_INFO_LOG_LENGTH, &size);
		info = malloc(size);

		piglit_GetShaderInfoLog(prog, size, NULL, info);
		if (!ok) {
			fprintf(stderr, "Failed to compile %s shader: %s\n",
				shader_name(target),
				info);
			piglit_DeleteShader(prog);
			prog = 0;
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

	piglit_require_GLSL();

	piglit_GetProgramiv(prog, GL_LINK_STATUS, &ok);

	/* Some drivers return a size of 1 for an empty log.  This is the size
	 * of a log that contains only a terminating NUL character.
	 */
	piglit_GetProgramiv(prog, GL_INFO_LOG_LENGTH, &size);
	if (size > 1) {
		info = malloc(size);
		piglit_GetProgramInfoLog(prog, size, NULL, info);
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

	piglit_require_GLSL();

	prog = piglit_CreateProgram();
	if (fs)
		piglit_AttachShader(prog, fs);
	if (vs)
		piglit_AttachShader(prog, vs);
	piglit_LinkProgram(prog);

	if (!piglit_link_check_status(prog)) {
		piglit_DeleteProgram(prog);
		prog = 0;
	}

	return prog;
}

float piglit_tolerance[4] = { 0.01, 0.01, 0.01, 0.01 };

void
piglit_set_tolerance_for_bits(int rbits, int gbits, int bbits, int abits)
{
	int bits[4] = {rbits, gbits, bbits, abits};
	int i;

	for (i = 0; i < 4; i++) {
		if (bits[i] < 2) {
			/* Don't try to validate channels when there's only 1
			 * bit of precision (or none).
			 */
			piglit_tolerance[i] = 1.0;
		} else {
			piglit_tolerance[i] = 3.0 / (1 << bits[i]);
		}
	}
}


#ifndef HAVE_STRCHRNUL
char *strchrnul(const char *s, int c)
{
	char *t = strchr(s, c);

	return (t == NULL) ? ((char *) s + strlen(s)) : t;
}
#endif


void
piglit_set_rlimit(unsigned long lim)
{
#if defined(USE_SETRLIMIT)
	struct rlimit rl;
	if (getrlimit(RLIMIT_AS, &rl) != -1) {
		printf("Address space limit = %lu, max = %lu\n",
		       (unsigned long) rl.rlim_cur,
		       (unsigned long) rl.rlim_max);

		if (rl.rlim_max > lim) {
			printf("Resetting limit to %lu.\n", lim);

			rl.rlim_cur = lim;
			rl.rlim_max = lim;
			if (setrlimit(RLIMIT_AS, &rl) == -1) {
				printf("Could not set rlimit "
				       "due to: %s (%d)\n",
				       strerror(errno), errno);
			}
		}
	}

	printf("\n");
#else
	printf("Cannot reset rlimit on this platform.\n\n");
#endif
}
