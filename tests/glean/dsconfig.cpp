// BEGIN_COPYRIGHT
// 
// Copyright (C) 1999  Allen Akin   All Rights Reserved.
// 
// multisample changes: Copyright (c) 2008 VMware, Inc.  All rights reserved.
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


// dsconfig.cpp:  Implementation of drawing surface configuration utilities
#include "dsconfig.h"
#include <iostream>
#include <sstream>
#include <string.h>
#include <map>
#include <limits.h>

#ifdef __WIN__
// disable the annoying warning : "forcing value to bool 'true' or 'false' (performance warning)"
#pragma warning (disable : 4800)
#endif


#include "lex.h"



namespace {

#ifdef __X11__

bool
haveGLXExtension(::Display* dpy, const char* extName) {
	const char* extString =
	    glXQueryExtensionsString(dpy, DefaultScreen(dpy));
		// We don't cache the result, so that subsequent calls
		// with different values of ``dpy'' will work correctly.
		// Would be nice to improve this, though.

	const char* start = extString;
	for (;;) {
		const char* where = strstr(start, extName);
		if (!where)
			return false;

		// Make sure we're not fooled by extensions whose names
		// have the desired extName as an initial substring:
		const char* terminator = where + strlen(extName);
		if ((where == start || where[-1] == ' ')
		  && (*terminator == ' ' || *terminator == 0))
			return true;

		start = terminator;
	}

	return false;
} // haveGLXExtension

#endif

typedef enum {		// These variable tags are used as array indices,
			// so they should represent a small dense set of
			// nonnegative integers.  0 is reserved.
	VID = 1,
	VFBCID,
	VCANRGBA,
	VR,
	VG,
	VB,
	VA,
	VCANCI,
	VBUFSIZE,
	VLEVEL,
	VDB,
	VSTEREO,
	VAUX,
	VZ,
	VS,
	VACCUMR,
	VACCUMG,
	VACCUMB,
	VACCUMA,
        VSAMPLES,
	VCANWINDOW,
	VCANPIXMAP,
	VCANPBUFFER,
	VMAXPBUFFERWIDTH,
	VMAXPBUFFERHEIGHT,
	VMAXPBUFFERPIXELS,
	VCANWINSYSRENDER,
	VFAST,
	VCONFORMANT,
	VTRANSPARENT,
	VTRANSR,
	VTRANSG,
	VTRANSB,
	VTRANSA,
	VTRANSI,
	V_LAST
} CanonVar;

static struct {CanonVar var; const char* name;} varNames[] = {
	{VID,			"id"},
	{VFBCID,		"fbcID"},
	{VCANRGBA,		"canRGBA"},
	{VR,			"r"},
	{VG,			"g"},
	{VB,			"b"},
	{VA,			"a"},
	{VCANCI,		"canCI"},
	{VBUFSIZE,		"bufSize"},
	{VLEVEL,		"level"},
	{VDB,			"db"},
	{VSTEREO,		"stereo"},
	{VAUX,			"aux"},
	{VZ,			"z"},
	{VS,			"s"},
	{VACCUMR,		"accumR"},
	{VACCUMG,		"accumG"},
	{VACCUMB,		"accumB"},
	{VACCUMA,		"accumA"},
        {VSAMPLES,              "multisample"},
	{VCANWINDOW,		"window"},
	{VCANPIXMAP,		"pixmap"},
	{VCANPBUFFER,		"pBuffer"},
	{VMAXPBUFFERWIDTH,	"maxPBufferWidth"},
	{VMAXPBUFFERHEIGHT,	"maxPBufferHeight"},
	{VMAXPBUFFERPIXELS,	"maxPBufferPixels"},
	{VCANWINSYSRENDER,	"winsys"},
	{VFAST,			"fast"},
	{VCONFORMANT,		"conformant"},
	{VTRANSPARENT,		"transparent"},
	{VTRANSR,		"transR"},
	{VTRANSG,		"transG"},
	{VTRANSB,		"transB"},
	{VTRANSA,		"transA"},
	{VTRANSI,		"transI"}
};

const char* mapVarToName[V_LAST];
map<string, CanonVar> mapNameToVar;
bool mapsInitialized = false;

void
initializeMaps() {
	for (unsigned i = 0; i < sizeof(varNames)/sizeof(varNames[0]); ++i) {
		mapVarToName[varNames[i].var] = varNames[i].name;
		mapNameToVar[varNames[i].name] = varNames[i].var;
		}
	mapsInitialized = true;
} // initializeMaps

template<class T> inline T abs(T a) {return (a < 0)? -a: a;}

} // anonymous namespace


