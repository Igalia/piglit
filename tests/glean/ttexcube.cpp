// BEGIN_COPYRIGHT -*- glean -*-
// 
// Copyright (C) 1999  Allen Akin   All Rights Reserved.
// 
// Permission is hereby granted, free of charge, to any person
// obtaining a copy of this software and associated documentation
// files (the "Software"), to deal in the Software without
// restriction, including without limitation the rights to use,
// copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following
// conditions:
// 
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the
// Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
// KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
// PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL ALLEN AKIN BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
// AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
// OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
// 
// END_COPYRIGHT


// ttexcube.cpp:  Test the GL_ARB_texture_cube_map extension
// Author: Brian Paul (brianp@valinux.com)  March 2001
//
// Test procedure:
//   We build a 6-sided texture cube map in which each side is a simple 2x2
//   checkboard pattern with known colors.  Then we do three sets of tests.
//   Each test draws a single quadrilateral.  The tests are:
//
//   1. Directly specify texture coordinates.  By changing the texture coords
//      we can sample specific regions of the cube map.  Check the rendered
//      quad colors for correctness.
//   2. Use GL_NORMAL_MAP_ARB texgen mode with specific normal vectors to
//      sample specific regions of the cube map.  Check for correctness.
//   3. Test GL_REFLECTION_MAP_ARB texgen mode by specifying a quad with
//      fixed vertices and normals but rotating the texture coordinate
//      matrix to select each side of the cube map.  Check that the rendered
//      quad's four colors match the cube face.
//      

#include "ttexcube.h"
#include <stdio.h>
#include <cmath>

