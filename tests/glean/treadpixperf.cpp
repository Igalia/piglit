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


#include "treadpixperf.h"
#include "rand.h"
#include "timer.h"
#include "image.h"
#include <cassert>
#include <cmath>

namespace GLEAN {


static PFNGLBINDBUFFERARBPROC BindBuffer = NULL;
static PFNGLBUFFERDATAARBPROC BufferData = NULL;
static PFNGLMAPBUFFERARBPROC MapBuffer = NULL;
static PFNGLUNMAPBUFFERARBPROC UnmapBuffer = NULL;
static PFNGLGETBUFFERSUBDATAARBPROC GetBufferSubData = NULL;

const GLuint PBO1 = 42, PBO2 = 43;

const double minInterval = 1.0; // seconds


struct ImageFormat
{
   const char *Name;
   GLuint Bytes;  // per pixel
   GLenum Format;
   GLenum Type;
};


static ImageFormat Formats[] =
{
	{ "GL_RGB, GL_UNSIGNED_BYTE", 3, GL_RGB, GL_UNSIGNED_BYTE },
	{ "GL_BGR, GL_UNSIGNED_BYTE", 3, GL_BGR, GL_UNSIGNED_BYTE },
	{ "GL_RGBA, GL_UNSIGNED_BYTE", 4, GL_RGBA, GL_UNSIGNED_BYTE },
	{ "GL_BGRA, GL_UNSIGNED_BYTE", 4, GL_BGRA, GL_UNSIGNED_BYTE },
	{ "GL_ABGR, GL_UNSIGNED_BYTE", 4, GL_ABGR_EXT, GL_UNSIGNED_BYTE },
	{ "GL_RGBA, GL_UNSIGNED_INT_8_8_8_8", 4, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8 },
	{ "GL_BGRA, GL_UNSIGNED_INT_8_8_8_8", 4, GL_BGRA_EXT, GL_UNSIGNED_INT_8_8_8_8 },
	{ "GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV", 4, GL_BGRA_EXT, GL_UNSIGNED_INT_8_8_8_8_REV },
#ifdef GL_EXT_packed_depth_stencil
	{ "GL_DEPTH_STENCIL_EXT, GL_UNSIGNED_INT_24_8", 4, GL_DEPTH_STENCIL_EXT, GL_UNSIGNED_INT_24_8_EXT },
#endif
	{ "GL_DEPTH_COMPONENT, GL_FLOAT", 4, GL_DEPTH_COMPONENT, GL_FLOAT },
	{ "GL_DEPTH_COMPONENT, GL_UNSIGNED_INT", 4, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT },
	{ "GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT", 2, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT },
	{ NULL, 0, 0, 0 } // end of list marker
};


static GLenum PBOmodes[4] =
{
	GL_NONE,
#ifdef GL_ARB_pixel_buffer_object
	GL_STREAM_READ_ARB,
	GL_STATIC_READ_ARB,
	GL_DYNAMIC_READ_ARB
#endif
};

static const char *PBOmodeStrings[4] =
{
	"No PBO",
	"GL_STREAM_READ PBO",
	"GL_STATIC_READ PBO",
	"GL_DYNAMIC_READ PBO"
};


static bool
isDepthFormat(GLenum format)
{
	switch (format) {
	case GL_DEPTH_COMPONENT:
#ifdef GL_EXT_packed_depth_stencil
	case GL_DEPTH_STENCIL_EXT:
#endif
		return true;
	default:
		return false;
	}
}


static bool
isStencilFormat(GLenum format)
{
	switch (format) {
	case GL_STENCIL_INDEX:
#ifdef GL_EXT_packed_depth_stencil
	case GL_DEPTH_STENCIL_EXT:
#endif
		return true;
	default:
		return false;
	}
}


static bool
isDepthStencilFormat(GLenum format)
{
#ifdef GL_EXT_packed_depth_stencil
	if (format == GL_DEPTH_STENCIL_EXT)
		return true;
#endif
	return false;
}



// print a SubResult test description in human-readable form
void
ReadpixPerfResult::SubResult::sprint(char *s) const
{
	sprintf(s, "glReadPixels(%d x %d, %s), %s, %s, GL_READ_BUFFER=%s",
		width, height, Formats[formatNum].Name,
		PBOmodeStrings[pboMode],
		work ? "pixel sum" : "no pixel sum",
		readBuf);
}


void
ReadpixPerfResult::SubResult::print(Environment *env) const
{
	char descrip[1000], str[1000];
	sprint(descrip);
	sprintf(str, "\t%.3f Mpixels/second: %s\n", rate, descrip);
	env->log << str;
}


static void
SimpleRender()
{
   glBegin(GL_POINTS);
   glVertex2f(0, 0);
   glEnd();
}


// Exercise glReadPixels for a particular image size, format and type.
// Return read rate in megapixels / second
double
ReadpixPerfTest::runNonPBOtest(int formatNum, GLsizei width, GLsizei height,
			       GLuint *sumOut)
{
	const GLint bufferSize = width * height * Formats[formatNum].Bytes;
	GLubyte *buffer = new GLubyte [bufferSize];

	glPixelStorei(GL_PACK_ALIGNMENT, 1);

	Timer t;
	double start = t.getClock();
	double elapsedTime = 0.0;
	int iter = 0;

	do {
		iter++;
		if (sumOut) {
		   SimpleRender();
		}
		glReadPixels(0, 0, width, height,
			     Formats[formatNum].Format,
			     Formats[formatNum].Type, buffer);
		if (sumOut) {
			GLuint sum = 0;
			for (int i = 0; i < bufferSize; i++) {
				sum += buffer[i];
			}
			*sumOut = sum;
		}
		double finish = t.getClock();
		elapsedTime = finish - start;
	} while (elapsedTime < minInterval);

	delete buffer;

	double rate = width * height * iter / elapsedTime / 1000000.0;
	return rate;
}

// use glMapBufferARB or glGetBufferSubDataARB:
#define MAP_BUFFER 1

double
ReadpixPerfTest::runPBOtest(int formatNum, GLsizei width, GLsizei height,
			    GLenum bufferUsage, GLuint *sumOut)
{
#ifdef GL_ARB_pixel_buffer_object
	const GLint bufferSize = width * height * Formats[formatNum].Bytes / 2;

	glPixelStorei(GL_PACK_ALIGNMENT, 1);

	// setup PBOs
	BindBuffer(GL_PIXEL_PACK_BUFFER_ARB, PBO1);
	BufferData(GL_PIXEL_PACK_BUFFER_ARB, bufferSize, NULL, bufferUsage);
	BindBuffer(GL_PIXEL_PACK_BUFFER_ARB, PBO2);
	BufferData(GL_PIXEL_PACK_BUFFER_ARB, bufferSize, NULL, bufferUsage);

#if !MAP_BUFFER
	GLubyte *b = new GLubyte [bufferSize];
#endif

	Timer t;
	double start = t.getClock();
	double elapsedTime = 0.0;
	int iter = 0;

	do {
		iter++;
		if (sumOut) {
		   SimpleRender();
		}
		// read lower half
		BindBuffer(GL_PIXEL_PACK_BUFFER_ARB, PBO1);
		glReadPixels(0, 0, width, height / 2,
			     Formats[formatNum].Format,
			     Formats[formatNum].Type, NULL);
		// read upper half
		BindBuffer(GL_PIXEL_PACK_BUFFER_ARB, PBO2);
		glReadPixels(0, height / 2, width, height / 2,
			     Formats[formatNum].Format,
			     Formats[formatNum].Type, NULL);
		if (sumOut) {
			GLuint sum = 0;
			// sum lower half
			BindBuffer(GL_PIXEL_PACK_BUFFER_ARB, PBO1);
#if MAP_BUFFER
			GLubyte *b = (GLubyte *)
				MapBuffer(GL_PIXEL_PACK_BUFFER_ARB,
					  GL_READ_ONLY);
#else
			GetBufferSubData(GL_PIXEL_PACK_BUFFER_ARB,
					 0, bufferSize, b);
#endif
			for (int i = 0; i < bufferSize; i++) {
				sum += b[i];
			}
#if MAP_BUFFER
			UnmapBuffer(GL_PIXEL_PACK_BUFFER_ARB);
#endif

			// sum upper half
			BindBuffer(GL_PIXEL_PACK_BUFFER_ARB, PBO2);
#if MAP_BUFFER
			b = (GLubyte *) MapBuffer(GL_PIXEL_PACK_BUFFER_ARB,
						  GL_READ_ONLY);
#else
			GetBufferSubData(GL_PIXEL_PACK_BUFFER_ARB,
					 0, bufferSize, b);
#endif
			for (int i = 0; i < bufferSize; i++) {
				sum += b[i];
			}
#if MAP_BUFFER
			UnmapBuffer(GL_PIXEL_PACK_BUFFER_ARB);
#endif
			*sumOut = sum;
		}
		double finish = t.getClock();
		elapsedTime = finish - start;
	} while (elapsedTime < minInterval);

	BindBuffer(GL_PIXEL_PACK_BUFFER_ARB, 0);

#if !MAP_BUFFER
	delete b;
#endif

	double rate = width * height * iter / elapsedTime / 1000000.0;
	return rate;
#else
	return 0.0;
#endif /* GL_ARB_pixel_buffer_object */
}



// Per visual setup.
void
ReadpixPerfTest::setup(void)
{
	env->log << name << ":\n";

	glGetIntegerv(GL_DEPTH_BITS, &depthBits);
	glGetIntegerv(GL_STENCIL_BITS, &stencilBits);

	if (GLUtils::haveExtensions("GL_ARB_pixel_buffer_object")) {
		BindBuffer = (PFNGLBINDBUFFERARBPROC)
			GLUtils::getProcAddress("glBindBufferARB");
		assert(BindBuffer);
		BufferData = (PFNGLBUFFERDATAARBPROC)
			GLUtils::getProcAddress("glBufferDataARB");
		assert(BufferData);
		MapBuffer = (PFNGLMAPBUFFERARBPROC)
			GLUtils::getProcAddress("glMapBufferARB");
		assert(MapBuffer);
		UnmapBuffer = (PFNGLUNMAPBUFFERARBPROC)
			GLUtils::getProcAddress("glUnmapBufferARB");
		assert(UnmapBuffer);
		GetBufferSubData = (PFNGLGETBUFFERSUBDATAARBPROC)
			GLUtils::getProcAddress("glGetBufferSubDataARB");
		assert(GetBufferSubData);
		numPBOmodes = 4;
	}
	else {
		numPBOmodes = 1;
	}

	// Fill colorbuffer with random data
	GLubyte *buffer = new GLubyte [windowSize * windowSize * 4];
	for (int i = 0; i < windowSize * windowSize * 4; i++)
		buffer[i] = 5;
	glDrawPixels(windowSize, windowSize, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
	if (depthBits > 0) {
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_ALWAYS);
		glDrawPixels(windowSize, windowSize,
					 GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, buffer);
	}
	if (stencilBits > 0) {
		glDrawPixels(windowSize, windowSize,
					 GL_STENCIL_INDEX, GL_UNSIGNED_BYTE, buffer);
	}
	delete buffer;
}



void
ReadpixPerfTest::runOne(ReadpixPerfResult &r, Window &w)
{
	ReadpixPerfResult::SubResult res;
	(void) w;  // silence warning

	setup();
	assert(numPBOmodes > 0);

	r.pass = true;
	res.width = windowSize;
	res.height = windowSize;

	{
		GLint readBuf;
		glGetIntegerv(GL_READ_BUFFER, &readBuf);
		if (readBuf == GL_FRONT)
			res.readBuf = "GL_FRONT";
		else
			res.readBuf = "GL_BACK";
	}

	for (res.formatNum = 0; Formats[res.formatNum].Name; res.formatNum++) {

		if (isDepthFormat(Formats[res.formatNum].Format) && depthBits == 0)
			continue;
		if (isStencilFormat(Formats[res.formatNum].Format) && stencilBits == 0)
			continue;

		if (isDepthStencilFormat(Formats[res.formatNum].Format) &&
			!GLUtils::haveExtensions("GL_EXT_packed_depth_stencil"))
			continue;

		for (res.work = 0; res.work < 2; res.work++) {
			GLuint firstSum = 0;

			for (res.pboMode = 0; res.pboMode < numPBOmodes; res.pboMode++) {
				GLuint sum = 0;

				if (res.pboMode) {
					GLenum usage = PBOmodes[res.pboMode];
					res.rate = runPBOtest(res.formatNum, res.width, res.height, usage,
															res.work ? &sum : NULL);
				}
				else {
					res.rate = runNonPBOtest(res.formatNum, res.width, res.height,
																 res.work ? &sum : NULL);
				}

				res.print(env);
				r.results.push_back(res);

				// sanity check
				if (res.pboMode == 0) {
					firstSum = sum;
				}
				else if (firstSum != sum) {
					// this should never happen, probably an OpenGL bug
					char s0[1000];
					res.sprint(s0);
					env->log << name
						 << " Error: glReadPixels returned inconsistant data:\n"
						 << s0
						 << " returned "
						 << firstSum
						 << " but expected sum is "
						 << sum << "\n";
					r.pass = false;
				}
			}
		}
	}
}


void
ReadpixPerfTest::logOne(ReadpixPerfResult &r)
{
	logPassFail(r);
	logConcise(r);
}


void
ReadpixPerfTest::compareOne(ReadpixPerfResult &oldR,
			    ReadpixPerfResult &newR)
{
	const double threshold = 2.0; // percent

	comparePassFail(oldR, newR);

	if (newR.pass && oldR.pass) {
		// if both tests failed, compare/report rates
		ReadpixPerfResult::sub_iterator it_old = oldR.results.begin();
		ReadpixPerfResult::sub_iterator it_new = newR.results.begin();
		assert(oldR.results.size() == newR.results.size());
		for ( ; it_old != oldR.results.end(); ++it_old, ++it_new) {
			const ReadpixPerfResult::SubResult &oldres = *it_old;
			const ReadpixPerfResult::SubResult &newres = *it_new;

			double diff = (newres.rate - oldres.rate) / newres.rate;
			diff *= 100.0;
			if (fabs(diff) >= threshold) {
				char descrip[1000];
				newres.sprint(descrip);
				env->log << name << ": Warning: rate for '"
					 << descrip
					 << "' changed by "
					 << diff
					 << " percent (new: "
					 << newres.rate
					 << " old: "
					 << oldres.rate
					 << " MPixels/sec)\n";
			}
		}
	}
	else {
		// one test or the other failed
		env->log << "\tNew: ";
		env->log << (newR.pass ? "PASS" : "FAIL");
		env->log << "\tOld: ";
		env->log << (oldR.pass ? "PASS" : "FAIL");
	}
}


// Write vector of sub results
void
ReadpixPerfResult::putresults(ostream &s) const
{
	s << pass << '\n';
	s << results.size() << '\n';
	for (ReadpixPerfResult::sub_iterator it = results.begin();
	     it != results.end();
	     ++it) {
		const ReadpixPerfResult::SubResult &res = *it;
		s << res.rate << '\n';
		s << res.width << '\n';
		s << res.height << '\n';
		s << res.formatNum << '\n';
		s << res.pboMode << '\n';
		s << res.work << '\n';
	}
}


// Read vector of sub results
bool
ReadpixPerfResult::getresults(istream &s)
{
	int count;

	s >> pass
	  >> count;

	results.reserve(count);
	for (int i = 0; i < count; i++) {
		ReadpixPerfResult::SubResult res;
		s >> res.rate
		  >> res.width
		  >> res.height
		  >> res.formatNum
		  >> res.pboMode
		  >> res.work;
		results.push_back(res);
	}
	return s.good();
}


// The test object itself:
ReadpixPerfTest readpixperfTest("readpixPerf", "window, rgb",
				"",
	"Test the performance of glReadPixels for a variety of pixel\n"
	"formats and datatypes.\n"
	"When GL_ARB_pixel_buffer_object is supported, we also test reading\n"
	"pixels into a PBO using the three types of buffer usage modes:\n"
	"GL_STREAM_READ_ARB, GL_STATIC_READ_ARB and GL_DYNAMIC_READ_ARB.\n"
	"Furthermore, test effect of summing the value of all image bytes\n"
	"to simulate host-based image processing.\n"
	);




} // namespace GLEAN

