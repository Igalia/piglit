

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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *   Ben Holmes <shranzel@hotmail.com>
 */

/*
 * this test draws 256 quads utilizing every permutation of texture
 * swizzling available.
 */

#include "piglit-util-gl-common.h"

PIGLIT_GL_TEST_MAIN(
    400 /*window_width*/,
    300 /*window_height*/,
    GLUT_RGB | GLUT_DOUBLE)

static GLuint tex[1];

static GLfloat verts[12] = {17.0, 1.0, 0.0,
				17.0, 17.0, 0.0,
				1.0, 1.0, 0.0,
				1.0, 17.0, 0.0};

static GLfloat texCoords[8] = {1.0, 0.0,
				1.0, 1.0,
				0.0, 0.0,
				0.0, 1.0};

static GLboolean
probes(void);

static void
loadTex(void)
{
	#define height 2
	#define width 2
	int i, j;

	GLfloat texData[width][height][4];
	for (i = 0; i < width; ++i) {
		for (j = 0; j < height; ++j) {
			if ((i+j) & 1) {
				texData[i][j][0] = 1.0;
				texData[i][j][1] = 0.0;
				texData[i][j][2] = 1.0;
				texData[i][j][3] = 0.0;
			}
			else {
				texData[i][j][0] = 0.0;
				texData[i][j][1] = 1.0;
				texData[i][j][2] = 0.0;
				texData[i][j][3] = 1.0;
			}
		}
	}

	glGenTextures(1, tex);
	glBindTexture(GL_TEXTURE_2D, tex[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
		     GL_FLOAT, texData);

	#undef height
	#undef width
}



void
piglit_init(int argc, char **argv)
{

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	piglit_require_extension("GL_EXT_texture_swizzle");

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_SRC_ALPHA);
	glClearColor(0.6, 0.6, 0.6, 1.0);

	loadTex();
}

enum piglit_result
piglit_display(void)
{
	GLenum rgba[4] = {GL_RED, GL_GREEN, GL_BLUE, GL_ALPHA};
	int i, j, k;
	GLboolean pass = GL_TRUE;

	glClear(GL_COLOR_BUFFER_BIT);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glVertexPointer(3, GL_FLOAT, 0, verts);
	glTexCoordPointer(2, GL_FLOAT, 0, texCoords);

	glBindTexture(GL_TEXTURE_2D, tex[0]);

	glPushMatrix();

	glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA_EXT, (GLint *) rgba);

	for(k = 0; k < 4; ++k) {
		glPushMatrix();
		for(j = 0; j < 4; ++j) {
			for(i = 0; i < 4; ++i) {
				glPushMatrix();
				rgba[3] = GL_ALPHA;
				glTexParameteriv(GL_TEXTURE_2D,
						 GL_TEXTURE_SWIZZLE_RGBA_EXT,
						 (GLint *) rgba);

				glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

				glTranslatef(17.0, 0.0, 0.0);
				rgba[3] = GL_RED;
				glTexParameteriv(GL_TEXTURE_2D,
						 GL_TEXTURE_SWIZZLE_RGBA_EXT,
						 (GLint *) rgba);

				glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

				glTranslatef(17.0, 0.0, 0.0);
				rgba[3] = GL_GREEN;
				glTexParameteriv(GL_TEXTURE_2D,
						 GL_TEXTURE_SWIZZLE_RGBA_EXT,
						 (GLint *) rgba);

				glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

				glTranslatef(17.0, 0.0, 0.0);
				rgba[3] = GL_BLUE;
				glTexParameteriv(GL_TEXTURE_2D,
						 GL_TEXTURE_SWIZZLE_RGBA_EXT,
						 (GLint *) rgba);

				glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
				glPopMatrix();

				switch (i) {
				case 0:
					rgba[2] = GL_RED;
					break;
				case 1:
					rgba[2] = GL_GREEN;
					break;
				case 2:
					rgba[2] = GL_ALPHA;
					break;
				}
				glTranslatef(0.0, 17.0, 0.0);
			}

			rgba[2] = GL_BLUE;

			switch(j) {
			case 0:
				rgba[1] = GL_RED;
				break;
			case 1:
				rgba[1] = GL_BLUE;
				break;
			case 2:
				rgba[1] = GL_ALPHA;
				break;
			}
		}
		glPopMatrix();
		glTranslatef(85.0, 0.0, 0.0);
		rgba[1] = GL_GREEN;

		switch (k) {
		case 0:
			rgba[0] = GL_GREEN;
			break;
		case 1:
			rgba[0] = GL_BLUE;
			break;
		case 2:
			rgba[0] = GL_ALPHA;
			break;
		}

	}

	glPopMatrix();

	pass = probes();

	glFinish();
	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

