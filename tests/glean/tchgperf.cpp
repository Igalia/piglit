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

// tchgperf.cpp:  Some basic tests of attribute-change performance.

#include "tchgperf.h"
#include <algorithm>
#include "rand.h"
#include "image.h"
#include "timer.h"
#include "geomutil.h"

#if 0
#ifdef __UNIX__
#include <unistd.h>
#endif

#include <iostream>
#include <fstream>
#include "dsconfig.h"
#include "dsfilt.h"
#include "dsurf.h"
#include "winsys.h"
#include "environ.h"
#include "rc.h"
#include "glutils.h"
#include "timer.h"
#include "tchgperf.h"
#include "misc.h"
#endif

namespace {

GLEAN::Image redImage(64, 64, GL_RGB, GL_UNSIGNED_BYTE, 1.0, 0.0, 0.0, 0.0);
GLuint redTex;
GLEAN::Image greenImage(64, 64, GL_RGB, GL_UNSIGNED_BYTE, 0.0, 1.0, 0.0, 0.0);
GLuint greenTex;

int nPoints;
float* vertices;
float* texCoords;

void
noBindDraw() {
	int rowSize = 2 * nPoints;
	for (int y = 0; y < nPoints - 1; ++y) {
		float* t0 = texCoords + y * rowSize;
		float* v0 = vertices + y * rowSize;
		for (int x = 0; x < nPoints - 1; ++x) {
			float* t1 = t0 + rowSize;
			float* t2 = t1 + 2;
			float* t3 = t0 + 2;
			float* v1 = v0 + rowSize;
			float* v2 = v1 + 2;
			float* v3 = v0 + 2;

			glBegin(GL_TRIANGLES);
				glTexCoord2fv(t0);
				glVertex2fv(v0);
				glTexCoord2fv(t1);
				glVertex2fv(v1);
				glTexCoord2fv(t2);
				glVertex2fv(v2);
			glEnd();
			glBegin(GL_TRIANGLES);
				glTexCoord2fv(t2);
				glVertex2fv(v2);
				glTexCoord2fv(t3);
				glVertex2fv(v3);
				glTexCoord2fv(t0);
				glVertex2fv(v0);
			glEnd();

			t0 += 2;
			v0 += 2;
		}
	}
} // noBindDraw

void
bindDraw() {
	int rowSize = 2 * nPoints;
	for (int y = 0; y < nPoints - 1; ++y) {
		float* v0 = vertices + y * rowSize;
		float* t0 = texCoords + y * rowSize;
		for (int x = 0; x < nPoints - 1; ++x) {
			float* t1 = t0 + rowSize;
			float* t2 = t1 + 2;
			float* t3 = t0 + 2;
			float* v1 = v0 + rowSize;
			float* v2 = v1 + 2;
			float* v3 = v0 + 2;

			glBindTexture(GL_TEXTURE_2D, redTex);
			glBegin(GL_TRIANGLES);
				glTexCoord2fv(t0);
				glVertex2fv(v0);
				glTexCoord2fv(t1);
				glVertex2fv(v1);
				glTexCoord2fv(t2);
				glVertex2fv(v2);
			glEnd();

			glBindTexture(GL_TEXTURE_2D, greenTex);
			glBegin(GL_TRIANGLES);
				glTexCoord2fv(t2);
				glVertex2fv(v2);
				glTexCoord2fv(t3);
				glVertex2fv(v3);
				glTexCoord2fv(t0);
				glVertex2fv(v0);
			glEnd();

			t0 += 2;
			v0 += 2;
		}
	}
} // BindDraw

class BindDrawTimer: public GLEAN::Timer {
	virtual void op()     { bindDraw(); }
	virtual void preop()  { glFinish(); }
	virtual void postop() { glFinish(); }
};

class NoBindDrawTimer: public GLEAN::Timer {
	virtual void op()     { noBindDraw(); }
	virtual void preop()  { glFinish();   }
	virtual void postop() { glFinish();   }
};

void
logStats(GLEAN::TexBindPerfResult& r, GLEAN::Environment* env) {
	env->log << "\tApproximate texture binding time = " << r.bindTime
		<< " microseconds.\n\tRange of valid measurements = ["
		<< r.lowerBound << ", " << r.upperBound << "]\n";
} // logStats

} // anonymous namespace