namespace GLEAN {

// init all config fields to zero
void
DrawingSurfaceConfig::zeroFields()
{
#if defined(__X11__)
	visID = 0;
#  if defined(GLX_VERSION_1_3)
	fbcID = 0;
#  endif
#elif defined(__WIN__)
	pfdID = 0;
#elif defined(__AGL__)
	pfID = 0;
#else
#  error "what's the config ID?"
#endif
	canRGBA = 0;
	canCI = 0;
	bufSize = 0;
	level = 0;
	db = 0;
	stereo = 0;
	aux = 0;
	r = 0;
	g = 0;
	b = 0;
	a = 0;
	z = 0;
	s = 0;
	accR = 0;
	accG = 0;
	accB = 0;
	accA = 0;
	samples = 0;
	canWindow = 0;
#if defined(__X11__)
	canPixmap = 0;
#if defined(GLX_VERSION_1_3)
	canPBuffer = 0;
	maxPBufferWidth = 0;
	maxPBufferHeight = 0;
	maxPBufferPixels = 0;
#endif
#endif
	canWinSysRender = 0;
	fast = 0;
	conformant = 0;
	transparent = 0;
	transR = 0;
	transG = 0;
	transB = 0;
	transA = 0;
	transI = 0;
}


#if defined(__X11__)

DrawingSurfaceConfig::DrawingSurfaceConfig(::Display* dpy, ::XVisualInfo* pvi) {
	if (!mapsInitialized)
		initializeMaps();

	int var;

	vi = pvi;
	visID = vi->visualid;
#	if defined(GLX_VERSION_1_3)
		fbcID = 0;
#	endif

	glXGetConfig(dpy, vi, GLX_RGBA, &var);
	canRGBA = var;
	canCI = !var;
		// There is no dual-personality Visual support in early
		// versions of GLX.

	glXGetConfig(dpy, vi, GLX_BUFFER_SIZE, &bufSize);

	glXGetConfig(dpy, vi, GLX_LEVEL, &level);

	glXGetConfig(dpy, vi, GLX_DOUBLEBUFFER, &var);
	db = var;

	glXGetConfig(dpy, vi, GLX_STEREO, &var);
	stereo = var;

	glXGetConfig(dpy, vi, GLX_AUX_BUFFERS, &aux);

	if (canRGBA) {
		glXGetConfig(dpy, vi, GLX_RED_SIZE, &r);
		glXGetConfig(dpy, vi, GLX_GREEN_SIZE, &g);
		glXGetConfig(dpy, vi, GLX_BLUE_SIZE, &b);
		glXGetConfig(dpy, vi, GLX_ALPHA_SIZE, &a);
	} else
		r = g = b = a = 0;

	glXGetConfig(dpy, vi, GLX_DEPTH_SIZE, &z);

	glXGetConfig(dpy, vi, GLX_STENCIL_SIZE, &s);

	if (canRGBA) {
		glXGetConfig(dpy, vi, GLX_ACCUM_RED_SIZE, &accR);
		glXGetConfig(dpy, vi, GLX_ACCUM_GREEN_SIZE, &accG);
		glXGetConfig(dpy, vi, GLX_ACCUM_BLUE_SIZE, &accB);
		glXGetConfig(dpy, vi, GLX_ACCUM_ALPHA_SIZE, &accA);
	} else
		accR = accG = accB = accA = 0;

	// Note that samples=0 means no multisampling!
	// One might think that one sample per pixel means non-multisampling
	// but that's not the convention used here.
	samples = 0;
	if (canRGBA) {
		int sampBuf = 0;
		glXGetConfig(dpy, vi, GLX_SAMPLE_BUFFERS, &sampBuf);
		if (sampBuf) {
			glXGetConfig(dpy, vi, GLX_SAMPLES, &samples);
		}
	}

	canWindow = canPixmap = true;
		// Only guaranteed in early versions of GLX.

#	if defined(GLX_VERSION_1_3)
		canPBuffer = 0;
		maxPBufferWidth = 0;
		maxPBufferHeight = 0;
		maxPBufferPixels = 0;
#	endif

	canWinSysRender = true;
		// Only guaranteed in early versions of GLX.

	fast = true;
	conformant = true;
#	if defined(GLX_EXT_visual_rating)
		if (haveGLXExtension(dpy, "GLX_EXT_visual_rating")) {
			glXGetConfig(dpy, vi, GLX_VISUAL_CAVEAT_EXT, &var);
			if (var == GLX_SLOW_VISUAL_EXT)
				fast = false;
			else if (var == GLX_NON_CONFORMANT_VISUAL_EXT)
				conformant = false;
		}
#	endif

	transparent = false;
	transR = transG = transB = transA = transI = 0;
#	if defined(GLX_EXT_visual_info)
		if (haveGLXExtension(dpy, "GLX_EXT_visual_info")) {
			glXGetConfig(dpy, vi, GLX_TRANSPARENT_TYPE_EXT, &var);
			if (var == GLX_TRANSPARENT_RGB_EXT) {
				glXGetConfig(dpy, vi,
				    GLX_TRANSPARENT_RED_VALUE_EXT, &transR);
				glXGetConfig(dpy, vi,
				    GLX_TRANSPARENT_GREEN_VALUE_EXT, &transG);
				glXGetConfig(dpy, vi,
				    GLX_TRANSPARENT_BLUE_VALUE_EXT, &transB);
				glXGetConfig(dpy, vi,
				    GLX_TRANSPARENT_ALPHA_VALUE_EXT, &transA);
			} else
				glXGetConfig(dpy, vi,
				    GLX_TRANSPARENT_INDEX_VALUE_EXT, &transI);
		}
#	endif
} // DrawingSurfaceConfig::DrawingSurfaceConfig

#if defined(GLX_VERSION_1_3)
DrawingSurfaceConfig::DrawingSurfaceConfig(::Display* dpy, ::GLXFBConfig* pfbc)
{
	// silence warnings about unused parameters:
	(void) dpy;
	(void) pfbc;

	if (!mapsInitialized)
		initializeMaps();
// XXX Need to write drawing surface config code for GLX 1.3
	cerr << "GLX 1.3 version of DrawingSurfaceConfig constructor is not"
		"implemented.\n";
} // DrawingSurfaceConfig::DrawingSurfaceConfig
#endif

#elif defined(__WIN__)

DrawingSurfaceConfig::DrawingSurfaceConfig(int id, ::PIXELFORMATDESCRIPTOR *ppfd)
{
	if (!mapsInitialized)
		initializeMaps();

	pfd = ppfd;
	pfdID = id;
	
	canRGBA = pfd->iPixelType == PFD_TYPE_RGBA;			
	canCI = pfd->iPixelType == PFD_TYPE_COLORINDEX;			

	bufSize = pfd->cColorBits + pfd->cAlphaBits;

	level = 0;

	db = pfd->dwFlags & PFD_DOUBLEBUFFER;

	stereo = pfd->dwFlags & PFD_STEREO;

	aux = pfd->cAuxBuffers;

	if (canRGBA)	{
		r = pfd->cRedBits;
		g = pfd->cGreenBits;
		b = pfd->cBlueBits;
		a = pfd->cAlphaBits;
	}
	else
		r = g = b = a = 0;

	z = pfd->cDepthBits;
	s = pfd->cStencilBits;

	accR = pfd->cAccumRedBits;
	accG = pfd->cAccumGreenBits;
	accB = pfd->cAccumBlueBits;
	accA = pfd->cAccumAlphaBits;

	samples = 0; // XXX implement properly for Windows!

	canWindow = pfd->dwFlags & PFD_DRAW_TO_WINDOW;			

	canWinSysRender = pfd->dwFlags & PFD_SUPPORT_GDI;		

	if (pfd->dwFlags & PFD_GENERIC_FORMAT)
	{
		if (pfd->dwFlags & PFD_GENERIC_ACCELERATED)
		{
			// it's an MCD - at least it has some acceleration
			fast = true;
		}
		else 
		{
			// it's software 
			fast = false;
		}
	}
	else 
	{
		// it's an ICD 
		fast = true;
	}

	// we'll assume that the OpenGL implementation thinks it is conformant
	conformant = true;		

	// chromakeying isn't supported
	transparent = false;
	transR = transG = transB = transA = transI = 0;
}
#elif defined(__BEWIN__)

DrawingSurfaceConfig::DrawingSurfaceConfig() {

	if (!mapsInitialized)
		initializeMaps();

	/* these values are estimates for the moment */
	level = 0;
	db = 1;
	stereo =0;
	r = g = b = a = 32;

	z = 30;
	accR = 32;
	accG = 32;
	accB = 32;
	accA = 32;

	samples = 0;

	canWindow = 1;			
	canWinSysRender = 1;		

	// This is a software-mode assumption
	fast = false;

	// we'll assume that the OpenGL implementation thinks it is conformant
	conformant = true;		

	// chromakeying isn't supported
	transparent = false;
	transR = transG = transB = transA = transI = 0;
}

#elif defined(__AGL__)

DrawingSurfaceConfig::DrawingSurfaceConfig(int id, ::AGLPixelFormat pfd)
{
	int			i;
	
	if (!mapsInitialized)
		initializeMaps();

	pf = pfd;
	
	if (aglDescribePixelFormat( pf, AGL_RGBA, &i))
		canRGBA = (i == GL_TRUE);
	canCI = (i == GL_FALSE);			

	if (aglDescribePixelFormat( pf, AGL_BUFFER_SIZE, &i))
		bufSize = i;			

	level = 0;

	if (aglDescribePixelFormat( pf, AGL_DOUBLEBUFFER, &i))
		db = (i == GL_TRUE);
	if (aglDescribePixelFormat( pf, AGL_STEREO, &i))
		stereo = (i == GL_TRUE);
	if (aglDescribePixelFormat( pf, AGL_AUX_BUFFERS, &i))
		aux = i;

	if (canRGBA)	{
		aglDescribePixelFormat( pf, AGL_RED_SIZE,   &r);
		aglDescribePixelFormat( pf, AGL_GREEN_SIZE, &g);
		aglDescribePixelFormat( pf, AGL_BLUE_SIZE,  &b);
		aglDescribePixelFormat( pf, AGL_ALPHA_SIZE, &a);

		//this is a workaround for some versions of AGL
		if (r == 10)
		{
			r=g=b=8;
			bufSize = r + g + b + a;
		}
	}
	else
		r = g = b = a = 0;

	samples = 0; // XXX implement properly for AGL

	aglDescribePixelFormat( pf, AGL_DEPTH_SIZE,   & z);
	aglDescribePixelFormat( pf, AGL_STENCIL_SIZE, & s);

	aglDescribePixelFormat( pf, AGL_ACCUM_RED_SIZE,   & accR);
	aglDescribePixelFormat( pf, AGL_ACCUM_GREEN_SIZE, & accG);
	aglDescribePixelFormat( pf, AGL_ACCUM_BLUE_SIZE,  & accB);
	aglDescribePixelFormat( pf, AGL_ACCUM_ALPHA_SIZE, & accA);

	aglDescribePixelFormat( pf, AGL_WINDOW, &i);
	canWindow = i;
	aglDescribePixelFormat( pf, AGL_OFFSCREEN, &i);
	canWinSysRender = i;
	aglDescribePixelFormat( pf, AGL_ACCELERATED, & i);
	fast = i;

	// we'll assume that the OpenGL implementation thinks it is conformant
	conformant = true;		

	// chromakeying isn't supported
	transparent = false;
	transR = transG = transB = transA = transI = 0;
}

#endif

DrawingSurfaceConfig::DrawingSurfaceConfig(string& str) {
	if (!mapsInitialized)
		initializeMaps();

	zeroFields();

	try {
		Lex lex(str.c_str());

		for (lex.next(); lex.token != Lex::END; lex.next()) {
			if (lex.token != Lex::ID)
				throw Syntax("expected variable name",
					lex.position());
			lex.next();
			if (lex.token != Lex::ICONST)
				throw Syntax("expected integer value",
					lex.position());

			// Yes, this is an unpleasantly verbose way to
			// handle this problem.  However, it will be
			// necessary when we have to deal with attributes
			// that aren't all of a simple integral type.

			switch (mapNameToVar[lex.id]) {
			case VID:
#			    if defined(__X11__)
				visID = lex.iValue;
#			    endif
				break;
			case VFBCID:
#			    if defined(GLX_VERSION_1_3)
				fbcID = lex.iValue;
#			    endif
				break;
			case VCANRGBA:
				canRGBA = lex.iValue;
				break;
			case VR:
				r = lex.iValue;
				break;
			case VG:
				g = lex.iValue;
				break;
			case VB:
				b = lex.iValue;
				break;
			case VA:
				a = lex.iValue;
				break;
			case VCANCI:
				canCI = lex.iValue;
				break;
			case VBUFSIZE:
				bufSize = lex.iValue;
				break;
			case VLEVEL:
				level = lex.iValue;
				break;
			case VDB:
				db = lex.iValue;
				break;
			case VSTEREO:
				stereo = lex.iValue;
				break;
			case VAUX:
				aux = lex.iValue;
				break;
			case VZ:
				z = lex.iValue;
				break;
			case VS:
				s = lex.iValue;
				break;
			case VACCUMR:
				accR = lex.iValue;
				break;
			case VACCUMG:
				accG = lex.iValue;
				break;
			case VACCUMB:
				accB = lex.iValue;
				break;
			case VACCUMA:
				accA = lex.iValue;
				break;
			case VSAMPLES:
				samples = lex.iValue;
				break;
			case VCANWINDOW:
				canWindow = lex.iValue;
				break;
			case VCANPIXMAP:
#				if defined(__X11__)
				canPixmap = lex.iValue;
#				endif
				break;
			case VCANPBUFFER:
#			    if defined(GLX_VERSION_1_3)
				canPBuffer = lex.iValue;
#			    endif
				break;
			case VMAXPBUFFERWIDTH:
#			    if defined(GLX_VERSION_1_3)
				maxPBufferWidth = lex.iValue;
#			    endif
				break;
			case VMAXPBUFFERHEIGHT:
#			    if defined(GLX_VERSION_1_3)
				maxPBufferHeight = lex.iValue;
#			    endif
				break;
			case VMAXPBUFFERPIXELS:
#			    if defined(GLX_VERSION_1_3)
				maxPBufferPixels = lex.iValue;
#			    endif
				break;
			case VCANWINSYSRENDER:
				canWinSysRender = lex.iValue;
				break;
			case VFAST:
				fast = lex.iValue;
				break;
			case VCONFORMANT:
				conformant = lex.iValue;
				break;
			case VTRANSPARENT:
				transparent = lex.iValue;
				break;
			case VTRANSR:
				transR = lex.iValue;
				break;
			case VTRANSG:
				transG = lex.iValue;
				break;
			case VTRANSB:
				transB = lex.iValue;
				break;
			case VTRANSA:
				transA = lex.iValue;
				break;
			case VTRANSI:
				transI = lex.iValue;
				break;
			default:
				throw Syntax("unrecognized variable",
					lex.position());
			}
		}
	}
	catch (Lex::Lexical e) {
		throw Syntax(e.err, e.position);
	}
} // DrawingSurfaceConfing::DrawingSurfaceConfig


///////////////////////////////////////////////////////////////////////////////
// canonicalDescription - return a description string that can be used
//	to reconstruct the essential attributes of a drawing surface
//	configuration.  Note that visual ID numbers are included for
//	completeness, but they must be ignored when attempting to compare
//	two surface configurations; there's no guarantee that they'll be
//	valid (or even relevant, since they may have been created on another
//	OS).
//
//	This is ugly code, but it keeps the names used here and in
//	the string-based constructor for DrawingSurfaceConfig in sync
//	automatically.
///////////////////////////////////////////////////////////////////////////////
string
DrawingSurfaceConfig::canonicalDescription() {
	ostringstream s;

#	if defined(__X11__)
	    s << mapVarToName[VID] << ' ' << visID;
#	    if defined(GLX_VERSION_1_3)
		s << ' ' << mapVarToName[VFBCID] << ' ' << fbcID;
#	    endif
#	elif defined(__WIN__)
		s << mapVarToName[VID] << ' ' << pfdID;	    
#	endif

	s << ' ' << mapVarToName[VCANRGBA] << ' ' << canRGBA;
	s << ' ' << mapVarToName[VR] << ' ' << r
	  << ' ' << mapVarToName[VG] << ' ' << g
	  << ' ' << mapVarToName[VB] << ' ' << b
	  << ' ' << mapVarToName[VA] << ' ' << a;

	s << ' ' << mapVarToName[VCANCI] << ' ' << canCI;

	s << ' ' << mapVarToName[VBUFSIZE] << ' ' << bufSize;

	s << ' ' << mapVarToName[VLEVEL] << ' ' << level;

	s << ' ' << mapVarToName[VDB] << ' ' << db;

	s << ' ' << mapVarToName[VSTEREO] << ' '<< stereo;

	s << ' ' << mapVarToName[VAUX] << ' ' << aux;

	s << ' ' << mapVarToName[VZ] << ' ' << z;

	s << ' ' << mapVarToName[VS] << ' ' << DrawingSurfaceConfig::s;

	s << ' ' << mapVarToName[VACCUMR] << ' ' << accR
	  << ' ' << mapVarToName[VACCUMG] << ' ' << accG
	  << ' ' << mapVarToName[VACCUMB] << ' ' << accB
	  << ' ' << mapVarToName[VACCUMA] << ' ' << accA;

	s << ' ' << mapVarToName[VSAMPLES] << ' ' << samples;

	s << ' ' << mapVarToName[VCANWINDOW] << ' ' << canWindow;

#	if defined(__X11__)
		s << ' ' << mapVarToName[VCANPIXMAP] << ' ' << canPixmap;

#	    if defined(GLX_VERSION_1_3)
		s << ' ' << mapVarToName[VCANPBUFFER] << ' ' << canPBuffer;
		s << ' ' << mapVarToName[VMAXPBUFFERWIDTH] << ' ' << maxPBufferWidth;
		s << ' ' << mapVarToName[VMAXPBUFFERHEIGHT] << ' ' << maxPBufferHeight;
		s << ' ' << mapVarToName[VMAXPBUFFERPIXELS] << ' ' << maxPBufferPixels;
#	    endif

#	endif

	s << ' ' << mapVarToName[VCANWINSYSRENDER] << ' ' << canWinSysRender;

	s << ' ' << mapVarToName[VFAST] << ' ' << fast;

	s << ' ' << mapVarToName[VCONFORMANT] << ' ' << conformant;

	s << ' ' << mapVarToName[VTRANSPARENT] << ' ' << transparent;
	s << ' ' << mapVarToName[VTRANSR] << ' ' << transR
	  << ' ' << mapVarToName[VTRANSG] << ' ' << transG
	  << ' ' << mapVarToName[VTRANSB] << ' ' << transB
	  << ' ' << mapVarToName[VTRANSA] << ' ' << transA
	  << ' ' << mapVarToName[VTRANSI] << ' ' << transI;

	return s.str();
} // DrawingSurfaceConfig::canonicalDescription

///////////////////////////////////////////////////////////////////////////////
// conciseDescription - return a description string that's appropriate for
//	reading by humans, rather than parsing by machine.
///////////////////////////////////////////////////////////////////////////////
string
DrawingSurfaceConfig::conciseDescription() {
	ostringstream s;

	if (canRGBA && canCI)
		s << "dual ";

	if (canRGBA) {
		if (a) {
			if (r == g && g == b && b == a)
				s << "rgba" << r;
			else
				s << "r" << r << "g" << g << "b" << b
					<< "a" << a;
		} else {
			if (r == g && g == b)
				s << "rgb" << r;
			else
				s << "r" << r << "g" << g << "b" << b;
		}
	}

	if (canCI) {
		if (canRGBA)  s << "+";
		s << "ci" << bufSize;
	}

	if (level < 0)
		s << ", underlay";
	else if (level > 0)
		s << ", overlay";

	if (db)
		s << ", db";

	if (stereo)
		s << ", stereo";

	if (aux)
		s << ", aux" << aux;

	if (z)
		s << ", z" << z;

	if (DrawingSurfaceConfig::s)
		s << ", s" << DrawingSurfaceConfig::s;

	if (accR) {
		if (accA) {
			if (accR == accG && accG == accB
			 && accB == accA)
				s << ", accrgba" << accR;
			else
				s << ", accr" << accR << "g" << accG
				  << "b" << accB << "a" << accA;
		} else {
			if (accR == accG && accG == accB)
				s << ", accrgb" << accR;
			else
				s << ", accr" << accR << "g" << accG
				  << "b" << accB;
		}
	}

	if (samples) {
		s << ", samples" << samples;
	}

	{
	s << ", ";
	bool sep = false;
	if (canWindow) {
		s << "win";
		sep = true;
	}
#	if defined(__X11__)
		if (canPixmap) {
			if (sep)
				s << "+";
			s << "pmap";
			sep = true;
		}
#	endif
#	if defined(GLX_VERSION_1_3)
		if (canPBuffer) {
			if (sep)
				s << "+";
			s << "pbuf";
		}
#	endif
	}

	if (!fast)
		s << ", slow";

	if (!conformant)
		s << ", nonconformant";

	if (transparent) {
		if (canRGBA) {
			s << ", transrgba (" << transR << "," << transG
				<< "," << transB << "," << transA << ")";
		}
		if (canCI) {
			s << ", transci (" << transI << ")";
		}
	}

#	if defined(__X11__)
		s << ", id " << visID;
#		if defined(GLX_VERSION_1_3)
			if (fbcID)
				s << ", fbcid " << fbcID;
#		endif
#	elif defined(__WIN__)
			s << ", id " << pfdID;
#	endif

	return s.str();
} // DrawingSurfaceConfig::conciseDescription

///////////////////////////////////////////////////////////////////////////////
// match - select a config that ``matches'' the current config.
//	To keep this problem manageable, we'll assume that both the config
//	to be matched (call it the ``A'' config) and the vector of configs to
//	choose from (call them the ``B'' configs) were selected by a test
//	using a single filter.  Thus we can ignore any differences in buffer
//	availability (because we know those are irrelevant to the test), and
//	concentrate on picking configs for which the available buffers are
//	(in some sense) closest in size.
//
//	This will not be an acceptable solution in all cases, but it should
//	suffice for many.
///////////////////////////////////////////////////////////////////////////////
int
DrawingSurfaceConfig::match(vector<DrawingSurfaceConfig*>& choices) {
	typedef vector<DrawingSurfaceConfig*>::iterator cptr;

	int best = -1;
	int bestError = INT_MAX;

	for (cptr p = choices.begin(); p < choices.end(); ++p) {
		DrawingSurfaceConfig& c = **p;
		int error = 0;

		if (bufSize && c.bufSize)
			error += abs(bufSize - c.bufSize);
		if (r && c.r)
			error += abs(r - c.r);
		if (g && c.g)
			error += abs(g - c.g);
		if (b && c.b)
			error += abs(b - c.b);
		if (a && c.a)
			error += abs(a - c.a);
		if (z && c.z)
			error += abs(z - c.z);
		if (s && c.s)
			error += abs(s - c.s);
		if (accR && c.accR)
			error += abs(accR - c.accR);
		if (accG && c.accG)
			error += abs(accG - c.accG);
		if (accB && c.accB)
			error += abs(accB - c.accB);
		if (accA && c.accA)
			error += abs(accA - c.accA);
		// Use a huge error value for multisample mismatch.
		// Not sure this is the best solution.
		if (samples != c.samples)
			error += 1000;

		if (error < bestError) {
			bestError = error;
			best = static_cast<int>(p - choices.begin());
		}
	}

	return best;
} // DrawingSurfaceConfig::match

// are two surface configs exactly the same?
bool
DrawingSurfaceConfig::equal(const DrawingSurfaceConfig &config) const
{
	if (
#if defined(__X11__)
	    visID == config.visID &&
#  if defined(GLX_VERSION_1_3)
	    fbcID == config.fbcID &&
#  endif
#elif defined(__WIN__)
	    pfdID == config.pfdID &&
#elif defined(__AGL__)
	    pfID == config.pfID &&
#else
#  error "what's the config ID?"
#endif
	    canRGBA == config.canRGBA &&
	    canCI == config.canCI &&
	    bufSize == config.bufSize &&
	    level == config.level &&
	    db == config.db &&
	    stereo == config.stereo &&
	    aux == config.aux &&
	    r == config.r &&
	    g == config.g &&
	    b == config.b &&
	    a == config.a &&
	    z == config.z &&
	    s == config.s &&
	    accR == config.accR &&
	    accG == config.accG &&
	    accB == config.accB &&
	    accA == config.accA &&
	    samples == config.samples &&
	    canWindow == config.canWindow &&
#if defined(__X11__)
	    canPixmap == config.canPixmap &&
#if defined(GLX_VERSION_1_3)
	    canPBuffer == config.canPBuffer &&
	    maxPBufferWidth == config.maxPBufferWidth &&
	    maxPBufferHeight == config.maxPBufferHeight &&
	    maxPBufferPixels == config.maxPBufferPixels &&
#endif
#endif
	    canWinSysRender == config.canWinSysRender &&
	    fast == config.fast &&
	    conformant == config.conformant &&
	    transparent == config.transparent &&
	    transR == config.transR &&
	    transG == config.transG &&
	    transB == config.transB &&
	    transA == config.transA &&
	    transI == config.transI
	    )
		return true;
	else
		return false;
}

} // namespace GLEAN
