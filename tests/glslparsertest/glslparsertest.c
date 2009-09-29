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
 * Tests that drawing to each face of a cube map FBO and then drawing views
 * of those faces to the window system framebuffer succeeds.
 */

#include <errno.h>
#include <sys/stat.h>

#include "piglit-util.h"

#define WIN_WIDTH 200
#define WIN_HEIGHT 100

static char *filename;
static int expected_pass;

static void
display(void)
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

	prog = glCreateShader(type);
	glShaderSource(prog, 1, (const GLchar **)&prog_string, NULL);
	glCompileShader(prog);
	glGetShaderiv(prog, GL_COMPILE_STATUS, &ok);

	pass = (expected_pass == (ok != 0));

	if (pass)
		out = stdout;
	else
		out = stderr;


	glGetShaderiv(prog, GL_INFO_LOG_LENGTH, &size);
	if (size != 0) {
		info = malloc(size);
		glGetShaderInfoLog(prog, size, NULL, info);
	} else {
		info = "(no compiler output)";
	}

	if (!ok) {
		fprintf(out, "Failed to compile %s shader %s: %s\n",
			type == GL_FRAGMENT_SHADER ? "fragment" : "vertex",
			filename, info);
	} else {
		fprintf(out, "Successfully compiled %s shader %s: %s\n",
			type == GL_FRAGMENT_SHADER ? "fragment" : "vertex",
			filename, info);
	}

	if (size != 0)
		free(info);
	free(prog_string);

	glDeleteShader(prog);

	piglit_report_result (pass ? PIGLIT_SUCCESS : PIGLIT_FAILURE);
}

static void usage(char *name)
{
	printf("%s <filename.frag|filename.vert> <pass|fail>\n", name);
	exit(1);
}

int main(int argc, char**argv)
{
	glutInit(&argc, argv);
	if (argc != 3)
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

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(WIN_WIDTH, WIN_HEIGHT);
	glutCreateWindow("glslparsertest");
	glutDisplayFunc(display);
	glewInit();

	if (!GLEW_VERSION_2_0) {
		printf("Requires OpenGL 2.0\n");
		piglit_report_result(PIGLIT_SKIP);
		exit(1);
	}

	glutMainLoop();

	return 0;
}
