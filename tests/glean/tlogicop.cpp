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

// tlogicop.cpp:  Test RGBA logic op functions.
// Based on Allen's blendFunc test.
// Brian Paul  10 May 2001

#include <stdlib.h>
#include <cmath>
#include "tlogicop.h"
#include "rand.h"
#include "image.h"

namespace {

struct logicopNameMapping {GLenum op; const char* name;};
logicopNameMapping logicopNames[] = {
	{GL_CLEAR,		"GL_CLEAR"},
	{GL_SET,		"GL_SET"},
	{GL_COPY,		"GL_COPY"},
	{GL_COPY_INVERTED,	"GL_COPY_INVERTED"},
	{GL_NOOP,		"GL_NOOP"},
	{GL_INVERT,		"GL_INVERT"},
	{GL_AND,		"GL_AND"},
	{GL_NAND,		"GL_NAND"},
	{GL_OR,			"GL_OR"},
	{GL_NOR,		"GL_NOR"},
	{GL_XOR,		"GL_XOR"},
	{GL_EQUIV,		"GL_EQUIV"},
	{GL_AND_REVERSE,	"GL_AND_REVERSE"},
	{GL_AND_INVERTED,	"GL_AND_INVERTED"},
	{GL_OR_REVERSE,		"GL_OR_REVERSE"},
	{GL_OR_INVERTED,	"GL_OR_INVERTED"}
};

const char*
logicopToName(GLenum op) {
	for (unsigned int i = 0;
	    i < sizeof(logicopNames) / sizeof(logicopNames[0]); ++i) {
		if (logicopNames[i].op == op)
			return logicopNames[i].name;
	}
	return 0;
} // logicopToName

GLenum
nameToLogicop(string& name) {
	for (unsigned int i = 0;
	    i < sizeof(logicopNames) / sizeof(logicopNames[0]); ++i) {
		if (logicopNames[i].name == name)
			return logicopNames[i].op;
	}
	return GL_ZERO;
} // nameToLogicop

void
makeRGBA(GLEAN::RandomBits& rRand,
    GLEAN::RandomBits& gRand,
    GLEAN::RandomBits& bRand,
    GLEAN::RandomBits& aRand,
    GLubyte* rgba) {
	rgba[0] = rRand.next() & 0xff;
	rgba[1] = gRand.next() & 0xff;
	rgba[2] = bRand.next() & 0xff;
	rgba[3] = aRand.next() & 0xff;
} // makeRGBA

void 
drawQuad(const int x, const int y, const GLubyte* color) {
	glColor4ubv(color);
	glBegin(GL_QUADS);
		glVertex2i(x, y);
		glVertex2i(x + 1, y);
		glVertex2i(x + 1, y + 1);
		glVertex2i(x, y + 1);
	glEnd();
} // drawQuad

void
applyLogicop(GLenum logicop, GLubyte dst[4], const GLubyte src[4]) {

	switch (logicop) {
	case GL_CLEAR:
		dst[0] = dst[1] = dst[2] = dst[3] = 0;
		break;
	case GL_SET:
		dst[0] = dst[1] = dst[2] = dst[3] = ~0;
		break;
	case GL_COPY:
		dst[0] = src[0];
		dst[1] = src[1];
		dst[2] = src[2];
		dst[3] = src[3];
		break;
	case GL_COPY_INVERTED:
		dst[0] = ~src[0];
		dst[1] = ~src[1];
		dst[2] = ~src[2];
		dst[3] = ~src[3];
		break;
	case GL_NOOP:
		break;
	case GL_INVERT:
		dst[0] = ~dst[0];
		dst[1] = ~dst[1];
		dst[2] = ~dst[2];
		dst[3] = ~dst[3];
		break;
	case GL_AND:
		dst[0] = src[0] & dst[0];
		dst[1] = src[1] & dst[1];
		dst[2] = src[2] & dst[2];
		dst[3] = src[3] & dst[3];
		break;
	case GL_NAND:
		dst[0] = ~(src[0] & dst[0]);
		dst[1] = ~(src[1] & dst[1]);
		dst[2] = ~(src[2] & dst[2]);
		dst[3] = ~(src[3] & dst[3]);
		break;
	case GL_OR:
		dst[0] = src[0] | dst[0];
		dst[1] = src[1] | dst[1];
		dst[2] = src[2] | dst[2];
		dst[3] = src[3] | dst[3];
		break;
	case GL_NOR:
		dst[0] = ~(src[0] | dst[0]);
		dst[1] = ~(src[1] | dst[1]);
		dst[2] = ~(src[2] | dst[2]);
		dst[3] = ~(src[3] | dst[3]);
		break;
	case GL_XOR:
		dst[0] = src[0] ^ dst[0];
		dst[1] = src[1] ^ dst[1];
		dst[2] = src[2] ^ dst[2];
		dst[3] = src[3] ^ dst[3];
		break;
	case GL_EQUIV:
		dst[0] = ~(src[0] ^ dst[0]);
		dst[1] = ~(src[1] ^ dst[1]);
		dst[2] = ~(src[2] ^ dst[2]);
		dst[3] = ~(src[3] ^ dst[3]);
		break;
	case GL_AND_REVERSE:
		dst[0] = src[0] & ~dst[0];
		dst[1] = src[1] & ~dst[1];
		dst[2] = src[2] & ~dst[2];
		dst[3] = src[3] & ~dst[3];
		break;
	case GL_AND_INVERTED:
		dst[0] = ~src[0] & dst[0];
		dst[1] = ~src[1] & dst[1];
		dst[2] = ~src[2] & dst[2];
		dst[3] = ~src[3] & dst[3];
		break;
	case GL_OR_REVERSE:
		dst[0] = src[0] | ~dst[0];
		dst[1] = src[1] | ~dst[1];
		dst[2] = src[2] | ~dst[2];
		dst[3] = src[3] | ~dst[3];
		break;
	case GL_OR_INVERTED:
		dst[0] = ~src[0] | dst[0];
		dst[1] = ~src[1] | dst[1];
		dst[2] = ~src[2] | dst[2];
		dst[3] = ~src[3] | dst[3];
		break;
	default:
		abort();  // implementation error
	}
} // applyLogicop

// return number of bits set differenty in a and b.
static int bitDifference(GLbyte a, GLubyte b) {
	int count = 0;
	for (int i = 0; i < 8; i++) {
		GLubyte mask = 1 << i;
		if ((a & mask) != (b & mask))
			count++;
	}
	return count;
}

static GLubyte redMask, greenMask, blueMask, alphaMask;

static void
computeError(const GLubyte aPix[4], const GLubyte ePix[4],
		int &er, int &eg, int &eb, int &ea) {
	if ((aPix[0] & redMask  ) == (ePix[0] & redMask  ) &&
	    (aPix[1] & greenMask) == (ePix[1] & greenMask) &&
	    (aPix[2] & blueMask ) == (ePix[2] & blueMask ) &&
	    (aPix[3] & alphaMask) == (ePix[3] & alphaMask)) {
		er = eg = eb = ea = 0;  // no error at all
	}
	else {
		// count up total bit difference
		er = bitDifference(aPix[0] & redMask,   ePix[0] & redMask);
		eg = bitDifference(aPix[1] & greenMask, ePix[1] & greenMask);
		eb = bitDifference(aPix[2] & blueMask,  ePix[2] & blueMask);
		ea = bitDifference(aPix[3] & alphaMask, ePix[3] & alphaMask);
	}
}

struct runResult {float readbackErrorBits; float logicopErrorBits;};

static runResult
runTest(GLenum logicop,
    GLEAN::DrawingSurfaceConfig& config, GLEAN::Environment& env) {
	using namespace GLEAN;
	
	runResult result;
	int y;

	// Compute error bitmasks depending on color channel sizes
	redMask   = ((1 << config.r) - 1) << (8 - config.r);
	greenMask = ((1 << config.g) - 1) << (8 - config.g);
	blueMask  = ((1 << config.b) - 1) << (8 - config.b);
	alphaMask = ((1 << config.a) - 1) << (8 - config.a);

	glDisable(GL_DITHER);
	glClear(GL_COLOR_BUFFER_BIT);

	Image dst(drawingSize, drawingSize, GL_RGBA, GL_UNSIGNED_BYTE);
	RandomBits rRand(config.r, 6021023);
	RandomBits gRand(config.g, 1137);
	RandomBits bRand(config.b, 1138);
	RandomBits aRand(config.a, 6);

	// Fill the framebuffer with random RGBA values, and place a copy
	// in ``dst'':
	glDisable(GL_COLOR_LOGIC_OP);
	char* dRow = dst.pixels();
	for (y = 0; y < drawingSize; ++y) {
		GLubyte* pix = reinterpret_cast<GLubyte*>(dRow);
		for (int x = 0; x < drawingSize; ++x) {
			GLubyte rgba[4];
			makeRGBA(rRand, gRand, bRand, aRand, rgba);
			drawQuad(x + 1, y + 1, rgba);
			pix[0] = rgba[0];
			pix[1] = rgba[1];
			pix[2] = rgba[2];
			pix[3] = rgba[3];
			pix += 4;
		}
		dRow += dst.rowSizeInBytes();
	}

	// Read back the contents of the framebuffer, and measure any
	// difference from what was actually written.  We can't tell
	// whether errors occurred when writing or when reading back,
	// but at least we can report anything unusual.
	Image fbDst(drawingSize, drawingSize, GL_RGBA, GL_UNSIGNED_BYTE);
	fbDst.read(1, 1);
	Image::Registration reg1(fbDst.reg(dst));
	result.readbackErrorBits =
		max(ErrorBits(reg1.stats[0].max(), config.r),
		max(ErrorBits(reg1.stats[1].max(), config.g),
		max(ErrorBits(reg1.stats[2].max(), config.b),
		    ErrorBits(reg1.stats[3].max(), config.a))));

	// Now generate random source pixels and apply the logicop
	// operation to both the framebuffer and a copy in the image
	// ``expected''.  Save the source pixels in the image ``src''
	// so we can diagnose any problems we find later.
	Image expected(fbDst);
	Image src(drawingSize, drawingSize, GL_RGBA, GL_UNSIGNED_BYTE);

	glLogicOp(logicop);
	glEnable(GL_COLOR_LOGIC_OP);

	dRow = expected.pixels();
	char* sRow = src.pixels();
	for (y = 0; y < drawingSize; ++y) {
		GLubyte* pix = reinterpret_cast<GLubyte*>(dRow);
		GLubyte* sPix = reinterpret_cast<GLubyte*>(sRow);
		for (int x = 0; x < drawingSize; ++x) {
			GLubyte rgba[4];
			makeRGBA(rRand, gRand, bRand, aRand, rgba);
			sPix[0] = rgba[0];
			sPix[1] = rgba[1];
			sPix[2] = rgba[2];
			sPix[3] = rgba[3];
			drawQuad(x + 1, y + 1, rgba);
			applyLogicop(logicop, pix, rgba);
			pix += 4;
			sPix += 4;
		}
		dRow += expected.rowSizeInBytes();
		sRow += src.rowSizeInBytes();
	}

	// Read the generated image (``actual'') and compare it to the
	// computed image (``expected'') to see if any pixels are
	// outside the expected tolerance range (one LSB).  If so,
	// report the first such pixel, along with the source and
	// destination values that generated it.  Keep track of the
	// maximum error encountered.
	Image actual(drawingSize, drawingSize, GL_RGBA, GL_UNSIGNED_BYTE);
	actual.read(1, 1);
	result.logicopErrorBits = 0.0;
	sRow = actual.pixels();
	dRow = expected.pixels();
	for (y = 0; y < drawingSize; ++y) {
		GLubyte* aPix = reinterpret_cast<GLubyte*>(sRow);
		GLubyte* ePix = reinterpret_cast<GLubyte*>(dRow);
		for (int x = 0; x < drawingSize; ++x) {
			int rErr, gErr, bErr, aErr;
			computeError(aPix, ePix, rErr, gErr, bErr, aErr);
			result.logicopErrorBits = rErr + gErr + bErr + aErr;

			if (result.logicopErrorBits > 1.0) {
				if (env.options.verbosity) {
GLubyte* sPix = reinterpret_cast<GLubyte*>(src.pixels()
	+ y * src.rowSizeInBytes() + x * 4 * sizeof(GLubyte));
GLubyte* dPix = reinterpret_cast<GLubyte*>(dst.pixels()
	+ y * dst.rowSizeInBytes() + x * 4 * sizeof(GLubyte));
env.log << '\n'
<< "First failing pixel is at row " << y << " column " << x << "\n"
<< "Actual values are (" << (int) aPix[0] << ", " << (int) aPix[1] << ", "
	<< (int) aPix[2] << ", " << (int) aPix[3] << ")\n"
<< "Expected values are (" << (int) ePix[0] << ", " << (int) ePix[1] << ", "
	<< (int) ePix[2] << ", " << (int) ePix[3] << ")\n"
<< "Errors (number of bad bits) are (" << rErr << ", " << gErr << ", "
	 << bErr << ", " << aErr << ")\n"
<< "Source values are (" << (int) sPix[0] << ", " << (int) sPix[1] << ", "
	<< (int) sPix[2] << ", " << (int) sPix[3] << ")\n"
<< "Destination values are (" << (int) dPix[0] << ", " << (int) dPix[1] << ", "
	<< (int) dPix[2] << ", " << (int) dPix[3] << ")\n";
				}
				return result;
			}
			aPix += 4;
			ePix += 4;
		}
		sRow += actual.rowSizeInBytes();
		dRow += expected.rowSizeInBytes();
	}

	return result;
} // runOneSet

} // anonymous namespace

