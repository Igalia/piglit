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

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#include <GL/glew.h>
#if defined(__APPLE__)
#include <GLUT/glut.h>
#else
#include "GL/glut.h"
#endif

#include "piglit-util.h"

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
		if (program[i] == '\n')
			line++;
	}
	return line;
}

void
piglit_report_result(enum piglit_result result)
{
	/* Currently we have no way of reporting the "skip" (required extension
	 * not supported) result.
	 */

	fflush(stderr);

	if (result == PIGLIT_SUCCESS) {
		printf("PIGLIT: {'result': 'pass' }\n");
		fflush(stdout);
		exit(0);
	} else {
		printf("PIGLIT: {'result': 'fail' }\n");
		fflush(stdout);
		exit(1);
	}
}

void piglit_require_extension(const char *name)
{
	if (!glutExtensionSupported(name)) {
		piglit_report_result(PIGLIT_SKIP);
		exit(1);
	}
}

/**
 * Read a pixel from the given location and compare its RGBA value to the
 * given expected values.
 *
 * Print a log message if the color value deviates from the expected value.
 * \return true if the color values match, false otherwise
 */
int piglit_probe_pixel_rgba(int x, int y, const float* expected)
{
	GLfloat probe[4];
	GLfloat delta[4];
	GLfloat deltamax;
	int i;

	glReadPixels(x, y, 1, 1, GL_RGBA, GL_FLOAT, probe);

	deltamax = 0.0;
	for(i = 0; i < 4; ++i) {
		delta[i] = probe[i] - expected[i];
		if (fabs(delta[i]) > deltamax)
			deltamax = fabs(delta[i]);
	}

	if (deltamax < 0.01)
		return 1;

	printf("Probe at (%i,%i)\n", x, y);
	printf("  Expected: %f %f %f %f\n", expected[0], expected[1], expected[2], expected[3]);
	printf("  Observed: %f %f %f %f\n", probe[0], probe[1], probe[2], probe[3]);

	return 0;
}

/**
 * Read a pixel from the given location and compare its RGB value to the
 * given expected values.
 *
 * Print a log message if the color value deviates from the expected value.
 * \return true if the color values match, false otherwise
 */
int piglit_probe_pixel_rgb(int x, int y, const float* expected)
{
	GLfloat probe[3];
	GLfloat delta[3];
	GLfloat deltamax;
	int i;

	glReadPixels(x, y, 1, 1, GL_RGB, GL_FLOAT, probe);

	deltamax = 0.0;
	for(i = 0; i < 3; ++i) {
		delta[i] = probe[i] - expected[i];
		if (fabs(delta[i]) > deltamax)
			deltamax = fabs(delta[i]);
	}

	if (deltamax < 0.01)
		return 1;

	printf("Probe at (%i,%i)\n", x, y);
	printf("  Expected: %f %f %f\n", expected[0], expected[1], expected[2]);
	printf("  Observed: %f %f %f\n", probe[0], probe[1], probe[2]);

	return 0;
}


PFNGLGENPROGRAMSARBPROC pglGenProgramsARB = 0;
PFNGLPROGRAMSTRINGARBPROC pglProgramStringARB = 0;
PFNGLBINDPROGRAMARBPROC pglBindProgramARB = 0;
PFNGLISPROGRAMARBPROC pglIsProgramARB = 0;
PFNGLDELETEPROGRAMSARBPROC pglDeleteProgramsARB = 0;
PFNGLPROGRAMLOCALPARAMETER4FVARBPROC pglProgramLocalParameter4fvARB = 0;
PFNGLPROGRAMLOCALPARAMETER4DARBPROC pglProgramLocalParameter4dARB = 0;
PFNGLGETPROGRAMIVARBPROC pglGetProgramivARB = 0;
PFNGLGETPROGRAMLOCALPARAMETERDVARBPROC pglGetProgramLocalParameterdvARB = 0;

static void get_program_functions(void)
{
	pglGenProgramsARB = (PFNGLGENPROGRAMSARBPROC) glutGetProcAddress("glGenProgramsARB");
	assert(pglGenProgramsARB);

	pglProgramStringARB = (PFNGLPROGRAMSTRINGARBPROC) glutGetProcAddress("glProgramStringARB");
	assert(pglProgramStringARB);

	pglBindProgramARB = (PFNGLBINDPROGRAMARBPROC) glutGetProcAddress("glBindProgramARB");
	assert(pglBindProgramARB);

	pglIsProgramARB = (PFNGLISPROGRAMARBPROC) glutGetProcAddress("glIsProgramARB");
	assert(pglIsProgramARB);

	pglDeleteProgramsARB = (PFNGLDELETEPROGRAMSARBPROC) glutGetProcAddress("glDeleteProgramsARB");
	assert(pglDeleteProgramsARB);

	pglProgramLocalParameter4fvARB = (PFNGLPROGRAMLOCALPARAMETER4FVARBPROC) glutGetProcAddress("glProgramLocalParameter4fvARB");
	assert(pglProgramLocalParameter4fvARB);

	pglProgramLocalParameter4dARB = (PFNGLPROGRAMLOCALPARAMETER4DARBPROC) glutGetProcAddress("glProgramLocalParameter4dARB");
	assert(pglProgramLocalParameter4dARB);

	pglGetProgramLocalParameterdvARB = (PFNGLGETPROGRAMLOCALPARAMETERDVARBPROC) glutGetProcAddress("glGetProgramLocalParameterdvARB");
	assert(pglGetProgramLocalParameterdvARB);

	pglGetProgramivARB = (PFNGLGETPROGRAMIVARBPROC) glutGetProcAddress("glGetProgramivARB");
	assert(pglGetProgramivARB);
}

