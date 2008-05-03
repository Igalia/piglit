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

// tdepthstencil.h:  Test GL_EXT_packed_depth_stencil extension.
// Brian Paul  1 October 2005


#include "tdepthstencil.h"
#include "rand.h"
#include "timer.h"
#include "image.h"
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <cstring>

#ifdef GL_EXT_packed_depth_stencil

namespace GLEAN {

static PFNGLWINDOWPOS2IARBPROC WindowPos2i = NULL;


DepthStencilResult::DepthStencilResult()
{
	pass = false;
	readDepthStencilRate = 0;
	readDepthUintRate = 0;
	readDepthUshortRate = 0;
}


bool
DepthStencilTest::checkError(const char *where)
{
	GLenum err = glGetError();
	if (err) {
		errorCode = err;
		errorPos = where;
		return true;
	}
	return false;
}

void
DepthStencilTest::setup(void)
{
	glGetIntegerv(GL_DEPTH_BITS, &depthBits);
	glGetIntegerv(GL_STENCIL_BITS, &stencilBits);

        WindowPos2i = (PFNGLWINDOWPOS2IARBPROC)
		GLUtils::getProcAddress("glWindowPos2iARB");
        assert(WindowPos2i);
}


// If we're lacking a depth and/or stencil buffer we'll just run this test.
// Return true if pass, false if fail.
bool
DepthStencilTest::testInsufficientVisual(void)
{
	GLuint p[1];

	glDrawPixels(1, 1, GL_DEPTH_STENCIL_EXT, GL_UNSIGNED_INT_24_8_EXT, p);
	if (glGetError() != GL_INVALID_OPERATION) {
		sprintf(errorMsg,
			"glDrawPixels failed to raise GL_INVALID_OPERATION"
			" when there's no depth or stencil buffer.");
		return false;
	}

	glCopyPixels(0, 0, 5, 5, GL_DEPTH_STENCIL_EXT);
	if (glGetError() != GL_INVALID_OPERATION) {
		sprintf(errorMsg,
			"glCopyPixels failed to raise GL_INVALID_OPERATION"
			" when there's no depth or stencil buffer.");
		return false;
	}

	glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8_EXT,
			 0, 0, 1, 1, 0);
	if (glGetError() != GL_INVALID_OPERATION) {
		sprintf(errorMsg,
			"glCopyTexImage2D failed to raise GL_INVALID_OPERATION"
			" when there's no depth or stencil buffer.");
		return false;
	}

	return true;
}


// Each of these OpenGL calls in this function should generate an error!
// Note to GL implementors: if you find any errors here, you better check
// your glTexImage functions too!
bool
DepthStencilTest::testErrorDetection(void)
{
	GLuint p[1];

	glDrawPixels(1, 1, GL_DEPTH_STENCIL_EXT, GL_UNSIGNED_INT, p);
	if (glGetError() != GL_INVALID_ENUM) {
		sprintf(errorMsg,
			"glDrawPixels(GL_DEPTH_STENCIL_EXT, GL_UNSIGNED_INT)"
			" failed to generate GL_INVALID_ENUM.");
		return false;
	}

	glDrawPixels(1, 1, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT_24_8_EXT, p);
	if (glGetError() != GL_INVALID_OPERATION) {
		sprintf(errorMsg,
			"glDrawPixels(GL_DEPTH_COMPONENT, GL_UNSIGNED_INT_24_8_EXT)"
			" failed to generate GL_INVALID_OPERATION.");
		return false;
	}

	glReadPixels(0, 0, 1, 1, GL_DEPTH_STENCIL_EXT, GL_FLOAT, p);
	if (glGetError() != GL_INVALID_ENUM) {
		sprintf(errorMsg,
			"glReadPixels(GL_DEPTH_STENCIL_EXT, GL_FLOAT)"
			" failed to generate GL_INVALID_ENUM.");
		return false;
	}

	glReadPixels(0, 0, 1, 1, GL_STENCIL_INDEX, GL_UNSIGNED_INT_24_8_EXT, p);
	if (glGetError() != GL_INVALID_OPERATION) {
		sprintf(errorMsg,
			"glReadPixels(GL_STENCIL_INDEX, GL_UNSIGNED_INT_24_8_EXT)"
			" failed to generate GL_INVALID_OPERATION.");
		return false;
	}

	return true;
}


