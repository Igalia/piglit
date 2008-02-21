// BEGIN_COPYRIGHT -*- glean -*-

/* 
 * Copyright (C) 2007  Intel Corporation
 * Copyright (C) 1999  Allen Akin   All Rights Reserved.
 * 
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 * 
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the
 * Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
 * KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL ALLEN AKIN BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 * 
 */

/* tpointsprite.cpp:  Test the ARB_point_sprite extension
 * Author: Nian Wu <nian.wu@intel.com>
 *
 * Test procedure:
 *   Create mipmap textures which size varies from 32x32 to 1x1, every texture 
 *   has different two colors: the upper half is one color and the lower half
 *   is another color.
 *   Draw point and polygon which mode is GL_POINT, and check that the point
 *   is rendered correctly.
 */

#define GL_GLEXT_PROTOTYPES
#include "tpointsprite.h"
#include <cassert>
#include <cmath>
#include <stdio.h>

namespace GLEAN {

//background color
static GLfloat   bgColor[4] = {0.0, 0.0, 0.0, 0.0};

//mipmap texture's color, every texture partite to upper and lower part that 
//has different colors
//for 1x1 texture, only lower part is used
static GLfloat   texColor[6][2][4] = {
          {{1.0, 0.0, 0.0, 1.0}, {0.0, 1.0, 0.0, 1.0}},  // 32x32
          {{0.0, 0.0, 1.0, 1.0}, {1.0, 1.0, 0.0, 1.0}},  // 16x16
          {{1.0, 0.0, 1.0, 1.0}, {0.0, 1.0, 1.0, 1.0}},  // 8x8
          {{1.0, 1.0, 1.0, 1.0}, {1.0, 0.0, 0.0, 1.0}},  // 4x4
          {{0.0, 1.0, 0.0, 1.0}, {0.0, 0.0, 1.0, 1.0}},  // 2x2
          {{1.0, 1.0, 0.0, 1.0}, {1.0, 1.0, 1.0, 1.0}},  // 1x1
};

//generate mipmap
void
PointSpriteTest::GenMipmap()
{
	int       level, i, j;
	GLint     texWidth;
	GLfloat   *texPtr;
	GLfloat   *upperColor, *lowColor;

	for (level = 0; level < 6; level++)
	{
		texWidth = 1 << (6 - level - 1);
		texImages[level] = (GLfloat *)malloc(texWidth * texWidth * 4 * sizeof(GLfloat));
		texPtr = texImages[level];
		upperColor = texColor[level][0];
		lowColor = texColor[level][1];

		for (i = 0; i < texWidth; i++)
		{
			for (j = 0; j < texWidth; j++)
			{
				if (i < texWidth / 2) //lower part
				{
					*texPtr++ = lowColor[0];
					*texPtr++ = lowColor[1];
					*texPtr++ = lowColor[2];
					*texPtr++ = lowColor[3];
				} else {  //upper part
					*texPtr++ = upperColor[0];
					*texPtr++ = upperColor[1];
					*texPtr++ = upperColor[2];
					*texPtr++ = upperColor[3];
				}
			}
		}
	}
}

//enable texture and setup mipmap
void
PointSpriteTest::SetupMipmap(GLuint *texID)
{
	glEnable(GL_TEXTURE_2D);

	glGenTextures(1, texID);
	glBindTexture(GL_TEXTURE_2D, *texID);

	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,
				GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 32, 32, 0,
				GL_RGBA, GL_FLOAT, texImages[0]);
	glTexImage2D(GL_TEXTURE_2D, 1, GL_RGBA, 16, 16, 0,
				GL_RGBA, GL_FLOAT, texImages[1]);
	glTexImage2D(GL_TEXTURE_2D, 2, GL_RGBA, 8, 8, 0,
				GL_RGBA, GL_FLOAT, texImages[2]);
	glTexImage2D(GL_TEXTURE_2D, 3, GL_RGBA, 4, 4, 0,
				GL_RGBA, GL_FLOAT, texImages[3]);
	glTexImage2D(GL_TEXTURE_2D, 4, GL_RGBA, 2, 2, 0,
				GL_RGBA, GL_FLOAT, texImages[4]);
	glTexImage2D(GL_TEXTURE_2D, 5, GL_RGBA, 1, 1, 0, 
				GL_RGBA, GL_FLOAT, texImages[5]);

	glTexEnvf(GL_POINT_SPRITE_ARB, GL_COORD_REPLACE_ARB, GL_TRUE);
}

