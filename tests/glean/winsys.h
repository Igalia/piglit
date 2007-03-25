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




// winsys.h:  facade for common window-system operations

// This class and related classes provide window system operations
// that are sufficient to support most basic rendering tests.  These
// operations include initializing the window system, creating and
// destroying windows and rendering contexts, selecting pixel
// configurations, etc.

// Tests using this set of classes for all window system services are
// ``portable'' in a useful sense.  Not all tests are portable,
// however; in particular, tests of window-system-specific
// functionality must execute window system commands directly.  Such
// tests may require access to class members that would ideally be
// private; for example, the X11 Display pointer.  Thus most members
// of this class are public.



#ifndef __winsys_h__
#define __winsys_h__

using namespace std;

#include <string>
#include <vector>
#include "glwrap.h"

namespace GLEAN {

class DrawingSurface;		// Forward and mutually-recursive references.
class Window;
class DrawingSurfaceConfig;
class RenderingContext;
class Options;

class WindowSystem {
    public:
    	// Constructors/Destructor:

	WindowSystem(Options& o);
	~WindowSystem();

	// Exceptions:

	struct Error { };		// Base class for window system errors.
	struct CantOpenDisplay: public Error {	// Can't initialize display.
	};
	struct NoOpenGL: public Error {	// Missing GLX, WGL, etc.
	};

	// Utilities:

	bool makeCurrent();		// Remove context/surface binding.
	bool makeCurrent(RenderingContext& r, Window& w);
					// Bind given context and surface.
	void quiesce();			// Wait for system to go idle.

	// State information:

	vector<DrawingSurfaceConfig*> surfConfigs;
				// All available drawing surface configurations.
	vector<DrawingSurface*> surfaces;
				// All currently-active surfaces.
	vector<RenderingContext*> contexts;
				// All currently-active rendering contexts.

#   if defined(__X11__)
	Display* dpy;		// Pointer to X11 Display structure.
	
	int GLXVersMajor;	// GLX major version number.
	int GLXVersMinor;	// GLX minor version number.

	XVisualInfo* vip;	// Array of raw XVisualInfo structures.

#   elif defined(__WIN__)

#   elif defined(__BEWIN__)
	BApplication *theApp;

#   endif

}; // class WindowSystem

} // namespace GLEAN

#endif // __winsys_h__
