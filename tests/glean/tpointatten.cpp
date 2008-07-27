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

// tpointatten.h:  Test GL_ARB_point_parameters extension.
// Brian Paul  6 October 2005


#include "tpointatten.h"
#include <cassert>
#include <cmath>


namespace GLEAN {

// Max tested point size
#define MAX_SIZE 25.0


/* Clamp X to [MIN,MAX] */
#define CLAMP( X, MIN, MAX )  ( (X)<(MIN) ? (MIN) : ((X)>(MAX) ? (MAX) : (X)) )


static PFNGLPOINTPARAMETERFVARBPROC PointParameterfvARB = NULL;
static PFNGLPOINTPARAMETERFARBPROC PointParameterfARB = NULL;


void
PointAttenuationTest::setup(void)
{
	PointParameterfvARB = (PFNGLPOINTPARAMETERFVARBPROC)
		GLUtils::getProcAddress("glPointParameterfvARB");
	assert(PointParameterfvARB);
	PointParameterfARB = (PFNGLPOINTPARAMETERFARBPROC)
		GLUtils::getProcAddress("glPointParameterfARB");
	assert(PointParameterfARB);

	glGetFloatv(GL_ALIASED_POINT_SIZE_RANGE, aliasedLimits);
	glGetFloatv(GL_SMOOTH_POINT_SIZE_RANGE, smoothLimits);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-10.0, 10.0, -10.0, 10.0, -10.0, 10.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}


void
PointAttenuationTest::reportFailure(GLfloat initSize,
				    const GLfloat attenuation[3],
				    GLfloat min, GLfloat max,
				    GLfloat eyeZ, GLboolean smooth,
				    GLfloat expected, GLfloat actual) const
{
	env->log << "\tFAILURE:\n";
	env->log << "\tExpected size: " << expected << "  Actual size: " << actual << "\n";
	env->log << "\tSize: " << initSize << "\n";
	env->log << "\tMin: " << min << "  Max: " << max << "\n";
	env->log << "\tAttenuation: " << attenuation[0] << " " << attenuation[1] << " " << attenuation[2] << "\n";
	env->log << "\tEye Z: " << eyeZ << "\n";
	if (smooth)
		env->log << "\tSmooth/antialiased\n";
	else
		env->log << "\tAliased\n";
}


void
PointAttenuationTest::reportSuccess(int count, GLboolean smooth) const
{
	env->log << "PASS: " << count;
	if (smooth)
		env->log << " aliased combinations tested.\n";
	else
		env->log << " antialiased combinations tested.\n";
}


// Compute the expected point size given various point state
GLfloat
PointAttenuationTest::expectedSize(GLfloat initSize,
				   const GLfloat attenuation[3],
				   GLfloat min, GLfloat max,
				   GLfloat eyeZ, GLboolean smooth) const
{
	const GLfloat dist = fabs(eyeZ);
	const GLfloat atten = sqrt(1.0 / (attenuation[0] +
					  attenuation[1] * dist +
					  attenuation[2] * dist * dist));

	float size = initSize * atten;

	size = CLAMP(size, min, max);

	if (smooth)
		size = CLAMP(size, smoothLimits[0], smoothLimits[1]);
	else
		size = CLAMP(size, aliasedLimits[0], aliasedLimits[1]);
	return size;
}


// measure size of rendered point at yPos (in model coords)
GLfloat
PointAttenuationTest::measureSize(GLfloat yPos) const
{
	assert(yPos >= -10.0);
	assert(yPos <= 10.0);
	float yNdc = (yPos + 10.0) / 20.0;  // See glOrtho above
	int x = 0;
	int y = (int) (yNdc * windowHeight);
	int w = windowWidth;
	int h = 1;
	GLfloat image[windowWidth * 3];
	// Read row of pixels and add up colors, which should be white
	// or shades of gray if smoothing is enabled.
	glReadPixels(x, y, w, h, GL_RGB, GL_FLOAT, image);
	float sum = 0.0;
	for (int i = 0; i < w; i++) {
		sum += (image[i*3+0] + image[i*3+1] + image[i*3+2]) / 3.0;
	}
	return sum;
}


bool
PointAttenuationTest::testPointRendering(GLboolean smooth)
{
	// epsilon is the allowed size difference in pixels between the
	// expected and actual rendering.
	const GLfloat epsilon = smooth ? 1.5 : 1.0;
	GLfloat atten[3];
	int count = 0;

	// Enable front buffer if you want to see the rendering
	//glDrawBuffer(GL_FRONT);
	//glReadBuffer(GL_FRONT);

	if (smooth) {
		glEnable(GL_POINT_SMOOTH);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
	else {
		glDisable(GL_POINT_SMOOTH);
		glDisable(GL_BLEND);
	}

	for (int a = 0; a < 3; a++) {
		atten[0] = pow(10.0, -a);
		for (int b = -2; b < 3; b++) {
			atten[1] = (b == -1) ? 0.0 : pow(10.0, -b);
			for (int c = -2; c < 3; c++) {
				atten[2] = (c == -1) ? 0.0 : pow(10.0, -c);
				PointParameterfvARB(GL_POINT_DISTANCE_ATTENUATION_ARB, atten);
				for (float min = 1.0; min < MAX_SIZE; min += 5) {
					PointParameterfARB(GL_POINT_SIZE_MIN_ARB, min);
					for (float max = min; max < MAX_SIZE; max += 5) {
						PointParameterfARB(GL_POINT_SIZE_MAX_ARB, max);
						for (float size = 1.0; size < MAX_SIZE; size += 4) {
							glPointSize(size);

							// draw column of points
							glClear(GL_COLOR_BUFFER_BIT);
							printf("atten: %f %f %f  min/max: %f %f  size: %f\n",
								atten[0], atten[1], atten[2], min, max, size);
							glBegin(GL_POINTS);
							for (float z = -6.0; z <= 6.0; z += 1.0) {
								glVertex3f(0, z, z);
							}
							glEnd();

							// test the column of points
							for (float z = -6.0; z <= 6.0; z += 1.0) {
								count++;
								float expected
									= expectedSize(size, atten, min, max,
												   z, smooth);
								float actual = measureSize(z);
								if (fabs(expected - actual) > epsilon) {
									reportFailure(size, atten, min, max,
												  z, smooth,
												  expected, actual);
									return false;
								}
							}
						}
					}
				}
			}
		}
	}
	reportSuccess(count, smooth);
	return true;
}

void
PointAttenuationTest::runOne(BasicResult &r, Window &w)
{
	(void) w;  // silence warning
	r.pass = true;
	errorCode = 0;
	errorPos = NULL;

	setup();

	if (r.pass)
		r.pass = testPointRendering(GL_FALSE);
									w.swap();
									sleep(10);
	if (r.pass)
		r.pass = testPointRendering(GL_TRUE);
}


void
PointAttenuationTest::logOne(BasicResult &r)
{
	if (r.pass) {
		logPassFail(r);
		logConcise(r);
	}
}


// constructor
PointAttenuationTest::PointAttenuationTest(const char *testName,
					   const char *filter,
					   const char *extensions,
					   const char *description)
	: BasicTest(testName, filter, extensions, description)
{
	fWidth  = windowWidth;
	fHeight = windowHeight;
}



// The test object itself:
PointAttenuationTest pointAttenuationTest("pointAtten", "window, rgb",
	"GL_ARB_point_parameters",
	"Test point size attenuation with the GL_ARB_point_parameters extension.\n");



} // namespace GLEAN
