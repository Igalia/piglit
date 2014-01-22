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
AttribFuncNames[] = {
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
	"glVertexAttrib4Nsv",
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
static void
SetAttrib(GLuint index, const GLfloat v[4], GLuint funcIndex, GLfloat refOut[4])
{
	switch (funcIndex) {
	// ** GLfloat-valued functions
#if defined(GL_NV_vertex_program)
	case 0:
		{
			glVertexAttrib1fNV(index, v[0]);
			COPY1(refOut, v);
		}
		break;
	case 1:
		{
			glVertexAttrib2fNV(index, v[0], v[1]);
			COPY2(refOut, v);
		}
		break;
	case 2:
		{
			glVertexAttrib3fNV(index, v[0], v[1], v[2]);
			COPY3(refOut, v);
		}
		break;
	case 3:
		{
			glVertexAttrib4fNV(index, v[0], v[1], v[2], v[3]);
			COPY4(refOut, v);
		}
		break;
	case 4:
		{
			glVertexAttrib1fvNV(index, v);
			COPY1(refOut, v);
		}
		break;
	case 5:
		{
			glVertexAttrib2fvNV(index, v);
			COPY2(refOut, v);
		}
		break;
	case 6:
		{
			glVertexAttrib3fvNV(index, v);
			COPY3(refOut, v);
		}
		break;
	case 7:
		{
			glVertexAttrib4fvNV(index, v);
			COPY4(refOut, v);
		}
		break;
	// ** GLdouble-valued functions
	case 8:
		{
			glVertexAttrib1dNV(index, v[0]);
			COPY1(refOut, v);
		}
		break;
	case 9:
		{
			glVertexAttrib2dNV(index, v[0], v[1]);
			COPY2(refOut, v);
		}
		break;
	case 10:
		{
			glVertexAttrib3dNV(index, v[0], v[1], v[2]);
			COPY3(refOut, v);
		}
		break;
	case 11:
		{
			glVertexAttrib4dNV(index, v[0], v[1], v[2], v[3]);
			COPY4(refOut, v);
		}
		break;
	case 12:
		{
			GLdouble d[1];
			d[0] = v[0];
			glVertexAttrib1dvNV(index, d);
			COPY1(refOut, v);
		}
		break;
	case 13:
		{
			GLdouble d[2];
			d[0] = v[0];
			d[1] = v[1];
			glVertexAttrib2dvNV(index, d);
			COPY2(refOut, v);
		}
		break;
	case 14:
		{
			GLdouble d[3];
			d[0] = v[0];
			d[1] = v[1];
			d[2] = v[2];
			glVertexAttrib3dvNV(index, d);
			COPY3(refOut, v);
		}
		break;
	case 15:
		{
			GLdouble d[4];
			d[0] = v[0];
			d[1] = v[1];
			d[2] = v[2];
			d[3] = v[3];
			glVertexAttrib4dvNV(index, d);
			COPY4(refOut, v);
		}
		break;
	// ** GLshort-valued functions
	case 16:
		{
			glVertexAttrib1sNV(index, (GLshort) v[0]);
			refOut[0] = (GLfloat) (GLshort) v[0];
			refOut[1] = 0.0F;
			refOut[2] = 0.0F;
			refOut[3] = 1.0F;
		}
		break;
	case 17:
		{
			glVertexAttrib2sNV(index, (GLshort) v[0], (GLshort) v[1]);
			refOut[0] = (GLfloat) (GLshort) v[0];
			refOut[1] = (GLfloat) (GLshort) v[1];
			refOut[2] = 0.0F;
			refOut[3] = 1.0F;
		}
		break;
	case 18:
		{
			glVertexAttrib3sNV(index, (GLshort) v[0], (GLshort) v[1], (GLshort) v[2]);
			refOut[0] = (GLfloat) (GLshort) v[0];
			refOut[1] = (GLfloat) (GLshort) v[1];
			refOut[2] = (GLfloat) (GLshort) v[2];
			refOut[3] = 1.0F;
		}
		break;
	case 19:
		{
			glVertexAttrib4sNV(index, (GLshort) v[0], (GLshort) v[1], (GLshort) v[2], (GLshort) v[3]);
			refOut[0] = (GLfloat) (GLshort) v[0];
			refOut[1] = (GLfloat) (GLshort) v[1];
			refOut[2] = (GLfloat) (GLshort) v[2];
			refOut[3] = (GLfloat) (GLshort) v[3];
		}
		break;
	case 20:
		{
			GLshort s[1];
			s[0] = (GLshort) v[0];
			glVertexAttrib1svNV(index, s);
			refOut[0] = (GLfloat) (GLshort) v[0];
			refOut[1] = 0.0F;
			refOut[2] = 0.0F;
			refOut[3] = 1.0F;
		}
		break;
	case 21:
		{
			GLshort s[2];
			s[0] = (GLshort) v[0];
			s[1] = (GLshort) v[1];
			glVertexAttrib2svNV(index, s);
			refOut[0] = (GLfloat) (GLshort) v[0];
			refOut[1] = (GLfloat) (GLshort) v[1];
			refOut[2] = 0.0F;
			refOut[3] = 1.0F;
		}
		break;
	case 22:
		{
			GLshort s[3];
			s[0] = (GLshort) v[0];
			s[1] = (GLshort) v[1];
			s[2] = (GLshort) v[2];
			glVertexAttrib3svNV(index, s);
			refOut[0] = (GLfloat) (GLshort) v[0];
			refOut[1] = (GLfloat) (GLshort) v[1];
			refOut[2] = (GLfloat) (GLshort) v[2];
			refOut[3] = 1.0F;
		}
		break;
	case 23:
		{
			GLshort s[4];
			s[0] = (GLshort) v[0];
			s[1] = (GLshort) v[1];
			s[2] = (GLshort) v[2];
			s[3] = (GLshort) v[3];
			glVertexAttrib4svNV(index, s);
			refOut[0] = (GLfloat) (GLshort) v[0];
			refOut[1] = (GLfloat) (GLshort) v[1];
			refOut[2] = (GLfloat) (GLshort) v[2];
			refOut[3] = (GLfloat) (GLshort) v[3];
		}
		break;
	// ** GLubyte-valued functions
	case 24:
		{
			glVertexAttrib4ubNV(index, FLOAT_TO_UBYTE(v[0]), FLOAT_TO_UBYTE(v[1]), FLOAT_TO_UBYTE(v[2]), FLOAT_TO_UBYTE(v[3]));
			refOut[0] = v[0];
			refOut[1] = v[1];
			refOut[2] = v[2];
			refOut[3] = v[3];
		}
		break;
	case 25:
		{
			GLubyte ub[4];
			for (int i = 0; i < 4; i++ )
			   ub[i] = FLOAT_TO_UBYTE(v[i]);
			glVertexAttrib4ubvNV(index, ub);
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
			glVertexAttrib1fARB(index, v[0]);
			COPY1(refOut, v);
		}
		break;
	case 27:
		{
			glVertexAttrib2fARB(index, v[0], v[1]);
			COPY2(refOut, v);
		}
		break;
	case 28:
		{
			glVertexAttrib3fARB(index, v[0], v[1], v[2]);
			COPY3(refOut, v);
		}
		break;
	case 29:
		{
			glVertexAttrib4fARB(index, v[0], v[1], v[2], v[3]);
			COPY4(refOut, v);
		}
		break;
	case 30:
		{
			glVertexAttrib1fvARB(index, v);
			COPY1(refOut, v);
		}
		break;
	case 31:
		{
			glVertexAttrib2fvARB(index, v);
			COPY2(refOut, v);
		}
		break;
	case 32:
		{
			glVertexAttrib3fvARB(index, v);
			COPY3(refOut, v);
		}
		break;
	case 33:
		{
			glVertexAttrib4fvARB(index, v);
			COPY4(refOut, v);
		}
		break;
	// ** GLdouble-valued functions
	case 34:
		{
			glVertexAttrib1dARB(index, v[0]);
			COPY1(refOut, v);
		}
		break;
	case 35:
		{
			glVertexAttrib2dARB(index, v[0], v[1]);
			COPY2(refOut, v);
		}
		break;
	case 36:
		{
			glVertexAttrib3dARB(index, v[0], v[1], v[2]);
			COPY3(refOut, v);
		}
		break;
	case 37:
		{
			glVertexAttrib4dARB(index, v[0], v[1], v[2], v[3]);
			COPY4(refOut, v);
		}
		break;
	case 38:
		{
			GLdouble d[1];
			d[0] = v[0];
			glVertexAttrib1dvARB(index, d);
			COPY1(refOut, v);
		}
		break;
	case 39:
		{
			GLdouble d[2];
			d[0] = v[0];
			d[1] = v[1];
			glVertexAttrib2dvARB(index, d);
			COPY2(refOut, v);
		}
		break;
	case 40:
		{
			GLdouble d[3];
			d[0] = v[0];
			d[1] = v[1];
			d[2] = v[2];
			glVertexAttrib3dvARB(index, d);
			COPY3(refOut, v);
		}
		break;
	case 41:
		{
			GLdouble d[4];
			d[0] = v[0];
			d[1] = v[1];
			d[2] = v[2];
			d[3] = v[3];
			glVertexAttrib4dvARB(index, d);
			COPY4(refOut, v);
		}
		break;
	// ** GLshort-valued functions
	case 42:
		{
			glVertexAttrib1sARB(index, (GLshort) v[0]);
			refOut[0] = (GLfloat) (GLshort) v[0];
			refOut[1] = 0.0F;
			refOut[2] = 0.0F;
			refOut[3] = 1.0F;
		}
		break;
	case 43:
		{
			glVertexAttrib2sARB(index, (GLshort) v[0], (GLshort) v[1]);
			refOut[0] = (GLfloat) (GLshort) v[0];
			refOut[1] = (GLfloat) (GLshort) v[1];
			refOut[2] = 0.0F;
			refOut[3] = 1.0F;
		}
		break;
	case 44:
		{
			glVertexAttrib3sARB(index, (GLshort) v[0], (GLshort) v[1], (GLshort) v[2]);
			refOut[0] = (GLfloat) (GLshort) v[0];
			refOut[1] = (GLfloat) (GLshort) v[1];
			refOut[2] = (GLfloat) (GLshort) v[2];
			refOut[3] = 1.0F;
		}
		break;
	case 45:
		{
			glVertexAttrib4sARB(index, (GLshort) v[0], (GLshort) v[1], (GLshort) v[2], (GLshort) v[3]);
			refOut[0] = (GLfloat) (GLshort) v[0];
			refOut[1] = (GLfloat) (GLshort) v[1];
			refOut[2] = (GLfloat) (GLshort) v[2];
			refOut[3] = (GLfloat) (GLshort) v[3];
		}
		break;
	case 46:
		{
			GLshort s[1];
			s[0] = (GLshort) v[0];
			glVertexAttrib1svARB(index, s);
			refOut[0] = (GLfloat) (GLshort) v[0];
			refOut[1] = 0.0F;
			refOut[2] = 0.0F;
			refOut[3] = 1.0F;
		}
		break;
	case 47:
		{
			GLshort s[2];
			s[0] = (GLshort) v[0];
			s[1] = (GLshort) v[1];
			glVertexAttrib2svARB(index, s);
			refOut[0] = (GLfloat) (GLshort) v[0];
			refOut[1] = (GLfloat) (GLshort) v[1];
			refOut[2] = 0.0F;
			refOut[3] = 1.0F;
		}
		break;
	case 48:
		{
			GLshort s[3];
			s[0] = (GLshort) v[0];
			s[1] = (GLshort) v[1];
			s[2] = (GLshort) v[2];
			glVertexAttrib3svARB(index, s);
			refOut[0] = (GLfloat) (GLshort) v[0];
			refOut[1] = (GLfloat) (GLshort) v[1];
			refOut[2] = (GLfloat) (GLshort) v[2];
			refOut[3] = 1.0F;
		}
		break;
	case 49:
		{
			GLshort s[4];
			s[0] = (GLshort) v[0];
			s[1] = (GLshort) v[1];
			s[2] = (GLshort) v[2];
			s[3] = (GLshort) v[3];
			glVertexAttrib4svARB(index, s);
			refOut[0] = (GLfloat) (GLshort) v[0];
			refOut[1] = (GLfloat) (GLshort) v[1];
			refOut[2] = (GLfloat) (GLshort) v[2];
			refOut[3] = (GLfloat) (GLshort) v[3];
		}
		break;
	case 50:
		{
			GLshort s[4];
			for (int i = 0; i < 4; i++)
				s[i] = FLOAT_TO_SHORT(v[i]);
			glVertexAttrib4NsvARB(index, s);
			COPY4(refOut, v);
		}
		break;
	// ** GLubyte-valued functions
	case 51:
		{
			glVertexAttrib4NubARB(index, FLOAT_TO_UBYTE(v[0]), FLOAT_TO_UBYTE(v[1]), FLOAT_TO_UBYTE(v[2]), FLOAT_TO_UBYTE(v[3]));
			COPY4(refOut, v);
		}
		break;
	case 52:
		{
			GLubyte ub[4];
			for (int i = 0; i < 4; i++ )
			   ub[i] = FLOAT_TO_UBYTE(v[i]);
			glVertexAttrib4NubvARB(index, ub);
			COPY4(refOut, v);
		}
		break;
	case 53:
		{
			GLubyte ub[4];
			for (int i = 0; i < 4; i++ )
			   ub[i] = (GLubyte) v[i];
			glVertexAttrib4ubvARB(index, ub);
			refOut[0] = (GLfloat) (GLubyte) v[0];
			refOut[1] = (GLfloat) (GLubyte) v[1];
			refOut[2] = (GLfloat) (GLubyte) v[2];
			refOut[3] = (GLfloat) (GLubyte) v[3];
		}
		break;
	// ** GLbyte-valued functions
	case 54:
		{
			GLbyte b[4];
			for (int i = 0; i < 4; i++ )
			   b[i] = FLOAT_TO_BYTE(v[i]);
			glVertexAttrib4NbvARB(index, b);
			COPY4(refOut, v);
		}
		break;
	case 55:
		{
			GLbyte b[4];
			for (int i = 0; i < 4; i++ )
			   b[i] = (GLbyte) v[i];
			glVertexAttrib4bvARB(index, b);
			refOut[0] = (GLfloat) (GLbyte) v[0];
			refOut[1] = (GLfloat) (GLbyte) v[1];
			refOut[2] = (GLfloat) (GLbyte) v[2];
			refOut[3] = (GLfloat) (GLbyte) v[3];
		}
		break;
	// ** GLint-valued functions
	case 56:
		{
			GLint iv[4];
			for (int i = 0; i < 4; i++ )
			   iv[i] = FLOAT_TO_INT(v[i]);
			glVertexAttrib4NivARB(index, iv);
			COPY4(refOut, v);
		}
		break;
	case 57:
		{
			GLint iv[4];
			for (int i = 0; i < 4; i++ )
			   iv[i] = (GLint) v[i];
			glVertexAttrib4ivARB(index, iv);
			refOut[0] = (GLfloat) (GLint) v[0];
			refOut[1] = (GLfloat) (GLint) v[1];
			refOut[2] = (GLfloat) (GLint) v[2];
			refOut[3] = (GLfloat) (GLint) v[3];
		}
		break;
	// ** GLuint-valued functions
	case 58:
		{
			GLuint ui[4];
			for (int i = 0; i < 4; i++ )
			   ui[i] = FLOAT_TO_UINT(v[i]);
			glVertexAttrib4NuivARB(index, ui);
			COPY4(refOut, v);
		}
		break;
	case 59:
		{
			GLuint ui[4];
			for (int i = 0; i < 4; i++ )
			   ui[i] = (GLint) v[i];
			glVertexAttrib4uivARB(index, ui);
			refOut[0] = (GLfloat) (GLint) v[0];
			refOut[1] = (GLfloat) (GLint) v[1];
			refOut[2] = (GLfloat) (GLint) v[2];
			refOut[3] = (GLfloat) (GLint) v[3];
		}
		break;
	// ** GLushort-valued functions
	case 60:
		{
			GLushort us[4];
			for (int i = 0; i < 4; i++ )
			   us[i] = FLOAT_TO_USHORT(v[i]);
			glVertexAttrib4NusvARB(index, us);
			COPY4(refOut, v);
		}
		break;
	case 61:
		{
			GLushort us[4];
			for (int i = 0; i < 4; i++ )
			   us[i] = (GLint) v[i];
			glVertexAttrib4usvARB(index, us);
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
			glVertexAttrib1f(index, v[0]);
			COPY1(refOut, v);
		}
		break;
	case 63:
		{
			glVertexAttrib2f(index, v[0], v[1]);
			COPY2(refOut, v);
		}
		break;
	case 64:
		{
			glVertexAttrib3f(index, v[0], v[1], v[2]);
			COPY3(refOut, v);
		}
		break;
	case 65:
		{
			glVertexAttrib4f(index, v[0], v[1], v[2], v[3]);
			COPY4(refOut, v);
		}
		break;
	case 66:
		{
			glVertexAttrib1fv(index, v);
			COPY1(refOut, v);
		}
		break;
	case 67:
		{
			glVertexAttrib2fv(index, v);
			COPY2(refOut, v);
		}
		break;
	case 68:
		{
			glVertexAttrib3fv(index, v);
			COPY3(refOut, v);
		}
		break;
	case 69:
		{
			glVertexAttrib4fv(index, v);
			COPY4(refOut, v);
		}
		break;
	// ** GLdouble-valued functions
	case 70:
		{
			glVertexAttrib1d(index, v[0]);
			COPY1(refOut, v);
		}
		break;
	case 71:
		{
			glVertexAttrib2d(index, v[0], v[1]);
			COPY2(refOut, v);
		}
		break;
	case 72:
		{
			glVertexAttrib3d(index, v[0], v[1], v[2]);
			COPY3(refOut, v);
		}
		break;
	case 73:
		{
			glVertexAttrib4d(index, v[0], v[1], v[2], v[3]);
			COPY4(refOut, v);
		}
		break;
	case 74:
		{
			GLdouble d[1];
			d[0] = v[0];
			glVertexAttrib1dv(index, d);
			COPY1(refOut, v);
		}
		break;
	case 75:
		{
			GLdouble d[2];
			d[0] = v[0];
			d[1] = v[1];
			glVertexAttrib2dv(index, d);
			COPY2(refOut, v);
		}
		break;
	case 76:
		{
			GLdouble d[3];
			d[0] = v[0];
			d[1] = v[1];
			d[2] = v[2];
			glVertexAttrib3dv(index, d);
			COPY3(refOut, v);
		}
		break;
	case 77:
		{
			GLdouble d[4];
			d[0] = v[0];
			d[1] = v[1];
			d[2] = v[2];
			d[3] = v[3];
			glVertexAttrib4dv(index, d);
			COPY4(refOut, v);
		}
		break;
	// ** GLshort-valued functions
	case 78:
		{
			glVertexAttrib1s(index, (GLshort) v[0]);
			refOut[0] = (GLfloat) (GLshort) v[0];
			refOut[1] = 0.0F;
			refOut[2] = 0.0F;
			refOut[3] = 1.0F;
		}
		break;
	case 79:
		{
			glVertexAttrib2s(index, (GLshort) v[0], (GLshort) v[1]);
			refOut[0] = (GLfloat) (GLshort) v[0];
			refOut[1] = (GLfloat) (GLshort) v[1];
			refOut[2] = 0.0F;
			refOut[3] = 1.0F;
		}
		break;
	case 80:
		{
			glVertexAttrib3s(index, (GLshort) v[0], (GLshort) v[1], (GLshort) v[2]);
			refOut[0] = (GLfloat) (GLshort) v[0];
			refOut[1] = (GLfloat) (GLshort) v[1];
			refOut[2] = (GLfloat) (GLshort) v[2];
			refOut[3] = 1.0F;
		}
		break;
	case 81:
		{
			glVertexAttrib4s(index, (GLshort) v[0], (GLshort) v[1], (GLshort) v[2], (GLshort) v[3]);
			refOut[0] = (GLfloat) (GLshort) v[0];
			refOut[1] = (GLfloat) (GLshort) v[1];
			refOut[2] = (GLfloat) (GLshort) v[2];
			refOut[3] = (GLfloat) (GLshort) v[3];
		}
		break;
	case 82:
		{
			GLshort s[1];
			s[0] = (GLshort) v[0];
			glVertexAttrib1sv(index, s);
			refOut[0] = (GLfloat) (GLshort) v[0];
			refOut[1] = 0.0F;
			refOut[2] = 0.0F;
			refOut[3] = 1.0F;
		}
		break;
	case 83:
		{
			GLshort s[2];
			s[0] = (GLshort) v[0];
			s[1] = (GLshort) v[1];
			glVertexAttrib2sv(index, s);
			refOut[0] = (GLfloat) (GLshort) v[0];
			refOut[1] = (GLfloat) (GLshort) v[1];
			refOut[2] = 0.0F;
			refOut[3] = 1.0F;
		}
		break;
	case 84:
		{
			GLshort s[3];
			s[0] = (GLshort) v[0];
			s[1] = (GLshort) v[1];
			s[2] = (GLshort) v[2];
			glVertexAttrib3sv(index, s);
			refOut[0] = (GLfloat) (GLshort) v[0];
			refOut[1] = (GLfloat) (GLshort) v[1];
			refOut[2] = (GLfloat) (GLshort) v[2];
			refOut[3] = 1.0F;
		}
		break;
	case 85:
		{
			GLshort s[4];
			s[0] = (GLshort) v[0];
			s[1] = (GLshort) v[1];
			s[2] = (GLshort) v[2];
			s[3] = (GLshort) v[3];
			glVertexAttrib4sv(index, s);
			refOut[0] = (GLfloat) (GLshort) v[0];
			refOut[1] = (GLfloat) (GLshort) v[1];
			refOut[2] = (GLfloat) (GLshort) v[2];
			refOut[3] = (GLfloat) (GLshort) v[3];
		}
		break;
	case 86:
		{
			GLshort s[4];
			for (int i = 0; i < 4; i++)
				s[i] = FLOAT_TO_SHORT(v[i]);
			glVertexAttrib4Nsv(index, s);
			COPY4(refOut, v);
		}
		break;
	// ** GLubyte-valued functions
	case 87:
		{
			glVertexAttrib4Nub(index, FLOAT_TO_UBYTE(v[0]), FLOAT_TO_UBYTE(v[1]), FLOAT_TO_UBYTE(v[2]), FLOAT_TO_UBYTE(v[3]));
			COPY4(refOut, v);
		}
		break;
	case 88:
		{
			GLubyte ub[4];
			for (int i = 0; i < 4; i++ )
			   ub[i] = FLOAT_TO_UBYTE(v[i]);
			glVertexAttrib4Nubv(index, ub);
			COPY4(refOut, v);
		}
		break;
	case 89:
		{
			GLubyte ub[4];
			for (int i = 0; i < 4; i++ )
			   ub[i] = (GLubyte) v[i];
			glVertexAttrib4ubv(index, ub);
			refOut[0] = (GLfloat) (GLubyte) v[0];
			refOut[1] = (GLfloat) (GLubyte) v[1];
			refOut[2] = (GLfloat) (GLubyte) v[2];
			refOut[3] = (GLfloat) (GLubyte) v[3];
		}
		break;
	// ** GLbyte-valued functions
	case 90:
		{
			GLbyte b[4];
			for (int i = 0; i < 4; i++ )
			   b[i] = FLOAT_TO_BYTE(v[i]);
			glVertexAttrib4Nbv(index, b);
			COPY4(refOut, v);
		}
		break;
	case 91:
		{
			GLbyte b[4];
			for (int i = 0; i < 4; i++ )
			   b[i] = (GLbyte) v[i];
			glVertexAttrib4bv(index, b);
			refOut[0] = (GLfloat) (GLbyte) v[0];
			refOut[1] = (GLfloat) (GLbyte) v[1];
			refOut[2] = (GLfloat) (GLbyte) v[2];
			refOut[3] = (GLfloat) (GLbyte) v[3];
		}
		break;
	// ** GLint-valued functions
	case 92:
		{
			GLint iv[4];
			for (int i = 0; i < 4; i++ )
			   iv[i] = FLOAT_TO_INT(v[i]);
			glVertexAttrib4Niv(index, iv);
			COPY4(refOut, v);
		}
		break;
	case 93:
		{
			GLint iv[4];
			for (int i = 0; i < 4; i++ )
			   iv[i] = (GLint) v[i];
			glVertexAttrib4iv(index, iv);
			refOut[0] = (GLfloat) (GLint) v[0];
			refOut[1] = (GLfloat) (GLint) v[1];
			refOut[2] = (GLfloat) (GLint) v[2];
			refOut[3] = (GLfloat) (GLint) v[3];
		}
		break;
	// ** GLuint-valued functions
	case 94:
		{
			GLuint ui[4];
			for (int i = 0; i < 4; i++ )
			   ui[i] = FLOAT_TO_UINT(v[i]);
			glVertexAttrib4Nuiv(index, ui);
			COPY4(refOut, v);
		}
		break;
	case 95:
		{
			GLuint ui[4];
			for (int i = 0; i < 4; i++ )
			   ui[i] = (GLint) v[i];
			glVertexAttrib4uiv(index, ui);
			refOut[0] = (GLfloat) (GLint) v[0];
			refOut[1] = (GLfloat) (GLint) v[1];
			refOut[2] = (GLfloat) (GLint) v[2];
			refOut[3] = (GLfloat) (GLint) v[3];
		}
		break;
	// ** GLushort-valued functions
	case 96:
		{
			GLushort us[4];
			for (int i = 0; i < 4; i++ )
			   us[i] = FLOAT_TO_USHORT(v[i]);
			glVertexAttrib4Nusv(index, us);
			COPY4(refOut, v);
		}
		break;
	case 97:
		{
			GLushort us[4];
			for (int i = 0; i < 4; i++ )
			   us[i] = (GLint) v[i];
			glVertexAttrib4usv(index, us);
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
			glGetVertexAttribfvNV(i, GL_CURRENT_VERTEX_ATTRIB_ARB, v);
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
	const GLint numAttribs = 16;
	const Aliasing aliasing = REQUIRED;

	r.numNVtested = 0;

	for (int attribFunc = 0; attribFunc < NUM_NV_ATTRIB_FUNCS; attribFunc++) {
		bool b;
		b = TestAttribs(r, attribFunc, aliasing, numAttribs);
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
	GLint numAttribs;

	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS_ARB, &numAttribs);
	assert(numAttribs > 0);

	r.numARBtested = 0;

	if (shader) {
		// test GL_ARB_vertex_shader (aliasing is disallowed)
		const Aliasing aliasing = DISALLOWED;
		for (int i = 0; i < NUM_ARB_ATTRIB_FUNCS; i++) {
			int attribFunc = NUM_NV_ATTRIB_FUNCS + i;
			bool b;
			b = TestAttribs(r, attribFunc, aliasing, numAttribs);
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
			b = TestAttribs(r, attribFunc, aliasing, numAttribs);
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
	GLint numAttribs;
	const Aliasing aliasing = DISALLOWED;

	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &numAttribs);
	assert(numAttribs > 0);

	r.num20tested = 0;

	for (int i = 0; i < NUM_2_0_ATTRIB_FUNCS; i++) {
		int attribFunc = NUM_NV_ATTRIB_FUNCS + NUM_ARB_ATTRIB_FUNCS+ i;
		bool b;
		b = TestAttribs(r, attribFunc, aliasing, numAttribs);
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

	assert(sizeof(AttribFuncNames) / sizeof(AttribFuncNames[0]) ==
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


// Instantiate this test object
VertAttribTest vertAttribTest("vertattrib", "window, rgb",
	"Verify that the glVertexAttribNV, glVertexAttribARB, and glVertexAttrib\n"
	"functions all work correctly.\n");


} // namespace GLEAN