bool
DepthStencilTest::testDrawAndRead(void)
{
	// the reference image
	static const GLuint image[4] = {
		0x00000000,
		0x000000ff,
		0xffffff00,
		0xffffffff
	};
	GLuint readback[4];

	WindowPos2i(0, 0);
	glDrawPixels(2, 2, GL_DEPTH_STENCIL_EXT,
		     GL_UNSIGNED_INT_24_8_EXT, image);
	if (checkError("glDrawPixels in testDrawAndRead"))
		return false;

	glReadPixels(0, 0, 2, 2, GL_DEPTH_STENCIL_EXT,
		     GL_UNSIGNED_INT_24_8_EXT, readback);
	if (checkError("glReadPixels in testDrawAndRead"))
		return false;

	for (int i = 0; i < 4; i++) {
		if (image[i] != readback[i]) {
			sprintf(errorMsg,
				"Image returned by glReadPixels didn't match"
				" the expected result (0x%x != 0x%x)",
				readback[i], image[i]);
			return false;
		}
	}

	// test depth scale/bias and stencil mapping (in a trivial way)
	glPixelTransferf(GL_DEPTH_SCALE, 0.0);  // map all depths to 1.0
	glPixelTransferf(GL_DEPTH_BIAS, 1.0);
	GLuint stencilMap[2] = { 2, 2 };  // map all stencil values to 2
	glPixelMapuiv(GL_PIXEL_MAP_S_TO_S, 2, stencilMap);
	glPixelTransferi(GL_MAP_STENCIL, 1);
	glReadPixels(0, 0, 2, 2, GL_DEPTH_STENCIL_EXT,
		     GL_UNSIGNED_INT_24_8_EXT, readback);
	if (checkError("glReadPixels in testDrawAndRead"))
		return false;
	for (int i = 0; i < 4; i++) {
		if (readback[i] != 0xffffff02) {
			sprintf(errorMsg,
				"Image returned by glReadPixels didn't match"
				" the expected result (0x%x != 0xffffff02)",
				readback[i]);
			return false;
		}
	}
	glPixelTransferf(GL_DEPTH_SCALE, 1.0);
	glPixelTransferf(GL_DEPTH_BIAS, 0.0);
	glPixelTransferi(GL_MAP_STENCIL, 0);

	return true;
}


bool
DepthStencilTest::testTextureOperations(void)
{
	glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8_EXT,
			 0, 0, 1, 1, 0);
	if (checkError("glCopyTexImage2D in testTextureOperations."))
		return false;

	glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, 1, 1);
	if (checkError("glCopyTexSubImage2D in testTextureOperations."))
		return false;

	return true;
}


double
DepthStencilTest::readPixelsRate(GLenum format, GLenum type)
{
	const int width = drawingSize, height = drawingSize;
	GLuint *img = new GLuint [width * height];

	WindowPos2i(0, 0);
	glDrawPixels(width, height, GL_DEPTH_STENCIL_EXT,
		     GL_UNSIGNED_INT_24_8_EXT, img);

	const double minInterval = 2.0; // two seconds
	Timer tTimer;
	double start = tTimer.getClock();
	double elapsedTime = 0.0;
	int iterations = 0;
	do {
		for (int i = 0; i < 50; i++) {
			glReadPixels(0, 0, width, height, format, type, img);
			iterations++;
		}

		double finish = tTimer.getClock();
		elapsedTime = finish - start;
	} while (elapsedTime < minInterval);

	delete [] img;

	double rate = width * height * iterations / elapsedTime;
	return rate;  // pixels/second
}


void
DepthStencilTest::testPerformance(DepthStencilResult &r)
{
	r.readDepthStencilRate = readPixelsRate(GL_DEPTH_STENCIL_EXT,
						GL_UNSIGNED_INT_24_8_EXT);
	r.readDepthUintRate = readPixelsRate(GL_DEPTH_COMPONENT,
					     GL_UNSIGNED_INT);
	r.readDepthUshortRate = readPixelsRate(GL_DEPTH_COMPONENT,
					       GL_UNSIGNED_SHORT);

	// XXX maybe also test glCopyTexImage, etc.
}


