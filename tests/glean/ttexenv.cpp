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


// ttexenv.cpp:  Test the basic texture env modes
// Author: Brian Paul (brianp@valinux.com)  April 2001
//
// Test procedure:
//   Setup a texture with 81 columns of unique RGBA colors, 3 texels each.
//   Draw a 81 uniquely-colored flat-shaded quads as wide horizontal bands,
//   with the above texture.  This makes a matrix of 81*81 colored squares
//   for which we test that the current texture environment mode and texture
//   format produced the correct color.
//   Finally, we blend over a gray background in order to verify that the
//   post-texture alpha value is correct.
//      

#include <stdlib.h>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include "ttexenv.h"


namespace GLEAN {


// If this is true, we enable blending over a gray background in order
// to test the alpha results of the texture env.  If this is false,
// we don't blend.  It might be useful to disable blending in order to
// diagnose failures
#define BLEND_WITH_BACKGROUND 1

static GLfloat BgColor[4] = { 0.5, 0.5, 0.5, 0.5 };


static const GLenum FormatEnums[] = {
	GL_ALPHA,
	GL_LUMINANCE,
	GL_LUMINANCE_ALPHA,
	GL_INTENSITY,
	GL_RGB,
	GL_RGBA
};

static const char *FormatNames[] = {
	"GL_ALPHA",
	"GL_LUMINANCE",
	"GL_LUMINANCE_ALPHA",
	"GL_INTENSITY",
	"GL_RGB",
	"GL_RGBA"
};

static const GLenum EnvModeEnums[] = {
	GL_REPLACE,
	GL_MODULATE,
	GL_DECAL,
	GL_BLEND,
	GL_ADD
};

static const char *EnvModeNames[] = {
	"GL_REPLACE",
	"GL_MODULATE",
	"GL_DECAL",
	"GL_BLEND",
	"GL_ADD"
};


//
// Test if two colors are close enough to be considered the same
//
bool
TexEnvTest::TestColor(const GLfloat c1[3], const GLfloat c2[3]) {
	if (fabs(c1[0] - c2[0]) <= mTolerance[0] &&
	    fabs(c1[1] - c2[1]) <= mTolerance[1] &&
	    fabs(c1[2] - c2[2]) <= mTolerance[2])
		return true;
	else
		return false;
}

//
// Compute expected texenv result given the texture env mode, the texture
// base format, texture color, fragment color, and texture env color.
// This also blends the result with the background color if that option
// is enabled (see above).
//
void
TexEnvTest::ComputeExpectedColor(GLenum envMode, GLenum texFormat,
	const GLfloat texColor[4], const GLfloat fragColor[4],
	const GLfloat envColor[4], GLfloat result[4]) {	

	switch (envMode) {
	case GL_REPLACE:
		switch (texFormat) {
		case GL_ALPHA:
			result[0] = fragColor[0];
			result[1] = fragColor[1];
			result[2] = fragColor[2];
			result[3] = texColor[3]; // alpha
			break;
		case GL_LUMINANCE:
			result[0] = texColor[0]; // lum
			result[1] = texColor[0];
			result[2] = texColor[0];
			result[3] = fragColor[3];
			break;
		case GL_LUMINANCE_ALPHA:
			result[0] = texColor[0]; // lum
			result[1] = texColor[0];
			result[2] = texColor[0];
			result[3] = texColor[3]; // alpha
			break;
		case GL_INTENSITY:
			result[0] = texColor[0]; // intensity
			result[1] = texColor[0];
			result[2] = texColor[0];
			result[3] = texColor[0];
			break;
		case GL_RGB:
			result[0] = texColor[0]; // r
			result[1] = texColor[1]; // g
			result[2] = texColor[2]; // b
			result[3] = fragColor[3];
			break;
		case GL_RGBA:
			result[0] = texColor[0]; // r
			result[1] = texColor[1]; // g
			result[2] = texColor[2]; // b
			result[3] = texColor[3]; // a
			break;
		default:
			abort();  // implementation error
		}
		break;
	case GL_MODULATE:
		switch (texFormat) {
		case GL_ALPHA:
			result[0] = fragColor[0];
			result[1] = fragColor[1];
			result[2] = fragColor[2];
			result[3] = fragColor[3] * texColor[3];
			break;
		case GL_LUMINANCE:
			result[0] = fragColor[0] * texColor[0];
			result[1] = fragColor[1] * texColor[0];
			result[2] = fragColor[2] * texColor[0];
			result[3] = fragColor[3];
			break;
		case GL_LUMINANCE_ALPHA:
			result[0] = fragColor[0] * texColor[0];
			result[1] = fragColor[1] * texColor[0];
			result[2] = fragColor[2] * texColor[0];
			result[3] = fragColor[3] * texColor[3];
			break;
		case GL_INTENSITY:
			result[0] = fragColor[0] * texColor[0];
			result[1] = fragColor[1] * texColor[0];
			result[2] = fragColor[2] * texColor[0];
			result[3] = fragColor[3] * texColor[0];
			break;
		case GL_RGB:
			result[0] = fragColor[0] * texColor[0];
			result[1] = fragColor[1] * texColor[1];
			result[2] = fragColor[2] * texColor[2];
			result[3] = fragColor[3];
			break;
		case GL_RGBA:
			result[0] = fragColor[0] * texColor[0];
			result[1] = fragColor[1] * texColor[1];
			result[2] = fragColor[2] * texColor[2];
			result[3] = fragColor[3] * texColor[3];
			break;
		default:
			abort();  // implementation error
		}
		break;
	case GL_DECAL:
		switch (texFormat) {
		case GL_ALPHA:
			result[0] = 0; // undefined
			result[1] = 0;
			result[2] = 0;
			result[3] = 0;
			break;
		case GL_LUMINANCE:
			result[0] = 0; // undefined
			result[1] = 0;
			result[2] = 0;
			result[3] = 0;
			break;
		case GL_LUMINANCE_ALPHA:
			result[0] = 0; // undefined
			result[1] = 0;
			result[2] = 0;
			result[3] = 0;
			break;
		case GL_INTENSITY:
			result[0] = 0; // undefined
			result[1] = 0;
			result[2] = 0;
			result[3] = 0;
			break;
		case GL_RGB:
			result[0] = texColor[0];
			result[1] = texColor[1];
			result[2] = texColor[2];
			result[3] = fragColor[3];
			break;
		case GL_RGBA: {
			const GLfloat a = texColor[3];
			const GLfloat oma = 1.0 - a;
			result[0] = fragColor[0] * oma + texColor[0] * a;
			result[1] = fragColor[1] * oma + texColor[1] * a;
			result[2] = fragColor[2] * oma + texColor[2] * a;
			result[3] = fragColor[3];
			} break;
		default:
			abort();  // implementation error
		}
		break;
	case GL_BLEND:
		switch (texFormat) {
		case GL_ALPHA:
			result[0] = fragColor[0];
			result[1] = fragColor[1];
			result[2] = fragColor[2];
			result[3] = fragColor[3] * texColor[3];
			break;
		case GL_LUMINANCE: {
			const GLfloat l = texColor[0];
			const GLfloat oml = 1.0 - l;
			result[0] = fragColor[0] * oml + envColor[0] * l;
			result[1] = fragColor[1] * oml + envColor[1] * l;
			result[2] = fragColor[2] * oml + envColor[2] * l;
			result[3] = fragColor[3];
			} break;
		case GL_LUMINANCE_ALPHA: {
			const GLfloat l = texColor[0];
			const GLfloat oml = 1.0 - l;
			result[0] = fragColor[0] * oml + envColor[0] * l;
			result[1] = fragColor[1] * oml + envColor[1] * l;
			result[2] = fragColor[2] * oml + envColor[2] * l;
			result[3] = fragColor[3] * texColor[3];
			} break;
		case GL_INTENSITY: {
			const GLfloat i = texColor[0];
			const GLfloat omi = 1.0 - i;
			result[0] = fragColor[0] * omi + envColor[0] * i;
			result[1] = fragColor[1] * omi + envColor[1] * i;
			result[2] = fragColor[2] * omi + envColor[2] * i;
			result[3] = fragColor[3] * omi + envColor[3] * i;
			} break;
		case GL_RGB: {
			const GLfloat r = texColor[0];
			const GLfloat omr = 1.0 - r;
			const GLfloat g = texColor[1];
			const GLfloat omg = 1.0 - g;
			const GLfloat b = texColor[2];
			const GLfloat omb = 1.0 - b;
			result[0] = fragColor[0] * omr + envColor[0] * r;
			result[1] = fragColor[1] * omg + envColor[1] * g;
			result[2] = fragColor[2] * omb + envColor[2] * b;
			result[3] = fragColor[3];
			} break;
		case GL_RGBA: {
			const GLfloat r = texColor[0];
			const GLfloat omr = 1.0 - r;
			const GLfloat g = texColor[1];
			const GLfloat omg = 1.0 - g;
			const GLfloat b = texColor[2];
			const GLfloat omb = 1.0 - b;
			result[0] = fragColor[0] * omr + envColor[0] * r;
			result[1] = fragColor[1] * omg + envColor[1] * g;
			result[2] = fragColor[2] * omb + envColor[2] * b;
			result[3] = fragColor[3] * texColor[3];
			} break;
		default:
			abort();  // implementation error
		}
		break;
	case GL_ADD:
		switch (texFormat) {
		case GL_ALPHA:
			result[0] = fragColor[0];
			result[1] = fragColor[1];
			result[2] = fragColor[2];
			result[3] = fragColor[3] * texColor[3];
			break;
		case GL_LUMINANCE:
			result[0] = fragColor[0] + texColor[0];
			result[1] = fragColor[1] + texColor[0];
			result[2] = fragColor[2] + texColor[0];
			result[3] = fragColor[3];
			break;
		case GL_LUMINANCE_ALPHA:
			result[0] = fragColor[0] + texColor[0];
			result[1] = fragColor[1] + texColor[0];
			result[2] = fragColor[2] + texColor[0];
			result[3] = fragColor[3] * texColor[3];
			break;
		case GL_INTENSITY:
			result[0] = fragColor[0] + texColor[0];
			result[1] = fragColor[1] + texColor[0];
			result[2] = fragColor[2] + texColor[0];
			result[3] = fragColor[3] + texColor[0];
			break;
		case GL_RGB:
			result[0] = fragColor[0] + texColor[0];
			result[1] = fragColor[1] + texColor[1];
			result[2] = fragColor[2] + texColor[2];
			result[3] = fragColor[3];
			break;
		case GL_RGBA:
			result[0] = fragColor[0] + texColor[0];
			result[1] = fragColor[1] + texColor[1];
			result[2] = fragColor[2] + texColor[2];
			result[3] = fragColor[3] * texColor[3];
			break;
		default:
			abort();  // implementation error
		}
		// clamping
		if (result[0] > 1.0)  result[0] = 1.0;
		if (result[1] > 1.0)  result[1] = 1.0;
		if (result[2] > 1.0)  result[2] = 1.0;
		if (result[3] > 1.0)  result[3] = 1.0;
		break;
	default:
		// implementation error
		abort();
	}

#if BLEND_WITH_BACKGROUND
	// now blend result over a gray background
	const GLfloat alpha = result[3];
	const GLfloat omAlpha = 1.0 - alpha;
	result[0] = result[0] * alpha + BgColor[0] * omAlpha;
	result[1] = result[1] * alpha + BgColor[1] * omAlpha;
	result[2] = result[2] * alpha + BgColor[2] * omAlpha;
	result[3] = result[3] * alpha + BgColor[3] * omAlpha;
#endif
}


// Make a texture in which the colors vary along the length
// according to the colors[] array.  For example, we use
// 243 columns of the texture to store 81 colors, 3 texels each.
void
TexEnvTest::MakeTexImage(GLenum baseFormat, int numColors,
	const GLfloat colors[][4]) {

	const int width = 256;
	const int height = 4;
	GLfloat img[width * height][4];

	assert(numColors == 81);  // for now

	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			int c = j / 3;
			if (c >= numColors) {
				img[i * width + j][0] = 0.0;
				img[i * width + j][1] = 0.0;
				img[i * width + j][2] = 0.0;
				img[i * width + j][3] = 0.0;
			}
			else {
				img[i * width + j][0] = colors[c][0];
				img[i * width + j][1] = colors[c][1];
				img[i * width + j][2] = colors[c][2];
				img[i * width + j][3] = colors[c][3];
			}
		}
	}
	glTexImage2D(GL_TEXTURE_2D, 0, baseFormat, width, height, 0,
		GL_RGBA, GL_FLOAT, (void *) img);

	// Recompute color tolerance now because it depends on the
	// texel resolution in the new texture.
	{
		// Get fb resolution
		GLint rBits, gBits, bBits;
		glGetIntegerv(GL_RED_BITS, &rBits);
		glGetIntegerv(GL_GREEN_BITS, &gBits);
		glGetIntegerv(GL_BLUE_BITS, &bBits);
		// Get tex resolution
		GLint rTexBits, gTexBits, bTexBits, aTexBits;
		GLint iTexBits, lTexBits;
		glGetTexLevelParameteriv(GL_TEXTURE_2D,
			0, GL_TEXTURE_RED_SIZE, &rTexBits);
		glGetTexLevelParameteriv(GL_TEXTURE_2D,
			0, GL_TEXTURE_GREEN_SIZE, &gTexBits);
		glGetTexLevelParameteriv(GL_TEXTURE_2D,
			0, GL_TEXTURE_BLUE_SIZE, &bTexBits);
		glGetTexLevelParameteriv(GL_TEXTURE_2D,
			0, GL_TEXTURE_ALPHA_SIZE, &aTexBits);
		glGetTexLevelParameteriv(GL_TEXTURE_2D,
			0, GL_TEXTURE_INTENSITY_SIZE, &iTexBits);
		glGetTexLevelParameteriv(GL_TEXTURE_2D,
			0, GL_TEXTURE_LUMINANCE_SIZE, &lTexBits);
		// Special cases
		if (baseFormat == GL_INTENSITY) {
			rTexBits = gTexBits = bTexBits = iTexBits;
		}
		if (baseFormat == GL_ALPHA) {
			rTexBits = gTexBits = bTexBits = aTexBits;
		}
		else if (baseFormat == GL_LUMINANCE ||
			baseFormat == GL_LUMINANCE_ALPHA) {
			rTexBits = gTexBits = bTexBits = lTexBits;
		}
		// Find smaller of frame buffer and texture bits
		rBits = (rBits < rTexBits) ? rBits : rTexBits;
		gBits = (gBits < gTexBits) ? gBits : gTexBits;
		bBits = (bBits < bTexBits) ? bBits : bTexBits;
		// If these fail, something's seriously wrong.
		assert(rBits > 0);
		assert(gBits > 0);
		assert(bBits > 0);
		mTolerance[0] = 3.0 / (1 << rBits);
		mTolerance[1] = 3.0 / (1 << gBits);
		mTolerance[2] = 3.0 / (1 << bBits);
		//printf("tol: %g %g %g\n", mTolerance[0], 
		//	mTolerance[1], mTolerance[2]);
	}


}


