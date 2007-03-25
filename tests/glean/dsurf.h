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




// dsurf.h:  utilities for manipulating drawing surfaces

#ifndef __dsurf_h__
#define __dsurf_h__

#include "glwrap.h"

namespace GLEAN {

class WindowSystem;		// Forward and mutually-recursive references
class DrawingSurfaceConfig;

class DrawingSurface {
    public:
	DrawingSurface(WindowSystem& ws, DrawingSurfaceConfig& c);
	virtual ~DrawingSurface()  { }

	WindowSystem* winSys;		// Window system that owns this surface.
	DrawingSurfaceConfig* config;	// Configuration of this surface.

    protected:
	void commonDestructorCode();

}; // class DrawingSurface

/* we have to create a utility test window for BeOS */
#	if defined(__BEWIN__)
class GLTestWindow : public BWindow {
public:
	GLTestWindow(BRect frame, const char *title);
	void		SwapBuffers();
//	void		SwapBuffers( bool vSync );

private:
	BGLView	*tView;
};
#	endif


class Window: public DrawingSurface {
    public:
    	Window(WindowSystem& ws, DrawingSurfaceConfig& c, int w, int h,
	    int x = 10, int y = 10);
	~Window();

	// Utilities:

	void swap();

	// XXX Add constructors for more specialized window creation --
	// for example, at a particular screen location, with a particular
	// parent window, etc. -- as needed by new tests.

#	if defined(__X11__)
		::Window xWindow;
#	elif defined(__WIN__)
		::HWND	 hWindow;
		::HDC	 hDC;

		::HDC get_dc() const {return hDC;}
		static LRESULT CALLBACK WindowProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam);

#	elif defined(__BEWIN__)
		GLTestWindow	*tWindow;
#	elif defined(__AGL__)
		::WindowRef	macWindow;
#	endif
}; // class Window

} // namespace GLEAN

#endif // __dsurf_h__
