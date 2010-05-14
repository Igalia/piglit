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

#include <assert.h>
#include <stdlib.h>
#include "tblend.h"
#include "rand.h"
#include "image.h"
#include <cmath>

#define ELEMENTS(ARRAY) (sizeof(ARRAY) / sizeof(ARRAY[0]))

namespace GLEAN {
static PFNGLBLENDFUNCSEPARATEPROC glBlendFuncSeparate_func = NULL;
static PFNGLBLENDCOLORPROC glBlendColor_func = NULL;
static PFNGLBLENDEQUATIONPROC glBlendEquation_func = NULL;
static PFNGLBLENDEQUATIONSEPARATEPROC glBlendEquationSeparate_func = NULL;
}

//namespace {

struct enumNameMapping {
	GLenum token;
	const char* name;
};

enumNameMapping factorNames[] = {
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
	{GL_ZERO,			"GL_ZERO"},
	{GL_CONSTANT_COLOR,		"GL_CONSTANT_COLOR"},
	{GL_ONE_MINUS_CONSTANT_COLOR,	"GL_ONE_MINUS_CONSTANT_COLOR"},
	{GL_CONSTANT_ALPHA,		"GL_CONSTANT_ALPHA"},
	{GL_ONE_MINUS_CONSTANT_ALPHA,	"GL_ONE_MINUS_CONSTANT_ALPHA"}
};

// aka blend "equation"
enumNameMapping blendopNames[] = {
	{GL_FUNC_ADD,			"GL_FUNC_ADD"},
	{GL_FUNC_SUBTRACT,		"GL_FUNC_SUBTRACT"},
	{GL_FUNC_REVERSE_SUBTRACT,	"GL_FUNC_REVERSE_SUBTRACT"},
	{GL_MIN,			"GL_MIN"},
	{GL_MAX,			"GL_MAX"}
};


const char*
factorToName(GLenum factor) {
	for (unsigned int i = 0; i < ELEMENTS(factorNames); ++i)
		if (factorNames[i].token == factor)
			return factorNames[i].name;
	assert(0);
	return 0;
} // factorToName

GLenum
nameToFactor(string& name) {
	for (unsigned int i = 0; i < ELEMENTS(factorNames); ++i)
		if (factorNames[i].name == name)
			return factorNames[i].token;
	assert(0);
	return GL_ZERO;
} // nameToFactor

const char *
opToName(GLenum op) {
	for (unsigned int i = 0; i < ELEMENTS(blendopNames); ++i)
		if (blendopNames[i].token == op)
			return blendopNames[i].name;
	assert(0);
	return 0;
} // opToName

GLenum
nameToOp(string& name) {
	for (unsigned int i = 0; i < ELEMENTS(blendopNames); ++i)
		if (blendopNames[i].name == name)
			return blendopNames[i].token;
	assert(0);
	return GL_ZERO;
} // nameToOp


bool
needsDstAlpha(const GLenum func) {
	return func == GL_DST_ALPHA || func == GL_ONE_MINUS_DST_ALPHA
		|| func == GL_SRC_ALPHA_SATURATE;
}