// Do numColors * numColors tests in one batch.
// Setup a texture in which the colors vary by column.
// Draw a quadstrip in which we draw horizontal bands of colors.
// Drawing the textured quadstrips will fill the window with
// numColors * numColors test squares.
// Verify that they're all correct.
// Return:  true = pass, false = fail
bool
TexEnvTest::MatrixTest(GLenum envMode, GLenum texFormat,
	const char *envName, const char *formatName,
	int numColors, const GLfloat colors[][4],
	const GLfloat envColor[4], Window &w) {

	if (envMode == GL_DECAL && (texFormat != GL_RGB &&
		texFormat != GL_RGBA)) {
		// undefined mode
		return true;
	}

	glClear(GL_COLOR_BUFFER_BIT);

	// The texture colors are the columns
	MakeTexImage(texFormat, numColors, colors);

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, envMode);
	glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, envColor);

	// The fragment colors are the rows
	GLfloat W = numColors * 3;
	GLfloat S = (float) (numColors*3) / (float) 256;
	glBegin(GL_QUAD_STRIP);
	glTexCoord2f(0, 0);  glVertex2f(0, 0);
	glTexCoord2f(S, 0);  glVertex2f(W, 0);
	for (int i = 0; i < numColors; i++) {
		glColor4fv(colors[i]);
		GLfloat y = i * 3 + 3;
		GLfloat t = y / (numColors * 3);
		glTexCoord2f(0, t);  glVertex2f(0, y);
		glTexCoord2f(S, t);  glVertex2f(W, y);
	}
	glEnd();

	GLfloat image[256][256][4];
	glReadPixels(0, 0, 256, 256, GL_RGBA, GL_FLOAT, image);

	w.swap(); // lets us watch the progress

	// Check results
	for (int row = 0; row < numColors; row++) {
		for (int col = 0; col < numColors; col++) {

			// compute expected
			GLfloat expected[4];
			ComputeExpectedColor(envMode, texFormat,
				colors[col], colors[row],
				envColor, expected);

			// fetch actual pixel
			int x = col * 3 + 1;
			int y = row * 3 + 1;
			const GLfloat *actual = image[y][x];

			// compare
			if (!TestColor(expected, actual)) {
				// Report the error
				env->log << name
					 << ":  FAIL:  GL_TEXTURE_ENV_MODE="
					 << envName
					 << "  Texture Format="
					 << formatName
					 << "  Fragment Color=("
					 << colors[row][0] << ", "
					 << colors[row][1] << ", "
					 << colors[row][2] << ", "
					 << colors[row][3] << ") "
					 << " Texture Color=("
					 << colors[col][0] << ", "
					 << colors[col][1] << ", "
					 << colors[col][2] << ", "
					 << colors[col][3] << ") "
					 << " Tex Env Color=("
					 << envColor[0] << ", "
					 << envColor[1] << ", "
					 << envColor[2] << ", "
					 << envColor[3] << ") "
#if BLEND_WITH_BACKGROUND
					 << " Blend over=("
					 << BgColor[0] << ", "
					 << BgColor[1] << ", "
					 << BgColor[2] << ", "
					 << BgColor[3] << ") "
#endif
					 << " Expected=("
					 << expected[0] << ", "
					 << expected[1] << ", "
					 << expected[2] << ", "
					 << expected[3] << ") "
					 << " Measured=("
					 << actual[0] << ", "
					 << actual[1] << ", "
					 << actual[2] << ", "
					 << actual[3] << ")\n";
				return false;
			}
		}
	}
	return true;
}


