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

// ttexture_srgb.h:  Test GL_EXT_texture_sRGB extension.
// Brian Paul  August 2006


#include <cassert>
#include <cstring>
#include <cmath>
#include "ttexture_srgb.h"
#include "rand.h"


namespace GLEAN {


static const struct {
	GLenum sFormat;
	GLenum baseFormat;
	GLint components;
} Formats[] = {
	{ GL_SRGB_EXT, GL_RGB, 3 },
	{ GL_SRGB8_EXT, GL_RGB, 3 },
	{ GL_SRGB_ALPHA_EXT, GL_RGBA, 4 },
	{ GL_SRGB8_ALPHA8_EXT, GL_RGBA, 4 },
	{ GL_SLUMINANCE_ALPHA_EXT, GL_LUMINANCE_ALPHA, 2 },
	{ GL_SLUMINANCE8_ALPHA8_EXT, GL_LUMINANCE_ALPHA, 2 },
	{ GL_SLUMINANCE_EXT, GL_LUMINANCE, 1 },
	{ GL_SLUMINANCE8_EXT, GL_LUMINANCE, 1 },
	{ 0, 0, 0 }
};




// Convert an 8-bit sRGB value from non-linear space to a
// linear RGB value in [0, 1].
// Implemented with a 256-entry lookup table.
static float
nonlinear_to_linear(GLubyte cs8)
{
	static GLfloat table[256];
	static GLboolean tableReady = GL_FALSE;
	if (!tableReady) {
		// compute lookup table now
		GLuint i;
		for (i = 0; i < 256; i++) {
			const GLfloat cs = i / 255.0;
			if (cs <= 0.04045) {
				table[i] = cs / 12.92;
			}
			else {
				table[i] = pow((cs + 0.055) / 1.055, 2.4);
			}
		}
		tableReady = GL_TRUE;
	}
	return table[cs8];
}


// allocate and fill an array with random values
static GLubyte *
randomArray(int bytes, int seed)
{
	GLEAN::RandomBits r(8, seed);
	GLubyte *img = new GLubyte [bytes];

        for (int i = 0; i < bytes; i++)
		img[i] = r.next();

	return img;
}


// Test glTexImage and glGetTexImage functionality
bool
TextureSRGBTest::testImageTransfer(void)
{
	const GLubyte *image = randomArray(128 * 128 * 4, 0);
        GLubyte image2[128 * 128 * 4];
        int i, j;

        for (i = 0; Formats[i].sFormat; i++) {
                // upload tex image
                glTexImage2D(GL_TEXTURE_2D, 0, Formats[i].sFormat, 128, 128, 0,
                             Formats[i].baseFormat, GL_UNSIGNED_BYTE, image);

                // retrieve tex image
                glGetTexImage(GL_TEXTURE_2D, 0,
                              Formats[i].baseFormat, GL_UNSIGNED_BYTE, image2);

                // compare original and returned images
                const int comps = Formats[i].components;
                for (j = 0; j < 128 * 128 * comps; j++) {
                        if (image[j] != image2[j]) {
				env->log << '\n'
					<< name
					<< " glGetTexImage failed for internalFormat "
					<< Formats[i].sFormat
					<< "\n";
				env->log << "Expected value at ["
					<< j
					<< "] should be "
					<< image[j]
					<< " found "
					<< image2[j]
					<< "\n";
				delete [] image;
                                return false;
                        }
			image2[j] = 0; // reset for next GetTexImage
                }
        }

	delete [] image;
        return true;
}


bool
TextureSRGBTest::testTextureFormat(GLenum intFormat, GLint components,
				   GLEAN::Environment &env)
{
	const GLubyte *image = randomArray(128 * 128 * 4, intFormat);
	GLfloat readback[128 * 128 * 4];
        int i;
	GLint redBits, alphaBits;

	glGetIntegerv(GL_RED_BITS, &redBits);
	glGetIntegerv(GL_ALPHA_BITS, &alphaBits);
	const float tolerance = 1.0 / ((1 << (redBits - 1)) - 1);

	// setup matrices
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glViewport(0, 0, windowSize, windowSize);

	// setup texture
	glTexImage2D(GL_TEXTURE_2D, 0, intFormat, 128, 128, 0,
		     GL_RGBA, GL_UNSIGNED_BYTE, image);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glEnable(GL_TEXTURE_2D);

	glDisable(GL_DITHER);

	glDrawBuffer(GL_FRONT);
	glReadBuffer(GL_FRONT);

	// draw test polygon
	glBegin(GL_POLYGON);
	glTexCoord2f(0, 0);  glVertex2f(-1, -1);
	glTexCoord2f(1, 0);  glVertex2f( 1, -1);
	glTexCoord2f(1, 1);  glVertex2f( 1,  1);
	glTexCoord2f(0, 1);  glVertex2f(-1,  1);
	glEnd();

	glReadPixels(0, 0, windowSize, windowSize,
		     GL_RGBA, GL_FLOAT, readback);

	// compare rendered results to expected values
	for (i = 0; i < 128 * 128; i++) {
		const GLfloat *actual = readback + i * 4;
		GLfloat expected[4];

		expected[0] = nonlinear_to_linear(image[i * 4 + 0]);
		expected[1] = nonlinear_to_linear(image[i * 4 + 1]);
		expected[2] = nonlinear_to_linear(image[i * 4 + 2]);
		expected[3] = image[i * 4 + 3] / 255.0;

		if (components <= 2) {
			if (fabs(actual[0] - expected[0]) > tolerance) {
				env.log << '\n'
					<< name
					<< " failed for internalFormat "
					<< intFormat
					<< "\n";
				env.log << "Expected luminance "
					<< expected[0]
					<< " found "
					<< actual[0]
					<< "\n";
				delete [] image;
				return GL_FALSE;
			}

		}
		else {
			assert(components == 3 || components == 4);
			if (fabs(actual[0] - expected[0]) > tolerance ||
			    fabs(actual[1] - expected[1]) > tolerance ||
			    fabs(actual[2] - expected[2]) > tolerance) {
				env.log << '\n'
					<< name
					<< " failed for internalFormat "
					<< intFormat
					<< "\n";
				env.log << "Expected color "
					<< expected[0]
					<< ", "
					<< expected[1]
					<< ", "
					<< expected[2]
					<< " found "
					<< actual[0]
					<< ", "
					<< actual[1]
					<< ", "
					<< actual[2]
					<< "\n";
				delete [] image;
				return GL_FALSE;
			}
		}

		if (alphaBits >= redBits
		    && components == 4
		    && fabs(actual[3] - expected[3]) > tolerance) {
			env.log << '\n'
				<< name
				<< " failed for internalFormat "
				<< intFormat
				<< "\n";
			env.log << "Expected alpha "
				<< expected[3]
				<< " found "
				<< actual[3]
				<< "\n";
			delete [] image;
			return GL_FALSE;
		}
	}

	delete [] image;
	return GL_TRUE;
}


// Test actual texture mapping using each of the sRGB formats
// Return GL_TRUE if all format tests pass, GL_FALSE if any fail.
bool
TextureSRGBTest::testTexturing(void)
{
	for (int i = 0; Formats[i].sFormat; i++) {
		if (!testTextureFormat(Formats[i].sFormat,
				       Formats[i].components, *env))
			return GL_FALSE;
	}

	return GL_TRUE;
}


void
TextureSRGBTest::runOne(TextureSRGBResult &r, Window &w)
{
	(void) w;  // silence warning
	r.pass = true;
	errorCode = 0;
	errorPos = NULL;
	errorMsg[0] = 0;

	if (r.pass)
		r.pass = testImageTransfer();
	if (r.pass)
		r.pass = testTexturing();
}


void
TextureSRGBTest::logOne(TextureSRGBResult &r)
{
	if (r.pass) {
		logPassFail(r);
		logConcise(r);
	}
	else {
		env->log << name << " FAIL\n";
		if (errorCode) {
			env->log << "\tOpenGL Error " << gluErrorString(errorCode)
				 << " at " << errorPos << "\n";
		}
		else if (errorMsg[0]) {
			env->log << "\t" << errorMsg << "\n";
		}
	}
}


void
TextureSRGBTest::compareOne(TextureSRGBResult &oldR,
			     TextureSRGBResult &newR)
{
	comparePassFail(oldR, newR);

	if (newR.pass && oldR.pass == newR.pass) {
		// XXX
	}
	else {
		env->log << "\tNew: ";
		env->log << (newR.pass ? "PASS" : "FAIL");
		env->log << "\tOld: ";
		env->log << (oldR.pass ? "PASS" : "FAIL");
	}
}


void
TextureSRGBResult::putresults(ostream &s) const
{
	if (pass) {
		s << "PASS\n";
	}
	else {
		s << "FAIL\n";
	}
}


bool
TextureSRGBResult::getresults(istream &s)
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
TextureSRGBTest srgbTest("texture_srgb", "window, rgb",
			 "GL_EXT_texture_sRGB",
			 "Test the GL_EXT_texture_sRGB extension.\n");



} // namespace GLEAN
