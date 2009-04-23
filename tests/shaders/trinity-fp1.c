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

/**
 * Test a fragment program that
 * \sa http://www.mail-archive.com/dri-devel%40lists.sourceforge.net/msg30180.html
 */

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#if defined(__APPLE__)
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include "piglit-util.h"

static GLuint TexDiffuse = 1;
static GLuint TexNormal = 2;
static GLuint TexSpecular = 3;
static GLuint TexLookup = 4;

static GLuint FragProg;

static int Automatic = 0;

static int Width = 200, Height = 100;


static void DoFrame(void)
{
	static float Local[3][4] = {
		{ 1.0, 0.8, 1.0, 1.0 },
		{ 0.5, 0.5, 0.5, 1.0 },
		{ 1.0, 0.0, 0.0, 1.0 }
	};
	static float Local2[3][4] = {
		{ 0.8, 1.0, 1.0, 1.0 },
		{ 0.5, 0.5, 0.5, 1.0 },
		{ 1.0, 0.0, 1.0, 1.0 }
	};
	int i;

	glClearColor(0.8, 0.8, 0.8, 0.8);
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, TexDiffuse);
	glEnable(GL_TEXTURE_2D);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, TexNormal);
	glEnable(GL_TEXTURE_2D);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, TexSpecular);
	glEnable(GL_TEXTURE_2D);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, TexLookup);
	glEnable(GL_TEXTURE_2D);

	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, TexLookup);
	glEnable(GL_TEXTURE_2D);

	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, TexLookup);
	glEnable(GL_TEXTURE_2D);

	glMultiTexCoord2f(0, 0.0, 0.0);
	glMultiTexCoord2f(1, 0.0, 0.0);
	glMultiTexCoord2f(2, 0.0, 0.0);
	glMultiTexCoord3f(3, 1.0, 0.05, 0.25);
	glMultiTexCoord3f(4, 4, -3, 0);
	glMultiTexCoord3f(5, 0, 3, 4);

	glEnable(GL_FRAGMENT_PROGRAM_ARB);
	for(i = 0; i < 3; ++i)
		pglProgramLocalParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, i, Local[i]);

	glBegin(GL_QUADS);
		glVertex2f(0.75, 0.75);
		glVertex2f(0.25, 0.75);
		glVertex2f(0.25, 0.25);
		glVertex2f(0.75, 0.25);
	glEnd();

	glEnable(GL_FRAGMENT_PROGRAM_ARB);
	for(i = 0; i < 3; ++i)
		pglProgramLocalParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, i, Local2[i]);

	glBegin(GL_QUADS);
		glVertex2f(1.75, 0.75);
		glVertex2f(1.25, 0.75);
		glVertex2f(1.25, 0.25);
		glVertex2f(1.75, 0.25);
	glEnd();

	glutSwapBuffers();
}

static int DoTest( void )
{
	static const float expected[2][3] = {
		{ 0.30, 0.23, 0.40 },
		{ 0.24, 0.29, 0.40 }
	};
	int i;
	GLfloat dmax = 0;

	glReadBuffer( GL_FRONT );

	for(i = 0; i < 2; ++i) {
		GLfloat probe[4];
		GLfloat delta[3];
		int j;

		glReadPixels(Width*(2*i+1)/4, Height/2, 1, 1, GL_RGBA, GL_FLOAT, probe);
		printf("Probe: %f,%f,%f\n", probe[0], probe[1], probe[2]);

		for(j = 0; j < 3; ++j) {
			delta[j] = probe[j] - expected[i][j];
			printf("   Delta: %f,%f,%f\n", delta[0], delta[1], delta[2]);
			if (delta[j] > dmax) dmax = delta[j];
			else if (-delta[j] > dmax) dmax = -delta[j];
		}
	}

	printf("Max delta: %f\n", dmax);

	if (dmax >= 0.02)
		return 0;
	else
		return 1;
}


static void Redisplay(void)
{
	int succ;

	DoFrame();
	succ = DoTest();

	if (Automatic) {
		printf("\nPIGLIT: { 'result': '%s' }\n", succ ? "pass" : "fail");
		exit(0);
	}
}


static void Reshape(int width, int height)
{
	Width = width;
	Height = height;
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, 2.0, 0.0, 1.0, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}


static void Key(unsigned char key, int x, int y)
{
	(void) x;
	(void) y;
	switch (key) {
		case 27:
			pglDeleteProgramsARB(1, &FragProg);
			exit(0);
			break;
	}
	glutPostRedisplay();
}


