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




// rc.cpp:  implementation of rendering context utilities

#include <iostream>
#include <algorithm>
#include "rc.h"
#include "dsconfig.h"
#include "winsys.h"

namespace {

#if defined (__WIN__)

// XXX 
// wglCreateContext requires a handle to a device context.
// The ctor of RenderingContext doesn't know which window
// it is creating a surface for, only what the pixelformat
// of that window is. The hDC passed to wglCreateContext
// doesn't have to be the same as the one use in SwapBuffers
// or wglMakeCurrent, their pixelformats have to be the
// same though. A limitation is that the pixelformat of
// a window can only be set once. That is why a 
// temporary window is created.


HGLRC create_context(GLEAN::DrawingSurfaceConfig &c)
{
	HWND hwnd = CreateWindow("STATIC","temp",WS_POPUP,
							 CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,
							 0,0,GetModuleHandle(NULL),0);

	if (!hwnd)
		return 0;

	HDC hDC = GetDC(hwnd);
	if (!hDC)
		return 0;

	PIXELFORMATDESCRIPTOR pfd; 

	if (!SetPixelFormat(hDC,c.pfdID,&pfd))
	{
		ReleaseDC(hwnd,hDC);
		DestroyWindow(hwnd);
		return 0;
	}
	
	HGLRC rc = wglCreateContext(hDC);
	if (!rc)
		return 0;

	ReleaseDC(hwnd,hDC);
	DestroyWindow(hwnd);

	return rc;
}

#endif

} // anonymous namespace


namespace GLEAN {

///////////////////////////////////////////////////////////////////////////////
// Constructors
///////////////////////////////////////////////////////////////////////////////
RenderingContext::RenderingContext(WindowSystem& ws, DrawingSurfaceConfig& c,
    RenderingContext* share, bool direct) {

	// Link back to enclosing window system, so as to simplify bookkeeping:
	winSys = &ws;
	ws.contexts.push_back(this);

#   if defined(__X11__)

#	if defined(GLX_VERSION_1_3)

	if (ws.GLXVersMajor == 1 && ws.GLXVersMinor < 3)
		goto legacyMethod;
	// XXX Need GLX 1.3 rc constructor code
	// For now, just fall through.

#	endif

legacyMethod:
	// Create the rendering context:
	rc = glXCreateContext(winSys->dpy, c.vi, (share? share->rc: 0),
		direct? True: False);
	if (!rc)
		throw Error();
	// XXX Ideally, we would deal with X11 and GLX errors here, too
	// (Badmatch, BadValue, GLXBadContext, BadAlloc)

#   elif defined(__WIN__)

	rc = create_context(c);
	if (!rc)
		throw Error();
#   elif defined(__AGL__)
	rc = aglCreateContext(c.pf, NULL);
	if(rc == NULL) 
		throw Error();
#   endif

} // RenderingContext::RenderingContext

///////////////////////////////////////////////////////////////////////////////
// Destructors
///////////////////////////////////////////////////////////////////////////////

RenderingContext::~RenderingContext() {
	remove(winSys->contexts.begin(), winSys->contexts.end(), this);
#   if defined(__X11__)
#	if defined(GLX_VERSION_1_3)
		if (winSys->GLXVersMajor == 1 && winSys->GLXVersMinor < 3)
			goto legacyMethod;
		// XXX Need to write GLX 1.3 rc destructor
		// For now, just fall through.
#       endif
legacyMethod:
		glXDestroyContext(winSys->dpy, rc);
#   elif defined(__WIN__)
		wglDeleteContext(rc);
#   endif
} // RenderingContext::~RenderingContext


} // namespace GLEAN