void
DepthStencilTest::runOne(DepthStencilResult &r, Window &w)
{
	(void) w;  // silence warning
	r.pass = true;
	errorCode = 0;
	errorPos = NULL;
	errorMsg[0] = 0;

	setup();

	if (depthBits == 0 || stencilBits == 0) {
		r.pass = testInsufficientVisual();
		return;
	}

	if (r.pass)
		r.pass = testErrorDetection();
	if (r.pass)
		r.pass = testDrawAndRead();
	if (r.pass)
		r.pass = testTextureOperations();
	if (r.pass)
		testPerformance(r);
}


void
DepthStencilTest::logOne(DepthStencilResult &r)
{
	if (r.pass) {
		logPassFail(r);
		logConcise(r);

		char str[1000];
		double mbps;

		env->log << "\tglReadPixels GL_DEPTH_STENCIL rate: ";
		mbps = r.readDepthStencilRate * sizeof(GLuint) / (1024*1024);
		sprintf(str, "%.2f", mbps);
		env->log << str << " MBytes per second.\n";

		env->log << "\tglReadPixels GL_DEPTH/GLuint rate: ";
		mbps = r.readDepthUintRate * sizeof(GLuint) / (1024*1024);
		sprintf(str, "%.2f", mbps);
		env->log << str << " MBytes per second.\n";

		env->log << "\tglReadPixels GL_DEPTH/GLushort rate: ";
		mbps = r.readDepthUshortRate * sizeof(GLshort) / (1024*1024);
		sprintf(str, "%.2f", mbps);
		env->log << str << " MBytes per second.\n";
	}
	else {
		env->log << name << "FAIL\n";
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
DepthStencilTest::compareOne(DepthStencilResult &oldR,
			     DepthStencilResult &newR)
{
	comparePassFail(oldR, newR);

	if (newR.pass && oldR.pass == newR.pass) {
		if (env->options.verbosity) {
			env->log << "\tReadPixels rate:\n";
			env->log << "\t\tGL_DEPTH_STENCIL:\n";
			env->log << "\t\t\told: " << oldR.readDepthStencilRate;
			env->log << "\t\t\tnew: " << newR.readDepthStencilRate;
			env->log << "\t\tGL_DEPTH/GL_UNSIGNED_INT:\n";
			env->log << "\t\t\told: " << oldR.readDepthUintRate;
			env->log << "\t\t\tnew: " << newR.readDepthUintRate;
			env->log << "\t\tGL_DEPTH/GL_UNSIGNED_SHORT:\n";
			env->log << "\t\t\told: " << oldR.readDepthUshortRate;
			env->log << "\t\t\tnew: " << newR.readDepthUshortRate;
		}
	}
	else {
		env->log << "\tNew: ";
		env->log << (newR.pass ? "PASS" : "FAIL");
		env->log << "\tOld: ";
		env->log << (oldR.pass ? "PASS" : "FAIL");
	}
}


void
DepthStencilResult::putresults(ostream &s) const
{
	if (pass) {
		s << "PASS\n";

		char str[1000];
		double mbps;

		mbps = readDepthStencilRate * sizeof(GLuint) / (1024*1024);
		sprintf(str, "%.2f", mbps);
		s << str << "\n";

		mbps = readDepthUintRate * sizeof(GLuint) / (1024*1024);
		sprintf(str, "%.2f", mbps);
		s << str << "\n";

		mbps = readDepthUshortRate * sizeof(GLushort) / (1024*1024);
		sprintf(str, "%.2f", mbps);
		s << str << "\n";
	}
	else {
		s << "FAIL\n";
	}
}


bool
DepthStencilResult::getresults(istream &s)
{
	char result[1000];
	s >> result;

	if (strcmp(result, "FAIL") == 0) {
		pass = false;
	}
	else {
		pass = true;
		s >> readDepthStencilRate;
		s >> readDepthUintRate;
		s >> readDepthUshortRate;
	}
	return s.good();
}


// The test object itself:
DepthStencilTest depthstencilTest("depthStencil", "window, rgb",
			"GL_EXT_packed_depth_stencil GL_ARB_window_pos",
			"Test the GL_EXT_packed_depth_stencil extension.\n");



} // namespace GLEAN

#endif // GL_EXT_packed_depth_stencil