void
TexEnvTest::runOne(BasicResult& r, Window& w) {

	(void) w;

#define COLORS (3*3*3*3)

	GLfloat colors[COLORS][4];

	// colors[] is an array of all possible RGBA colors with component
	// values of 0, 0.5, and 1.0
	for (int i = 0; i < COLORS; i++) {
		GLint r = i % 3;
		GLint g = (i / 3) % 3;
		GLint b = (i / 9) % 3;
		GLint a = (i / 27) % 3;
		colors[i][0] = (float) r / 2.0;
		colors[i][1] = (float) g / 2.0;
		colors[i][2] = (float) b / 2.0;
		colors[i][3] = (float) a / 2.0;
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glEnable(GL_TEXTURE_2D);

#if BLEND_WITH_BACKGROUND
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
#endif

	glClearColor(BgColor[0], BgColor[1], BgColor[2], BgColor[3]);
	glShadeModel(GL_FLAT);

	glViewport(0, 0, 256, 256);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, 256, 0, 256, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.375, 0.375, 0.0);

	int numModes;
	if (GLUtils::haveExtensions("GL_EXT_texture_env_add") ||
		GLUtils::haveExtensions("GL_ARB_texture_env_add"))
		numModes = 5;
	else
		numModes = 4;

	r.pass = true;

	for (int fmt = 0; fmt < 6; fmt++) {
		const GLenum format = FormatEnums[fmt];
		const char *formatName = FormatNames[fmt];
		for (int mode = 0; mode < numModes; mode++) {
			const GLenum envMode = EnvModeEnums[mode];
			const char *envName = EnvModeNames[mode];
			//printf("format %s mode %s\n", FormatNames[fmt],
			//	EnvModeNames[mode]);
			if (envMode == GL_BLEND && format != GL_ALPHA) {
				// also vary texenv color, every 5th is OK.
				for (int eCol = 0; eCol < COLORS; eCol += 5) {
					const GLfloat *envColor = colors[eCol];
					if (!MatrixTest(envMode, format,
						envName, formatName,
						COLORS, colors, envColor, w)) {
						r.pass = false;
						break;
					}
				}
			}
			else {
				// texenv color not significant
				if (!MatrixTest(envMode, format,
					envName, formatName,
					COLORS, colors, colors[0], w)) {
					r.pass = false;
				}
			}
		}
	}
} // TexEnvTest::runOne


void
TexEnvTest::logOne(BasicResult& r) {
	logPassFail(r);
	logConcise(r);
} // TexEnvTest::logOne


///////////////////////////////////////////////////////////////////////////////
// The test object itself:
///////////////////////////////////////////////////////////////////////////////
TexEnvTest texEnvTest("texEnv", "window, rgb",

	"Test basic texture env modes for all base texture formats.\n");


} // namespace GLEAN
