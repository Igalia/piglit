// BEGIN_COPYRIGHT -*- glean -*-
// 
// Copyright (C) 2009  VMware, Inc. All Rights Reserved.
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
// PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL VMWARE BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
// AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
// OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
// 
// END_COPYRIGHT


// Test GL_EXT_texture_swizzle for all possible swizzle combinations
// both with fixed function and a fragment program.
// Brian Paul
// 28 Jan 2009


#define GL_GLEXT_PROTOTYPES

#include <cassert>
#include <cmath>
#include <cstring>
#include <stdlib.h>
#include "ttexswizzle.h"
#include "rand.h"
#include "timer.h"
#include "image.h"


namespace GLEAN {

static PFNGLPROGRAMSTRINGARBPROC glProgramStringARB_func = NULL;
static PFNGLBINDPROGRAMARBPROC glBindProgramARB_func = NULL;
static PFNGLGENPROGRAMSARBPROC glGenProgramsARB_func = NULL;

static const int TexSize = 16;

static const GLfloat vertexData[4][4] = {
	//   x,    y,    s,   t
	{ -1.0, -1.0,  0.0, 0.0 },
	{  1.0, -1.0,  1.0, 0.0 },
	{  1.0,  1.0,  1.0, 1.0 },
	{ -1.0,  1.0,  0.0, 1.0 }
};


TexSwizzleResult::TexSwizzleResult()
{
	pass = false;
}


void
TexSwizzleTest::RandomColor(GLubyte *color)
{
	color[0] = rand.next() & 0xff;
	color[1] = rand.next() & 0xff;
	color[2] = rand.next() & 0xff;
	color[3] = rand.next() & 0xff;
}


void
TexSwizzleTest::SetTextureColor(const GLubyte *color)
{
	GLubyte texImage[TexSize][TexSize][4];
	int i, j;

	for (i = 0; i < TexSize; i++) {
		for (j = 0; j < TexSize; j++) {
			texImage[i][j][0] = color[0];
			texImage[i][j][1] = color[1];
			texImage[i][j][2] = color[2];
			texImage[i][j][3] = color[3];
		}
	}

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, TexSize, TexSize, 0,
				 GL_RGBA, GL_UNSIGNED_BYTE, texImage);
}


GLubyte
TexSwizzleTest::Swizzle(const GLubyte *texColor, GLenum swizzle)
{
	switch (swizzle) {
	case GL_RED:
		return texColor[0];
	case GL_GREEN:
		return texColor[1];
	case GL_BLUE:
		return texColor[2];
	case GL_ALPHA:
		return texColor[3];
	case GL_ONE:
		return 255;
	case GL_ZERO:
		return 0;
	default:
		assert(0);
	}
}


void
TexSwizzleTest::ComputeExpectedColor(const GLubyte *texColor,
									 GLenum swizzleR,
									 GLenum swizzleG,
									 GLenum swizzleB,
									 GLenum swizzleA,
									 GLubyte *expectedColor)
{
	expectedColor[0] = Swizzle(texColor, swizzleR);
	expectedColor[1] = Swizzle(texColor, swizzleG);
	expectedColor[2] = Swizzle(texColor, swizzleB);
	expectedColor[3] = Swizzle(texColor, swizzleA);
}


const char *
TexSwizzleTest::SwizzleString(GLenum swizzle)
{
	switch (swizzle) {
	case GL_RED:
		return "GL_RED";
	case GL_GREEN:
		return "GL_GREEN";
	case GL_BLUE:
		return "GL_BLUE";
	case GL_ALPHA:
		return "GL_ALPHA";
	case GL_ZERO:
		return "GL_ZERO";
	case GL_ONE:
		return "GL_ONE";
	default:
		assert(0);
		return "???";
	}
}