namespace GLEAN {


#define VP_SIZE 20

static const char *faceName[6] = {
	"POSITIVE_X",
	"NEGATIVE_X",
	"POSITIVE_Y",
	"NEGATIVE_Y",
	"POSITIVE_Z",
	"NEGATIVE_Z"
};


//
// Test if two colors are close enough to be considered the same
//
bool
TexCubeTest::TestColor(const GLfloat c1[3], const GLfloat c2[3]) {
	if (fabs(c1[0] - c2[0]) <= mTolerance[0] &&
	    fabs(c1[1] - c2[1]) <= mTolerance[1] &&
	    fabs(c1[2] - c2[2]) <= mTolerance[2])
		return true;
	else
		return false;
}


//
// Define a 2x2 checkerboard texture image using the given four colors.
//
void
TexCubeTest::BuildTexImage(GLenum target, const GLfloat color[4][3]) {
	const GLint w = 8, h = 8;
	GLfloat texImage[8][8][4];
	for (int i = 0; i < h; i++) {
		const int ibit = (i >= (h / 2));
		for (int j = 0; j < w; j++) {
			const int jbit = (j >= (w / 2));
			const int c = ibit * 2 + jbit;
			texImage[i][j][0] = color[c][0];
			texImage[i][j][1] = color[c][1];
			texImage[i][j][2] = color[c][2];
			texImage[i][j][3] = 1.0;
		}
	}
	glTexImage2D(target, 0, GL_RGB, w, h, 0, GL_RGBA, GL_FLOAT, texImage);
}


//
// Draw a polygon either with texcoords or normal vectors and check that
// we hit the correct quadrant of each of the six cube faces.
// Return: true = pass, false = fail
//
bool
TexCubeTest::TestNormalMap(bool texCoordMode, const char *modeName) {

	// We use the coordinates both directly as texture coordinates
	// and as normal vectors for testing NORMAL_MAP_ARB texgen mode).
	static const GLfloat coords[6][4][3] = {
		// +X
		{
			{ 1.0,  0.5,  0.5 },
			{ 1.0,  0.5, -0.5 },
			{ 1.0, -0.5,  0.5 },
			{ 1.0, -0.5, -0.5 },
		},
		// -X
		{
			{ -1.0,  0.5, -0.5 },
			{ -1.0,  0.5,  0.5 },
			{ -1.0, -0.5, -0.5 },
			{ -1.0, -0.5,  0.5 },
		},
		// +Y
		{
			{ -0.5, 1.0, -0.5 },
			{  0.5, 1.0, -0.5 },
			{ -0.5, 1.0,  0.5 },
			{  0.5, 1.0,  0.5 },
		},
		// -Y
		{
			{ -0.5, -1.0,  0.5 },
			{  0.5, -1.0,  0.5 },
			{ -0.5, -1.0, -0.5 },
			{  0.5, -1.0, -0.5 },
		},
		// +Z
		{
			{ -0.5,  0.5, 1.0 },
			{  0.5,  0.5, 1.0 },
			{ -0.5, -0.5, 1.0 },
			{  0.5, -0.5, 1.0 },
		},
		// -Z
		{
			{  0.5,  0.5, -1.0 },
			{ -0.5,  0.5, -1.0 },
			{  0.5, -0.5, -1.0 },
			{ -0.5, -0.5, -1.0 },
		}
	};

	// normal vectors to hit the four colors of each cube face when

	for (int face = 0; face < 6; face++) {
		for (int quadrant = 0; quadrant < 4; quadrant++) {

			// draw the test quad
			if (texCoordMode)
				glTexCoord3fv(coords[face][quadrant]);
			else
				glNormal3fv(coords[face][quadrant]);
			glColor3f(0, 1, 0);
			glBegin(GL_POLYGON);
			glVertex2f(-1, -1);
			glVertex2f( 1, -1);
			glVertex2f( 1,  1);
			glVertex2f(-1,  1);
			glEnd();

			// check the color
			GLfloat result[3];
			glReadPixels(1, 1, 1, 1, GL_RGB, GL_FLOAT, result);

			if (!TestColor(mColors[face][quadrant], result)) {
				env->log << name
					 << ":  FAIL: mode='"
					 << modeName
					 << "' face="
					 << faceName[face]
					 << " quadrant="
					 << quadrant
					 << " expected=("
					 << mColors[face][quadrant][0] << ", "
					 << mColors[face][quadrant][1] << ", "
					 << mColors[face][quadrant][2]
					 << ") measured=("
					 << result[0] << ", "
					 << result[1] << ", "
					 << result[2]
					 << ")\n";
				return false;
			}
		}
	}
	return true;
}


//
// Test GL_REFLECTION_MAP_ARB texgen mode.
// Return: true = pass, false = fail
//
bool
TexCubeTest::TestReflectionMap(const char *modeName) {

// These are the glReadPixels coords we'll use for pixel testing
#define X0 ((int) (VP_SIZE * 0.25))
#define X1 ((int) (VP_SIZE * 0.75))
#define Y0 ((int) (VP_SIZE * 0.25))
#define Y1 ((int) (VP_SIZE * 0.75))

	// We'll rotate the texture coordinates to map each cube face
	// onto a screen-aligned quad.
	static const GLfloat rotation[6][4] = {
		{ -90, 0, 1, 0 },	// +X
		{  90, 0, 1, 0 },	// -X
		{  90, 1, 0, 0 },	// +Y
		{ -90, 1, 0, 0 },	// -Y
		{ 180, 1, 0, 0 },	// -Z
		{   0, 1, 0, 0 }	// +Z
	};

	// For each face we'll test the four quadrants to be sure test
	// if the expected color is where it should be.
	// These are the glReadPixels coordinates at which we should
	// find the colors in the mColors[6][4] array.
	static const GLint readPos[6][4][2] = {
		// +X
		{
			{ X1, Y1 }, { X0, Y1 }, { X1, Y0 }, { X0, Y0 }
		},
		// -X
		{
			{ X1, Y1 }, { X0, Y1 }, { X1, Y0 }, { X0, Y0 }
		},
		// +Y
		{
			{ X0, Y0 }, { X1, Y0 }, { X0, Y1 }, { X1, Y1 }
		},
		// -Y
		{
			{ X0, Y0 }, { X1, Y0 }, { X0, Y1 }, { X1, Y1 }
		},
		// +Z
		{
			{ X0, Y0 }, { X1, Y0 }, { X0, Y1 }, { X1, Y1 }
		},
		// -Z
		{
			{ X1, Y1 }, { X0, Y1 }, { X1, Y0 }, { X0, Y0 }
		}
	};

	for (int face = 0; face < 6; face++) {

		// Draw the test quad.
		// It'll be textured with one face of the cube map texture.
		glMatrixMode(GL_TEXTURE);
		glPushMatrix();
			glRotatef(rotation[face][0], rotation[face][1],
				rotation[face][2], rotation[face][3]);
			glNormal3f(0, 0, 1);
			glColor3f(0, 1, 0);
			glBegin(GL_POLYGON);
			glVertex3f(-1, -1, 1);
			glVertex3f( 1, -1, 1);
			glVertex3f( 1,  1, 1);
			glVertex3f(-1,  1, 1);
			glEnd();
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);

		// Verify the colors
		for (int quadrant = 0; quadrant < 4; quadrant++) {

			GLfloat result[3];
			glReadPixels(readPos[face][quadrant][0],
				readPos[face][quadrant][1],
				1, 1, GL_RGB, GL_FLOAT, result);

			if (!TestColor(mColors[face][quadrant], result)) {
				env->log << name
					 << ":  FAIL: mode='"
					 << modeName
					 << "' face="
					 << faceName[face]
					 << " quadrant="
					 << quadrant
					 << " expected=("
					 << mColors[face][quadrant][0] << ", "
					 << mColors[face][quadrant][1] << ", "
					 << mColors[face][quadrant][2]
					 << ") measured=("
					 << result[0] << ", "
					 << result[1] << ", "
					 << result[2]
					 << ")\n";
				return false;
			}
		}
	}
	return true;
}