//probing is done left-to-right, bottom-to-top, by column
//and two probes are done per quad
static GLboolean
probes(void)
{

	int i;

	static const GLfloat greyGreen[3] = {0.6, 1.0, 0.6};
	static const GLfloat green[3] = {0.0, 1.0, 0.0};
	static const GLfloat greyPink[3] = {1.0, 0.6, 1.0};
	static const GLfloat pink[3] = {1.0, 0.0, 1.0};
	static const GLfloat greyBlueGreen[3] = {0.6, 1.0, 1.0};
	static const GLfloat red[3] = {1.0, 0.0, 0.0};
	static const GLfloat blueGreen[3] = {0.0, 1.0, 1.0};
	static const GLfloat greyRed[3] = {1.0, 0.6, 0.6};
	static const GLfloat grey[3] = {0.6, 0.6, 0.6};
	static const GLfloat white[3] = {1.0, 1.0, 1.0};
	static const GLfloat black[3] = {0.0, 0.0, 0.0};
	static const GLfloat blue[3] = {0.0, 0.0, 1.0};
	static const GLfloat yellow[3] = {1.0, 1.0, 0.0};
	static const GLfloat greyYellow[3] = {1.0, 1.0, 0.6};
	static const GLfloat greyBlue[3] = {0.6, 0.6, 1.0};

	GLboolean pass = GL_TRUE;

	static const struct {
		int x, y;
		const GLfloat *color;
	} expect[] = {

	//first column
	{2,2,greyGreen},
	{10,2,pink},
	{20,2,green},
	{28,2,greyPink},
	{38,2,greyGreen},
	{46,2,pink},
	{56,2,green},
	{64,2,greyPink},

	{2,19,greyGreen},
	{10,19,pink},
	{20,19,green},
	{28,19,greyPink},
	{38,19,greyGreen},
	{46,19,pink},
	{56,19,green},
	{64,19,greyPink},

	{2,36,greyBlueGreen},
	{10,36,red},
	{20,36,blueGreen},
	{28,36,greyRed},
	{38,36,greyBlueGreen},
	{46,36,red},
	{56,36,blueGreen},
	{64,36,greyRed},

	{2,53,greyBlueGreen},
	{10,53,red},
	{20,53,blueGreen},
	{28,53,greyRed},
	{38,53,greyBlueGreen},
	{46,53,red},
	{56,53,blueGreen},
	{64,53,greyRed},

	{2,70,grey},
	{10,70,white},
	{20,70,black},
	{28,70,white},
	{38,70,grey},
	{46,70,white},
	{56,70,black},
	{64,70,white},

	{2,87,grey},
	{10,87,white},
	{20,87,black},
	{28,87,white},
	{38,87,grey},
	{46,87,white},
	{56,87,black},
	{64,87,white},

	{2,104,greyBlue},
	{10,104,yellow},
	{20,104,blue},
	{28,104,greyYellow},
	{38,104,greyBlue},
	{46,104,yellow},
	{56,104,blue},
	{64,104,greyYellow},

	{2,121,greyBlue},
	{10,121,yellow},
	{20,121,blue},
	{28,121,greyYellow},
	{38,121,greyBlue},
	{46,121,yellow},
	{56,121,blue},
	{64,121,greyYellow},

	{2,138,grey},
	{10,138,white},
	{20,138,black},
	{28,138,white},
	{38,138,grey},
	{46,138,white},
	{56,138,black},
	{64,138,white},

	{2,155,grey},
	{10,155,white},
	{20,155,black},
	{28,155,white},
	{38,155,grey},
	{46,155,white},
	{56,155,black},
	{64,155,white},

	{2,172,greyBlue},
	{10,172,yellow},
	{20,172,blue},
	{28,172,greyYellow},
	{38,172,greyBlue},
	{46,172,yellow},
	{56,172,blue},
	{64,172,greyYellow},

	{2,189,greyBlue},
	{10,189,yellow},
	{20,189,blue},
	{28,189,greyYellow},
	{38,189,greyBlue},
	{46,189,yellow},
	{56,189,blue},
	{64,189,greyYellow},

	{2,206,greyGreen},
	{10,206,pink},
	{20,206,green},
	{28,206,greyPink},
	{38,206,greyGreen},
	{46,206,pink},
	{56,206,green},
	{64,206,greyPink},

	{2,223,greyGreen},
	{10,223,pink},
	{20,223,green},
	{28,223,greyPink},
	{38,223,greyGreen},
	{46,223,pink},
	{56,223,green},
	{64,223,greyPink},

	{2,240,greyBlueGreen},
	{10,240,red},
	{20,240,blueGreen},
	{28,240,greyRed},
	{38,240,greyBlueGreen},
	{46,240,red},
	{56,240,blueGreen},
	{64,240,greyRed},

	{2,257,greyBlueGreen},
	{10,257,red},
	{20,257,blueGreen},
	{28,257,greyRed},
	{38,257,greyBlueGreen},
	{46,257,red},
	{56,257,blueGreen},
	{64,257,greyRed},

	//second column
	{87,2,greyYellow},
	{95,2,blue},
	{105,2,yellow},
	{113,2,greyBlue},
	{123,2,greyYellow},
	{131,2,blue},
	{141,2,yellow},
	{149,2,greyBlue},

	{87,19,greyYellow},
	{95,19,blue},
	{105,19,yellow},
	{113,19,greyBlue},
	{123,19,greyYellow},
	{131,19,blue},
	{141,19,yellow},
	{149,19,greyBlue},

	{87,36,white},
	{95,36,black},
	{105,36,white},
	{113,36,grey},
	{123,36,white},
	{131,36,black},
	{141,36,white},
	{149,36,grey},

	{87,53,white},
	{95,53,black},
	{105,53,white},
	{113,53,grey},
	{123,53,white},
	{131,53,black},
	{141,53,white},
	{149,53,grey},

	{87,70,greyRed},
	{95,70,blueGreen},
	{105,70,red},
	{113,70,greyBlueGreen},
	{123,70,greyRed},
	{131,70,blueGreen},
	{141,70,red},
	{149,70,greyBlueGreen},

	{87,87,greyRed},
	{95,87,blueGreen},
	{105,87,red},
	{113,87,greyBlueGreen},
	{123,87,greyRed},
	{131,87,blueGreen},
	{141,87,red},
	{149,87,greyBlueGreen},

	{87,104,greyPink},
	{95,104,green},
	{105,104,pink},
	{113,104,greyGreen},
	{123,104,greyPink},
	{131,104,green},
	{141,104,pink},
	{149,104,greyGreen},

	{87,121,greyPink},
	{95,121,green},
	{105,121,pink},
	{113,121,greyGreen},
	{123,121,greyPink},
	{131,121,green},
	{141,121,pink},
	{149,121,greyGreen},

	{87,138,greyRed},
	{95,138,blueGreen},
	{105,138,red},
	{113,138,greyBlueGreen},
	{123,138,greyRed},
	{131,138,blueGreen},
	{141,138,red},
	{149,138,greyBlueGreen},

	{87,155,greyRed},
	{95,155,blueGreen},
	{105,155,red},
	{113,155,greyBlueGreen},
	{123,155,greyRed},
	{131,155,blueGreen},
	{141,155,red},
	{149,155,greyBlueGreen},

	{87,172,greyPink},
	{95,172,green},
	{105,172,pink},
	{113,172,greyGreen},
	{123,172,greyPink},
	{131,172,green},
	{141,172,pink},
	{149,172,greyGreen},

	{87,189,greyPink},
	{95,189,green},
	{105,189,pink},
	{113,189,greyGreen},
	{123,189,greyPink},
	{131,189,green},
	{141,189,pink},
	{149,189,greyGreen},

	{87,206,greyYellow},
	{95,206,blue},
	{105,206,yellow},
	{113,206,greyBlue},
	{123,206,greyYellow},
	{131,206,blue},
	{141,206,yellow},
	{149,206,greyBlue},

	{87,223,greyYellow},
	{95,223,blue},
	{105,223,yellow},
	{113,223,greyBlue},
	{123,223,greyYellow},
	{131,223,blue},
	{141,223,yellow},
	{149,223,greyBlue},

	{87,240,white},
	{95,240,black},
	{105,240,white},
	{113,240,grey},
	{123,240,white},
	{131,240,black},
	{141,240,white},
	{149,240,grey},

	{87,257,white},
	{95,257,black},
	{105,257,white},
	{113,257,grey},
	{123,257,white},
	{131,257,black},
	{141,257,white},
	{149,257,grey},

	//third column
	{172,2,greyGreen},
	{180,2,pink},
	{190,2,green},
	{198,2,greyPink},
	{208,2,greyGreen},
	{216,2,pink},
	{226,2,green},
	{234,2,greyPink},

	{172,19,greyGreen},
	{180,19,pink},
	{190,19,green},
	{198,19,greyPink},
	{208,19,greyGreen},
	{216,19,pink},
	{226,19,green},
	{234,19,greyPink},

	{172,36,greyBlueGreen},
	{180,36,red},
	{190,36,blueGreen},
	{198,36,greyRed},
	{208,36,greyBlueGreen},
	{216,36,red},
	{226,36,blueGreen},
	{234,36,greyRed},

	{172,53,greyBlueGreen},
	{180,53,red},
	{190,53,blueGreen},
	{198,53,greyRed},
	{208,53,greyBlueGreen},
	{216,53,red},
	{226,53,blueGreen},
	{234,53,greyRed},

	{172,70,grey},
	{180,70,white},
	{190,70,black},
	{198,70,white},
	{208,70,grey},
	{216,70,white},
	{226,70,black},
	{234,70,white},

	{172,87,grey},
	{180,87,white},
	{190,87,black},
	{198,87,white},
	{208,87,grey},
	{216,87,white},
	{226,87,black},
	{234,87,white},

	{172,104,greyBlue},
	{180,104,yellow},
	{190,104,blue},
	{198,104,greyYellow},
	{208,104,greyBlue},
	{216,104,yellow},
	{226,104,blue},
	{234,104,greyYellow},

	{172,121,greyBlue},
	{180,121,yellow},
	{190,121,blue},
	{198,121,greyYellow},
	{208,121,greyBlue},
	{216,121,yellow},
	{226,121,blue},
	{234,121,greyYellow},

	{172,138,grey},
	{180,138,white},
	{190,138,black},
	{198,138,white},
	{208,138,grey},
	{216,138,white},
	{226,138,black},
	{234,138,white},

	{172,155,grey},
	{180,155,white},
	{190,155,black},
	{198,155,white},
	{208,155,grey},
	{216,155,white},
	{226,155,black},
	{234,155,white},

	{172,172,greyBlue},
	{180,172,yellow},
	{190,172,blue},
	{198,172,greyYellow},
	{208,172,greyBlue},
	{216,172,yellow},
	{226,172,blue},
	{234,172,greyYellow},

	{172,189,greyBlue},
	{180,189,yellow},
	{190,189,blue},
	{198,189,greyYellow},
	{208,189,greyBlue},
	{216,189,yellow},
	{226,189,blue},
	{234,189,greyYellow},

	{172,206,greyGreen},
	{180,206,pink},
	{190,206,green},
	{198,206,greyPink},
	{208,206,greyGreen},
	{216,206,pink},
	{226,206,green},
	{234,206,greyPink},

	{172,223,greyGreen},
	{180,223,pink},
	{190,223,green},
	{198,223,greyPink},
	{208,223,greyGreen},
	{216,223,pink},
	{226,223,green},
	{234,223,greyPink},

	{172,240,greyBlueGreen},
	{180,240,red},
	{190,240,blueGreen},
	{198,240,greyRed},
	{208,240,greyBlueGreen},
	{216,240,red},
	{226,240,blueGreen},
	{234,240,greyRed},

	{172,257,greyBlueGreen},
	{180,257,red},
	{190,257,blueGreen},
	{198,257,greyRed},
	{208,257,greyBlueGreen},
	{216,257,red},
	{226,257,blueGreen},
	{234,257,greyRed},

	//fourth column
	{257,2,greyYellow},
	{265,2,blue},
	{275,2,yellow},
	{283,2,greyBlue},
	{293,2,greyYellow},
	{301,2,blue},
	{311,2,yellow},
	{319,2,greyBlue},

	{257,19,greyYellow},
	{265,19,blue},
	{275,19,yellow},
	{283,19,greyBlue},
	{293,19,greyYellow},
	{301,19,blue},
	{311,19,yellow},
	{319,19,greyBlue},

	{257,36,white},
	{265,36,black},
	{275,36,white},
	{283,36,grey},
	{293,36,white},
	{301,36,black},
	{311,36,white},
	{319,36,grey},

	{257,53,white},
	{265,53,black},
	{275,53,white},
	{283,53,grey},
	{293,53,white},
	{301,53,black},
	{311,53,white},
	{319,53,grey},

	{257,70,greyRed},
	{265,70,blueGreen},
	{275,70,red},
	{283,70,greyBlueGreen},
	{293,70,greyRed},
	{301,70,blueGreen},
	{311,70,red},
	{319,70,greyBlueGreen},

	{257,87,greyRed},
	{265,87,blueGreen},
	{275,87,red},
	{283,87,greyBlueGreen},
	{293,87,greyRed},
	{301,87,blueGreen},
	{311,87,red},
	{319,87,greyBlueGreen},

	{257,104,greyPink},
	{265,104,green},
	{275,104,pink},
	{283,104,greyGreen},
	{293,104,greyPink},
	{301,104,green},
	{311,104,pink},
	{319,104,greyGreen},

	{257,121,greyPink},
	{265,121,green},
	{275,121,pink},
	{283,121,greyGreen},
	{293,121,greyPink},
	{301,121,green},
	{311,121,pink},
	{319,121,greyGreen},

	{257,138,greyRed},
	{265,138,blueGreen},
	{275,138,red},
	{283,138,greyBlueGreen},
	{293,138,greyRed},
	{301,138,blueGreen},
	{311,138,red},
	{319,138,greyBlueGreen},

	{257,155,greyRed},
	{265,155,blueGreen},
	{275,155,red},
	{283,155,greyBlueGreen},
	{293,155,greyRed},
	{301,155,blueGreen},
	{311,155,red},
	{319,155,greyBlueGreen},

	{257,172,greyPink},
	{265,172,green},
	{275,172,pink},
	{283,172,greyGreen},
	{293,172,greyPink},
	{301,172,green},
	{311,172,pink},
	{319,172,greyGreen},

	{257,189,greyPink},
	{265,189,green},
	{275,189,pink},
	{283,189,greyGreen},
	{293,189,greyPink},
	{301,189,green},
	{311,189,pink},
	{319,189,greyGreen},

	{257,206,greyYellow},
	{265,206,blue},
	{275,206,yellow},
	{283,206,greyBlue},
	{293,206,greyYellow},
	{301,206,blue},
	{311,206,yellow},
	{319,206,greyBlue},

	{257,223,greyYellow},
	{265,223,blue},
	{275,223,yellow},
	{283,223,greyBlue},
	{293,223,greyYellow},
	{301,223,blue},
	{311,223,yellow},
	{319,223,greyBlue},

	{257,240,white},
	{265,240,black},
	{275,240,white},
	{283,240,grey},
	{293,240,white},
	{301,240,black},
	{311,240,white},
	{319,240,grey},

	{257,257,white},
	{265,257,black},
	{275,257,white},
	{283,257,grey},
	{293,257,white},
	{301,257,black},
	{311,257,white},
	{319,257,grey},
	{-1,-1,NULL}
	};


	for(i = 0; pass && (expect[i].color != NULL); ++i) {
		pass = pass && piglit_probe_pixel_rgb(expect[i].x,
		expect[i].y,
		expect[i].color);
	}

	return pass;
}
