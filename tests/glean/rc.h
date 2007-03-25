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




// rc.h:  utilities for manipulating rendering contexts

#ifndef __rc_h__
#define __rc_h__

#include "glwrap.h"

namespace GLEAN {

class WindowSystem;		// Forward and mutually-recursive references
class DrawingSurfaceConfig;

class RenderingContext {
    public:
	RenderingContext(WindowSystem& ws, DrawingSurfaceConfig& c,
		RenderingContext* share = 0, bool direct = true);
	~RenderingContext();

	// Exceptions:
	struct Error { };		// Base class for all errors.

	// Members:
	WindowSystem* winSys;		// Window system that owns this context.

#   if defined(__X11__)
	GLXContext rc;
#   elif defined(__WIN__)
	::HGLRC rc;
#   elif defined(__AGL__)
	::AGLContext rc;
#   endif

}; // class RenderingContext

} // namespace GLEAN

#endif // __rc_h__
