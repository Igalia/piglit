// BEGIN_COPYRIGHT -*- glean -*-

/* 
 * Copyright (C) 2007  Intel Coporation  All Rights Reserved.
 * 
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 * 
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the
 * Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
 * KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL ALLEN AKIN BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 * 
 */

// tpointsprite.h:  Test basic ARB_point_sprite support.
// Author: Nian Wu <nian.wu@intel.com>


#ifndef __tpointsprite_h__
#define __tpointsprite_h__

#include "tmultitest.h"

namespace GLEAN {

#define WINSIZE 80 

class PointSpriteTest: public MultiTest
{
    public:
	PointSpriteTest(const char* testName, const char* filter,
		 const char *extensions, const char* description):
		 MultiTest(testName, filter, extensions, description){
	}

	virtual void runOne(MultiTestResult &r, Window &w);

private:
	GLfloat *texImages[6];
	GLfloat mTolerance[3];

	void GenMipmap();
	void SetupMipmap(GLuint *texID);
	void CheckDefaultState(MultiTestResult &r);
	void CalculateTolerance();
	GLboolean OutOfPoint(int x, int y, int pSize, int x0, int y0);
	GLfloat *GetTexColor(int pSize, int dir);
	GLboolean CompareColor(GLfloat *actual, GLfloat *expected);
	GLboolean ComparePixels(GLfloat *buf, int pSize, int coordOrigin);
}; // class PointSpriteTest

} // namespace GLEAN

#endif // __tpointsprite_h__