int piglit_use_fragment_program(void)
{
	if (!glutExtensionSupported("GL_ARB_fragment_program"))
		return 0;

	get_program_functions();
	return 1;
}

void piglit_require_fragment_program(void)
{
	if (!piglit_use_fragment_program()) {
		printf("GL_ARB_fragment_program not supported.\n");
		piglit_report_result(PIGLIT_SKIP);
		exit(1);
	}
}

int piglit_use_vertex_program(void)
{
	if (!glutExtensionSupported("GL_ARB_vertex_program"))
		return 0;

	get_program_functions();
	return 1;
}

void piglit_require_vertex_program(void)
{
	if (!piglit_use_vertex_program()) {
		printf("GL_ARB_vertex_program not supported.\n");
		piglit_report_result(PIGLIT_SKIP);
		exit(1);
	}
}

GLuint piglit_compile_program(GLenum target, const char* text)
{
	GLuint program;
	GLint errorPos;

	pglGenProgramsARB(1, &program);
	pglBindProgramARB(target, program);
	pglProgramStringARB(
			target,
			GL_PROGRAM_FORMAT_ASCII_ARB,
			strlen(text),
			(const GLubyte *)text);
	glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &errorPos);
	if (glGetError() != GL_NO_ERROR || errorPos != -1) {
		int l = FindLine(text, errorPos);
		int a;

		fprintf(stderr, "Compiler Error (pos=%d line=%d): %s\n",
			errorPos, l,
			(char *) glGetString(GL_PROGRAM_ERROR_STRING_ARB));

		for (a=-10; a<10; a++)
		{
			if (errorPos+a < 0)
				continue;
			if (errorPos+a >= strlen(text))
				break;
			fprintf(stderr, "%c", text[errorPos+a]);
		}
		fprintf(stderr, "\nin program:\n%s", text);
		piglit_report_result(PIGLIT_FAILURE);
	}
	if (!pglIsProgramARB(program)) {
		fprintf(stderr, "pglIsProgramARB failed\n");
		piglit_report_result(PIGLIT_FAILURE);
	}

	return program;
}

/**
 * Convenience function to compile a GLSL shader from a file.
 */
GLint
piglit_compile_shader(GLenum target, char *filename)
{
	GLint prog;
	GLint ok;
	struct stat st;
	int err;
	GLchar *prog_string;
	FILE *f;

	err = stat(filename, &st);
	if (err == -1) {
		fprintf(stderr, "Couldn't stat program: %s\n", strerror(errno));
		exit(1);
	}

	prog_string = malloc(st.st_size + 1);
	if (prog_string == NULL) {
		fprintf(stderr, "malloc\n");
		exit(1);
	}

	f = fopen(filename, "ro");
	if (f == NULL) {
		fprintf(stderr, "Couldn't open program: %s\n", strerror(errno));
		exit(1);
	}
	fread(prog_string, 1, st.st_size, f);
	prog_string[st.st_size] = '\0';
	fclose(f);

	prog = glCreateShader(target);
	glShaderSource(prog, 1, (const GLchar **)&prog_string, NULL);
	glCompileShader(prog);
	glGetShaderiv(prog, GL_COMPILE_STATUS, &ok);
	if (!ok) {
		GLchar *info;
		GLint size;

		glGetShaderiv(prog, GL_INFO_LOG_LENGTH, &size);
		info = malloc(size);

		glGetShaderInfoLog(prog, size, NULL, info);
		fprintf(stderr, "Failed to compile %s: %s\n",
			target == GL_FRAGMENT_SHADER ? "FS" : "VS",
			info);
	}

	free(prog_string);

	return prog;
}

GLint piglit_link_simple_program(GLint vs, GLint fs)
{
	GLint prog, ok;

	prog = glCreateProgram();
	glAttachShader(prog, fs);
	glAttachShader(prog, vs);
	glLinkProgram(prog);
	glGetProgramiv(prog, GL_LINK_STATUS, &ok);
	if (!ok) {
		GLchar *info;
		GLint size;

		glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &size);
		info = malloc(size);

		glGetProgramInfoLog(prog, size, NULL, info);
		fprintf(stderr, "Failed to link: %s\n", info);

		free(info);
	}

	return prog;
}

void
piglit_escape_exit_key(unsigned char key, int x, int y)
{
	(void) x;
	(void) y;
	switch (key) {
		case 27:
			exit(0);
			break;
	}
	glutPostRedisplay();
}

/**
 * Convenience function to draw an axis-aligned rectangle.
 */
GLvoid
piglit_draw_rect(float x, float y, float w, float h)
{
	float verts[4][4];

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
	glEnable(GL_VERTEX_ARRAY);

	glDrawArrays(GL_QUADS, 0, 4);

	glDisable(GL_VERTEX_ARRAY);
}
