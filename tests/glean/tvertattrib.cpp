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


// tvertattrib.cpp:  Test vertex attribute functions.
//
// Indexed vertex attributes may either alias with conventional attributes
// or name a separate set of generic attributes.  The following extensions/
// versions are tested (and whether aliasing is allowed):
//   GL_NV_vertex_program (aliasing required)
//   GL_ARB_vertex_program (aliasing optional)
//   GL_ARB_vertex_shader (aliasing disallowed)
//   OpenGL 2.0 (aliasing disallowed)
//
// If either GL_ARB_vertex_shader or OpenGL 2.0 is supported, that means
// aliasing is required for GL_ARB_vertex_program too.
//
// We test both immediate mode and display list mode.
//
// Author: Brian Paul (brian.paul a t tungstengraphics.com)  October 2004


#include <stdlib.h>
#include <cassert>
#include <math.h>
#include "tvertattrib.h"
#include "glutils.h"

namespace GLEAN {

#define COPY1(DST, SRC) DST[0] = SRC[0]; DST[1] = 0.0F; DST[2] = 0.0F; DST[3] = 1.0F

#define COPY2(DST, SRC) DST[0] = SRC[0]; DST[1] = SRC[1]; DST[2] = 0.0F; DST[3] = 1.0F

#define COPY3(DST, SRC) DST[0] = SRC[0]; DST[1] = SRC[1]; DST[2] = SRC[2]; DST[3] = 1.0F

#define COPY4(DST, SRC) DST[0] = SRC[0]; DST[1] = SRC[1]; DST[2] = SRC[2]; DST[3] = SRC[3]

#define FLOAT_TO_BYTE(X)    ( (((GLint) (255.0F * (X))) - 1) / 2 )

#define FLOAT_TO_UBYTE(X)   ((GLubyte) (GLint) ((X) * 255.0F))

#define FLOAT_TO_SHORT(X)   ( (((GLint) (65535.0F * (X))) - 1) / 2 )

#define FLOAT_TO_USHORT(X)  ((GLushort) (GLint) ((X) * 65535.0F))

#define FLOAT_TO_INT(X)     ( (GLint) (2147483647.0 * (X)) )

#define FLOAT_TO_UINT(X)    ((GLuint) ((X) * 4294967295.0))


#define NUM_NV_ATTRIB_FUNCS 26
#define NUM_ARB_ATTRIB_FUNCS 36
#define NUM_2_0_ATTRIB_FUNCS 36

static const char *
AttribFuncNames[NUM_NV_ATTRIB_FUNCS + NUM_ARB_ATTRIB_FUNCS + NUM_2_0_ATTRIB_FUNCS] = {
	"glVertexAttrib1fNV",
	"glVertexAttrib2fNV",
	"glVertexAttrib3fNV",
	"glVertexAttrib4fNV",
	"glVertexAttrib1fvNV",
	"glVertexAttrib2fvNV",
	"glVertexAttrib3fvNV",
	"glVertexAttrib4fvNV",
	"glVertexAttrib1dNV",
	"glVertexAttrib2dNV",
	"glVertexAttrib3dNV",
	"glVertexAttrib4dNV",
	"glVertexAttrib1dvNV",
	"glVertexAttrib2dvNV",
	"glVertexAttrib3dvNV",
	"glVertexAttrib4dvNV",
	"glVertexAttrib1sNV",
	"glVertexAttrib2sNV",
	"glVertexAttrib3sNV",
	"glVertexAttrib4sNV",
	"glVertexAttrib1svNV",
	"glVertexAttrib2svNV",
	"glVertexAttrib3svNV",
	"glVertexAttrib4svNV",
	"glVertexAttrib4ubNV",
	"glVertexAttrib4ubvNV",

	"glVertexAttrib1fARB",
	"glVertexAttrib2fARB",
	"glVertexAttrib3fARB",
	"glVertexAttrib4fARB",
	"glVertexAttrib1fvARB",
	"glVertexAttrib2fvARB",
	"glVertexAttrib3fvARB",
	"glVertexAttrib4fvARB",
	"glVertexAttrib1dARB",
	"glVertexAttrib2dARB",
	"glVertexAttrib3dARB",
	"glVertexAttrib4dARB",
	"glVertexAttrib1dvARB",
	"glVertexAttrib2dvARB",
	"glVertexAttrib3dvARB",
	"glVertexAttrib4dvARB",
	"glVertexAttrib1sARB",
	"glVertexAttrib2sARB",
	"glVertexAttrib3sARB",
	"glVertexAttrib4sARB",
	"glVertexAttrib1svARB",
	"glVertexAttrib2svARB",
	"glVertexAttrib3svARB",
	"glVertexAttrib4svARB",
	"glVertexAttrib4NsvARB",
	"glVertexAttrib4NubARB",
	"glVertexAttrib4NubvARB",
	"glVertexAttrib4ubvARB",
	"glVertexAttrib4NbvARB",
	"glVertexAttrib4bvARB",
	"glVertexAttrib4NivARB",
	"glVertexAttrib4ivARB",
	"glVertexAttrib4NuivARB",
	"glVertexAttrib4uivARB",
	"glVertexAttrib4NusvARB",
	"glVertexAttrib4usvARB",

	"glVertexAttrib1f",
	"glVertexAttrib2f",
	"glVertexAttrib3f",
	"glVertexAttrib4f",
	"glVertexAttrib1fv",
	"glVertexAttrib2fv",
	"glVertexAttrib3fv",
	"glVertexAttrib4fv",
	"glVertexAttrib1d",
	"glVertexAttrib2d",
	"glVertexAttrib3d",
	"glVertexAttrib4d",
	"glVertexAttrib1dv",
	"glVertexAttrib2dv",
	"glVertexAttrib3dv",
	"glVertexAttrib4dv",
	"glVertexAttrib1s",
	"glVertexAttrib2s",
	"glVertexAttrib3s",
	"glVertexAttrib4s",
	"glVertexAttrib1sv",
	"glVertexAttrib2sv",
	"glVertexAttrib3sv",
	"glVertexAttrib4sv",
	"glVertexAttrib4Nsv"
	"glVertexAttrib4Nub",
	"glVertexAttrib4Nubv",
	"glVertexAttrib4ubv",
	"glVertexAttrib4Nbv",
	"glVertexAttrib4bv",
	"glVertexAttrib4Niv",
	"glVertexAttrib4iv",
	"glVertexAttrib4Nuiv",
	"glVertexAttrib4uiv",
	"glVertexAttrib4Nusv",
	"glVertexAttrib4usv"
};


// Set a vertex attribute with one of the many glVertexAttrib* functions.
// index = the vertex attribute
// v = the 4-element attribute value
// funcIndex = indicates which glVertexAttrib* function to use
// refOut = returns the value which should now be in the attribute register
//
// Yeah, calling getProcAddress every time isn't very efficient.  Oh well.
//
static void
SetAttrib(GLuint index, const GLfloat v[4], GLuint funcIndex, GLfloat refOut[4])
{
	switch (funcIndex) {
	// ** GLfloat-valued functions
#if defined(GL_NV_vertex_program)
	case 0:
		{
			PFNGLVERTEXATTRIB1FNVPROC f = (PFNGLVERTEXATTRIB1FNVPROC)
				GLUtils::getProcAddress("glVertexAttrib1fNV");
			f(index, v[0]);
			COPY1(refOut, v);
		}
		break;
	case 1:
		{
			PFNGLVERTEXATTRIB2FNVPROC f = (PFNGLVERTEXATTRIB2FNVPROC)
				GLUtils::getProcAddress("glVertexAttrib2fNV");
			f(index, v[0], v[1]);
			COPY2(refOut, v);
		}
		break;
	case 2:
		{
			PFNGLVERTEXATTRIB3FNVPROC f = (PFNGLVERTEXATTRIB3FNVPROC)
				GLUtils::getProcAddress("glVertexAttrib3fNV");
			f(index, v[0], v[1], v[2]);
			COPY3(refOut, v);
		}
		break;
	case 3:
		{
			PFNGLVERTEXATTRIB4FNVPROC f = (PFNGLVERTEXATTRIB4FNVPROC)
				GLUtils::getProcAddress("glVertexAttrib4fNV");
			f(index, v[0], v[1], v[2], v[3]);
			COPY4(refOut, v);
		}
		break;
	case 4:
		{
			PFNGLVERTEXATTRIB1FVNVPROC f = (PFNGLVERTEXATTRIB1FVNVPROC)
				GLUtils::getProcAddress("glVertexAttrib1fvNV");
			f(index, v);
			COPY1(refOut, v);
		}
		break;
	case 5:
		{
			PFNGLVERTEXATTRIB2FVNVPROC f = (PFNGLVERTEXATTRIB2FVNVPROC)
				GLUtils::getProcAddress("glVertexAttrib2fvNV");
			f(index, v);
			COPY2(refOut, v);
		}
		break;
	case 6:
		{
			PFNGLVERTEXATTRIB3FVNVPROC f = (PFNGLVERTEXATTRIB3FVNVPROC)
				GLUtils::getProcAddress("glVertexAttrib3fvNV");
			f(index, v);
			COPY3(refOut, v);
		}
		break;
	case 7:
		{
			PFNGLVERTEXATTRIB4FVNVPROC f = (PFNGLVERTEXATTRIB4FVNVPROC)
				GLUtils::getProcAddress("glVertexAttrib4fvNV");
			f(index, v);
			COPY4(refOut, v);
		}
		break;
	// ** GLdouble-valued functions
	case 8:
		{
			PFNGLVERTEXATTRIB1DNVPROC f = (PFNGLVERTEXATTRIB1DNVPROC)
				GLUtils::getProcAddress("glVertexAttrib1dNV");
			f(index, v[0]);
			COPY1(refOut, v);
		}
		break;
	case 9:
		{
			PFNGLVERTEXATTRIB2DNVPROC f = (PFNGLVERTEXATTRIB2DNVPROC)
				GLUtils::getProcAddress("glVertexAttrib2dNV");
			f(index, v[0], v[1]);
			COPY2(refOut, v);
		}
		break;
	case 10:
		{
			PFNGLVERTEXATTRIB3DNVPROC f = (PFNGLVERTEXATTRIB3DNVPROC)
				GLUtils::getProcAddress("glVertexAttrib3dNV");
			f(index, v[0], v[1], v[2]);
			COPY3(refOut, v);
		}
		break;
	case 11:
		{
			PFNGLVERTEXATTRIB4DNVPROC f = (PFNGLVERTEXATTRIB4DNVPROC)
				GLUtils::getProcAddress("glVertexAttrib4dNV");
			f(index, v[0], v[1], v[2], v[3]);
			COPY4(refOut, v);
		}
		break;
	case 12:
		{
			PFNGLVERTEXATTRIB1DVNVPROC f = (PFNGLVERTEXATTRIB1DVNVPROC)
				GLUtils::getProcAddress("glVertexAttrib1dvNV");
			GLdouble d[1];
			d[0] = v[0];
			f(index, d);
			COPY1(refOut, v);
		}
		break;
	case 13:
		{
			PFNGLVERTEXATTRIB2DVNVPROC f = (PFNGLVERTEXATTRIB2DVNVPROC)
				GLUtils::getProcAddress("glVertexAttrib2dvNV");
			GLdouble d[2];
			d[0] = v[0];
			d[1] = v[1];
			f(index, d);
			COPY2(refOut, v);
		}
		break;
	case 14:
		{
			PFNGLVERTEXATTRIB3DVNVPROC f = (PFNGLVERTEXATTRIB3DVNVPROC)
				GLUtils::getProcAddress("glVertexAttrib3dvNV");
			GLdouble d[3];
			d[0] = v[0];
			d[1] = v[1];
			d[2] = v[2];
			f(index, d);
			COPY3(refOut, v);
		}
		break;
	case 15:
		{
			PFNGLVERTEXATTRIB4DVNVPROC f = (PFNGLVERTEXATTRIB4DVNVPROC)
				GLUtils::getProcAddress("glVertexAttrib4dvNV");
			GLdouble d[4];
			d[0] = v[0];
			d[1] = v[1];
			d[2] = v[2];
			d[3] = v[3];
			f(index, d);
			COPY4(refOut, v);
		}
		break;
	// ** GLshort-valued functions
	case 16:
		{
			PFNGLVERTEXATTRIB1SNVPROC f = (PFNGLVERTEXATTRIB1SNVPROC)
				GLUtils::getProcAddress("glVertexAttrib1sNV");
			f(index, (GLshort) v[0]);
			refOut[0] = (GLfloat) (GLshort) v[0];
			refOut[1] = 0.0F;
			refOut[2] = 0.0F;
			refOut[3] = 1.0F;
		}
		break;
	case 17:
		{
			PFNGLVERTEXATTRIB2SNVPROC f = (PFNGLVERTEXATTRIB2SNVPROC)
				GLUtils::getProcAddress("glVertexAttrib2sNV");
			f(index, (GLshort) v[0], (GLshort) v[1]);
			refOut[0] = (GLfloat) (GLshort) v[0];
			refOut[1] = (GLfloat) (GLshort) v[1];
			refOut[2] = 0.0F;
			refOut[3] = 1.0F;
		}
		break;
	case 18:
		{
			PFNGLVERTEXATTRIB3SNVPROC f = (PFNGLVERTEXATTRIB3SNVPROC)
				GLUtils::getProcAddress("glVertexAttrib3sNV");
			f(index, (GLshort) v[0], (GLshort) v[1], (GLshort) v[2]);
			refOut[0] = (GLfloat) (GLshort) v[0];
			refOut[1] = (GLfloat) (GLshort) v[1];
			refOut[2] = (GLfloat) (GLshort) v[2];
			refOut[3] = 1.0F;
		}
		break;
	case 19:
		{
			PFNGLVERTEXATTRIB4SNVPROC f = (PFNGLVERTEXATTRIB4SNVPROC)
				GLUtils::getProcAddress("glVertexAttrib4sNV");
			f(index, (GLshort) v[0], (GLshort) v[1], (GLshort) v[2], (GLshort) v[3]);
			refOut[0] = (GLfloat) (GLshort) v[0];
			refOut[1] = (GLfloat) (GLshort) v[1];
			refOut[2] = (GLfloat) (GLshort) v[2];
			refOut[3] = (GLfloat) (GLshort) v[3];
		}
		break;
	case 20:
		{
			PFNGLVERTEXATTRIB1SVNVPROC f = (PFNGLVERTEXATTRIB1SVNVPROC)
				GLUtils::getProcAddress("glVertexAttrib1svNV");
			GLshort s[1];
			s[0] = (GLshort) v[0];
			f(index, s);
			refOut[0] = (GLfloat) (GLshort) v[0];
			refOut[1] = 0.0F;
			refOut[2] = 0.0F;
			refOut[3] = 1.0F;
		}
		break;
	case 21:
		{
			PFNGLVERTEXATTRIB2SVNVPROC f = (PFNGLVERTEXATTRIB2SVNVPROC)
				GLUtils::getProcAddress("glVertexAttrib2svNV");
			GLshort s[2];
			s[0] = (GLshort) v[0];
			s[1] = (GLshort) v[1];
			f(index, s);
			refOut[0] = (GLfloat) (GLshort) v[0];
			refOut[1] = (GLfloat) (GLshort) v[1];
			refOut[2] = 0.0F;
			refOut[3] = 1.0F;
		}
		break;
	case 22:
		{
			PFNGLVERTEXATTRIB3SVNVPROC f = (PFNGLVERTEXATTRIB3SVNVPROC)
				GLUtils::getProcAddress("glVertexAttrib3svNV");
			GLshort s[3];
			s[0] = (GLshort) v[0];
			s[1] = (GLshort) v[1];
			s[2] = (GLshort) v[2];
			f(index, s);
			refOut[0] = (GLfloat) (GLshort) v[0];
			refOut[1] = (GLfloat) (GLshort) v[1];
			refOut[2] = (GLfloat) (GLshort) v[2];
			refOut[3] = 1.0F;
		}
		break;
	case 23:
		{
			PFNGLVERTEXATTRIB4SVNVPROC f = (PFNGLVERTEXATTRIB4SVNVPROC)
				GLUtils::getProcAddress("glVertexAttrib4svNV");
			GLshort s[4];
			s[0] = (GLshort) v[0];
			s[1] = (GLshort) v[1];
			s[2] = (GLshort) v[2];
			s[3] = (GLshort) v[3];
			f(index, s);
			refOut[0] = (GLfloat) (GLshort) v[0];
			refOut[1] = (GLfloat) (GLshort) v[1];
			refOut[2] = (GLfloat) (GLshort) v[2];
			refOut[3] = (GLfloat) (GLshort) v[3];
		}
		break;
	// ** GLubyte-valued functions
	case 24:
		{
			PFNGLVERTEXATTRIB4UBNVPROC f = (PFNGLVERTEXATTRIB4UBNVPROC)
				GLUtils::getProcAddress("glVertexAttrib4ubNV");
			f(index, FLOAT_TO_UBYTE(v[0]), FLOAT_TO_UBYTE(v[1]), FLOAT_TO_UBYTE(v[2]), FLOAT_TO_UBYTE(v[3]));
			refOut[0] = v[0];
			refOut[1] = v[1];
			refOut[2] = v[2];
			refOut[3] = v[3];
		}
		break;
	case 25:
		{
			PFNGLVERTEXATTRIB4UBVNVPROC f = (PFNGLVERTEXATTRIB4UBVNVPROC)
				GLUtils::getProcAddress("glVertexAttrib4ubvNV");
			GLubyte ub[4];
			for (int i = 0; i < 4; i++ )
			   ub[i] = FLOAT_TO_UBYTE(v[i]);
			f(index, ub);
			refOut[0] = v[0];
			refOut[1] = v[1];
			refOut[2] = v[2];
			refOut[3] = v[3];
		}
		break;
	/* XXX Also test glVertexAttribs* functions? */
#endif

#if defined(GL_ARB_vertex_program) || defined (GL_ARB_vertex_shader)
	// ** GLfloat-valued functions
	case 26:
		{
			PFNGLVERTEXATTRIB1FARBPROC f = (PFNGLVERTEXATTRIB1FARBPROC)
				GLUtils::getProcAddress("glVertexAttrib1fARB");
			f(index, v[0]);
			COPY1(refOut, v);
		}
		break;
	case 27:
		{
			PFNGLVERTEXATTRIB2FARBPROC f = (PFNGLVERTEXATTRIB2FARBPROC)
				GLUtils::getProcAddress("glVertexAttrib2fARB");
			f(index, v[0], v[1]);
			COPY2(refOut, v);
		}
		break;
	case 28:
		{
			PFNGLVERTEXATTRIB3FARBPROC f = (PFNGLVERTEXATTRIB3FARBPROC)
				GLUtils::getProcAddress("glVertexAttrib3fARB");
			f(index, v[0], v[1], v[2]);
			COPY3(refOut, v);
		}
		break;
	case 29:
		{
			PFNGLVERTEXATTRIB4FARBPROC f = (PFNGLVERTEXATTRIB4FARBPROC)
				GLUtils::getProcAddress("glVertexAttrib4fARB");
			f(index, v[0], v[1], v[2], v[3]);
			COPY4(refOut, v);
		}
		break;
	case 30:
		{
			PFNGLVERTEXATTRIB1FVARBPROC f = (PFNGLVERTEXATTRIB1FVARBPROC)
				GLUtils::getProcAddress("glVertexAttrib1fvARB");
			f(index, v);
			COPY1(refOut, v);
		}
		break;
	case 31:
		{
			PFNGLVERTEXATTRIB2FVARBPROC f = (PFNGLVERTEXATTRIB2FVARBPROC)
				GLUtils::getProcAddress("glVertexAttrib2fvARB");
			f(index, v);
			COPY2(refOut, v);
		}
		break;
	case 32:
		{
			PFNGLVERTEXATTRIB3FVARBPROC f = (PFNGLVERTEXATTRIB3FVARBPROC)
				GLUtils::getProcAddress("glVertexAttrib3fvARB");
			f(index, v);
			COPY3(refOut, v);
		}
		break;
	case 33:
		{
			PFNGLVERTEXATTRIB4FVARBPROC f = (PFNGLVERTEXATTRIB4FVARBPROC)
				GLUtils::getProcAddress("glVertexAttrib4fvARB");
			f(index, v);
			COPY4(refOut, v);
		}
		break;
	// ** GLdouble-valued functions
	case 34:
		{
			PFNGLVERTEXATTRIB1DARBPROC f = (PFNGLVERTEXATTRIB1DARBPROC)
				GLUtils::getProcAddress("glVertexAttrib1dARB");
			f(index, v[0]);
			COPY1(refOut, v);
		}
		break;
	case 35:
		{
			PFNGLVERTEXATTRIB2DARBPROC f = (PFNGLVERTEXATTRIB2DARBPROC)
				GLUtils::getProcAddress("glVertexAttrib2dARB");
			f(index, v[0], v[1]);
			COPY2(refOut, v);
		}
		break;
	case 36:
		{
			PFNGLVERTEXATTRIB3DARBPROC f = (PFNGLVERTEXATTRIB3DARBPROC)
				GLUtils::getProcAddress("glVertexAttrib3dARB");
			f(index, v[0], v[1], v[2]);
			COPY3(refOut, v);
		}
		break;
	case 37:
		{
			PFNGLVERTEXATTRIB4DARBPROC f = (PFNGLVERTEXATTRIB4DARBPROC)
				GLUtils::getProcAddress("glVertexAttrib4dARB");
			f(index, v[0], v[1], v[2], v[3]);
			COPY4(refOut, v);
		}
		break;
	case 38:
		{
			PFNGLVERTEXATTRIB1DVARBPROC f = (PFNGLVERTEXATTRIB1DVARBPROC)
				GLUtils::getProcAddress("glVertexAttrib1dvARB");
			GLdouble d[1];
			d[0] = v[0];
			f(index, d);
			COPY1(refOut, v);
		}
		break;
	case 39:
		{
			PFNGLVERTEXATTRIB2DVARBPROC f = (PFNGLVERTEXATTRIB2DVARBPROC)
				GLUtils::getProcAddress("glVertexAttrib2dvARB");
			GLdouble d[2];
			d[0] = v[0];
			d[1] = v[1];
			f(index, d);
			COPY2(refOut, v);
		}
		break;
	case 40:
		{
			PFNGLVERTEXATTRIB3DVARBPROC f = (PFNGLVERTEXATTRIB3DVARBPROC)
				GLUtils::getProcAddress("glVertexAttrib3dvARB");
			GLdouble d[3];
			d[0] = v[0];
			d[1] = v[1];
			d[2] = v[2];
			f(index, d);
			COPY3(refOut, v);
		}
		break;
	case 41:
		{
			PFNGLVERTEXATTRIB4DVARBPROC f = (PFNGLVERTEXATTRIB4DVARBPROC)
				GLUtils::getProcAddress("glVertexAttrib4dvARB");
			GLdouble d[4];
			d[0] = v[0];
			d[1] = v[1];
			d[2] = v[2];
			d[3] = v[3];
			f(index, d);
			COPY4(refOut, v);
		}
		break;
	// ** GLshort-valued functions
	case 42:
		{
			PFNGLVERTEXATTRIB1SARBPROC f = (PFNGLVERTEXATTRIB1SARBPROC)
				GLUtils::getProcAddress("glVertexAttrib1sARB");
			f(index, (GLshort) v[0]);
			refOut[0] = (GLfloat) (GLshort) v[0];
			refOut[1] = 0.0F;
			refOut[2] = 0.0F;
			refOut[3] = 1.0F;
		}
		break;
	case 43:
		{
			PFNGLVERTEXATTRIB2SARBPROC f = (PFNGLVERTEXATTRIB2SARBPROC)
				GLUtils::getProcAddress("glVertexAttrib2sARB");
			f(index, (GLshort) v[0], (GLshort) v[1]);
			refOut[0] = (GLfloat) (GLshort) v[0];
			refOut[1] = (GLfloat) (GLshort) v[1];
			refOut[2] = 0.0F;
			refOut[3] = 1.0F;
		}
		break;
	case 44:
		{
			PFNGLVERTEXATTRIB3SARBPROC f = (PFNGLVERTEXATTRIB3SARBPROC)
				GLUtils::getProcAddress("glVertexAttrib3sARB");
			f(index, (GLshort) v[0], (GLshort) v[1], (GLshort) v[2]);
			refOut[0] = (GLfloat) (GLshort) v[0];
			refOut[1] = (GLfloat) (GLshort) v[1];
			refOut[2] = (GLfloat) (GLshort) v[2];
			refOut[3] = 1.0F;
		}
		break;
	case 45:
		{
			PFNGLVERTEXATTRIB4SARBPROC f = (PFNGLVERTEXATTRIB4SARBPROC)
				GLUtils::getProcAddress("glVertexAttrib4sARB");
			f(index, (GLshort) v[0], (GLshort) v[1], (GLshort) v[2], (GLshort) v[3]);
			refOut[0] = (GLfloat) (GLshort) v[0];
			refOut[1] = (GLfloat) (GLshort) v[1];
			refOut[2] = (GLfloat) (GLshort) v[2];
			refOut[3] = (GLfloat) (GLshort) v[3];
		}
		break;
	case 46:
		{
			PFNGLVERTEXATTRIB1SVARBPROC f = (PFNGLVERTEXATTRIB1SVARBPROC)
				GLUtils::getProcAddress("glVertexAttrib1svARB");
			GLshort s[1];
			s[0] = (GLshort) v[0];
			f(index, s);
			refOut[0] = (GLfloat) (GLshort) v[0];
			refOut[1] = 0.0F;
			refOut[2] = 0.0F;
			refOut[3] = 1.0F;
		}
		break;
	case 47:
		{
			PFNGLVERTEXATTRIB2SVARBPROC f = (PFNGLVERTEXATTRIB2SVARBPROC)
				GLUtils::getProcAddress("glVertexAttrib2svARB");
			GLshort s[2];
			s[0] = (GLshort) v[0];
			s[1] = (GLshort) v[1];
			f(index, s);
			refOut[0] = (GLfloat) (GLshort) v[0];
			refOut[1] = (GLfloat) (GLshort) v[1];
			refOut[2] = 0.0F;
			refOut[3] = 1.0F;
		}
		break;
	case 48:
		{
			PFNGLVERTEXATTRIB3SVARBPROC f = (PFNGLVERTEXATTRIB3SVARBPROC)
				GLUtils::getProcAddress("glVertexAttrib3svARB");
			GLshort s[3];
			s[0] = (GLshort) v[0];
			s[1] = (GLshort) v[1];
			s[2] = (GLshort) v[2];
			f(index, s);
			refOut[0] = (GLfloat) (GLshort) v[0];
			refOut[1] = (GLfloat) (GLshort) v[1];
			refOut[2] = (GLfloat) (GLshort) v[2];
			refOut[3] = 1.0F;
		}
		break;
	case 49:
		{
			PFNGLVERTEXATTRIB4SVARBPROC f = (PFNGLVERTEXATTRIB4SVARBPROC)
				GLUtils::getProcAddress("glVertexAttrib4svARB");
			GLshort s[4];
			s[0] = (GLshort) v[0];
			s[1] = (GLshort) v[1];
			s[2] = (GLshort) v[2];
			s[3] = (GLshort) v[3];
			f(index, s);
			refOut[0] = (GLfloat) (GLshort) v[0];
			refOut[1] = (GLfloat) (GLshort) v[1];
			refOut[2] = (GLfloat) (GLshort) v[2];
			refOut[3] = (GLfloat) (GLshort) v[3];
		}
		break;
	case 50:
		{
			PFNGLVERTEXATTRIB4NSVARBPROC f = (PFNGLVERTEXATTRIB4NSVARBPROC)
				GLUtils::getProcAddress("glVertexAttrib4NsvARB");
			GLshort s[4];
			for (int i = 0; i < 4; i++)
				s[i] = FLOAT_TO_SHORT(v[i]);
			f(index, s);
			COPY4(refOut, v);
		}
		break;
	// ** GLubyte-valued functions
	case 51:
		{
			PFNGLVERTEXATTRIB4NUBARBPROC f = (PFNGLVERTEXATTRIB4NUBARBPROC)
				GLUtils::getProcAddress("glVertexAttrib4NubARB");
			f(index, FLOAT_TO_UBYTE(v[0]), FLOAT_TO_UBYTE(v[1]), FLOAT_TO_UBYTE(v[2]), FLOAT_TO_UBYTE(v[3]));
			COPY4(refOut, v);
		}
		break;
	case 52:
		{
			PFNGLVERTEXATTRIB4NUBVARBPROC f = (PFNGLVERTEXATTRIB4NUBVARBPROC)
				GLUtils::getProcAddress("glVertexAttrib4NubvARB");
			GLubyte ub[4];
			for (int i = 0; i < 4; i++ )
			   ub[i] = FLOAT_TO_UBYTE(v[i]);
			f(index, ub);
			COPY4(refOut, v);
		}
		break;
	case 53:
		{
			PFNGLVERTEXATTRIB4UBVARBPROC f = (PFNGLVERTEXATTRIB4UBVARBPROC)
				GLUtils::getProcAddress("glVertexAttrib4ubvARB");
			GLubyte ub[4];
			for (int i = 0; i < 4; i++ )
			   ub[i] = (GLubyte) v[i];
			f(index, ub);
			refOut[0] = (GLfloat) (GLubyte) v[0];
			refOut[1] = (GLfloat) (GLubyte) v[1];
			refOut[2] = (GLfloat) (GLubyte) v[2];
			refOut[3] = (GLfloat) (GLubyte) v[3];
		}
		break;
	// ** GLbyte-valued functions
	case 54:
		{
			PFNGLVERTEXATTRIB4NBVARBPROC f = (PFNGLVERTEXATTRIB4NBVARBPROC)
				GLUtils::getProcAddress("glVertexAttrib4NbvARB");
			GLbyte b[4];
			for (int i = 0; i < 4; i++ )
			   b[i] = FLOAT_TO_BYTE(v[i]);
			f(index, b);
			COPY4(refOut, v);
		}
		break;
	case 55:
		{
			PFNGLVERTEXATTRIB4BVARBPROC f = (PFNGLVERTEXATTRIB4BVARBPROC)
				GLUtils::getProcAddress("glVertexAttrib4bvARB");
			GLbyte b[4];
			for (int i = 0; i < 4; i++ )
			   b[i] = (GLbyte) v[i];
			f(index, b);
			refOut[0] = (GLfloat) (GLbyte) v[0];
			refOut[1] = (GLfloat) (GLbyte) v[1];
			refOut[2] = (GLfloat) (GLbyte) v[2];
			refOut[3] = (GLfloat) (GLbyte) v[3];
		}
		break;
	// ** GLint-valued functions
	case 56:
		{
			PFNGLVERTEXATTRIB4NIVARBPROC f = (PFNGLVERTEXATTRIB4NIVARBPROC)
				GLUtils::getProcAddress("glVertexAttrib4NivARB");
			GLint iv[4];
			for (int i = 0; i < 4; i++ )
			   iv[i] = FLOAT_TO_INT(v[i]);
			f(index, iv);
			COPY4(refOut, v);
		}
		break;
	case 57:
		{
			PFNGLVERTEXATTRIB4IVARBPROC f = (PFNGLVERTEXATTRIB4IVARBPROC)
				GLUtils::getProcAddress("glVertexAttrib4ivARB");
			GLint iv[4];
			for (int i = 0; i < 4; i++ )
			   iv[i] = (GLint) v[i];
			f(index, iv);
			refOut[0] = (GLfloat) (GLint) v[0];
			refOut[1] = (GLfloat) (GLint) v[1];
			refOut[2] = (GLfloat) (GLint) v[2];
			refOut[3] = (GLfloat) (GLint) v[3];
		}
		break;
	// ** GLuint-valued functions
	case 58:
		{
			PFNGLVERTEXATTRIB4NUIVARBPROC f = (PFNGLVERTEXATTRIB4NUIVARBPROC)
				GLUtils::getProcAddress("glVertexAttrib4NuivARB");
			GLuint ui[4];
			for (int i = 0; i < 4; i++ )
			   ui[i] = FLOAT_TO_UINT(v[i]);
			f(index, ui);
			COPY4(refOut, v);
		}
		break;
	case 59:
		{
			PFNGLVERTEXATTRIB4UIVARBPROC f = (PFNGLVERTEXATTRIB4UIVARBPROC)
				GLUtils::getProcAddress("glVertexAttrib4uivARB");
			GLuint ui[4];
			for (int i = 0; i < 4; i++ )
			   ui[i] = (GLint) v[i];
			f(index, ui);
			refOut[0] = (GLfloat) (GLint) v[0];
			refOut[1] = (GLfloat) (GLint) v[1];
			refOut[2] = (GLfloat) (GLint) v[2];
			refOut[3] = (GLfloat) (GLint) v[3];
		}
		break;
	// ** GLushort-valued functions
	case 60:
		{
			PFNGLVERTEXATTRIB4NUSVARBPROC f = (PFNGLVERTEXATTRIB4NUSVARBPROC)
				GLUtils::getProcAddress("glVertexAttrib4NusvARB");
			GLushort us[4];
			for (int i = 0; i < 4; i++ )
			   us[i] = FLOAT_TO_USHORT(v[i]);
			f(index, us);
			COPY4(refOut, v);
		}
		break;
	case 61:
		{
			PFNGLVERTEXATTRIB4USVARBPROC f = (PFNGLVERTEXATTRIB4USVARBPROC)
				GLUtils::getProcAddress("glVertexAttrib4usvARB");
			GLushort us[4];
			for (int i = 0; i < 4; i++ )
			   us[i] = (GLint) v[i];
			f(index, us);
			refOut[0] = (GLfloat) (GLint) v[0];
			refOut[1] = (GLfloat) (GLint) v[1];
			refOut[2] = (GLfloat) (GLint) v[2];
			refOut[3] = (GLfloat) (GLint) v[3];
		}
		break;
#endif

#if defined(GL_VERSION_2_0)
	case 62:
		{
			PFNGLVERTEXATTRIB1FPROC f = (PFNGLVERTEXATTRIB1FPROC)
				GLUtils::getProcAddress("glVertexAttrib1f");
			f(index, v[0]);
			COPY1(refOut, v);
		}
		break;
	case 63:
		{
			PFNGLVERTEXATTRIB2FPROC f = (PFNGLVERTEXATTRIB2FPROC)
				GLUtils::getProcAddress("glVertexAttrib2f");
			f(index, v[0], v[1]);
			COPY2(refOut, v);
		}
		break;
	case 64:
		{
			PFNGLVERTEXATTRIB3FPROC f = (PFNGLVERTEXATTRIB3FPROC)
				GLUtils::getProcAddress("glVertexAttrib3f");
			f(index, v[0], v[1], v[2]);
			COPY3(refOut, v);
		}
		break;
	case 65:
		{
			PFNGLVERTEXATTRIB4FPROC f = (PFNGLVERTEXATTRIB4FPROC)
				GLUtils::getProcAddress("glVertexAttrib4f");
			f(index, v[0], v[1], v[2], v[3]);
			COPY4(refOut, v);
		}
		break;
	case 66:
		{
			PFNGLVERTEXATTRIB1FVPROC f = (PFNGLVERTEXATTRIB1FVPROC)
				GLUtils::getProcAddress("glVertexAttrib1fv");
			f(index, v);
			COPY1(refOut, v);
		}
		break;
	case 67:
		{
			PFNGLVERTEXATTRIB2FVPROC f = (PFNGLVERTEXATTRIB2FVPROC)
				GLUtils::getProcAddress("glVertexAttrib2fv");
			f(index, v);
			COPY2(refOut, v);
		}
		break;
	case 68:
		{
			PFNGLVERTEXATTRIB3FVPROC f = (PFNGLVERTEXATTRIB3FVPROC)
				GLUtils::getProcAddress("glVertexAttrib3fv");
			f(index, v);
			COPY3(refOut, v);
		}
		break;
	case 69:
		{
			PFNGLVERTEXATTRIB4FVPROC f = (PFNGLVERTEXATTRIB4FVPROC)
				GLUtils::getProcAddress("glVertexAttrib4fv");
			f(index, v);
			COPY4(refOut, v);
		}
		break;
	// ** GLdouble-valued functions
	case 70:
		{
			PFNGLVERTEXATTRIB1DPROC f = (PFNGLVERTEXATTRIB1DPROC)
				GLUtils::getProcAddress("glVertexAttrib1d");
			f(index, v[0]);
			COPY1(refOut, v);
		}
		break;
	case 71:
		{
			PFNGLVERTEXATTRIB2DPROC f = (PFNGLVERTEXATTRIB2DPROC)
				GLUtils::getProcAddress("glVertexAttrib2d");
			f(index, v[0], v[1]);
			COPY2(refOut, v);
		}
		break;
	case 72:
		{
			PFNGLVERTEXATTRIB3DPROC f = (PFNGLVERTEXATTRIB3DPROC)
				GLUtils::getProcAddress("glVertexAttrib3d");
			f(index, v[0], v[1], v[2]);
			COPY3(refOut, v);
		}
		break;
	case 73:
		{
			PFNGLVERTEXATTRIB4DPROC f = (PFNGLVERTEXATTRIB4DPROC)
				GLUtils::getProcAddress("glVertexAttrib4d");
			f(index, v[0], v[1], v[2], v[3]);
			COPY4(refOut, v);
		}
		break;
	case 74:
		{
			PFNGLVERTEXATTRIB1DVPROC f = (PFNGLVERTEXATTRIB1DVPROC)
				GLUtils::getProcAddress("glVertexAttrib1dv");
			GLdouble d[1];
			d[0] = v[0];
			f(index, d);
			COPY1(refOut, v);
		}
		break;
	case 75:
		{
			PFNGLVERTEXATTRIB2DVPROC f = (PFNGLVERTEXATTRIB2DVPROC)
				GLUtils::getProcAddress("glVertexAttrib2dv");
			GLdouble d[2];
			d[0] = v[0];
			d[1] = v[1];
			f(index, d);
			COPY2(refOut, v);
		}
		break;
	case 76:
		{
			PFNGLVERTEXATTRIB3DVPROC f = (PFNGLVERTEXATTRIB3DVPROC)
				GLUtils::getProcAddress("glVertexAttrib3dv");
			GLdouble d[3];
			d[0] = v[0];
			d[1] = v[1];
			d[2] = v[2];
			f(index, d);
			COPY3(refOut, v);
		}
		break;
	case 77:
		{
			PFNGLVERTEXATTRIB4DVPROC f = (PFNGLVERTEXATTRIB4DVPROC)
				GLUtils::getProcAddress("glVertexAttrib4dv");
			GLdouble d[4];
			d[0] = v[0];
			d[1] = v[1];
			d[2] = v[2];
			d[3] = v[3];
			f(index, d);
			COPY4(refOut, v);
		}
		break;
	// ** GLshort-valued functions
	case 78:
		{
			PFNGLVERTEXATTRIB1SPROC f = (PFNGLVERTEXATTRIB1SPROC)
				GLUtils::getProcAddress("glVertexAttrib1s");
			f(index, (GLshort) v[0]);
			refOut[0] = (GLfloat) (GLshort) v[0];
			refOut[1] = 0.0F;
			refOut[2] = 0.0F;
			refOut[3] = 1.0F;
		}
		break;
	case 79:
		{
			PFNGLVERTEXATTRIB2SPROC f = (PFNGLVERTEXATTRIB2SPROC)
				GLUtils::getProcAddress("glVertexAttrib2s");
			f(index, (GLshort) v[0], (GLshort) v[1]);
			refOut[0] = (GLfloat) (GLshort) v[0];
			refOut[1] = (GLfloat) (GLshort) v[1];
			refOut[2] = 0.0F;
			refOut[3] = 1.0F;
		}
		break;
	case 80:
		{
			PFNGLVERTEXATTRIB3SPROC f = (PFNGLVERTEXATTRIB3SPROC)
				GLUtils::getProcAddress("glVertexAttrib3s");
			f(index, (GLshort) v[0], (GLshort) v[1], (GLshort) v[2]);
			refOut[0] = (GLfloat) (GLshort) v[0];
			refOut[1] = (GLfloat) (GLshort) v[1];
			refOut[2] = (GLfloat) (GLshort) v[2];
			refOut[3] = 1.0F;
		}
		break;
	case 81:
		{
			PFNGLVERTEXATTRIB4SPROC f = (PFNGLVERTEXATTRIB4SPROC)
				GLUtils::getProcAddress("glVertexAttrib4s");
			f(index, (GLshort) v[0], (GLshort) v[1], (GLshort) v[2], (GLshort) v[3]);
			refOut[0] = (GLfloat) (GLshort) v[0];
			refOut[1] = (GLfloat) (GLshort) v[1];
			refOut[2] = (GLfloat) (GLshort) v[2];
			refOut[3] = (GLfloat) (GLshort) v[3];
		}
		break;
	case 82:
		{
			PFNGLVERTEXATTRIB1SVPROC f = (PFNGLVERTEXATTRIB1SVPROC)
				GLUtils::getProcAddress("glVertexAttrib1sv");
			GLshort s[1];
			s[0] = (GLshort) v[0];
			f(index, s);
			refOut[0] = (GLfloat) (GLshort) v[0];
			refOut[1] = 0.0F;
			refOut[2] = 0.0F;
			refOut[3] = 1.0F;
		}
		break;
	case 83:
		{
			PFNGLVERTEXATTRIB2SVPROC f = (PFNGLVERTEXATTRIB2SVPROC)
				GLUtils::getProcAddress("glVertexAttrib2sv");
			GLshort s[2];
			s[0] = (GLshort) v[0];
			s[1] = (GLshort) v[1];
			f(index, s);
			refOut[0] = (GLfloat) (GLshort) v[0];
			refOut[1] = (GLfloat) (GLshort) v[1];
			refOut[2] = 0.0F;
			refOut[3] = 1.0F;
		}
		break;
	case 84:
		{
			PFNGLVERTEXATTRIB3SVPROC f = (PFNGLVERTEXATTRIB3SVPROC)
				GLUtils::getProcAddress("glVertexAttrib3sv");
			GLshort s[3];
			s[0] = (GLshort) v[0];
			s[1] = (GLshort) v[1];
			s[2] = (GLshort) v[2];
			f(index, s);
			refOut[0] = (GLfloat) (GLshort) v[0];
			refOut[1] = (GLfloat) (GLshort) v[1];
			refOut[2] = (GLfloat) (GLshort) v[2];
			refOut[3] = 1.0F;
		}
		break;
	case 85:
		{
			PFNGLVERTEXATTRIB4SVPROC f = (PFNGLVERTEXATTRIB4SVPROC)
				GLUtils::getProcAddress("glVertexAttrib4sv");
			GLshort s[4];
			s[0] = (GLshort) v[0];
			s[1] = (GLshort) v[1];
			s[2] = (GLshort) v[2];
			s[3] = (GLshort) v[3];
			f(index, s);
			refOut[0] = (GLfloat) (GLshort) v[0];
			refOut[1] = (GLfloat) (GLshort) v[1];
			refOut[2] = (GLfloat) (GLshort) v[2];
			refOut[3] = (GLfloat) (GLshort) v[3];
		}
		break;
	case 86:
		{
			PFNGLVERTEXATTRIB4NSVPROC f = (PFNGLVERTEXATTRIB4NSVPROC)
				GLUtils::getProcAddress("glVertexAttrib4Nsv");
			GLshort s[4];
			for (int i = 0; i < 4; i++)
				s[i] = FLOAT_TO_SHORT(v[i]);
			f(index, s);
			COPY4(refOut, v);
		}
		break;
	// ** GLubyte-valued functions
	case 87:
		{
			PFNGLVERTEXATTRIB4NUBPROC f = (PFNGLVERTEXATTRIB4NUBPROC)
				GLUtils::getProcAddress("glVertexAttrib4Nub");
			f(index, FLOAT_TO_UBYTE(v[0]), FLOAT_TO_UBYTE(v[1]), FLOAT_TO_UBYTE(v[2]), FLOAT_TO_UBYTE(v[3]));
			COPY4(refOut, v);
		}
		break;
	case 88:
		{
			PFNGLVERTEXATTRIB4NUBVPROC f = (PFNGLVERTEXATTRIB4NUBVPROC)
				GLUtils::getProcAddress("glVertexAttrib4Nubv");
			GLubyte ub[4];
			for (int i = 0; i < 4; i++ )
			   ub[i] = FLOAT_TO_UBYTE(v[i]);
			f(index, ub);
			COPY4(refOut, v);
		}
		break;
	case 89:
		{
			PFNGLVERTEXATTRIB4UBVPROC f = (PFNGLVERTEXATTRIB4UBVPROC)
				GLUtils::getProcAddress("glVertexAttrib4ubv");
			GLubyte ub[4];
			for (int i = 0; i < 4; i++ )
			   ub[i] = (GLubyte) v[i];
			f(index, ub);
			refOut[0] = (GLfloat) (GLubyte) v[0];
			refOut[1] = (GLfloat) (GLubyte) v[1];
			refOut[2] = (GLfloat) (GLubyte) v[2];
			refOut[3] = (GLfloat) (GLubyte) v[3];
		}
		break;
	// ** GLbyte-valued functions
	case 90:
		{
			PFNGLVERTEXATTRIB4NBVPROC f = (PFNGLVERTEXATTRIB4NBVPROC)
				GLUtils::getProcAddress("glVertexAttrib4Nbv");
			GLbyte b[4];
			for (int i = 0; i < 4; i++ )
			   b[i] = FLOAT_TO_BYTE(v[i]);
			f(index, b);
			COPY4(refOut, v);
		}
		break;
	case 91:
		{
			PFNGLVERTEXATTRIB4BVPROC f = (PFNGLVERTEXATTRIB4BVPROC)
				GLUtils::getProcAddress("glVertexAttrib4bv");
			GLbyte b[4];
			for (int i = 0; i < 4; i++ )
			   b[i] = (GLbyte) v[i];
			f(index, b);
			refOut[0] = (GLfloat) (GLbyte) v[0];
			refOut[1] = (GLfloat) (GLbyte) v[1];
			refOut[2] = (GLfloat) (GLbyte) v[2];
			refOut[3] = (GLfloat) (GLbyte) v[3];
		}
		break;
	// ** GLint-valued functions
	case 92:
		{
			PFNGLVERTEXATTRIB4NIVPROC f = (PFNGLVERTEXATTRIB4NIVPROC)
				GLUtils::getProcAddress("glVertexAttrib4Niv");
			GLint iv[4];
			for (int i = 0; i < 4; i++ )
			   iv[i] = FLOAT_TO_INT(v[i]);
			f(index, iv);
			COPY4(refOut, v);
		}
		break;
	case 93:
		{
			PFNGLVERTEXATTRIB4IVPROC f = (PFNGLVERTEXATTRIB4IVPROC)
				GLUtils::getProcAddress("glVertexAttrib4iv");
			GLint iv[4];
			for (int i = 0; i < 4; i++ )
			   iv[i] = (GLint) v[i];
			f(index, iv);
			refOut[0] = (GLfloat) (GLint) v[0];
			refOut[1] = (GLfloat) (GLint) v[1];
			refOut[2] = (GLfloat) (GLint) v[2];
			refOut[3] = (GLfloat) (GLint) v[3];
		}
		break;
	// ** GLuint-valued functions
	case 94:
		{
			PFNGLVERTEXATTRIB4NUIVPROC f = (PFNGLVERTEXATTRIB4NUIVPROC)
				GLUtils::getProcAddress("glVertexAttrib4Nuiv");
			GLuint ui[4];
			for (int i = 0; i < 4; i++ )
			   ui[i] = FLOAT_TO_UINT(v[i]);
			f(index, ui);
			COPY4(refOut, v);
		}
		break;
	case 95:
		{
			PFNGLVERTEXATTRIB4UIVPROC f = (PFNGLVERTEXATTRIB4UIVPROC)
				GLUtils::getProcAddress("glVertexAttrib4uiv");
			GLuint ui[4];
			for (int i = 0; i < 4; i++ )
			   ui[i] = (GLint) v[i];
			f(index, ui);
			refOut[0] = (GLfloat) (GLint) v[0];
			refOut[1] = (GLfloat) (GLint) v[1];
			refOut[2] = (GLfloat) (GLint) v[2];
			refOut[3] = (GLfloat) (GLint) v[3];
		}
		break;
	// ** GLushort-valued functions
	case 96:
		{
			PFNGLVERTEXATTRIB4NUSVPROC f = (PFNGLVERTEXATTRIB4NUSVPROC)
				GLUtils::getProcAddress("glVertexAttrib4Nusv");
			GLushort us[4];
			for (int i = 0; i < 4; i++ )
			   us[i] = FLOAT_TO_USHORT(v[i]);
			f(index, us);
			COPY4(refOut, v);
		}
		break;
	case 97:
		{
			PFNGLVERTEXATTRIB4USVPROC f = (PFNGLVERTEXATTRIB4USVPROC)
				GLUtils::getProcAddress("glVertexAttrib4usv");
			GLushort us[4];
			for (int i = 0; i < 4; i++ )
			   us[i] = (GLint) v[i];
			f(index, us);
			refOut[0] = (GLfloat) (GLint) v[0];
			refOut[1] = (GLfloat) (GLint) v[1];
			refOut[2] = (GLfloat) (GLint) v[2];
			refOut[3] = (GLfloat) (GLint) v[3];
		}
		break;
#endif

	default:
		// never get here!
		abort();
	}

	assert(98 == NUM_NV_ATTRIB_FUNCS + NUM_ARB_ATTRIB_FUNCS + NUM_2_0_ATTRIB_FUNCS);
}


// Test if 'a and 'b' are within an epsilon of each other
static bool
NearlyEqual(const GLfloat a[4], const GLfloat b[4])
{
	const GLfloat epsilon = 0.05;
	if (fabsf(a[0] - b[0]) > epsilon ||
	    fabsf(a[1] - b[1]) > epsilon ||
	    fabsf(a[2] - b[2]) > epsilon ||
	    fabsf(a[3] - b[3]) > epsilon)
		return 0;
	else
		return 1;
}
	   

void VertAttribTest::FailMessage(VertAttribResult &r, const char *msg,
				 const char *func, int dlistMode) const
{
	// record the failure
	r.pass = false;

	// print the message
	env->log << name << ":  FAIL "
		 << r.config->conciseDescription() << '\n';
	env->log << "\t" << msg << " (Testing " << func << " in ";

	if (dlistMode)
		env->log << "display list mode)\n";
	else
		env->log << "immediate mode)\n";
}



// Test setting/getting a set of vertex attribute values
// Return true if pass, false if fail
bool
VertAttribTest::TestAttribs(VertAttribResult &r,
			    int attribFunc,
			    PFNGLGETVERTEXATTRIBFVARBPROC getAttribfv,
			    Aliasing aliasing,
			    int numAttribs)
{
	static const GLfloat refValues[7] = { 1.0F, 0.8F, 0.6F, 0.5F, 0.4F, 0.2F, 0.0F };
	GLfloat refValue[32][4];
	GLfloat refOut[32][4];
	bool result = true;

	assert(numAttribs <= 32);

	// Initialize the refValue array
	int refIndex = 0;
	for (int i = 1; i < numAttribs; i++) {
		refValue[i][0] = refValues[refIndex++ % 7];
		refValue[i][1] = refValues[refIndex++ % 7];
		refValue[i][2] = refValues[refIndex++ % 7];
		refValue[i][3] = refValues[refIndex++ % 7];
	}

	for (int dlist = 0; dlist < 2; dlist++) {

		// set a couple conventional attribs for later aliasing tests
		glNormal3f(-1.0F, -2.0F, -3.0F);
		glTexCoord4f(-1.0F, -2.0F, -3.0F, -4.0F);

		if (dlist == 1) {
			glNewList(42, GL_COMPILE);
		}

		// Set all the vertex attributes
		for (int i = 1; i < numAttribs; i++) {
			SetAttrib(i, refValue[i], attribFunc, refOut[i]);
		}

		if (dlist == 1) {
			glEndList();
			glCallList(42);
		}

		// Test all the vertex attributes
		for (int i = 1; i < numAttribs; i++) {
			const GLfloat *expected = refOut[i];
			GLfloat v[4];
			getAttribfv(i, GL_CURRENT_VERTEX_ATTRIB_ARB, v);
			if (!NearlyEqual(v, expected)) {
				char msg[1000];
				sprintf(msg, "Vertex Attribute %d is (%g, %g, %g, %g) but expected (%g, %g, %g, %g)",
					i, v[0], v[1], v[2], v[3],
					expected[0], expected[1], expected[2], expected[3]);
				FailMessage(r, msg, AttribFuncNames[attribFunc], dlist);
				result = false;
			}
		}

		if (aliasing == REQUIRED) {
			// spot check a few aliased attribs
			GLfloat v[4];
			glGetFloatv(GL_CURRENT_NORMAL, v);
			v[3] = refOut[2][3];
			if (!NearlyEqual(v, refOut[2])) {
				FailMessage(r, "Setting attribute 2 did not update GL_CURRENT_NORMAL", AttribFuncNames[attribFunc], dlist);
				result = false;
			}

			glGetFloatv(GL_CURRENT_TEXTURE_COORDS, v);
			if (!NearlyEqual(v, refOut[8])) {
				FailMessage(r, "Setting attribute 8 did not update GL_CURRENT_TEXTURE_COORDS", AttribFuncNames[attribFunc], dlist);
				result = false;
			}
		}
		else if (aliasing == DISALLOWED) {
			// spot check a few non-aliased attribs
			GLfloat v[4];
			glGetFloatv(GL_CURRENT_NORMAL, v);
			if (v[0] != -1.0F ||
				v[1] != -2.0F ||
				v[2] != -3.0F) {
				FailMessage(r, "GL_CURRENT_NORMAL was erroneously set by a glVertexAttrib call", AttribFuncNames[attribFunc], dlist);
				result = false;
			}
			glGetFloatv(GL_CURRENT_TEXTURE_COORDS, v);
			if (v[0] != -1.0F ||
				v[1] != -2.0F ||
				v[2] != -3.0F ||
				v[3] != -4.0F) {
				FailMessage(r, "GL_CURRENT_TEXTURE_COORDS was erroneously set by a glVertexAttrib call", AttribFuncNames[attribFunc], dlist);
				result = false;
			}
		}

	} // dlist loop

	return result;
}


// Test the GL_NV_vertex_program functions
// Return true if pass, false if fail
bool
VertAttribTest::TestNVfuncs(VertAttribResult &r)
{
	bool result = true;
#ifdef GL_NV_vertex_program
	PFNGLGETVERTEXATTRIBFVNVPROC getAttribfv;
	const GLint numAttribs = 16;
	const Aliasing aliasing = REQUIRED;

	getAttribfv = (PFNGLGETVERTEXATTRIBFVNVPROC) GLUtils::getProcAddress("glGetVertexAttribfvNV");
	assert(getAttribfv);

	r.numNVtested = 0;

	for (int attribFunc = 0; attribFunc < NUM_NV_ATTRIB_FUNCS; attribFunc++) {
		bool b;
		b = TestAttribs(r, attribFunc, getAttribfv, aliasing, numAttribs);
		if (!b)
			result = false;
		r.numNVtested++;
	}
#else
	(void) r;
#endif
	return result;
}


// Test the GL_ARB_vertex_program/shader functions
// Return true if pass, false if fail
bool
VertAttribTest::TestARBfuncs(VertAttribResult &r, bool shader)
{
	bool result = true;
#if defined(GL_ARB_vertex_program) || defined(GL_ARB_vertex_shader)
	PFNGLGETVERTEXATTRIBFVARBPROC getAttribfv;
	GLint numAttribs;

	getAttribfv = (PFNGLGETVERTEXATTRIBFVARBPROC) GLUtils::getProcAddress("glGetVertexAttribfvARB");
	assert(getAttribfv);

	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS_ARB, &numAttribs);
	assert(numAttribs > 0);