void
PointSpriteTest::CheckDefaultState(MultiTestResult &r)
{
	GLboolean enable;
	GLint     coordReplace;
	GLint     coordOrigin;

	// check point sprite status, default is GL_FALSE
	enable = glIsEnabled(GL_POINT_SPRITE_ARB);
	if (enable != GL_FALSE)
	{
		env->log << name << "subcase FAIL: "
			 << "PointSprite should be disabled defaultlly\n";
		r.numFailed++;
	} else {
		r.numPassed++;
	}

	// check coordinate replacement, default is GL_FALSE
	glGetTexEnviv(GL_POINT_SPRITE_ARB, GL_COORD_REPLACE_ARB, &coordReplace);

	if (coordReplace != GL_FALSE)
	{
		env->log << name << "subcase FAIL: "
			 << "default value of COORD_REPLACE should be GL_FALSE\n";
		r.numFailed++;
	} else {
		r.numPassed++;
	}

	// check coordinate origin, default is UPPER_LEFT
	glEnable(GL_POINT_SPRITE);
	glGetIntegerv(GL_POINT_SPRITE_COORD_ORIGIN, &coordOrigin);
	if (coordOrigin != GL_UPPER_LEFT)
	{
		env->log << name << "subcase FAIL: "
			 << "defult value of COORD_ORIGIN should be GL_UPPER_LEFT\n";
		r.numFailed++;
	} else {
		r.numPassed++;
	}

	glDisable(GL_POINT_SPRITE);
}

GLboolean
PointSpriteTest::OutOfPoint(int x, int y, int pSize, int x0, int y0)
{
	if ((x < x0) ||
	    (y < y0) ||
	    (x >= x0 + pSize) ||
	    (y >= y0 + pSize))
		return GL_TRUE;
	else
		return GL_FALSE;
}

GLfloat *
PointSpriteTest::GetTexColor(int pSize, int dir)
{
	int level;

	// Note: we use GL_NEAREST_MIPMAP_NEAREST for  GL_TEXTURE_MIN_FILTER
	if (pSize <= 1) level = 5;
	else if (pSize < 3) level = 4;
	else if (pSize < 6) level = 3;
	else if (pSize < 12) level = 2;
	else if (pSize < 24) level = 1;
	else  level = 0;

	return texColor[level][dir];
}

void
PointSpriteTest::CalculateTolerance()
{
        GLint rBits, gBits, bBits;
        GLint rTexBits, gTexBits, bTexBits;

        // Get fb resolution
        glGetIntegerv(GL_RED_BITS, &rBits);
        glGetIntegerv(GL_GREEN_BITS, &gBits);
        glGetIntegerv(GL_BLUE_BITS, &bBits);

	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0,
                        GL_TEXTURE_RED_SIZE, &rTexBits);
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0,
                        GL_TEXTURE_GREEN_SIZE, &gTexBits);
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0,
                        GL_TEXTURE_BLUE_SIZE, &bTexBits);

        // Find smaller of frame buffer and texture bits
        rBits = (rBits < rTexBits) ? rBits : rTexBits;
        gBits = (gBits < gTexBits) ? gBits : gTexBits;
        bBits = (bBits < bTexBits) ? bBits : bTexBits;

        mTolerance[0] = 3.0 / (1 << rBits);
        mTolerance[1] = 3.0 / (1 << gBits);
        mTolerance[2] = 3.0 / (1 << bBits);
}

//Test if two colors are colse enough to be considered the same.
GLboolean
PointSpriteTest::CompareColor(GLfloat *actual, GLfloat *expected)
{
	return (fabs(actual[0] - expected[0]) <= mTolerance[0] &&
		fabs(actual[1] - expected[1]) <= mTolerance[1] &&
		fabs(actual[2] - expected[2]) <= mTolerance[2] );
}


static void
FindNonBlack(const GLfloat *buf, GLint w, GLint h, GLint *x0, GLint *y0)
{
	GLint i, j;
	for (i = 0; i < h; i++) {
		for (j = 0; j < w; j++) {
			int k = (i * w + j) * 3;
			if (buf[k+0] != bgColor[0] ||
				buf[k+1] != bgColor[1] ||
				buf[k+2] != bgColor[2]) {
				*x0 = j;
				*y0 = i;
				return;
			}
		}
	}
	abort();
}


/**
 * compare pixels located at (0,0) to (WINSIZE/2, WINSIZE/2).
 * @param buf: pixels' RGB value
 * @param pSize: point size
 * @param coordOrigin: coordinate origin--UPPER_LEFT or LOWER_LEFT
 */
