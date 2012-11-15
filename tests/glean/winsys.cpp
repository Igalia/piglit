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




// winsys.cpp:  implementation of window-system services class

#include <iostream>
#include "options.h"
#include "winsys.h"
#include "dsconfig.h"
#include "dsfilt.h"
#include "dsurf.h"
#include "rc.h"

using namespace std;

namespace GLEAN {


///////////////////////////////////////////////////////////////////////////////
// Constructors
///////////////////////////////////////////////////////////////////////////////
#if defined(__X11__)
WindowSystem::WindowSystem(Options& o) {
	// Open the X11 display:
	dpy = XOpenDisplay(o.dpyName.c_str());
	if (!dpy)
		throw CantOpenDisplay();

	// Verify that GLX is supported:
	int error_base, event_base;
	if (glXQueryExtension(dpy, &error_base, &event_base) == False)
		throw NoOpenGL();

	// Record version numbers for later use:
	if (glXQueryVersion(dpy, &GLXVersMajor, &GLXVersMinor) == False)
		throw Error();	// this should never happen :-)

	// Get the list of raw XVisualInfo structures:
	XVisualInfo vit;
	vit.screen = DefaultScreen(dpy);
	int n;
	vip = XGetVisualInfo(dpy, VisualScreenMask, &vit, &n);

	// Construct a vector of DrawingSurfaceConfigs corresponding to the
	// XVisualInfo structures that indicate they support OpenGL:
	vector<DrawingSurfaceConfig*> glxv;
	for (int i = 0; i < n; ++i) {
		int supportsOpenGL;
		glXGetConfig(dpy, &vip[i], GLX_USE_GL, &supportsOpenGL);
		if (supportsOpenGL)
			glxv.push_back(new DrawingSurfaceConfig (dpy, &vip[i]));
	}

	// Filter the basic list of DrawingSurfaceConfigs according to
	// constraints provided by the user.  (This makes it convenient
	// to run tests on just a subset of all available configs.)
	DrawingSurfaceFilter f(o.visFilter);	// may throw an exception!
	surfConfigs = f.filter(glxv, o.maxVisuals);
} // WindowSystem::WindowSystem

#elif defined(__WIN__)
WindowSystem::WindowSystem(Options& o) {
	// register an window class
	WNDCLASS	wc;

	wc.style = CS_OWNDC;
    wc.lpfnWndProc = Window::WindowProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = GetModuleHandle(NULL);
    wc.hIcon = LoadIcon(wc.hInstance, "glean");
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
    wc.lpszMenuName =  NULL;
	wc.lpszClassName = "glean";

    if (!RegisterClass(&wc))
		throw Error();
		
	
	HDC hDC = GetDC(GetDesktopWindow());
	
	PIXELFORMATDESCRIPTOR pfd;
	int n = DescribePixelFormat(hDC,0,sizeof(pfd),0);
	
	vector<DrawingSurfaceConfig*> glpf;

	for (int i = 1;i <= n;++i) {
		DescribePixelFormat(hDC,i,sizeof(pfd),&pfd);
		
		glpf.push_back(new DrawingSurfaceConfig(i,&pfd));
	}

	ReleaseDC(GetDesktopWindow(),hDC);

	// Filter the basic list of DrawingSurfaceConfigs according to
	// constraints provided by the user.  (This makes it convenient
	// to run tests on just a subset of all available configs.)
	DrawingSurfaceFilter f(o.visFilter);	// may throw an exception!
	surfConfigs = f.filter(glpf, o.maxVisuals);
}

#elif defined(__BEWIN__)
WindowSystem::WindowSystem(Options& o) {
	//cout << "Implement Me!  WindowSystem::WindowSystem(Options& o)\n";

	theApp = new BApplication("application/x-AJH-glean");

	/* for BeOS, we just stack the current config onto the vector so */
	/* there is at least one thing to iterate over  */
	vector<DrawingSurfaceConfig*> glconfigs;
	glconfigs.push_back(new DrawingSurfaceConfig());

	DrawingSurfaceFilter f(o.visFilter);	// may throw an exception!
	surfConfigs = f.filter(glconfigs, o.maxVisuals);

}

#elif defined(__AGL__)
WindowSystem::WindowSystem(Options& o) {
	GDHandle	mainGD;
									//HW/SW		Depth		Reserved
	GLint		testTypes[][3] 	= 	{	
									{AGL_ACCELERATED, 16, 0 },
									{AGL_ACCELERATED, 16, 0 },
									{AGL_ACCELERATED, 16, 0 },
									{0, 16, 0 },
									{0, 16, 0 },
									{0, 0, 0 }
									};
	GLint		testAttrib[][10]= 	{
									{ AGL_RGBA, 	AGL_DOUBLEBUFFER, 	AGL_ACCELERATED, 	16, 			AGL_NONE,	AGL_NONE,	AGL_NONE,	AGL_NONE,	AGL_NONE,	AGL_NONE},
									{ AGL_RGBA, 	AGL_ACCELERATED, 	AGL_DOUBLEBUFFER, 	AGL_DEPTH_SIZE, 16, 		AGL_NONE,	AGL_NONE,	AGL_NONE,	AGL_NONE,	AGL_NONE},
									{ AGL_RGBA, 	AGL_DOUBLEBUFFER, 	AGL_NONE,			AGL_NONE, 		AGL_NONE,	AGL_NONE,	AGL_NONE,	AGL_NONE,	AGL_NONE,	AGL_NONE},
									{ AGL_RENDERER_ID, AGL_RENDERER_GENERIC_ID,	AGL_RGBA, 	AGL_DOUBLEBUFFER, AGL_NONE,	AGL_NONE,	AGL_NONE,	AGL_NONE,	AGL_NONE,	AGL_NONE},
									{ AGL_RENDERER_ID, AGL_RENDERER_GENERIC_ID,	AGL_RGBA, 	AGL_DOUBLEBUFFER, AGL_DEPTH_SIZE, 16 ,	AGL_NONE,	AGL_NONE,	AGL_NONE,	AGL_NONE}
									};
	AGLPixelFormat	pf; 
	GLint		index = 0;

	mainGD = GetMainDevice();
	if (!mainGD)
		throw CantOpenDisplay();

	// Construct a vector of DrawingSurfaceConfigs corresponding to the
	// returned pixel formats
	vector<DrawingSurfaceConfig*> glpf;

	while (testTypes[index][1] != 0)
	{
		pf = aglChoosePixelFormat(&mainGD, 1, testAttrib[index]);
		if ( (pf == NULL) && ( testTypes[index][0] == 0) )
		{
			testAttrib[index][1] = 0x30300;
			pf = aglChoosePixelFormat(&mainGD, 1, testAttrib[index]);
		}
		if (pf != NULL) glpf.push_back(new DrawingSurfaceConfig (index+1, pf));
		
		index++;
	}

	// Filter the basic list of DrawingSurfaceConfigs according to
	// constraints provided by the user.  (This makes it convenient
	// to run tests on just a subset of all available configs.)
	DrawingSurfaceFilter f(o.visFilter);	// may throw an exception!
	surfConfigs = f.filter(glpf, o.maxVisuals);
}

#endif

///////////////////////////////////////////////////////////////////////////////
// Destructors
///////////////////////////////////////////////////////////////////////////////
#if defined(__X11__)
WindowSystem::~WindowSystem() {
	XFree(vip);
} // WindowSystem:: ~WindowSystem

#elif defined(__WIN__)
WindowSystem::~WindowSystem() {
}
#elif defined(__BEWIN__)
WindowSystem::~WindowSystem() {
	delete theApp;
} // WindowSystem:: ~WindowSystem
#elif defined(__AGL__)
WindowSystem::~WindowSystem() {
}
#endif

///////////////////////////////////////////////////////////////////////////////
// makeCurrent and friends - binding contexts and drawing surfaces
///////////////////////////////////////////////////////////////////////////////

bool
WindowSystem::makeCurrent() {
#   if defined(__X11__)
#	if defined(GLX_VERSION_1_3)
	    // XXX Need to write GLX 1.3 MakeCurrent code
#	endif
	    return glXMakeCurrent(dpy, None, 0);
#   elif defined(__WIN__)
		return wglMakeCurrent(0,0);
#   elif defined(__AGL__)
		return aglSetCurrentContext(NULL);
#   endif
} // WindowSystem::makeCurrent

bool
WindowSystem::makeCurrent(RenderingContext& r, Window& w) {
#   if defined(__X11__)
#	if defined(GLX_VERSION_1_3)
	    // XXX Need to write GLX 1.3 MakeCurrent code
#	endif
	    return glXMakeCurrent(dpy, w.xWindow, r.rc);
#   elif defined(__WIN__)
		return wglMakeCurrent(w.get_dc(),r.rc);
#   elif defined(__AGL__)
		if (GL_FALSE == aglSetDrawable(r.rc, (AGLDrawable) GetWindowPort (w.macWindow)))
			return GL_FALSE;
		if (GL_FALSE == aglSetCurrentContext(r.rc))
			return GL_FALSE;
		return true;
#   endif
} // WindowSystem::makeCurrent

} // namespace GLEAN