namespace GLEAN {

///////////////////////////////////////////////////////////////////////////////
// runOne:  Run a single test case
///////////////////////////////////////////////////////////////////////////////
void
LogicopFuncTest::runOne(LogicopFuncResult& r, Window& w) {
	GLUtils::useScreenCoords(drawingSize + 2, drawingSize + 2);

	static GLenum logicopModes[] = {
		GL_CLEAR,
		GL_SET,
		GL_COPY,
		GL_COPY_INVERTED,
		GL_NOOP,
		GL_INVERT,
		GL_AND,
		GL_NAND,
		GL_OR,
		GL_NOR,
		GL_XOR,
		GL_EQUIV,
		GL_AND_REVERSE,
		GL_AND_INVERTED,
		GL_OR_REVERSE,
		GL_OR_INVERTED
	};

	bool allPassed = true;
	for (unsigned int op = 0;
		op < sizeof(logicopModes)/sizeof(logicopModes[0]); ++op) {

		LogicopFuncResult::PartialResult p;
		p.logicop = logicopModes[op];

		runResult res = runTest(p.logicop, *(r.config), *env);
		w.swap();

		p.rbErr = res.readbackErrorBits;
		p.opErr = res.logicopErrorBits;
		r.results.push_back(p);

		if (p.rbErr > 1.0 || p.opErr > 1.0) {
			env->log << name << ":  FAIL "
				<< r.config->conciseDescription()<< '\n'
				<< "\tlogicop mode = "
				<< logicopToName(p.logicop)
				<< "\n\tReadback had " << p.rbErr
				<< " bits in error; logicop had "
				<< p.opErr << " bits in error.\n";
			allPassed = false;
		}
	}

	r.pass = allPassed;
} // LogicopFuncTest::runOne

///////////////////////////////////////////////////////////////////////////////
// logOne:  Log a single test case
///////////////////////////////////////////////////////////////////////////////
void
LogicopFuncTest::logOne(LogicopFuncResult& r) {
	if (r.pass) {
		logPassFail(r);
		logConcise(r);
	}
}


///////////////////////////////////////////////////////////////////////////////
// compareOne:  Compare results for a single test case
///////////////////////////////////////////////////////////////////////////////
void
LogicopFuncTest::compareOne(LogicopFuncResult& oldR, LogicopFuncResult& newR) {
	BasicStats readbackStats;
	BasicStats logicopStats;

	vector<LogicopFuncResult::PartialResult>::const_iterator np;
	vector<LogicopFuncResult::PartialResult>::const_iterator op;

	for (np = newR.results.begin(); np != newR.results.end(); ++np) {
		// Find the matching case, if any, in the old results:
		for (op = oldR.results.begin(); op != oldR.results.end(); ++op)
			if (np->logicop == op->logicop) {
				readbackStats.sample(np->rbErr - op->rbErr);
				logicopStats.sample(np->opErr - op->opErr);
			}
	}

	if (readbackStats.n() == static_cast<int>(newR.results.size())
	 && newR.results.size() == oldR.results.size()
	 && readbackStats.mean() == 0.0 && logicopStats.mean() == 0.0) {
		if (env->options.verbosity)
			env->log << name << ": SAME "
				<< newR.config->conciseDescription() << '\n';
	} else {
		env->log << name << ": DIFF "
			<< newR.config->conciseDescription() << '\n';

		if (readbackStats.mean() < 0.0)
			env->log << '\t' << env->options.db2Name
				<< " appears to have more accurate readback.\n";
		else if (readbackStats.mean() > 0.0)
			env->log << '\t' << env->options.db1Name
				<< " appears to have more accurate readback.\n";
		if (logicopStats.mean() < 0.0)
			env->log << '\t' << env->options.db2Name
				<< " appears to have more accurate logicoping.\n";
		else if (logicopStats.mean() > 0.0)
			env->log << '\t' << env->options.db1Name
				<< " appears to have more accurate logicoping.\n";
		if (readbackStats.n() != static_cast<int>(newR.results.size())){
			env->log << "\tThe following cases in "
				<< env->options.db2Name
				<< " have no matching test in "
				<< env->options.db1Name
				<< ":\n";
			for (np = newR.results.begin();
			    np != newR.results.end(); ++np) {
				for (op = oldR.results.begin();
				    op != oldR.results.end(); ++op)
					if (np->logicop == op->logicop)
						break;
				if (op == oldR.results.end())
					env->log << "\t\t"
						<< logicopToName(np->logicop)
						<< '\n';
			}
		}
		if (readbackStats.n() != static_cast<int>(oldR.results.size())){
			env->log << "\tThe following cases in "
				<< env->options.db1Name
				<< " have no matching test in "
				<< env->options.db2Name
				<< ":\n";
			for (op = oldR.results.begin();
			    op != oldR.results.end(); ++op) {
				for (np = newR.results.begin();
				    np != newR.results.end(); ++np)
					if (op->logicop == np->logicop)
						break;
				if (np == newR.results.end())
					env->log << "\t\t"
						<< logicopToName(op->logicop)
						<< '\n';
			}
		}
		if (env->options.verbosity) {
			env->log << "\tThe following cases appear in both "
				<< env->options.db1Name
				<< " and "
				<< env->options.db2Name
				<< ":\n";
			for (np = newR.results.begin();
			    np != newR.results.end(); ++np){
				for (op = oldR.results.begin();
				    op != oldR.results.end(); ++op)
					if (np->logicop == op->logicop)
						break;
				if (op != oldR.results.end())
					env->log << "\t\t"
						<< logicopToName(np->logicop)
						<< '\n';
			}
		}
	}
} // LogicopFuncTest::compareOne

///////////////////////////////////////////////////////////////////////////////
// Result I/O functions:
///////////////////////////////////////////////////////////////////////////////
void
LogicopFuncResult::putresults(ostream& s) const {
	s << results.size() << '\n';
	for (vector<PartialResult>::const_iterator p = results.begin();
	     p != results.end(); ++p) {
		s << logicopToName(p->logicop) << ' '
		  << p->rbErr << ' ' << p->opErr << '\n';
	}
} // LogicopFuncResult::put

bool
LogicopFuncResult::getresults(istream& s) {
	int n;
	s >> n;
	for (int i = 0; i < n; ++i) {
		PartialResult p;
		string src;
		s >> src >> p.rbErr >> p.opErr;
		p.logicop = nameToLogicop(src);
		results.push_back(p);
	}

	return s.good();
} // LogicopFuncResult::get

///////////////////////////////////////////////////////////////////////////////
// The test object itself:
///////////////////////////////////////////////////////////////////////////////
LogicopFuncTest logicopFuncTest("logicOp", "window, rgb",

	"This test checks the logicop functions in RGBA mode.\n");


} // namespace GLEAN
