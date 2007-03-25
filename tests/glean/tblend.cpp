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

// tblend.cpp:  Test blending functions.

#include "tblend.h"
#include "rand.h"
#include "image.h"
#include <cmath>

namespace {

struct factorNameMapping {GLenum factor; char* name;};
factorNameMapping factorNames[] = {
	{GL_DST_ALPHA,			"GL_DST_ALPHA"},
	{GL_DST_COLOR,			"GL_DST_COLOR"},
	{GL_ONE,			"GL_ONE"},
	{GL_ONE_MINUS_DST_ALPHA,	"GL_ONE_MINUS_DST_ALPHA"},
	{GL_ONE_MINUS_DST_COLOR,	"GL_ONE_MINUS_DST_COLOR"},
	{GL_ONE_MINUS_SRC_ALPHA,	"GL_ONE_MINUS_SRC_ALPHA"},
	{GL_ONE_MINUS_SRC_COLOR,	"GL_ONE_MINUS_SRC_COLOR"},
	{GL_SRC_ALPHA,			"GL_SRC_ALPHA"},
	{GL_SRC_ALPHA_SATURATE,		"GL_SRC_ALPHA_SATURATE"},
	{GL_SRC_COLOR,			"GL_SRC_COLOR"},
	{GL_ZERO,			"GL_ZERO"}
};

char*
factorToName(GLenum factor) {
	for (unsigned int i = 0;
	    i < sizeof(factorNames) / sizeof(factorNames[0]);
	    ++i)
		if (factorNames[i].factor == factor)
			return factorNames[i].name;
	return 0;
} // factorToName

GLenum
nameToFactor(string& name) {
	for (unsigned int i = 0;
	    i < sizeof(factorNames) / sizeof(factorNames[0]);
	    ++i)
		if (factorNames[i].name == name)
			return factorNames[i].factor;
	return GL_ZERO;
} // nameToFactor

bool
needsDstAlpha(const GLenum func) {
	return func == GL_DST_ALPHA || func == GL_ONE_MINUS_DST_ALPHA
		|| func == GL_SRC_ALPHA_SATURATE;
}

void
makeRGBA(GLEAN::RandomBitsDouble& rRand,
    GLEAN::RandomBitsDouble& gRand,
    GLEAN::RandomBitsDouble& bRand,
    GLEAN::RandomBitsDouble& aRand,
    float* rgba) {
	rgba[0] = rRand.next();
	rgba[1] = gRand.next();
	rgba[2] = bRand.next();
	rgba[3] = aRand.next();
} // makeRGBA

void
drawQuad(const int x, const int y, const float* color) {
	glColor4fv(color);
	glBegin(GL_QUADS);
		glVertex2i(x, y);
		glVertex2i(x + 1, y);
		glVertex2i(x + 1, y + 1);
		glVertex2i(x, y + 1);
	glEnd();
} // drawQuad

inline float
clamp(float f) {
	if (f < 0.0)
		return 0.0;
	else if (f > 1.0)
		return 1.0;
	else
		return f;
} // clamp

void
applyBlend(GLenum srcFactor, GLenum dstFactor, float* dst, float* src) {
	// XXX Currently we don't test any of the const-color blend factors.
	// It would be a good idea to do so as soon as we have access to an
	// implementation that supports the OpenGL 1.2 imaging extensions.

	float sf[4];
	switch (srcFactor) {
	case GL_ZERO:
		sf[0] = sf[1] = sf[2] = sf[3] = 0.0;
		break;
	case GL_ONE:
		sf[0] = sf[1] = sf[2] = sf[3] = 1.0;
		break;
	case GL_DST_COLOR:
		sf[0] = dst[0]; sf[1] = dst[1]; sf[2] = dst[2]; sf[3] = dst[3];
		break;
	case GL_ONE_MINUS_DST_COLOR:
		sf[0] = 1.0 - dst[0]; sf[1] = 1.0 - dst[1];
		sf[2] = 1.0 - dst[2]; sf[3] = 1.0 - dst[3];
		break;
	case GL_SRC_ALPHA:
		sf[0] = sf[1] = sf[2] = sf[3] = src[3];
		break;
	case GL_ONE_MINUS_SRC_ALPHA:
		sf[0] = sf[1] = sf[2] = sf[3] = 1.0 - src[3];
		break;
	case GL_DST_ALPHA:
		sf[0] = sf[1] = sf[2] = sf[3] = dst[3];
		break;
	case GL_ONE_MINUS_DST_ALPHA:
		sf[0] = sf[1] = sf[2] = sf[3] = 1.0 - dst[3];
		break;
	case GL_SRC_ALPHA_SATURATE: {
		float f = 1.0 - dst[3];
		if (src[3] < f)
			f = src[3];
		sf[0] = sf[1] = sf[2] = f; sf[3] = 1.0;
		}
		break;
	default:
		sf[0] = sf[1] = sf[2] = sf[3] = 0.0;
		break;
	}

	float df[4];
	switch (dstFactor) {
	case GL_ZERO:
		df[0] = df[1] = df[2] = df[3] = 0.0;
		break;
	case GL_ONE:
		df[0] = df[1] = df[2] = df[3] = 1.0;
		break;
	case GL_SRC_COLOR:
		df[0] = src[0]; df[1] = src[1]; df[2] = src[2]; df[3] = src[3];
		break;
	case GL_ONE_MINUS_SRC_COLOR:
		df[0] = 1.0 - src[0]; df[1] = 1.0 - src[1];
		df[2] = 1.0 - src[2]; df[3] = 1.0 - src[3];
		break;
	case GL_SRC_ALPHA:
		df[0] = df[1] = df[2] = df[3] = src[3];
		break;
	case GL_ONE_MINUS_SRC_ALPHA:
		df[0] = df[1] = df[2] = df[3] = 1.0 - src[3];
		break;
	case GL_DST_ALPHA:
		df[0] = df[1] = df[2] = df[3] = dst[3];
		break;
	case GL_ONE_MINUS_DST_ALPHA:
		df[0] = df[1] = df[2] = df[3] = 1.0 - dst[3];
		break;
	default:
		df[0] = df[1] = df[2] = df[3] = 0.0;
		break;
	}

	dst[0] = clamp(src[0] * sf[0] + dst[0] * df[0]);
	dst[1] = clamp(src[1] * sf[1] + dst[1] * df[1]);
	dst[2] = clamp(src[2] * sf[2] + dst[2] * df[2]);
	dst[3] = clamp(src[3] * sf[3] + dst[3] * df[3]);
} // applyBlend

struct runFactorsResult {
	float readbackErrorBits;
	float blendRGBErrorBits;
	float blendAlphaErrorBits;
};

runFactorsResult
runFactors(GLenum srcFactor, GLenum dstFactor,
    GLEAN::DrawingSurfaceConfig& config, GLEAN::Environment& env,
    float rgbTolerance, float alphaTolerance) {
	using namespace GLEAN;

	runFactorsResult result;
	int y;

	glDisable(GL_DITHER);
	glClear(GL_COLOR_BUFFER_BIT);

	Image dst(drawingSize, drawingSize, GL_RGBA, GL_FLOAT);
	RandomBitsDouble rRand(config.r, 6021023);
	RandomBitsDouble gRand(config.g, 1137);
	RandomBitsDouble bRand(config.b, 1138);
	RandomBitsDouble dstARand(config.a? config.a: 1, 6);

	// Fill the framebuffer with random RGBA values, and place a copy
	// in ``dst'':
	glDisable(GL_BLEND);
	char* dRow = dst.pixels();
	for (/*int */y = 0; y < drawingSize; ++y) {
		float* pix = reinterpret_cast<float*>(dRow);
		for (int x = 0; x < drawingSize; ++x) {
			float rgba[4];
			makeRGBA(rRand, gRand, bRand, dstARand, rgba);
			if (!config.a)
				rgba[3] = 1.0;
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
	Image fbDst(drawingSize, drawingSize, GL_RGBA, GL_FLOAT);
	fbDst.read(1, 1);
	Image::Registration reg1(fbDst.reg(dst));
	result.readbackErrorBits =
		max(ErrorBits(reg1.stats[0].max(), config.r),
		max(ErrorBits(reg1.stats[1].max(), config.g),
		max(ErrorBits(reg1.stats[2].max(), config.b),
		    ErrorBits(reg1.stats[3].max(), config.a))));

	// Now generate random source pixels and apply the blending
	// operation to both the framebuffer and a copy in the image
	// ``expected''.  Note that a fresh source alpha must be
	// generated here, because the range of source alpha values is
	// not limited by the range of alpha values that can be
	// represented in the framebuffer.  Save the source pixels in
	// the image ``src'' so we can diagnose any problems we find
	// later.
	Image expected(dst);
	Image src(drawingSize, drawingSize, GL_RGBA, GL_FLOAT);
	RandomBitsDouble srcARand(16, 42);

	glBlendFunc(srcFactor, dstFactor);
	glEnable(GL_BLEND);

	dRow = expected.pixels();
	char* sRow = src.pixels();
	for (/*int */y = 0; y < drawingSize; ++y) {
		float* pix = reinterpret_cast<float*>(dRow);
		float* sPix = reinterpret_cast<float*>(sRow);
		for (int x = 0; x < drawingSize; ++x) {
			float rgba[4];
			makeRGBA(rRand, gRand, bRand, srcARand, rgba);
			sPix[0] = rgba[0];
			sPix[1] = rgba[1];
			sPix[2] = rgba[2];
			sPix[3] = rgba[3];
			drawQuad(x + 1, y + 1, rgba);
			applyBlend(srcFactor, dstFactor, pix, rgba);
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
	Image actual(drawingSize, drawingSize, GL_RGBA, GL_FLOAT);
	actual.read(1, 1);
	result.blendRGBErrorBits = 0.0;
	result.blendAlphaErrorBits = 0.0;
	sRow = actual.pixels();
	dRow = expected.pixels();
	for (/*int */y = 0; y < drawingSize; ++y) {
		float* aPix = reinterpret_cast<float*>(sRow);
		float* ePix = reinterpret_cast<float*>(dRow);
		for (int x = 0; x < drawingSize; ++x) {
			float rError = fabs(aPix[0] - ePix[0]);
			float gError = fabs(aPix[1] - ePix[1]);
			float bError = fabs(aPix[2] - ePix[2]);
			float aError = fabs(aPix[3] - ePix[3]);
			result.blendRGBErrorBits =
				max(static_cast<double>(result.blendRGBErrorBits),
				max(ErrorBits(rError, config.r),
				max(ErrorBits(gError, config.g),
				    ErrorBits(bError, config.b))));
			result.blendAlphaErrorBits =
			    max(static_cast<double>(result.blendAlphaErrorBits),
				    ErrorBits(aError, config.a));
			if (result.blendRGBErrorBits > rgbTolerance || result.blendAlphaErrorBits > alphaTolerance) {
				if (env.options.verbosity) {
float* sPix = reinterpret_cast<float*>(src.pixels()
	+ y * src.rowSizeInBytes() + x * 4 * sizeof(float));
float* dPix = reinterpret_cast<float*>(dst.pixels()
	+ y * dst.rowSizeInBytes() + x * 4 * sizeof(float));
env.log << '\n'
<< "First failing pixel is at row " << y << " column " << x << "\n"
<< "Actual values are (" << aPix[0] << ", " << aPix[1] << ", " << aPix[2]
	<< ", " << aPix[3] << ")\n"
<< "Expected values are (" << ePix[0] << ", " << ePix[1] << ", " << ePix[2]
	<< ", " << ePix[3] << ")\n"
<< "Errors are (" << rError << ", " << gError << ", " << bError << ", "
        << aError << ")\n"
<< "Source values are (" << sPix[0] << ", " << sPix[1] << ", " << sPix[2]
	<< ", " << sPix[3] << ")\n"
<< "Destination values are (" << dPix[0] << ", " << dPix[1] << ", " << dPix[2]
	<< ", " << dPix[3] << ")\n";
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
BlendFuncTest::runOne(BlendFuncResult& r, Window& w) {
	GLUtils::useScreenCoords(drawingSize + 2, drawingSize + 2);

	static GLenum srcFactors[] = {
		GL_ZERO,
		GL_ONE,
		GL_DST_COLOR,
		GL_ONE_MINUS_DST_COLOR,
		GL_SRC_ALPHA,
		GL_ONE_MINUS_SRC_ALPHA,
		GL_DST_ALPHA,
		GL_ONE_MINUS_DST_ALPHA,
		GL_SRC_ALPHA_SATURATE
	};
	static GLenum dstFactors[] = {
		GL_ZERO,
		GL_ONE,
		GL_SRC_COLOR,
		GL_ONE_MINUS_SRC_COLOR,
		GL_SRC_ALPHA,
		GL_ONE_MINUS_SRC_ALPHA,
		GL_DST_ALPHA,
		GL_ONE_MINUS_DST_ALPHA
	};

	// Hack: Make driver tests on incorrect hardware feasible
	// by adjusting the error tolerance to whatever the hardware can do
	float rgbTolerance = 1.0;
	float alphaTolerance = 1.0;
	const char* s;

	s = getenv("GLEAN_BLEND_RGB_TOLERANCE");
	if (s) {
		rgbTolerance = atof(s);
		env->log << "Note: RGB tolerance adjusted to " << rgbTolerance << "\n";
	}
	s = getenv("GLEAN_BLEND_ALPHA_TOLERANCE");
	if (s) {
		alphaTolerance = atof(s);
		env->log << "Note: Alpha tolerance adjusted to " << alphaTolerance << "\n";
	}

	bool allPassed = true;
	for (unsigned int sf = 0; sf < sizeof(srcFactors)/sizeof(srcFactors[0]);
	    ++sf)

		for (unsigned int df = 0;
		    df < sizeof(dstFactors)/sizeof(dstFactors[0]); ++df) {

			BlendFuncResult::PartialResult p;
			p.src = srcFactors[sf];
			p.dst = dstFactors[df];

			if ((needsDstAlpha(p.src) || needsDstAlpha(p.dst))
			    && (r.config->a == 0))
				continue;

			runFactorsResult res(runFactors(p.src, p.dst,
				*(r.config), *env, rgbTolerance, alphaTolerance));
			w.swap();

			p.rbErr = res.readbackErrorBits;
			p.blRGBErr = res.blendRGBErrorBits;
			p.blAErr = res.blendAlphaErrorBits;
			r.results.push_back(p);

			if (p.rbErr > 1.0 || p.blRGBErr > rgbTolerance || p.blAErr > alphaTolerance) {
				env->log << name << ":  FAIL "
					<< r.config->conciseDescription()<< '\n'
					<< "\tsource factor = "
					<< factorToName(p.src)
					<< ", dest factor = "
					<< factorToName(p.dst)
					<< "\n\tReadback had " << p.rbErr
					<< " bits in error; RGB blending had "
					<< p.blRGBErr << " bits in error, Alpha blending had "
					<< p.blAErr << " bits in error.\n";
				allPassed = false;
			}
		}

	r.pass = allPassed;
} // BlendFuncTest::runOne

///////////////////////////////////////////////////////////////////////////////
// logOne:  Log a single test case
///////////////////////////////////////////////////////////////////////////////
void
BlendFuncTest::logOne(BlendFuncResult& r) {
	if (r.pass) {
		logPassFail(r);
		logConcise(r);
	}
}


///////////////////////////////////////////////////////////////////////////////
// compareOne:  Compare results for a single test case
///////////////////////////////////////////////////////////////////////////////
void
BlendFuncTest::compareOne(BlendFuncResult& oldR, BlendFuncResult& newR) {
	BasicStats readbackStats;
	BasicStats blendStats;

	vector<BlendFuncResult::PartialResult>::const_iterator np;
	vector<BlendFuncResult::PartialResult>::const_iterator op;

	for (np = newR.results.begin(); np != newR.results.end(); ++np)
		// Find the matching case, if any, in the old results:
		for (op = oldR.results.begin(); op != oldR.results.end(); ++op)
			if (np->src == op->src && np->dst == op->dst) {
				readbackStats.sample(np->rbErr - op->rbErr);
				blendStats.sample(np->blRGBErr - op->blRGBErr);
				blendStats.sample(np->blAErr - op->blAErr);
			}

	if (readbackStats.n() == static_cast<int>(newR.results.size())
	 && newR.results.size() == oldR.results.size()
	 && readbackStats.mean() == 0.0 && blendStats.mean() == 0.0) {
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
		if (blendStats.mean() < 0.0)
			env->log << '\t' << env->options.db2Name
				<< " appears to have more accurate blending.\n";
		else if (blendStats.mean() > 0.0)
			env->log << '\t' << env->options.db1Name
				<< " appears to have more accurate blending.\n";
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
					if (np->src == op->src
					 && np->dst == op->dst)
						break;
				if (op == oldR.results.end())
					env->log << "\t\t"
						<< factorToName(np->src)
						<< ' '
						<< factorToName(np->dst)
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
					if (op->src == np->src
					 && op->dst == np->dst)
						break;
				if (np == newR.results.end())
					env->log << "\t\t"
						<< factorToName(op->src)
						<< ' '
						<< factorToName(op->dst)
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
					if (np->src == op->src
					  && np->dst == op->dst)
						break;
				if (op != oldR.results.end())
					env->log << "\t\t"
						<< factorToName(np->src)
						<< ' '
						<< factorToName(np->dst)
						<< '\n';
			}
		}
	}
} // BlendFuncTest::compareOne

///////////////////////////////////////////////////////////////////////////////
// Result I/O functions:
///////////////////////////////////////////////////////////////////////////////
void
BlendFuncResult::putresults(ostream& s) const {
	s << results.size() << '\n';
	for (vector<PartialResult>::const_iterator p = results.begin();
	     p != results.end(); ++p)
		s << factorToName(p->src) << ' '
		  << factorToName(p->dst) << ' '
		  << p->rbErr << ' ' << p->blRGBErr << ' ' << p->blAErr << '\n';
} // BlendFuncResult::put

bool
BlendFuncResult::getresults(istream& s) {
	int n;
	s >> n;
	for (int i = 0; i < n; ++i) {
		PartialResult p;
		string src;
		string dst;
		s >> src >> dst >> p.rbErr >> p.blRGBErr >> p.blAErr;
		p.src = nameToFactor(src);
		p.dst = nameToFactor(dst);
		results.push_back(p);
	}

	return s.good();
} // BlendFuncResult::get

///////////////////////////////////////////////////////////////////////////////
// The test object itself:
///////////////////////////////////////////////////////////////////////////////
BlendFuncTest blendFuncTest("blendFunc", "window, rgb",

	"This test checks all combinations of source and destination\n"
	"blend factors for the GL_FUNC_ADD blend equation.  It operates\n"
	"on all RGB or RGBA drawing surface configurations that support\n"
	"the creation of windows.\n"
	"\n"
	"Note that a common cause of failures for this test is small errors\n"
	"introduced when an implementation scales color values incorrectly;\n"
	"for example, converting an 8-bit color value to float by\n"
	"dividing by 256 rather than 255, or computing a blending result\n"
	"by shifting a double-width intermediate value rather than scaling\n"
	"it.  Also, please note that the OpenGL spec requires that when\n"
	"converting from floating-point colors to integer form, the result\n"
	"must be rounded to the nearest integer, not truncated.\n"
	"[1.2.1, 2.13.9]\n"
	"\n"
	"The test reports two error measurements.  The first (readback) is\n"
	"the error detected when reading back raw values that were written\n"
	"to the framebuffer.  The error in this case should be very close\n"
	"to zero, since the values are carefully constructed so that they\n"
	"can be represented accurately in the framebuffer.  The second\n"
	"(blending) is the error detected in the result of the blending\n"
	"computation.  For the test to pass, these errors must both be\n"
	"no greater than one least-significant bit in the framebuffer\n"
	"representation of a color.\n");


} // namespace GLEAN
