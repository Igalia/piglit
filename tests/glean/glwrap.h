// BEGIN_COPYRIGHT
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




// Microsoft's version of gl.h invokes macros that are defined in
// windows.h.  To avoid a conditional #include <windows.h> in
// every file, we wrap gl.h with the proper conditions here, and
// have our source files #include "glwrap.h" instead.

// As a bonus we ensure that all declarations for GLU are included,
// and on X11-based systems, we cover X11 and GLX as well.  This
// should cover nearly everything needed by a typical glean test.

// It's unfortunate that both Windows and Xlib are so casual about
// polluting the global namespace.  The problem isn't easily resolved,
// even with the use of C++ namespace directives, because (a) macros
// in the include files refer to unqualified global variables, and (b)
// preprocessor macros themselves aren't affected by namespaces.


#ifndef __glwrap_h__
#define __glwrap_h__

#include "../util/glew.h"

#if defined(__WIN__)
#  include <windows.h>
#  include <GL/glu.h>
#elif defined(__X11__)
#  include <GL/glx.h>
   // glx.h covers Xlib.h and gl.h, among others
#  include <GL/glu.h>
#elif defined(__AGL__)
#  include <Carbon/Carbon.h>
#  include <OpenGL/glu.h>
#  include <OpenGL/glext.h>
#  include <AGL/agl.h>
#  include <AGL/aglRenderers.h>
#  if !defined(sinf)
#      define sinf sin
#      define cosf cos
#      define sqrtf sqrt
#  endif
#else
#  error "Improper window system configuration; must be __WIN__ or __X11__."
#endif

typedef unsigned short GLhalfARB;

#endif // __glwrap_h__