GLboolean
PointSpriteTest::ComparePixels(GLfloat *buf, int pSize, int coordOrigin)
{
	GLfloat *lowerColor,  *upperColor, *expectedColor;
	GLint  i, j;
	GLint x0, y0;

	lowerColor = GetTexColor(pSize, coordOrigin ? 0 : 1);
	upperColor = GetTexColor(pSize, coordOrigin ? 1 : 0);

	// Find first (lower-left) pixel that's not black.
	// The pixels hit by sprite rasterization may vary from one GL to
	// another so try to compensate for that.
	FindNonBlack(buf, WINSIZE/2, WINSIZE/2, &x0, &y0);

	for (i = 0; i < WINSIZE / 2; i++)
	{
		for (j = 0; j < WINSIZE / 2; j++)
		{
			if (OutOfPoint(i, j, pSize, x0, y0))
			{ //pixel (i, j) is out of point
			  //its color should bebackground
				if (!CompareColor(buf, bgColor))
				{
					env->log << "Incorrect pixel at (" << i << ", " << j << "):\n"
						<<"\tit should be backgound color: ("
						<< bgColor[0] << ", " << bgColor[1] << ", " << bgColor[2]
						<< "), actual read: (" << buf[0] << ", " << buf[1] << ", " << buf[2] << ")\n" ;
					return GL_FALSE;
				}
			} else { //inside point
				if (i - x0 < pSize/2)
					expectedColor = lowerColor;
				else
					expectedColor = upperColor;

				if (!CompareColor(buf, expectedColor))
				{
					env->log << "Incorrect pixel at (" << i << ", " << j << "):\n"
						<<"\tit should be rendered with color: ("
						<< expectedColor[0] << ", " << expectedColor[1] << ", " << expectedColor[2]
						<< "), actual read: (" << buf[0] << ", " << buf[1] << ", " << buf[2] << ")\n" ;
					return GL_FALSE;
				} 		
			}
			buf += 3;
		}
	}

	return GL_TRUE;
}

// Test default state.
// Test point and polygon which mode is GL_POINT, and texture's coordinate
// origin is UPPER_LEFT or LOWER_LEFT.
// Result will indicate number of passes and failures.
void
PointSpriteTest::runOne(MultiTestResult &r, Window &w)
{
	GLfloat maxPointSize, pointSize;
	GLint   expectedSize;
	GLint   primType, coordOrigin;
	GLfloat *buf;
	GLuint  texID;
	int i;

	(void) w;

	CheckDefaultState(r);
	
	glDrawBuffer(GL_FRONT);
	glReadBuffer(GL_FRONT);

	glClearColor(bgColor[0], bgColor[1], bgColor[2], bgColor[3]);

        glViewport(0, 0, WINSIZE, WINSIZE);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, WINSIZE, 0, WINSIZE, -1, 1);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

	GenMipmap();
	SetupMipmap(&texID);
	CalculateTolerance();

	buf = (GLfloat *)malloc(3 * WINSIZE * WINSIZE / 4 * sizeof(GLfloat));

	// enable point_sprite_ARB
	glEnable(GL_POINT_SPRITE_ARB);

	glGetFloatv(GL_POINT_SIZE_MAX_ARB, &maxPointSize);
	if (maxPointSize > WINSIZE / 2)
		maxPointSize = WINSIZE / 2;

	//primitive may be point or polygon which mode is GL_POINT
	for (primType = 0; primType < 2; primType ++)
	{
		for (coordOrigin = 0; coordOrigin < 2; coordOrigin++)
		{

			if (coordOrigin)
				glPointParameterf(GL_POINT_SPRITE_COORD_ORIGIN, GL_UPPER_LEFT);
			else
				glPointParameterf(GL_POINT_SPRITE_COORD_ORIGIN, GL_LOWER_LEFT);

			pointSize = 1.85;
			for (; pointSize <= maxPointSize; pointSize += 2.0)
			{
				expectedSize = (int)(pointSize + 0.2);

				glPointSize(pointSize);
				glClear(GL_COLOR_BUFFER_BIT);

				if (primType == 0)
				{
					glBegin(GL_POINTS);
						glVertex2i(WINSIZE/4, WINSIZE/4);
					glEnd();
				} else {
					glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
					glBegin(GL_POLYGON);
						glVertex2i(WINSIZE/4, WINSIZE/4);
						glVertex2i(WINSIZE, WINSIZE / 4);
						glVertex2i(WINSIZE, WINSIZE);
						glVertex2i(WINSIZE / 4, WINSIZE);
					glEnd();
				}

				glReadPixels(0, 0, WINSIZE/2, WINSIZE/2, GL_RGB, GL_FLOAT, buf);
		
				if (!ComparePixels(buf, expectedSize, coordOrigin))
				{
					env->log << "\tPrimitive type: " << (primType ? "GL_POLYGON" : "GL_POINTS") << "\n";
					env->log << "\tCoord Origin at: " << (coordOrigin ? "GL_LOWER_LEFT" : "GL_UPPER_LEFT") << "\n";
					env->log << "\tPointSize: " << pointSize << "\n";
					r.numFailed++;
					r.numPassed--;
					break;
				}
			}
			r.numPassed++;
		}
	}

	glDeleteTextures(1, &texID);
	glDisable(GL_POINT_SPRITE_ARB);
	free(buf);
	for (i = 0; i < 6; i++)
		free(texImages[i]);

	r.pass = (r.numFailed == 0);
}

// The test object itself:
PointSpriteTest pointSpriteTest("pointSprite", "window, rgb",
			"GL_ARB_point_sprite",
                        "Test basic point sprite functionality.\n");

} // namespace GLEAN
