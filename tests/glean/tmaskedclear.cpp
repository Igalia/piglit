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

// tmaskedclear.cpp:  Test color/index masking with glClear.

#include "tmaskedclear.h"

namespace GLEAN {

void
MaskedClearTest::failRGB(BasicResult &r, GLint chan, GLfloat expected,
			 GLfloat actual, GLint buffer)
{
	static const char *chanNames[] = { "Red", "Green", "Blue", "Alpha" };
	static const char *bufferNames[] = { "GL_FRONT", "GL_BACK" };
	GLboolean mask[4];
	glGetBooleanv(GL_COLOR_WRITEMASK, mask);
	env->log << name << ":  FAIL "
		 << r.config->conciseDescription() << '\n'
		 << "\t" << chanNames[chan] << " is " << actual
		 << ", expected " << expected
		 << " in " << bufferNames[buffer] << " buffer\n";
	env->log << "\tGL_COLOR_WRITEMASK = ("
		 << (mask[0] ? "GL_TRUE" : "GL_FALSE") << ", "
		 << (mask[1] ? "GL_TRUE" : "GL_FALSE") << ", "
		 << (mask[2] ? "GL_TRUE" : "GL_FALSE") << ", "
		 << (mask[3] ? "GL_TRUE" : "GL_FALSE") << ")\n";
}

void
MaskedClearTest::failCI(BasicResult& r, GLuint expected, GLuint actual,
			GLint buffer)
{
	static const char *bufferNames[] = { "GL_FRONT", "GL_BACK" };
	GLint mask;
	glGetIntegerv(GL_INDEX_WRITEMASK, &mask);
	env->log << name << ":  FAIL "
		 << r.config->conciseDescription() << '\n'
		 << "\tcolor index is " << actual
		 << ", expected " << expected
		 << " in " << bufferNames[buffer] << " buffer\n";
	env->log << "\tGL_INDEX_WRITEMASK = " << mask << "\n";
}

void
MaskedClearTest::failZ(BasicResult& r, GLfloat expected, GLfloat actual)
{
	GLboolean mask;
	glGetBooleanv(GL_DEPTH_WRITEMASK, &mask);
	env->log << name << ":  FAIL "
		 << r.config->conciseDescription() << '\n'
		 << "\tdepth buffer value is " << actual
		 << ", expected " << expected << "\n";
	env->log << "\tGL_DEPTH_WRITEMASK = "
		 << (mask ? "GL_TRUE" : "GL_FALSE") << "\n";
}

void
MaskedClearTest::failStencil(BasicResult& r, GLuint expected, GLuint actual)
{
	GLint mask;
	glGetIntegerv(GL_STENCIL_WRITEMASK, &mask);
	env->log << name << ":  FAIL "
		 << r.config->conciseDescription() << '\n'
		 << "\tstencil buffer value is " << actual
		 << ", expected " << expected << "\n";
	env->log << "\tGL_STENCIL_WRITEMASK = " << mask << "\n";
}

///////////////////////////////////////////////////////////////////////////////
// runOne:  Run a single test case
///////////////////////////////////////////////////////////////////////////////
void
MaskedClearTest::runOne(BasicResult& r, Window&) {

	bool passed = true;

	// GL init, just to be safe
	glDisable(GL_SCISSOR_TEST);

	// only test front/back-left buffers, quad-buffer stereo in the future
	const GLint numBuffers = r.config->db ? 2 : 1;
	for (GLint buffer = 0; buffer < numBuffers && passed; buffer++) {

		if (buffer == 0) {
			glReadBuffer(GL_FRONT);
			glDrawBuffer(GL_FRONT);
		} else {
			glReadBuffer(GL_BACK);
			glDrawBuffer(GL_BACK);
		}

		if (r.config->canRGBA) {
			const GLint numChannels = (r.config->a > 0) ? 4 : 3;
			for (GLint chan = 0;
			    chan < numChannels && passed; chan++) {
				// clear to black
				glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
				glClearColor(0.0, 0.0, 0.0, 0.0);
				glClear(GL_COLOR_BUFFER_BIT);

				// select one channel to "clear" to 1.0
				glColorMask(chan == 0, chan == 1,
					    chan == 2, chan == 3);

				// try to clear surface to white
				glClearColor(1.0, 1.0, 1.0, 1.0);
				glClear(GL_COLOR_BUFFER_BIT);

				// read 1x1 image at (x,y)=(4,4)
				GLfloat pixel[4];
				glReadPixels(4, 4, 1, 1,
				    GL_RGBA, GL_FLOAT, pixel);

				// test results
				for (GLint comp = 0;
				    comp < numChannels && passed; comp++) {
					if (comp == chan) {
						// component should be 1.0
						if (pixel[comp] < 0.5) {
							passed = false;
							failRGB(r, comp, 1.0,
							  pixel[comp], buffer);
						}
					} else {
						// component should be 0.0
						if (pixel[comp] > 0.5) {
							passed = false;
							failRGB(r, comp, 0.0,
							  pixel[comp], buffer);
						}
					}
				}
			}
		}
		else {
			const GLint indexBits = r.config->bufSize;
			// We just run <indexBits> tests rather than 2^indexBits
			for (GLint bit = 0; bit < indexBits && passed; bit++) {
				// clear to 0
				glIndexMask(~0);
				glClearIndex(0);
				glClear(GL_COLOR_BUFFER_BIT);

				// select one bit to "clear" to 1
				glIndexMask(1 << bit);

				// try to clear surface to ~0
				glClearIndex(~0);
				glClear(GL_COLOR_BUFFER_BIT);

				// read 1x1 image at (x,y)=(4,4)
				GLuint pixel;
				glReadPixels(4, 4, 1, 1,
				    GL_COLOR_INDEX, GL_UNSIGNED_INT, &pixel);

				// test results
				if (pixel != (1U << bit)) {
					passed = false;
					failCI(r, 1 << bit, pixel, buffer);
				}
			}
		}
	}

	if (passed && r.config->z > 0) {
		// clear depth buffer to zero
		glDepthMask(GL_TRUE);
		glClearDepth(0.0);
		glClear(GL_DEPTH_BUFFER_BIT);

		// disable Z writes, try to clear to one
		glDepthMask(GL_FALSE);
		glClearDepth(1.0);
		glClear(GL_DEPTH_BUFFER_BIT);

		// read 1x1 image at (x,y)=(4,4);
		GLfloat depth;
		glReadPixels(4, 4, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);

		// test result
		if (depth != 0.0) {
			passed = false;
			failZ(r, 0.0, depth);
		}
	}

	if (passed && r.config->s > 0) {
		const GLint stencilBits = r.config->s;
		// We just run <stencilBits> tests rather than 2^stencilBits
		for (GLint bit = 0; bit < stencilBits && passed; bit++) {
			// clear to 0
			glStencilMask(~0);
			glClearStencil(0);
			glClear(GL_STENCIL_BUFFER_BIT);

			// select one bit to "clear" to 1
			glStencilMask(1 << bit);

			// try to clear stencil buffer to ~0
			glClearStencil(~0);
			glClear(GL_STENCIL_BUFFER_BIT);

			// read 1x1 image at (x,y)=(4,4)
			GLuint stencil;
			glReadPixels(4, 4, 1, 1,
			    GL_STENCIL_INDEX, GL_UNSIGNED_INT, &stencil);

			// test results
			if (stencil != (1U << bit)) {
				passed = false;
				failStencil(r, 1 << bit, stencil);
			}
		}
	}
	r.pass = passed;
} // MaskedClearTest::runOne


///////////////////////////////////////////////////////////////////////////////
// logOne:  Log a single test case
///////////////////////////////////////////////////////////////////////////////
void
MaskedClearTest::logOne(BasicResult& r) {
	if (r.pass) {
		logPassFail(r);
		logConcise(r);
	}
} // MaskedClearTest::logOne

///////////////////////////////////////////////////////////////////////////////
// The test object itself:
///////////////////////////////////////////////////////////////////////////////
MaskedClearTest maskedClearTest("maskedClear", "window",
	"This test checks that glClear works correctly with glColorMask,\n"
	"glIndexMask, glDepthMask and glStencilMask.\n");


} // namespace GLEAN
