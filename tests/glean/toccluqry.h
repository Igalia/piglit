// BEGIN_COPYRIGHT -*- glean -*-

/*
 * Copyright © 2006 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *	Wei Wang <wei.z.wang@intel.com>
 *
 */


// toccluqry.h:  Test basic ARB_occlusion_query support.

#ifndef __toccluqry_h__
#define __toccluqry_h__

#include "tmultitest.h"

namespace GLEAN {

class OccluQryTest: public MultiTest {
    public:
	OccluQryTest(const char* testName, const char* filter,
		    const char *prereqs, const char* description):
	    MultiTest(testName, filter, prereqs, description) {
	}

	virtual void runOne(MultiTestResult& r, Window& w);
   protected:
	bool conformOQ_GetObjivAval_multi1(GLuint id);
	bool conformOQ_GetObjivAval_multi2();
	bool conformOQ_GetQry_CnterBit();
	bool conformOQ_Begin_unused_id();
	bool conformOQ_EndAfter(GLuint id);
	bool conformOQ_GenIn(GLuint id);
	bool conformOQ_BeginIn(GLuint id);
	bool conformOQ_DeleteIn(GLuint id);
	bool conformOQ_GetObjAvalIn(GLuint id);
	bool conformOQ_GetObjResultIn(GLuint id);
	bool conformOQ_GetObjivAval(GLuint id);
	bool conformOQ_Gen_Delete(unsigned int id_n);
	bool conformOQ_IsIdZero(void);
	bool conformOQ_BeginIdZero(void);
   private:
	void gen_box(GLfloat left, GLfloat right, GLfloat top, GLfloat btm);
	GLuint find_unused_id();
	bool chk_ext();
	void setup();
	void reportPassFail(MultiTestResult &r, bool pass, const char *msg) const;
	void reportError(const char *msg);
	void reportWarning(const char *msg);
};

} // namespace GLEAN

#endif
