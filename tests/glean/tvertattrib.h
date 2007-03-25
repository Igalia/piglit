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


#ifndef __tvertattrib_h__
#define __tvertattrib_h__

#include "tbase.h"

namespace GLEAN {


class VertAttribResult: public BaseResult
{
public:
	bool pass;
	int numNVtested, numARBtested, num20tested;
	
	VertAttribResult()
	{
		numNVtested = numARBtested = num20tested = 0;
	}

	virtual void putresults(ostream& s) const
	{
		s << pass
		  << ' ' << numNVtested
		  << ' ' << numARBtested
		  << ' ' << num20tested
		  << '\n';
	}

	virtual bool getresults(istream& s)
	{
		s >> pass >> numNVtested >> numARBtested >> num20tested;
		return s.good();
	}
};


class VertAttribTest: public BaseTest<VertAttribResult>
{
public:
	GLEAN_CLASS(VertAttribTest, VertAttribResult);
	virtual void logStats(VertAttribResult& r);

private:
	enum Aliasing {
		REQUIRED,
		DISALLOWED,
		OPTIONAL
	};

	void FailMessage(VertAttribResult &r, const char *msg,
			 const char *ext, int dlistMode) const;
	bool TestAttribs(VertAttribResult &r,
			 int attribFunc,
			 PFNGLGETVERTEXATTRIBFVARBPROC getAttribfv,
			 Aliasing aliasing,
			 int numAttribs);
	bool TestNVfuncs(VertAttribResult &r);
	bool TestARBfuncs(VertAttribResult &r, bool shader);
	bool Test20funcs(VertAttribResult &r);
};

} // namespace GLEAN

#endif // __tvertattrib_h__
