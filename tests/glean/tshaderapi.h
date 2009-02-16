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


#ifndef __tshaderapi_h__
#define __tshaderapi_h__

#include "tbase.h"

namespace GLEAN {

#define windowSize 100


class ShaderAPIResult: public BaseResult
{
public:
	bool pass;

	ShaderAPIResult();

	virtual void putresults(ostream& s) const;
	virtual bool getresults(istream& s);
};


class ShaderAPITest: public BaseTest<ShaderAPIResult>
{
public:
	// "WHO" = width, height and one config flag
	GLEAN_CLASS_WHO(ShaderAPITest, ShaderAPIResult,
					windowSize, windowSize, true);

	virtual bool isApplicable() const;

private:
	bool error;

	void assert_test(const char *file, int line, int cond, const char *msg);
	void assert_no_error_test(const char *file, int line);
	void assert_error_test(const char *file, int line, GLenum expect);

	void check_status(GLuint id, GLenum pname, void (*query)(GLuint, GLenum, GLint *));
	void check_compile_status(GLuint id);
	void check_link_status(GLuint id);

	GLuint make_shader(GLenum type, const char *src);
	GLuint make_program(const char *vs_src, const char *fs_src);

	void test_uniform_size_type1(const char *glslType, GLenum glType, const char *el);
	void test_attrib_size_type1(const char *glslType, GLenum glType, const char *el);

	void test_uniform_size_type(void);
	void test_attrib_size_type(void);
	void test_uniform_array_overflow(void);
	void test_uniform_scalar_count(void);
	void test_uniform_query_matrix(void);
	void test_uniform_neg_location(void);
	void test_uniform_bool_conversion(void);
	void test_uniform_multiple_samplers(void);
	void run_tests(void);

	void get_ext_procs(void);
};

} // namespace GLEAN

#endif // __tshaderapi_h__