void
TexSwizzleTest::ReportFailure(GLenum swizzleR, GLenum swizzleG,
							  GLenum swizzleB, GLenum swizzleA,
							  const GLubyte *texColor,
							  const GLubyte *actual,
							  const GLubyte *expected)
{
	char str[100];

	env->log << name << ": Error: GL_EXT_texure_swizzle test failed\n";
	env->log << "\tGL_TEXTURE_SWIZZLE_R_EXT = " << SwizzleString(swizzleR) << "\n";
	env->log << "\tGL_TEXTURE_SWIZZLE_G_EXT = " << SwizzleString(swizzleG) << "\n";
	env->log << "\tGL_TEXTURE_SWIZZLE_B_EXT = " << SwizzleString(swizzleB) << "\n";
	env->log << "\tGL_TEXTURE_SWIZZLE_A_EXT = " << SwizzleString(swizzleA) << "\n";
	if (glIsEnabled(GL_FRAGMENT_PROGRAM_ARB)) {
		env->log << "\tGL_FRAGMENT_PROGRAM enabled\n";
	}
	sprintf(str, "%d, %d, %d, %d",
			texColor[0], texColor[1], texColor[2], texColor[3]);
	env->log << "\tTexture color: " << str << "\n";
	sprintf(str, "%d, %d, %d, %d",
			expected[0], expected[1], expected[2], expected[3]);
	env->log << "\tExpected color: " << str << "\n";
	sprintf(str, "%d, %d, %d, %d",
			actual[0], actual[1], actual[2], actual[3]);
	env->log << "\tRendered color: " << str << "\n";
}


// Test state setting/getting for texture swizzle.
bool
TexSwizzleTest::TestAPI(void)
{
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R_EXT, GL_ONE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G_EXT, GL_ZERO);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B_EXT, GL_RED);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A_EXT, GL_BLUE);

	if (glGetError() != GL_NO_ERROR) {
		env->log << "\tSetting GL_TEXTURE_SWIZZLE_R/G/B/A generated an error.\n";
		return false;
	}

	GLint val;
	glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R_EXT, &val);
	if (val != GL_ONE) {
		env->log << "\tQuery of GL_TEXTURE_SWIZZLE_R_EXT failed.\n";
		return false;
	}
	glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G_EXT, &val);
	if (val != GL_ZERO) {
		env->log << "\tQuery of GL_TEXTURE_SWIZZLE_G_EXT failed.\n";
		return false;
	}
	glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B_EXT, &val);
	if (val != GL_RED) {
		env->log << "\tQuery of GL_TEXTURE_SWIZZLE_B_EXT failed.\n";
		return false;
	}
	glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A_EXT, &val);
	if (val != GL_BLUE) {
		env->log << "\tQuery of GL_TEXTURE_SWIZZLE_A_EXT failed.\n";
		return false;
	}
		
	// set all at once
	const GLint swz[4] = { GL_BLUE, GL_GREEN, GL_ALPHA, GL_ZERO };
	glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA_EXT, swz);
	if (glGetError() != GL_NO_ERROR) {
		env->log << "\tSetting GL_TEXTURE_SWIZZLE_RGBA_EXT generated an error.\n";
		return false;
	}

	GLint swzOut[4];
	glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA_EXT, swzOut);
	if (swzOut[0] != swz[0] ||
		swzOut[1] != swz[1] ||
		swzOut[2] != swz[2] ||
		swzOut[3] != swz[3]) {
		env->log << "\tQuerying GL_TEXTURE_SWIZZLE_RGBA_EXT failed.\n";
		return false;
	}

	return true;
}


// Loop over all possible combinations of texture swizzles,
// drawing with a texture and checking if the results are correct.
// return true/false for pass/fail
bool
TexSwizzleTest::TestSwizzles(void)
{
	static const GLenum swizzles[6] = {
		GL_RED, GL_GREEN, GL_BLUE, GL_ALPHA, GL_ZERO, GL_ONE
	};
	int ir, ig, ib, ia;
	GLubyte texColor[4];
	int err = 1; // XXX this should be computed from framebuffer depth

	for (ir = 0; ir < 6; ir++) {
		glTexParameteri(GL_TEXTURE_2D,
						GL_TEXTURE_SWIZZLE_R_EXT,
						swizzles[ir]);
		for (ig = 0; ig < 6; ig++) {
			glTexParameteri(GL_TEXTURE_2D,
							GL_TEXTURE_SWIZZLE_G_EXT,
							swizzles[ig]);

			// Setup random texture color here (not in the innermost loop
			// for _every_ iteration) just to speed things up a bit.
			RandomColor(texColor);
			SetTextureColor(texColor);

			for (ib = 0; ib < 6; ib++) {
				glTexParameteri(GL_TEXTURE_2D,
								GL_TEXTURE_SWIZZLE_B_EXT,
								swizzles[ib]);
				for (ia = 0; ia < 6; ia++) {
					glTexParameteri(GL_TEXTURE_2D,
									GL_TEXTURE_SWIZZLE_A_EXT,
									swizzles[ia]);

					GLubyte expected[4];
					ComputeExpectedColor(texColor,
										 swizzles[ir],
										 swizzles[ig],
										 swizzles[ib],
										 swizzles[ia],
										 expected);

					// draw something and read back a pixel
					glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
					GLubyte actual[4];
					glReadPixels(windowSize / 2, windowSize / 2, 1, 1,
								 GL_RGBA, GL_UNSIGNED_BYTE, actual);

					if (abs((int) actual[0] - (int) expected[0]) > err ||
						abs((int) actual[1] - (int) expected[1]) > err ||
						abs((int) actual[2] - (int) expected[2]) > err) {

						ReportFailure(swizzles[ir],
									  swizzles[ig],
									  swizzles[ib],
									  swizzles[ia],
									  texColor, actual, expected);

						return false;
					}						
				}
			}
		}
	}

	return true;
}