	r.numARBtested = 0;

	if (shader) {
		// test GL_ARB_vertex_shader (aliasing is disallowed)
		const Aliasing aliasing = DISALLOWED;
		for (int i = 0; i < NUM_ARB_ATTRIB_FUNCS; i++) {
			int attribFunc = NUM_NV_ATTRIB_FUNCS + i;
			bool b;
			b = TestAttribs(r, attribFunc, getAttribfv, aliasing, numAttribs);
			if (!b)
				result = false;
			r.numARBtested++;
		}
	}
	else {
		// test GL_ARB_vertex_program:
		// Determine if attribute aliasing is allowed
		Aliasing aliasing;
		if (GLUtils::haveExtension("GL_ARB_vertex_shader")) {
			aliasing = DISALLOWED;
		}
		else {
			// check for OpenGL 2.x support
			char *vers = (char *) glGetString(GL_VERSION);
			if (vers[0] == '2' && vers[1] == '.') {
				aliasing = DISALLOWED;
			}
			else {
				assert(vers[0] == '1'); /* revisit when we have OpenGL 3.x */
				aliasing = OPTIONAL;
			}
		}
		for (int i = 0; i < NUM_ARB_ATTRIB_FUNCS; i++) {
			int attribFunc = NUM_NV_ATTRIB_FUNCS + i;
			bool b;
			b = TestAttribs(r, attribFunc, getAttribfv, aliasing, numAttribs);
			if (!b)
				result = false;
			r.numARBtested++;
		}
	}
#else
	(void) r;
#endif
	return result;
}


