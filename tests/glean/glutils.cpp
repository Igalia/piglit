// BEGIN_COPYRIGHT -*- glean -*-
// 
// Copyright (C) 1999, 2000  Allen Akin   All Rights Reserved.
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




// glutils.cpp:  frequently-used OpenGL operations

#define GLX_GLXEXT_PROTOTYPES
#include <stdlib.h>
#include "glwrap.h"
#include "environ.h"
#include "lex.h"
#include "glutils.h"
#if defined(__X11__) || defined(__AGL__)
#   include <dlfcn.h>
#endif
#if defined(__AGL__)
#   include <cstring>
#endif
#include "piglit-util-gl.h"

namespace GLEAN {

namespace GLUtils {

///////////////////////////////////////////////////////////////////////////////
// useScreenCoords:  Map object coords directly to screen coords.
///////////////////////////////////////////////////////////////////////////////
void
useScreenCoords(int windowW, int windowH) {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, windowW, 0, windowH, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glViewport(0, 0, windowW, windowH);
	glTranslatef(0.375, 0.375, 0.0);
} // useScreenCoords

///////////////////////////////////////////////////////////////////////////////
// haveExtensions:  See if the current rendering context supports a given
//	set of extensions.
///////////////////////////////////////////////////////////////////////////////
bool
haveExtensions(const char* required) {
	const char* available = reinterpret_cast<const char*>
		(glGetString(GL_EXTENSIONS));

	if (!required)
		return true;
	if (!available)
		return false;

	bool haveAll = true;
	Lex lRequired(required);
	for (lRequired.next(); lRequired.token != Lex::END; lRequired.next()) {
		if (lRequired.token != Lex::ID)
			continue;
		bool haveOne = false;
		Lex lAvailable(available);
		for (lAvailable.next(); lAvailable.token != Lex::END;
		    lAvailable.next())
			if (lAvailable.token == Lex::ID
			  && lAvailable.id == lRequired.id) {
				haveOne = true;
				break;
			}
		haveAll &= haveOne;
		if (!haveAll)
			break;
	}

	return haveAll;
} // haveExtensions


float
getVersion()
{
   const GLubyte *version = glGetString(GL_VERSION);
   // we rely on atof() stopping parsing at whitespace
   return atof((const char *) version);
}

///////////////////////////////////////////////////////////////////////////////
// logGLErrors: Check for OpenGL errors and log any that have occurred.
///////////////////////////////////////////////////////////////////////////////
void
logGLErrors(Environment& env) {
	GLenum err;
	while ((err = glGetError()))
		env.log << "\tOpenGL error: " << gluErrorString(err) << '\n';
} // logGLErrors

} // namespace GLUtils

} // namespace GLEAN
