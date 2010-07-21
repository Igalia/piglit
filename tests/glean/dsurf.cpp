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




// dsurf.cpp:  implementation of drawing surface utilities

#include <iostream>
#include <algorithm>
#include "dsurf.h"
#include "dsconfig.h"
#include "winsys.h"

namespace {

#if defined(__X11__)

Colormap
ChooseColormap(Display* dpy, XVisualInfo* vi) {
	// We could be polite here and search for a standard colormap,
	// but the normal mode of operation should be that glean is
	// running alone, so there doesn't seem to be much point in sharing.

	return XCreateColormap(dpy, RootWindow(dpy, vi->screen),
		vi->visual, AllocNone);
} // ChooseColormap

#endif

} // anonymous namespace

namespace GLEAN {

///////////////////////////////////////////////////////////////////////////////
// Constructors
///////////////////////////////////////////////////////////////////////////////
DrawingSurface::DrawingSurface(WindowSystem& ws, DrawingSurfaceConfig& c) {
	// Link back to enclosing window system, so as to simplify bookkeeping:
	winSys = &ws;
	ws.surfaces.push_back(this);

	// Save pointer to configuration information:
	config = &c;
} // DrawingSurface::DrawingSurface

Window::Window(WindowSystem& ws, DrawingSurfaceConfig& c, int w, int h,
    int x, int y):
    DrawingSurface(ws, c) {

#if defined(__X11__)

#if defined(GLX_VERSION_1_3)
	if (ws.GLXVersMajor == 1 && ws.GLXVersMinor < 3)
		goto legacyMethod;
// XXX Need GLX 1.3 window-creation code.  For now, just fall through.
#endif

legacyMethod:
	// XXX There's basically no error-handling code here.
	// See XErrorHandler().

	// Create the window:
	XSetWindowAttributes xswa;
	XWMHints *wmHints;
	xswa.background_pixmap = None;
	xswa.border_pixel = 0;
	xswa.colormap = ChooseColormap(winSys->dpy, config->vi);
	xswa.event_mask = StructureNotifyMask;
	xWindow = XCreateWindow(winSys->dpy,
		RootWindow(winSys->dpy, config->vi->screen),
		x, y, w, h,
		0,
		config->vi->depth,
		InputOutput,
		config->vi->visual,
		CWBackPixmap|CWBorderPixel|CWColormap|CWEventMask,
		&xswa);

	// Set attributes for the benefit of the window manager:
	XSizeHints sizeHints;
	sizeHints.width = w;
	sizeHints.height = h;
	sizeHints.x = x;
	sizeHints.y = y;
	sizeHints.flags = USSize | USPosition;
	XSetStandardProperties(winSys->dpy, xWindow, "glean", "glean",
		None, 0, 0, &sizeHints);

	// Try to prevent test window from stealing focus
	wmHints = XAllocWMHints();
	wmHints->flags |= InputHint;
	wmHints->input = False;

	XSetWMHints(winSys->dpy, xWindow, wmHints);

	XFree(wmHints);

	// Map the window and wait for it to appear:
	XMapWindow(winSys->dpy, xWindow);
	XEvent event;
	for (;;) {
		XNextEvent(winSys->dpy, &event);
		if (event.type == MapNotify && event.xmap.window == xWindow)
			break;
	}


#elif defined(__WIN__)
	// XXX There's basically no error-handling code here.
	// create the window
	RECT r;
	int style = WS_POPUP | WS_CAPTION | WS_BORDER;

	r.left = x;
	r.top = y;
	r.right = r.left + w;
	r.bottom = r.top + h;
	AdjustWindowRect(&r,style,FALSE);
		
	hWindow = CreateWindow("glean","glean",
		style | WS_VISIBLE,
		r.left,r.top,r.right - r.left,r.bottom - r.top,
		NULL, NULL,
		GetModuleHandle(NULL),
		NULL);

    if (!hWindow)
        return;

	hDC = GetDC(hWindow);

	PIXELFORMATDESCRIPTOR pfd;
	SetPixelFormat(hDC,config->pfdID,&pfd);
	
#elif defined(__BEWIN__)

	tWindow = new GLTestWindow (BRect(x,y, x+w, y+h), "GL Test Window");
	tWindow->Show();

#elif defined(__AGL__)
	Rect	 r ;
	
	//we need some extra room for the menu bar
	r.left = x+16;
	r.top = y+20;
	if (h < 20)
		r.bottom = r.top + 32;
	else
		r.bottom = r.top+w;
		
	if (w < 20)
		r.right = r.left + 32;
	else
		r.right = r.left + w;

	macWindow = NewCWindow(nil, &r, (unsigned char*)"glean", true, documentProc, 
						(WindowPtr) -1, false, 0);

	SetPortWindowPort(macWindow);

#endif
} // Window::Window

///////////////////////////////////////////////////////////////////////////////
// Destructors
///////////////////////////////////////////////////////////////////////////////

void
DrawingSurface::commonDestructorCode() {
	remove(winSys->surfaces.begin(), winSys->surfaces.end(), this);
} // DrawingSurface::commonDestructorCode

Window::~Window() {

#if defined(__X11__)
	XDestroyWindow(winSys->dpy, xWindow);
#elif defined(__WIN__)
	ReleaseDC(hWindow,hDC);
	DestroyWindow(hWindow);

#elif defined(__BEWIN__)

	tWindow->Lock();
	tWindow->Quit();

#elif defined(__AGL__)
//	::CloseWindow(macWindow);
#endif

} // Window::~Window

///////////////////////////////////////////////////////////////////////////////
// Utilities
///////////////////////////////////////////////////////////////////////////////
void
Window::swap() {
#   if defined(__X11__)
	glXSwapBuffers(winSys->dpy, xWindow);
#   elif defined(__WIN__)
	SwapBuffers(hDC);
#   elif defined(__BEWIN__)
	tWindow->SwapBuffers();
#   elif defined(__AGL__)
	aglSwapBuffers(aglGetCurrentContext());
#   endif
} // Window::swap

#if defined(__WIN__)

///////////////////////////////////////////////////////////////////////////////
// Window procedure
///////////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK 
Window::WindowProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	switch (message)
    {
		default :
			return DefWindowProc(hwnd, message, wParam, lParam);

	}

	return FALSE;
}

#endif


#if defined(__BEWIN__)

GLTestWindow::GLTestWindow(BRect frame, const char *title) :
	BWindow(frame, title, B_TITLED_WINDOW, B_NOT_RESIZABLE)
{
	/* right now we just create all the buffers we can */
	tView = new BGLView(Bounds(), "glean_view", B_FOLLOW_ALL, B_WILL_DRAW,
		BGL_RGB | BGL_DOUBLE | BGL_DEPTH | BGL_ALPHA | BGL_STENCIL | BGL_ACCUM );
	AddChild(tView);
}

void
GLTestWindow::SwapBuffers()
{
	tView->SwapBuffers();
}
#endif
} // namespace GLEAN
