// BEGIN_COPYRIGHT -*- glean -*-
// 
// Copyright (C) 2009  VMware, Inc. All Rights Reserved.
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
// PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL VMWARE BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
// AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
// OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
// 
// END_COPYRIGHT

// tclipflat.h:  Test clipping and flat shading

#ifndef __tclipflat_h__
#define __tclipflat_h__

#include "tbase.h"

namespace GLEAN {

#define windowSize 100

class ClipFlatResult: public BaseResult
{
public:
	bool pass;

        ClipFlatResult();

	virtual void putresults(ostream& s) const;
	virtual bool getresults(istream& s);
};


class ClipFlatTest: public BaseTest<ClipFlatResult>
{
public:
	GLEAN_CLASS_WH(ClipFlatTest, ClipFlatResult,
		       windowSize, windowSize);

private:
        bool provoking_vertex_first;
        bool quads_follows_pv_convention;
        bool testing_first_pv;

        void drawArrays(GLenum mode, const GLfloat *verts, GLuint count);
        void drawElements(GLenum mode, const GLfloat *verts, GLuint count);
        void drawBeginEnd(GLenum mode, const GLfloat *verts, GLuint count);
        bool testPositions(Window &w, GLenum mode,
                           const GLfloat *verts, GLuint count);
        void reportFailure(GLenum mode, int drawMode, GLuint facing,
                           GLuint fill,
                           const GLfloat badColor[3], GLfloat x, GLfloat y);
        bool checkResult(Window &w, GLfloat badColor[3]);

        void setup(void);
};

} // namespace GLEAN

#endif // __tclipflat_h__

