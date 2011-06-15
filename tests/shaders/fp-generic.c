/*
 * Copyright (c) The Piglit project 2008
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

/**
 * @file
 * Generic ARB_fragment_program test, to test ALU correctness.
 * Takes an input file of the following form:
 *
 * nr-tests nr-texcoords nr-teximages
 * tc
 * s t r q   [input texture coordinates]
 * ...
 * tex
 * r g b a   [color of texture images]
 * ...
 * expected
 * r g b a
 * tc
 * ...
 * !!ARBfp1.0
 * ...
 */

#include "piglit-util.h"

/*
================================================================

Testcase helpers

*/

struct testinstance {
	GLfloat* texcoords;
	GLfloat* teximages;
	GLfloat expected[4];
};

struct testcase {
	char* programtext;
	int nrTexCoords;
	int nrTexImages;
	int nrInstances;
	struct testinstance* instances;
};

static void expect(FILE* filp, const char* str)
{
	char buf[41];
	fscanf(filp, "%40s", buf);
	if (strcmp(buf, str)) {
		fprintf(stderr, "Expected '%s', got '%s'\n", str, buf);
		exit(-1);
	}
}

static GLfloat* readfloatarray(FILE* filp, int count)
{
	GLfloat* dest = (GLfloat*)malloc(sizeof(GLfloat)*count);
	int i;

	for(i = 0; i < count; ++i)
		fscanf(filp, "%f", &dest[i]);

	return dest;
}

static void readTestcase(struct testcase* tc, const char* filename)
{
	FILE* filp = fopen(filename, "rt");
	char buf[256];
	int i;

	if (!filp) {
		fprintf(stderr, "Failed to read test data: %s\n", filename);
		exit(-1);
	}

	fscanf(filp, "%i %i %i", &tc->nrInstances, &tc->nrTexCoords, &tc->nrTexImages);
	tc->instances = (struct testinstance*)malloc(tc->nrInstances * sizeof(struct testinstance));

	for(i = 0; i < tc->nrInstances; ++i) {
		struct testinstance* inst = tc->instances + i;

		expect(filp, "tc");
		inst->texcoords = readfloatarray(filp, tc->nrTexCoords*4);

		expect(filp, "tex");
		inst->teximages = readfloatarray(filp, tc->nrTexImages*4);

		expect(filp, "expected");
		fscanf(filp, "%f %f %f %f",
			&inst->expected[0], &inst->expected[1],
			&inst->expected[2], &inst->expected[3]);
	}

	/* Yeah, this is not especially efficient... */
	tc->programtext = strdup("");
	while(fgets(buf, sizeof(buf), filp)) {
		int newlen;
		if (!*tc->programtext && buf[0] != '!')
			continue;
		newlen = tc->programtext ? strlen(tc->programtext) : 0;
		newlen += strlen(buf);
		tc->programtext = (char*)realloc(tc->programtext, newlen+1);
		strcat(tc->programtext, buf);
		if (!strncmp(buf, "END", 3))
			break;
	}

	fclose(filp);
}


/*
================================================================

GL program

*/

int piglit_width = 100, piglit_height = 100;
int piglit_window_mode = GLUT_RGB | GLUT_DOUBLE | GLUT_ALPHA;

static const char* Filename = 0;
static struct testcase Testcase;
static GLuint FragProg;



static void TestInstance(struct testinstance* instance)
{
	int i;

	glClearColor(0.0, 0.0, 1.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glEnable(GL_FRAGMENT_PROGRAM_ARB);
	glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, FragProg);

	for(i = 0; i < Testcase.nrTexCoords; ++i)
		glMultiTexCoord4fv(GL_TEXTURE0+i, instance->texcoords + 4*i);

	for(i = 0; i < Testcase.nrTexImages; ++i) {
		glActiveTexture(GL_TEXTURE0+i);
		glBindTexture(GL_TEXTURE_2D, i+1);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_FLOAT, instance->teximages + 4*i);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}

	glBegin(GL_QUADS);
		glVertex2f(0, 0);
		glVertex2f(1, 0);
		glVertex2f(1, 1);
		glVertex2f(0, 1);
	glEnd();

	if (!piglit_probe_pixel_rgba(piglit_width/2, piglit_height/2, instance->expected)) {
		fprintf(stderr, "Test %s, instance #%li failed\n", Filename, (long)(instance-Testcase.instances));
		piglit_report_result(PIGLIT_FAIL);
	}

	glutSwapBuffers();
}

enum piglit_result
piglit_display(void)
{
	int i;
	for(i = 0; i < Testcase.nrInstances; ++i)
		TestInstance(&Testcase.instances[i]);

	return PIGLIT_PASS;
}


static void Reshape(int width, int height)
{
	piglit_width = width;
	piglit_height = height;
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}


void
piglit_init(int argc, char **argv)
{
	int i;

	piglit_automatic = GL_TRUE;

	if (!GLEW_VERSION_1_3) {
		printf("Requires OpenGL 1.3\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	for(i = 1; i < argc; ++i) {
		if (!Filename)
			Filename = argv[i];
	}
	if (!Filename) {
		fprintf(stderr, "Need to give a testcase file\n");
		printf("PIGLIT: {'result': 'fail' }\n");
		exit(-1);
	}
	readTestcase(&Testcase, Filename);

	glutReshapeFunc(Reshape);

	piglit_require_fragment_program();
	FragProg = piglit_compile_program(GL_FRAGMENT_PROGRAM_ARB, Testcase.programtext);

	Reshape(piglit_width, piglit_height);
}