// Same test as above, but using a fragment program instead of fixed-function.
bool
TexSwizzleTest::TestSwizzlesWithProgram(void)
{
	const char *text =
		"!!ARBfp1.0\n"
		"TEX result.color, fragment.texcoord[0], texture[0], 2D; \n"
		"END\n";
	GLuint prog;

	glGenProgramsARB_func(1, &prog);
	glBindProgramARB_func(GL_FRAGMENT_PROGRAM_ARB, prog);
	glProgramStringARB_func(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB,
					   strlen(text), (const GLubyte *) text);

	assert(glGetError() == GL_NO_ERROR);

	glEnable(GL_FRAGMENT_PROGRAM_ARB);

	bool pass = TestSwizzles();

	glDisable(GL_FRAGMENT_PROGRAM_ARB);

	return pass;
}


void
TexSwizzleTest::Setup(void)
{
	glProgramStringARB_func = (PFNGLPROGRAMSTRINGARBPROC) GLUtils::getProcAddress("glProgramStringARB");
	assert(glProgramStringARB_func);
	glBindProgramARB_func = (PFNGLBINDPROGRAMARBPROC) GLUtils::getProcAddress("glBindProgramARB");
	assert(glBindProgramARB_func);
	glGenProgramsARB_func = (PFNGLGENPROGRAMSARBPROC) GLUtils::getProcAddress("glGenProgramsARB");
	assert(glGenProgramsARB_func);

	// setup transformation
	glViewport(0, 0, windowSize, windowSize);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	// setup vertex arrays (draw textured quad)
	glVertexPointer(2, GL_FLOAT, 16, vertexData);
	glTexCoordPointer(2, GL_FLOAT, 16, &vertexData[0][2]);
	glEnable(GL_VERTEX_ARRAY);
	glEnable(GL_TEXTURE_COORD_ARRAY);

	// setup texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glEnable(GL_TEXTURE_2D);
}


void
TexSwizzleTest::runOne(TexSwizzleResult &r, Window &w)
{
	(void) w;  // silence warning

	Setup();

	r.pass = TestAPI();

	if (r.pass) {
		r.pass = TestSwizzles();
	}

	if (r.pass && GLUtils::haveExtension("GL_ARB_fragment_program")) {
		r. pass = TestSwizzlesWithProgram();
	}
}


void
TexSwizzleTest::logOne(TexSwizzleResult &r)
{
	if (r.pass) {
		logPassFail(r);
		logConcise(r);
	}
	else {
		env->log << name << " FAIL\n";
	}
}


void
TexSwizzleTest::compareOne(TexSwizzleResult &oldR,
			     TexSwizzleResult &newR)
{
	comparePassFail(oldR, newR);
}


void
TexSwizzleResult::putresults(ostream &s) const
{
	if (pass) {
		s << "PASS\n";
	}
	else {
		s << "FAIL\n";
	}
}


bool
TexSwizzleResult::getresults(istream &s)
{
	char result[1000];
	s >> result;

	if (strcmp(result, "FAIL") == 0) {
		pass = false;
	}
	else {
		pass = true;
	}
	return s.good();
}


// The test object itself:
TexSwizzleTest texSwizzleTest("texSwizzle", "window, rgb",
                              "GL_EXT_texture_swizzle",
                              "Test the GL_EXT_texture_swizzle extension.\n");



} // namespace GLEAN