static void Init(void)
{
	GLint errorPos;
	GLubyte data[256][256][4];
	int x,y;

	static const char *fragProgramText =
		"!!ARBfp1.0\n"
		"# $Id$\n"
		"# Copyright (C) 2006  Oliver McFadden <z3ro.geek@gmail.com>\n"
		"#\n"
		"# This program is free software; you can redistribute it and/or modify\n"
		"# it under the terms of the GNU General Public License as published by\n"
		"# the Free Software Foundation; either version 2 of the License, or\n"
		"# (at your option) any later version.\n"
		"#\n"
		"# This program is distributed in the hope that it will be useful,\n"
		"# but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
		"# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
		"# GNU General Public License for more details.\n"
		"#\n"
		"# You should have received a copy of the GNU General Public License\n"
		"# along with this program; if not, write to the Free Software\n"
		"# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA\n"

		"TEMP H, L, N, V, attenuationxy, attenuationz, color, diffuse, dot, specular, tmp;\n"

		"DP3 L.x, fragment.texcoord[4], fragment.texcoord[4];\n"
		"RSQ L.x, L.x;\n"
		"MUL L.xyz, L.x, fragment.texcoord[4];\n"

		"DP3 V.x, fragment.texcoord[5], fragment.texcoord[5];\n"
		"RSQ V.x, V.x;\n"
		"MUL V.xyz, V.x, fragment.texcoord[5];\n"

		"ADD tmp, L, V;\n"
		"DP3 H.x, tmp, tmp;\n"
		"RSQ H.x, H.x;\n"
		"MUL H.xyz, H.x, tmp;\n"

		"TEX tmp.xyz, fragment.texcoord[1], texture[1], 2D;\n"
		"MAD tmp.xyz, tmp, 2.0, -1.0;\n"
		"DP3 N.x, tmp, tmp;\n"
		"RSQ N.x, N.x;\n"
		"MUL N.xyz, N.x, tmp;\n"

		"DP3_SAT dot.x, N, L;\n"
		"MUL dot.xyz, program.local[0], dot.x;\n"

		"TEX diffuse.xyz, fragment.texcoord[0], texture[0], 2D;\n"

		"DP3_SAT tmp.x, N, H;\n"
		"POW tmp.x, tmp.x, program.local[2].x;\n"
		"TEX specular.xyz, fragment.texcoord[2], texture[2], 2D;\n"
		"MUL specular.xyz, specular, program.local[0];\n"
		"MUL specular.xyz, specular, tmp.x;\n"

		"TEX attenuationxy.xyz, fragment.texcoord[3], texture[3], 2D;\n"

		"MOV tmp.x, fragment.texcoord[3].z;\n"
		"MOV tmp.y, 0;\n"
		"TEX attenuationz.xyz, tmp, texture[4], 2D;\n"

		"MOV color, diffuse;\n"
		"MUL color.xyz, color, dot;\n"
		"ADD color.xyz, color, specular;\n"
		"MUL color.xyz, color, attenuationxy;\n"
		"MUL color.xyz, color, attenuationz;\n"
		"MUL color.xyz, color, program.local[1].x;\n"
		"MOV result.color, color;\n"

		"END";

	printf("GL_RENDERER = %s\n", (char *) glGetString(GL_RENDERER));

	piglit_require_fragment_program();
	FragProg = piglit_compile_program(GL_FRAGMENT_PROGRAM_ARB, fragProgramText);

	/*
	 * Initialize textures
	 */
	// Diffuse
	for(y = 0; y < 256; ++y) {
		for(x = 0; x < 256; ++x) {
			data[y][x][0] = 255; // 1.0
			data[y][x][1] = 192; // 0.75
			data[y][x][2] = 255; // 1.0
			data[y][x][3] = 0;
		}
	}

	glBindTexture(GL_TEXTURE_2D, TexDiffuse);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, data );

	// Normal
	for(y = 0; y < 256; ++y) {
		for(x = 0; x < 256; ++x) {
			data[y][x][0] = 255; // 1.0
			data[y][x][1] = 0; // 0.0
			data[y][x][2] = 0; // 0.0
			data[y][x][3] = 0;
		}
	}

	glBindTexture(GL_TEXTURE_2D, TexNormal);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, data );

	// Specular
	for(y = 0; y < 256; ++y) {
		for(x = 0; x < 256; ++x) {
			data[y][x][0] = 255; // 1.0
			data[y][x][1] = 255; // 1.0
			data[y][x][2] = 192; // 0.75
			data[y][x][3] = 0;
		}
	}

	glBindTexture(GL_TEXTURE_2D, TexSpecular);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, data );

	// Lookup texture
	for(y = 0; y < 256; ++y) {
		for(x = 0; x < 256; ++x) {
			data[y][x][0] = 255-x;
			data[y][x][1] = 255-y;
			data[y][x][2] = 255;
			data[y][x][3] = 0;
		}
	}

	glBindTexture(GL_TEXTURE_2D, TexLookup);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, data );

	Reshape(Width,Height);
}


int main(int argc, char *argv[])
{
	glutInit(&argc, argv);
	if (argc == 2 && !strcmp(argv[1], "-auto"))
		Automatic = 1;
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(Width, Height);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutCreateWindow(argv[0]);
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Key);
	glutDisplayFunc(Redisplay);
	Init();
	glutMainLoop();
	return 0;
}