bool
needsBlendColor(const GLenum func) {
	switch (func) {
	case GL_CONSTANT_COLOR:
	case GL_ONE_MINUS_CONSTANT_COLOR:
	case GL_CONSTANT_ALPHA:
	case GL_ONE_MINUS_CONSTANT_ALPHA:
		return true;
	default:
		return false;
	}
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

static void
applyBlend(GLenum srcFactorRGB, GLenum srcFactorA,
	   GLenum dstFactorRGB, GLenum dstFactorA,
	   GLenum opRGB, GLenum opA,
	   float* dst, const float* src,
	   const GLfloat constantColor[4])
{
	float sf[4], df[4];

	if (opRGB != GL_MIN && opRGB != GL_MAX) {
		// Src RGB term
		switch (srcFactorRGB) {
		case GL_ZERO:
			sf[0] = sf[1] = sf[2] = 0.0;
			break;
		case GL_ONE:
			sf[0] = sf[1] = sf[2] = 1.0;
			break;
		case GL_DST_COLOR:
			sf[0] = dst[0];
			sf[1] = dst[1];
			sf[2] = dst[2];
			break;
		case GL_ONE_MINUS_DST_COLOR:
			sf[0] = 1.0 - dst[0];
			sf[1] = 1.0 - dst[1];
			sf[2] = 1.0 - dst[2];
			break;
		case GL_SRC_ALPHA:
			sf[0] = sf[1] = sf[2] = src[3];
			break;
		case GL_ONE_MINUS_SRC_ALPHA:
			sf[0] = sf[1] = sf[2] = 1.0 - src[3];
			break;
		case GL_DST_ALPHA:
			sf[0] = sf[1] = sf[2] = dst[3];
			break;
		case GL_ONE_MINUS_DST_ALPHA:
			sf[0] = sf[1] = sf[2] = 1.0 - dst[3];
			break;
		case GL_SRC_ALPHA_SATURATE: {
			float f = 1.0 - dst[3];
			if (src[3] < f)
				f = src[3];
			sf[0] = sf[1] = sf[2] = f;
			}
			break;
		case GL_CONSTANT_COLOR:
			sf[0] = constantColor[0];
			sf[1] = constantColor[1];
			sf[2] = constantColor[2];
			break;
		case GL_ONE_MINUS_CONSTANT_COLOR:
			sf[0] = 1.0 - constantColor[0];
			sf[1] = 1.0 - constantColor[1];
			sf[2] = 1.0 - constantColor[2];
			break;
		case GL_CONSTANT_ALPHA:
			sf[0] =
			sf[1] =
			sf[2] = constantColor[3];
			break;
		case GL_ONE_MINUS_CONSTANT_ALPHA:
			sf[0] =
			sf[1] =
			sf[2] = 1.0 - constantColor[3];
			break;
		default:
			sf[0] = sf[1] = sf[2] = 0.0;
			abort();
			break;
		}

		// Dest RGB term
		switch (dstFactorRGB) {
		case GL_ZERO:
			df[0] = df[1] = df[2] = 0.0;
			break;
		case GL_ONE:
			df[0] = df[1] = df[2] = 1.0;
			break;
		case GL_SRC_COLOR:
			df[0] = src[0];
			df[1] = src[1];
			df[2] = src[2];
			break;
		case GL_ONE_MINUS_SRC_COLOR:
			df[0] = 1.0 - src[0];
			df[1] = 1.0 - src[1];
			df[2] = 1.0 - src[2];
			break;
		case GL_SRC_ALPHA:
			df[0] = df[1] = df[2] = src[3];
			break;
		case GL_ONE_MINUS_SRC_ALPHA:
			df[0] = df[1] = df[2] = 1.0 - src[3];
			break;
		case GL_DST_ALPHA:
			df[0] = df[1] = df[2] = dst[3];
			break;
		case GL_ONE_MINUS_DST_ALPHA:
			df[0] = df[1] = df[2] = 1.0 - dst[3];
			break;
		case GL_CONSTANT_COLOR:
			df[0] = constantColor[0];
			df[1] = constantColor[1];
			df[2] = constantColor[2];
			break;
		case GL_ONE_MINUS_CONSTANT_COLOR:
			df[0] = 1.0 - constantColor[0];
			df[1] = 1.0 - constantColor[1];
			df[2] = 1.0 - constantColor[2];
			break;
		case GL_CONSTANT_ALPHA:
			df[0] =
			df[1] =
			df[2] = constantColor[3];
			break;
		case GL_ONE_MINUS_CONSTANT_ALPHA:
			df[0] =
			df[1] =
			df[2] = 1.0 - constantColor[3];
			break;
		default:
			df[0] = df[1] = df[2] = 0.0;
			abort();
			break;
		}
	}

	if (opA != GL_MIN && opA != GL_MAX) {
		// Src Alpha term
		switch (srcFactorA) {
		case GL_ZERO:
			sf[3] = 0.0;
			break;
		case GL_ONE:
			sf[3] = 1.0;
			break;
		case GL_DST_COLOR:
			sf[3] = dst[3];
			break;
		case GL_ONE_MINUS_DST_COLOR:
			sf[3] = 1.0 - dst[3];
			break;
		case GL_SRC_ALPHA:
			sf[3] = src[3];
			break;
		case GL_ONE_MINUS_SRC_ALPHA:
			sf[3] = 1.0 - src[3];
			break;
		case GL_DST_ALPHA:
			sf[3] = dst[3];
			break;
		case GL_ONE_MINUS_DST_ALPHA:
			sf[3] = 1.0 - dst[3];
			break;
		case GL_SRC_ALPHA_SATURATE:
			sf[3] = 1.0;
			break;
		case GL_CONSTANT_COLOR:
			sf[3] = constantColor[3];
			break;
		case GL_ONE_MINUS_CONSTANT_COLOR:
			sf[3] = 1.0 - constantColor[3];
			break;
		case GL_CONSTANT_ALPHA:
			sf[3] = constantColor[3];
			break;
		case GL_ONE_MINUS_CONSTANT_ALPHA:
			sf[3] = 1.0 - constantColor[3];
			break;
		default:
			sf[3] = 0.0;
			abort();
			break;
		}

		// Dst Alpha term
		switch (dstFactorA) {
		case GL_ZERO:
			df[3] = 0.0;
			break;
		case GL_ONE:
			df[3] = 1.0;
			break;
		case GL_SRC_COLOR:
			df[3] = src[3];
			break;
		case GL_ONE_MINUS_SRC_COLOR:
			df[3] = 1.0 - src[3];
			break;
		case GL_SRC_ALPHA:
			df[3] = src[3];
			break;
		case GL_ONE_MINUS_SRC_ALPHA:
			df[3] = 1.0 - src[3];
			break;
		case GL_DST_ALPHA:
			df[3] = dst[3];
			break;
		case GL_ONE_MINUS_DST_ALPHA:
			df[3] = 1.0 - dst[3];
			break;
		case GL_CONSTANT_COLOR:
			df[3] = constantColor[3];
			break;
		case GL_ONE_MINUS_CONSTANT_COLOR:
			df[3] = 1.0 - constantColor[3];
			break;
		case GL_CONSTANT_ALPHA:
			df[3] = constantColor[3];
			break;
		case GL_ONE_MINUS_CONSTANT_ALPHA:
			df[3] = 1.0 - constantColor[3];
			break;
		default:
			df[3] = 0.0;
			abort();
			break;
		}
	}

	switch (opRGB) {
	case GL_FUNC_ADD:
		dst[0] = clamp(src[0] * sf[0] + dst[0] * df[0]);
		dst[1] = clamp(src[1] * sf[1] + dst[1] * df[1]);
		dst[2] = clamp(src[2] * sf[2] + dst[2] * df[2]);
		break;
	case GL_FUNC_SUBTRACT:
		dst[0] = clamp(src[0] * sf[0] - dst[0] * df[0]);
		dst[1] = clamp(src[1] * sf[1] - dst[1] * df[1]);
		dst[2] = clamp(src[2] * sf[2] - dst[2] * df[2]);
		break;
	case GL_FUNC_REVERSE_SUBTRACT:
		dst[0] = clamp(dst[0] * df[0] - src[0] * sf[0]);
		dst[1] = clamp(dst[1] * df[1] - src[1] * sf[1]);
		dst[2] = clamp(dst[2] * df[2] - src[2] * sf[2]);
		break;
	case GL_MIN:
		dst[0] = min(src[0], dst[0]);
		dst[1] = min(src[1], dst[1]);
		dst[2] = min(src[2], dst[2]);
		break;
	case GL_MAX:
		dst[0] = max(src[0], dst[0]);
		dst[1] = max(src[1], dst[1]);
		dst[2] = max(src[2], dst[2]);
		break;
        default:
		abort();
        }

	switch (opA) {
	case GL_FUNC_ADD:
		dst[3] = clamp(src[3] * sf[3] + dst[3] * df[3]);
		break;
	case GL_FUNC_SUBTRACT:
		dst[3] = clamp(src[3] * sf[3] - dst[3] * df[3]);
		break;
	case GL_FUNC_REVERSE_SUBTRACT:
		dst[3] = clamp(dst[3] * df[3] - src[3] * sf[3]);
		break;
	case GL_MIN:
		dst[3] = min(src[3], dst[3]);
		break;
	case GL_MAX:
		dst[3] = max(src[3], dst[3]);
		break;
        default:
		abort();
        }

} // applyBlend


namespace GLEAN {


BlendFuncTest::runFactorsResult
BlendFuncTest::runFactors(GLenum srcFactorRGB, GLenum srcFactorA,
			  GLenum dstFactorRGB, GLenum dstFactorA,
			  GLenum opRGB, GLenum opA,
			  const GLfloat constantColor[4],
			  GLEAN::DrawingSurfaceConfig& config,
			  GLEAN::Environment& env)
{
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

	if (haveSepFunc)
		glBlendFuncSeparate_func(srcFactorRGB, dstFactorRGB,
					 srcFactorA, dstFactorA);
	else
		glBlendFunc(srcFactorRGB, dstFactorRGB);

	if (haveBlendEquationSep)
		glBlendEquationSeparate_func(opRGB, opA);
	else if (haveBlendEquation)
		glBlendEquation_func(opRGB);

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
			applyBlend(srcFactorRGB, srcFactorA,
				   dstFactorRGB, dstFactorA,
				   opRGB, opA,
				   pix, rgba, constantColor);
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
	result.blendErrorBits = 0.0;
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
			result.blendErrorBits =
				max(static_cast<double>(result.blendErrorBits),
				max(ErrorBits(rError, config.r),
				max(ErrorBits(gError, config.g),
				max(ErrorBits(bError, config.b),
				    ErrorBits(aError, config.a)))));
			if (result.blendErrorBits > 1.0) {
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


bool
BlendFuncTest::runCombo(BlendFuncResult& r, Window& w,
			BlendFuncResult::PartialResult p,
			GLEAN::Environment& env)
{
	runFactorsResult res(runFactors(p.srcRGB, p.srcA, p.dstRGB, p.dstA,
					p.opRGB, p.opA, p.constColor,
					*(r.config), env));
	if (!env.options.quick)
		w.swap();

	p.rbErr = res.readbackErrorBits;
	p.blErr = res.blendErrorBits;
	r.results.push_back(p);

	if (p.rbErr > 1.0 || p.blErr > 1.0) {
		env.log << name << ":  FAIL "
			<< r.config->conciseDescription() << '\n'
			<< "\tsource factor RGB = " << factorToName(p.srcRGB)
			<< ", source factor A = " << factorToName(p.srcA)
			<< "\n\tdest factor RGB = " << factorToName(p.dstRGB)
			<< ", dest factor A = " << factorToName(p.dstA)
			<< "\n\tequation RGB = " << opToName(p.opRGB)
			<< ", equation A = " << opToName(p.opA)
			<< "\n\tconst color = { "
			<< p.constColor[0] << ", "
			<< p.constColor[1] << ", "
			<< p.constColor[2] << ", "
			<< p.constColor[3] << " }"
			<< "\n\tReadback had " << p.rbErr
			<< " bits in error; blending had "
			<< p.blErr << " bits in error.\n";
		return false;
	}
	return true;
}



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
		GL_SRC_ALPHA_SATURATE,
		GL_CONSTANT_COLOR,
		GL_ONE_MINUS_CONSTANT_COLOR,
		GL_CONSTANT_ALPHA,
		GL_ONE_MINUS_CONSTANT_ALPHA
	};
	static GLenum dstFactors[] = {
		GL_ZERO,
		GL_ONE,
		GL_SRC_COLOR,
		GL_ONE_MINUS_SRC_COLOR,
		GL_SRC_ALPHA,
		GL_ONE_MINUS_SRC_ALPHA,
		GL_DST_ALPHA,
		GL_ONE_MINUS_DST_ALPHA,
		GL_CONSTANT_COLOR,
		GL_ONE_MINUS_CONSTANT_COLOR,
		GL_CONSTANT_ALPHA,
		GL_ONE_MINUS_CONSTANT_ALPHA
	};
	static GLenum operators[] = {
		GL_FUNC_ADD,
		GL_FUNC_SUBTRACT,
		GL_FUNC_REVERSE_SUBTRACT,
		GL_MIN,
		GL_MAX
	};

	unsigned numSrcFactorsSep, numDstFactorsSep;
	unsigned numOperatorsRGB, numOperatorsA;
	BlendFuncResult::PartialResult p;
	bool allPassed = true;

	// test for features, get function pointers
	if (GLUtils::getVersion() >= 1.4) {
		haveSepFunc = true;
		glBlendFuncSeparate_func = (PFNGLBLENDFUNCSEPARATEPROC)
			GLUtils::getProcAddress("glBlendFuncSeparate");
	}
	else if (GLUtils::haveExtension("GL_EXT_blend_func_separate")) {
		haveSepFunc = true;
		glBlendFuncSeparate_func = (PFNGLBLENDFUNCSEPARATEPROC)
			GLUtils::getProcAddress("glBlendFuncSeparateEXT");
	}

	if (GLUtils::getVersion() >= 1.4) {
		haveBlendColor = true;
		glBlendColor_func = (PFNGLBLENDCOLORPROC)
			GLUtils::getProcAddress("glBlendColor");
	}
	else if (GLUtils::haveExtension("GL_EXT_blend_color")) {
		haveBlendColor = true;
		glBlendColor_func = (PFNGLBLENDCOLORPROC)
			GLUtils::getProcAddress("glBlendColorEXT");
	}

	if (GLUtils::getVersion() >= 1.4) {
		haveBlendEquation = true;
		glBlendEquation_func = (PFNGLBLENDEQUATIONPROC)
			GLUtils::getProcAddress("glBlendEquation");
	}
	else if (GLUtils::haveExtension("GL_EXT_blend_subtract") &&
		 GLUtils::haveExtension("GL_EXT_blend_min_max")) {
		haveBlendEquation = true;
		glBlendEquation_func = (PFNGLBLENDEQUATIONPROC)
			GLUtils::getProcAddress("glBlendEquationEXT");
	}

	if (GLUtils::getVersion() >= 2.0) {
		haveBlendEquationSep = true;
		glBlendEquationSeparate_func = (PFNGLBLENDEQUATIONSEPARATEPROC)
			GLUtils::getProcAddress("glBlendEquationSeparate");
	}
	else if (GLUtils::haveExtension("GL_EXT_blend_equation_separate")) {
		haveBlendEquationSep = true;
		glBlendEquationSeparate_func = (PFNGLBLENDEQUATIONSEPARATEPROC)
			GLUtils::getProcAddress("glBlendEquationSeparateEXT");
	}

	if (haveBlendColor) {
		// Just one blend color setting for all tests
		p.constColor[0] = 0.25;
		p.constColor[1] = 0.0;
		p.constColor[2] = 1.0;
		p.constColor[3] = 0.75;
		glBlendColor_func(p.constColor[0], p.constColor[1],
				  p.constColor[2], p.constColor[3]);
	}

	if (haveSepFunc) {
		numSrcFactorsSep = ELEMENTS(srcFactors);
		numDstFactorsSep = ELEMENTS(dstFactors);
	}
	else {
		numSrcFactorsSep = 1;
		numDstFactorsSep = 1;
	}

	if (haveBlendEquation) {
		numOperatorsRGB = ELEMENTS(operators);
		numOperatorsA = ELEMENTS(operators);
	}
	else {
		numOperatorsRGB = 1; // just ADD
		numOperatorsA = 1; // just ADD
	}

#if 0
	// use this to test a single combination:
	p.srcRGB = p.srcA = GL_SRC_ALPHA;
	p.dstRGB = p.dstA = GL_ONE_MINUS_SRC_ALPHA;
	p.opRGB = GL_FUNC_ADD;
	p.opA = GL_FUNC_ADD;
	allPassed = runCombo(r, w, p, *env);
#else
	for (unsigned int op = 0; op < numOperatorsRGB; ++op) {
		p.opRGB = operators[op];

		for (unsigned int opa = 0; opa < numOperatorsA; ++opa) {
			p.opA = operators[opa];

			unsigned int step;
			if (p.opRGB == GL_FUNC_ADD && p.opA == GL_FUNC_ADD) {
				// test _all_ blend term combinations
				step = 1;
			}
			else if (p.opRGB == GL_MIN || p.opRGB == GL_MAX ||
				 p.opA == GL_MIN || p.opA == GL_MAX) {
				// blend terms are N/A so only do one iteration of loops
				step = 1000;
			}
			else {
				// subtract modes: do every 3rd blend term for speed
				step = 3;
			}

			for (unsigned int sf = 0; sf < ELEMENTS(srcFactors); sf += step) {
				for (unsigned int sfa = 0; sfa < numSrcFactorsSep; sfa += step) {
					for (unsigned int df = 0; df < ELEMENTS(dstFactors); df += step) {
						for (unsigned int dfa = 0; dfa < numDstFactorsSep; dfa += step) {

							if (haveSepFunc) {
								p.srcRGB = srcFactors[sf];
								p.srcA = srcFactors[sfa];
								p.dstRGB = dstFactors[df];
								p.dstA = dstFactors[dfa];
							}
							else {
								p.srcRGB = p.srcA = srcFactors[sf];
								p.dstRGB = p.dstA = dstFactors[df];
							}

							// skip test if it depends on non-existant alpha channel
							if ((r.config->a == 0)
								&& (needsDstAlpha(p.srcRGB) ||
									needsDstAlpha(p.srcA) ||
									needsDstAlpha(p.dstRGB) ||
									needsDstAlpha(p.dstA)))
								continue;

							// skip test if blend color used, but not supported.
							if (!haveBlendColor
								&& (needsBlendColor(p.srcRGB) ||
									needsBlendColor(p.srcA) ||
									needsBlendColor(p.dstRGB) ||
									needsBlendColor(p.dstA)))
								continue;

							if (!runCombo(r, w, p, *env)) {
								allPassed = false;
							}
						}
					}
				}
			}
		}
	}
#endif

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


bool
BlendFuncTest::equalMode(const BlendFuncResult::PartialResult &r1,
			 const BlendFuncResult::PartialResult &r2) const
{
	return (r1.srcRGB == r2.srcRGB &&
		r1.srcA == r2.srcA &&
		r1.dstRGB == r2.dstRGB &&
		r1.dstA == r2.dstA &&
		r1.opRGB == r2.opRGB &&
		r1.opA == r2.opA);
}


void
BlendFuncTest::printMode(const BlendFuncResult::PartialResult &r) const
{
	env->log << "\t\t"
		 << factorToName(r.srcRGB)
		 << ' '
		 << factorToName(r.srcA)
		 << ' '
		 << factorToName(r.dstRGB)
		 << ' '
		 << factorToName(r.dstA)
		 << ' '
		 << opToName(r.opRGB)
		 << ' '
		 << opToName(r.opA)
		 << '\n';
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
			if (equalMode(*np, *op)) {
				readbackStats.sample(np->rbErr - op->rbErr);
				blendStats.sample(np->blErr - op->blErr);
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
					if (equalMode(*np, *op))
						break;
				if (op == oldR.results.end())
					printMode(*np);
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
					if (equalMode(*op, *np))
						break;
				if (np == newR.results.end())
					printMode(*op);
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
					if (equalMode(*op, *np))
						break;
				if (op != oldR.results.end())
					printMode(*op);
			}
		}
	}
} // BlendFuncTest::compareOne

///////////////////////////////////////////////////////////////////////////////
// Result I/O functions:
///////////////////////////////////////////////////////////////////////////////
void
BlendFuncResult::putresults(ostream& s) const {
	// write number of lines first
	s << results.size() << '\n';
	// write each result as one line of text
	for (vector<PartialResult>::const_iterator p = results.begin();
	     p != results.end(); ++p)
		s << factorToName(p->srcRGB) << ' '
		  << factorToName(p->srcA) << ' '
		  << factorToName(p->dstRGB) << ' '
		  << factorToName(p->dstA) << ' '
		  << opToName(p->opRGB) << ' '
		  << opToName(p->opA) << ' '
		  << p->rbErr << ' ' << p->blErr << '\n';
} // BlendFuncResult::put

bool
BlendFuncResult::getresults(istream& s) {
	int n;
	// read number of lines
	s >> n;
	// parse each line/result
	for (int i = 0; i < n; ++i) {
		PartialResult p;
		string srcRGB, srcA;
		string dstRGB, dstA;
		string opRGB, opA;
		s >> srcRGB >> srcA >> dstRGB >> dstA >> opRGB >> opA >> p.rbErr >> p.blErr;
		p.srcRGB = nameToFactor(srcRGB);
		p.srcA = nameToFactor(srcA);
		p.dstRGB = nameToFactor(dstRGB);
		p.dstA = nameToFactor(dstA);
		p.opRGB = nameToOp(opRGB);
		p.opA = nameToOp(opA);
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