void
TexCubeTest::runOne(BasicResult& r, Window& w) {

	(void) w;

	// each of six faces needs four test colors
	for (int i = 0; i < 6 * 4; i++) {
		GLint r = i % 3;
		GLint g = (i / 3) % 3;
		GLint b = (i / 9) % 3;
		mColors[i / 4][i % 4][0] = r * 0.5;
		mColors[i / 4][i % 4][1] = g * 0.5;
		mColors[i / 4][i % 4][2] = b * 0.5;
		//printf("mColors[%d][%d] = %g %g %g\n", i/4, i%4,
		//	mColors[i/4][i%4][0],
		//	mColors[i/4][i%4][1],
		//	mColors[i/4][i%4][2]);
	}

	glDrawBuffer(GL_FRONT);
	glReadBuffer(GL_FRONT);

	BuildTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB, mColors[0]);
	BuildTexImage(GL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB, mColors[1]);
	BuildTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB, mColors[2]);
	BuildTexImage(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB, mColors[3]);
	BuildTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB, mColors[4]);
	BuildTexImage(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB, mColors[5]);

	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_MIN_FILTER,
			GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_MAG_FILTER,
			GL_NEAREST);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glEnable(GL_TEXTURE_CUBE_MAP_ARB);

	// compute RGB error tolerance
	{
		GLint rBits, gBits, bBits;
		GLint rTexBits, gTexBits, bTexBits;
		glGetIntegerv(GL_RED_BITS, &rBits);
		glGetIntegerv(GL_GREEN_BITS, &gBits);
		glGetIntegerv(GL_BLUE_BITS, &bBits);
		glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB,
			0, GL_TEXTURE_RED_SIZE, &rTexBits);
		glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB,
			0, GL_TEXTURE_GREEN_SIZE, &gTexBits);
		glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB,
			0, GL_TEXTURE_BLUE_SIZE, &bTexBits);
		// find smaller of frame buffer and texture bits
		rBits = (rBits < rTexBits) ? rBits : rTexBits;
		gBits = (gBits < gTexBits) ? gBits : gTexBits;
		bBits = (bBits < bTexBits) ? bBits : bTexBits;
		mTolerance[0] = 2.0 / (1 << rBits);
		mTolerance[1] = 2.0 / (1 << gBits);
		mTolerance[2] = 2.0 / (1 << bBits);
	}

	glViewport(0, 0, VP_SIZE, VP_SIZE);

	bool passed = true;

	if (passed) {
		// Test directly specifying texture coords
		glDisable(GL_TEXTURE_GEN_S);
		glDisable(GL_TEXTURE_GEN_T);
		glDisable(GL_TEXTURE_GEN_R);
		passed = TestNormalMap(true,
			"Direct specification of texture coordinates");
	}

	if (passed) {
		// Test GL_NORMAL_MAP_ARB mode
		glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_NORMAL_MAP_ARB);
		glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_NORMAL_MAP_ARB);
		glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_NORMAL_MAP_ARB);
		glEnable(GL_TEXTURE_GEN_S);
		glEnable(GL_TEXTURE_GEN_T);
		glEnable(GL_TEXTURE_GEN_R);
		passed = TestNormalMap(false, "GL_NORMAL_MAP_ARB texgen");
	}

	if (passed) {
		// Test GL_REFLECTION_MAP_ARB mode
		glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP_ARB);
		glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP_ARB);
		glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP_ARB);
		glEnable(GL_TEXTURE_GEN_S);
		glEnable(GL_TEXTURE_GEN_T);
		glEnable(GL_TEXTURE_GEN_R);
		glEnable(GL_NORMALIZE);
		passed = TestReflectionMap("GL_REFLECTION_MAP_ARB texgen");
	}

	r.pass = passed;
} // TexCubeTest::runOne


void
TexCubeTest::logOne(BasicResult& r) {
	logPassFail(r);
	logConcise(r);
} // TexCubeTest::logOne


///////////////////////////////////////////////////////////////////////////////
// The test object itself:
///////////////////////////////////////////////////////////////////////////////
TexCubeTest texCubeTest("texCube", "window, rgb",

	"GL_ARB_texture_cube_map verification test.\n");


} // namespace GLEAN