// Test the OpenGL 2.x glVertexAttrib functions
// Return true if pass, false if fail
bool
VertAttribTest::Test20funcs(VertAttribResult &r)
{
	bool result = true;
#ifdef GL_VERSION_2_0
	PFNGLGETVERTEXATTRIBFVPROC getAttribfv;
	GLint numAttribs;
	const Aliasing aliasing = DISALLOWED;

	getAttribfv = (PFNGLGETVERTEXATTRIBFVPROC) GLUtils::getProcAddress("glGetVertexAttribfv");
	assert(getAttribfv);

	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &numAttribs);
	assert(numAttribs > 0);

	r.num20tested = 0;

	for (int i = 0; i < NUM_2_0_ATTRIB_FUNCS; i++) {
		int attribFunc = NUM_NV_ATTRIB_FUNCS + NUM_ARB_ATTRIB_FUNCS+ i;
		bool b;
		b = TestAttribs(r, attribFunc, getAttribfv, aliasing, numAttribs);
		if (!b)
			result = false;
		r.num20tested++;
	}
#else
	(void) r;
#endif
	return result;
}


void
VertAttribTest::runOne(VertAttribResult& r, Window&)
{

	assert(sizeof(AttribFuncNames) / sizeof(char *) ==
	       NUM_NV_ATTRIB_FUNCS + NUM_ARB_ATTRIB_FUNCS + NUM_2_0_ATTRIB_FUNCS);

	r.pass = true;
#ifdef GL_NV_vertex_program
	if (GLUtils::haveExtension("GL_NV_vertex_program")) {
		bool p = TestNVfuncs(r);
		if (!p)
			r.pass = false;
	}
#endif
#ifdef GL_ARB_vertex_program
	if (GLUtils::haveExtension("GL_ARB_vertex_program")) {
		bool p = TestARBfuncs(r, false);
		if (!p)
			r.pass = false;
	}
#endif
#ifdef GL_ARB_vertex_shader
	if (GLUtils::haveExtension("GL_ARB_vertex_shader")) {
		bool p = TestARBfuncs(r, true);
		if (!p)
			r.pass = false;
	}
#endif
#ifdef GL_VERSION_2_0
	const char *vers = (const char *) glGetString(GL_VERSION);
	if (vers[0] == '2' && vers[1] == '.') {
		bool p = Test20funcs(r);
		if (!p)
			r.pass = false;
	}
#endif
}


void
VertAttribTest::logOne(VertAttribResult& r)
{
	logPassFail(r);
	logConcise(r);
	logStats(r);
}


void
VertAttribTest::logStats(VertAttribResult& r)
{
	env->log << "\t" << r.numNVtested << " GL_NV_vertex_program functions tested\n";
	env->log << "\t" << r.numARBtested << " GL_ARB_vertex_program/shader functions tested\n";
	env->log << "\t" << r.num20tested << " OpenGL 2.0 functions tested\n";
}


void
VertAttribTest::compareOne(VertAttribResult& oldR, VertAttribResult& newR)
{
	if (env->options.verbosity) {
		env->log << env->options.db1Name << ':';
		logStats(oldR);
		env->log << env->options.db2Name << ':';
		logStats(newR);
	}
}


// Instantiate this test object
VertAttribTest vertAttribTest("vertattrib", "window, rgb",
	"Verify that the glVertexAttribNV, glVertexAttribARB, and glVertexAttrib\n"
	"functions all work correctly.\n");


} // namespace GLEAN
