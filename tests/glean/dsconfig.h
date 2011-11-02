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




// dsconfig.h:  Drawing surface configuration utilities

// This class abstracts the basic characteristics of drawing surfaces
// (size, depth, ancillary buffers, etc.) and operations on them.  It
// serves as a wrapper for X11 Visual and FBConfig information on
// X11-based systems, and PixelFormatDescriptor information on
// Win32-based systems.


#ifndef __dsconfig_h__
#define __dsconfig_h__

#include <string>
#include <vector>
#include "glwrap.h"

using namespace std;

namespace GLEAN {

class DrawingSurfaceConfig {
    public:

	// Constructors/Destructor:

#   if defined(__X11__)
	DrawingSurfaceConfig(::Display* dpy, ::XVisualInfo* pvi);
#     if defined(GLX_VERSION_1_3)
	DrawingSurfaceConfig(::Display* dpy, ::GLXFBConfig* pfbc);
#     endif
#   elif defined(__WIN__)
	DrawingSurfaceConfig(int id, ::PIXELFORMATDESCRIPTOR *ppfd);
#   elif defined(__BEWIN__)
	DrawingSurfaceConfig();
#	elif defined(__AGL__)
	DrawingSurfaceConfig(int id, ::AGLPixelFormat pfd);
#   endif

	DrawingSurfaceConfig(string& s);	// s is a canonical description

	// Exceptions:

	struct Error { };		// Base class for errors.
	struct Syntax: public Error {	// Syntax error in constructor string.
		const char* err;
		int position;
		Syntax(const char* e, int p) {
			err = e;
			position = p;
		}
	};

	// Attributes:

#   if defined(__X11__)
	::XVisualInfo* vi;		// XVisualInfo pointer
	::XID visID;			// Visual ID.
#     if defined(GLX_VERSION_1_3)
	::GLXFBConfig* fbc;
	::XID fbcID;			// Framebuffer Config ID.
#     endif
#   elif defined(__WIN__)
	::PIXELFORMATDESCRIPTOR pfd;
	int pfdID;
#   elif defined(__AGL__)
	AGLPixelFormat    pf;
	int 			pfID;
#	endif

	bool canRGBA;			// Can be used with RGBA contexts.

	bool canCI;			// Can be used with color index
					// contexts.

	int bufSize;			// Total depth of color buffer.

	int level;			// Framebuffer level.
					// (<0 for underlay, 0 for main,
					// >0 for overlay)

	bool db;			// True if double buffered.

	bool stereo;			// True if stereo-capable.

	int aux;			// Number of aux color buffers.

	int r;				// Depth of red channel.

	int g;				// Depth of green channel.

	int b;				// Depth of blue channel.

	int a;				// Depth of alpha channel.

	int z;				// Depth of ``z'' (depth) buffer.

	int s;				// Depth of stencil buffer.

	int accR;			// Depth of accum buf red channel.

	int accG;			// Depth of accum buf green channel.

	int accB;			// Depth of accum buf blue channel.

	int accA;			// Depth of accum buf alpha channel.

	int samples;                    // Number of samples per pixel.
					// Zero indicates a non-ms config.

	bool canWindow;			// True if can be used for windows.

#   if defined(__X11__)
	bool canPixmap;			// True if can be used for pixmaps.
#     if defined(GLX_VERSION_1_3)
	bool canPBuffer;		// True if can be used for pbuffers.

	int maxPBufferWidth;		// Maximum width of PBuffer that
					// may be created with this config.

	int maxPBufferHeight;		// Maximum height of PBuffer that
					// may be created with this config.

	int maxPBufferPixels;		// Maximum size (in pixels) of
					// PBuffer that may be created with
					// this config.
#     endif
#   endif

	bool canWinSysRender;		// True if the native window system
					// can render to a drawable created
					// with this config.

	bool fast;			// True if config is probably
					// hardware accelerated.  (On GLX,
					// it must not be marked ``slow.'')

	bool conformant;		// True if config is advertised as
					// conforming to the OpenGL spec.

	bool transparent;		// True if config has some pixel value
					// that is transparent (e.g., for
					// overlays).

	int transR;			// Transparent color red value.

	int transG;			// Transparent color green value.

	int transB;			// Transparent color blue value.

	int transA;			// Transparent color alpha value.

	int transI;			// Transparent color index value.

	// Utilities:

	void zeroFields();

	string canonicalDescription();
		// Return a string containing all the attributes in a
		// drawing surface configuration.  This allows the config
		// to be stored and recreated (essentially for use by
		// configuration-matching algorithms in test result
		// comparisons).

	string conciseDescription();
		// Return a description string that elides default
		// attribute values and expresses some attributes in
		// compressed form.  Intended to be more easily readable
		// for humans than the canonical description, at the
		// cost of some ambiguity.

	int match(vector<DrawingSurfaceConfig*>& choices);
		// Find the index of the config from ``choices'' that most
		// closely matches the config specified by ``*this''. 
		// The matching scheme is heuristic, but intended to
		// be good enough that test results for configs on one
		// machine may be compared with test results for
		// configs on another machine.

        bool equal(const DrawingSurfaceConfig &config) const;

}; // class DrawingSurfaceConfig

} // namespace GLEAN

#endif // __dsconfig_h__
