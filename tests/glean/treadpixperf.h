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

// Brian Paul  September 2006

#ifndef __treadpixperf_h__
#define __treadpixperf_h__

#include "tbase.h"

namespace GLEAN {

#define windowSize 1000


class ReadpixPerfResult: public BaseResult
{
public:
	struct SubResult
	{
		double rate;
		GLsizei width, height;
		int formatNum;
		int pboMode;
		int work; // really bool
		void sprint(char *s) const;
		void print(Environment *env) const;
		char readBuf[10]; // "GL_FRONT" or "GL_BACK"
	};	

	bool pass;

	vector<SubResult> results;

	typedef vector<ReadpixPerfResult::SubResult>::const_iterator sub_iterator;

	virtual void putresults(ostream& s) const;
	virtual bool getresults(istream& s);
};


class ReadpixPerfTest: public BaseTest<ReadpixPerfResult>
{
public:
	GLEAN_CLASS_WH(ReadpixPerfTest, ReadpixPerfResult,
		       windowSize, windowSize);

private:
        int depthBits, stencilBits;
	int numPBOmodes;

	double runPBOtest(int formatNum, GLsizei width, GLsizei height,
			  GLenum bufferUsage, GLuint *sumOut);
	double runNonPBOtest(int formatNum, GLsizei width, GLsizei height,
			     GLuint *sumOut);

        void setup(void);
};

} // namespace GLEAN

#endif // __treadpixperf_h__