namespace GLEAN {

///////////////////////////////////////////////////////////////////////////////
// runOne:  Run a single test case
///////////////////////////////////////////////////////////////////////////////

void
TexBindPerf::runOne(TexBindPerfResult& r, Window& w) {
	glGenTextures(1, &redTex);
	glBindTexture(GL_TEXTURE_2D, redTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	redImage.makeMipmaps(GL_RGB);

	glGenTextures(1, &greenTex);
	glBindTexture(GL_TEXTURE_2D, greenTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	greenImage.makeMipmaps(GL_RGB);

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

	GLUtils::useScreenCoords(drawingSize + 2, drawingSize + 2);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_TEXTURE_2D);
	glColor4f(1.0, 1.0, 1.0, 1.0);

	nPoints = drawingSize / 2;	// Yields 1-pixel triangles.

	RandomDouble vRand(142857);
	RandomMesh2D v(1.0, drawingSize, nPoints, 1.0, drawingSize, nPoints,
		vRand);
	vertices = v(0, 0);

	RandomDouble tRand(314159);
	RandomMesh2D t(0.0, 1.0, nPoints, 0.0, 1.0, nPoints, tRand);
	texCoords = t(0, 0);

	int nTris = (nPoints - 1) * (nPoints - 1) / 2;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	BindDrawTimer   bindDrawTimer;
	NoBindDrawTimer noBindDrawTimer;

	bindDrawTimer.calibrate();
	noBindDrawTimer.calibrate();

	vector<float> measurements;
	for (int i = 0; i < 5; ++i) {
		env->quiesce();
		double tBind = bindDrawTimer.time();
		w.swap();	// So the user can see something happening.

		env->quiesce();
		double tNoBind = noBindDrawTimer.time();
		w.swap();

		double bindTime = 1E6 * (tBind - tNoBind) / nTris;
		if (bindTime < 0.0) {
			// This can happen if the system isn't quiescent;
			// some process sneaks in and takes wall-clock time
			// when ``noBindDraw'' is running.  Just flush it
			// and try again.  (Note:  You really shouldn't be
			// running timing tests on a system where other
			// processes are active!)
			--i;
			continue;
		}

		measurements.push_back(bindTime);
	}

	sort(measurements.begin(), measurements.end());
	r.bindTime = (measurements[1]+measurements[2]+measurements[3]) / 3.0;
	r.lowerBound = measurements[1];
	r.upperBound = measurements[3];
	r.pass = true;
} // TexBindPerf::runOne

///////////////////////////////////////////////////////////////////////////////
// logOne:  Log a single test case
///////////////////////////////////////////////////////////////////////////////
void
TexBindPerf::logOne(TexBindPerfResult& r) {
	logPassFail(r);
	logConcise(r);
	logStats(r, env);
} // TexBindPerf::logOne

///////////////////////////////////////////////////////////////////////////////
// compareOne:  Compare results for a single test case
///////////////////////////////////////////////////////////////////////////////
void
TexBindPerf::compareOne(TexBindPerfResult& oldR, TexBindPerfResult& newR) {
	if (newR.bindTime < oldR.lowerBound) {
		int percent = static_cast<int>(
			100.0 * (oldR.bindTime - newR.bindTime) / newR.bindTime
			+ 0.5);
		env->log << name << ":  DIFF "
			<< newR.config->conciseDescription() << '\n'
			<< '\t' << env->options.db2Name << " may be "
			<< percent << "% faster.\n";
	} else if (newR.bindTime > oldR.upperBound) {
		int percent = static_cast<int>(
			100.0 * (newR.bindTime - oldR.bindTime) / oldR.bindTime
			+ 0.5);
		env->log << name << ":  DIFF "
			<< oldR.config->conciseDescription() << '\n'
			<< '\t' << env->options.db1Name << " may be "
			<< percent << "% faster.\n";
	} else {
		if (env->options.verbosity)
			env->log << name << ":  SAME "
				<< newR.config->conciseDescription()
				<< "\n\t"
				<< env->options.db2Name
				<< " test time falls within the "
				<< "valid measurement range of "
				<< env->options.db1Name
				<< " test time.\n";
	}
	if (env->options.verbosity) {
		env->log << env->options.db1Name << ':';
		logStats(oldR, env);
		env->log << env->options.db2Name << ':';
		logStats(newR, env);
	}
} // TexBindPerf::compareOne

///////////////////////////////////////////////////////////////////////////////
// The test object itself:
///////////////////////////////////////////////////////////////////////////////
TexBindPerf texBindPerfTest("texBindPerf", "window, rgb, z",

	"This test makes a rough estimate of the cost of a glBindTexture()\n"
	"operation, expressed in microseconds.\n"
	"\n"
	"Since the apparent cost of a texture bind is dependent on many\n"
	"factors (including the fraction of the texture map that's actually\n"
	"used for drawing, on machines that cache textures; texture map\n"
	"size; texel format; etc.), a general-purpose test can only estimate\n"
	"it.  In this test we do so by drawing random triangles of very\n"
	"small size, and reporting simple statistics concerning the cost.\n");


} // namespace GLEAN
