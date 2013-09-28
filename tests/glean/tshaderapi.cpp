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


// Test GLSL-related API functions for correct behaviour
// Based on the "shader_api.c" test from Mesa, written by Bruce Merry.


#include <cstring>
#include "tshaderapi.h"
#include "rand.h"
#include "image.h"


namespace GLEAN {

static PFNGLATTACHSHADERPROC glAttachShader_func = NULL;
static PFNGLBINDATTRIBLOCATIONPROC glBindAttribLocation_func = NULL;
static PFNGLCOMPILESHADERPROC glCompileShader_func = NULL;
static PFNGLCREATEPROGRAMPROC glCreateProgram_func = NULL;
static PFNGLCREATESHADERPROC glCreateShader_func = NULL;
static PFNGLDELETEPROGRAMPROC glDeleteProgram_func = NULL;
static PFNGLDELETESHADERPROC glDeleteShader_func = NULL;
static PFNGLDETACHSHADERPROC glDetachShader_func = NULL;
static PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray_func = NULL;
static PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray_func = NULL;
static PFNGLGETACTIVEATTRIBPROC glGetActiveAttrib_func = NULL;
static PFNGLGETACTIVEUNIFORMPROC glGetActiveUniform_func = NULL;
static PFNGLGETATTACHEDSHADERSPROC glGetAttachedShaders_func = NULL;
static PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation_func = NULL;
static PFNGLGETPROGRAMIVPROC glGetProgramiv_func = NULL;
static PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog_func = NULL;
static PFNGLGETSHADERIVPROC glGetShaderiv_func = NULL;
static PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog_func = NULL;
static PFNGLGETSHADERSOURCEPROC glGetShaderSource_func = NULL;
static PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation_func = NULL;
static PFNGLGETUNIFORMFVPROC glGetUniformfv_func = NULL;
static PFNGLGETUNIFORMIVPROC glGetUniformiv_func = NULL;
static PFNGLLINKPROGRAMPROC glLinkProgram_func = NULL;
static PFNGLSHADERSOURCEPROC glShaderSource_func = NULL;
static PFNGLUSEPROGRAMPROC glUseProgram_func = NULL;
static PFNGLUNIFORM1FPROC glUniform1f_func = NULL;
static PFNGLUNIFORM2FPROC glUniform2f_func = NULL;
static PFNGLUNIFORM3FPROC glUniform3f_func = NULL;
static PFNGLUNIFORM4FPROC glUniform4f_func = NULL;
static PFNGLUNIFORM1IPROC glUniform1i_func = NULL;
static PFNGLUNIFORM2IPROC glUniform2i_func = NULL;
static PFNGLUNIFORM3IPROC glUniform3i_func = NULL;
static PFNGLUNIFORM4IPROC glUniform4i_func = NULL;
static PFNGLUNIFORM1FVPROC glUniform1fv_func = NULL;
static PFNGLUNIFORM2FVPROC glUniform2fv_func = NULL;
static PFNGLUNIFORM3FVPROC glUniform3fv_func = NULL;
static PFNGLUNIFORM4FVPROC glUniform4fv_func = NULL;
static PFNGLUNIFORM1IVPROC glUniform1iv_func = NULL;
static PFNGLUNIFORM2IVPROC glUniform2iv_func = NULL;
static PFNGLUNIFORM3IVPROC glUniform3iv_func = NULL;
static PFNGLUNIFORM4IVPROC glUniform4iv_func = NULL;
static PFNGLUNIFORMMATRIX2FVPROC glUniformMatrix2fv_func = NULL;
static PFNGLUNIFORMMATRIX3FVPROC glUniformMatrix3fv_func = NULL;
static PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv_func = NULL;
static PFNGLVALIDATEPROGRAMPROC glValidateProgram_func = NULL;


ShaderAPIResult::ShaderAPIResult()
{
	pass = false;
}


void
ShaderAPITest::get_ext_procs(void)
{
	glAttachShader_func = (PFNGLATTACHSHADERPROC) GLUtils::getProcAddress("glAttachShader");
	glBindAttribLocation_func = (PFNGLBINDATTRIBLOCATIONPROC) GLUtils::getProcAddress("glBindAttribLocation");
	glCompileShader_func = (PFNGLCOMPILESHADERPROC) GLUtils::getProcAddress("glCompileShader");
	glCreateProgram_func = (PFNGLCREATEPROGRAMPROC) GLUtils::getProcAddress("glCreateProgram");
	glCreateShader_func = (PFNGLCREATESHADERPROC) GLUtils::getProcAddress("glCreateShader");
	glDeleteProgram_func = (PFNGLDELETEPROGRAMPROC) GLUtils::getProcAddress("glDeleteProgram");
	glDeleteShader_func = (PFNGLDELETESHADERPROC) GLUtils::getProcAddress("glDeleteShader");
	glDetachShader_func = (PFNGLDETACHSHADERPROC) GLUtils::getProcAddress("glDetachShader");
	glDisableVertexAttribArray_func = (PFNGLDISABLEVERTEXATTRIBARRAYPROC) GLUtils::getProcAddress("glDisableVertexAttribArray");
	glEnableVertexAttribArray_func = (PFNGLENABLEVERTEXATTRIBARRAYPROC) GLUtils::getProcAddress("glEnableVertexAttribArray");
	glGetActiveAttrib_func = (PFNGLGETACTIVEATTRIBPROC) GLUtils::getProcAddress("glGetActiveAttrib");
	glGetActiveUniform_func = (PFNGLGETACTIVEUNIFORMPROC) GLUtils::getProcAddress("glGetActiveUniform");
	glGetAttachedShaders_func = (PFNGLGETATTACHEDSHADERSPROC) GLUtils::getProcAddress("glGetAttachedShaders");
	glGetAttribLocation_func = (PFNGLGETATTRIBLOCATIONPROC) GLUtils::getProcAddress("glGetAttribLocation");
	glGetProgramiv_func = (PFNGLGETPROGRAMIVPROC) GLUtils::getProcAddress("glGetProgramiv");
	glGetProgramInfoLog_func = (PFNGLGETPROGRAMINFOLOGPROC) GLUtils::getProcAddress("glGetProgramInfoLog");
	glGetShaderiv_func = (PFNGLGETSHADERIVPROC) GLUtils::getProcAddress("glGetShaderiv");
	glGetShaderInfoLog_func = (PFNGLGETSHADERINFOLOGPROC) GLUtils::getProcAddress("glGetShaderInfoLog");
	glGetShaderSource_func = (PFNGLGETSHADERSOURCEPROC) GLUtils::getProcAddress("glGetShaderSource");
	glGetUniformLocation_func = (PFNGLGETUNIFORMLOCATIONPROC) GLUtils::getProcAddress("glGetUniformLocation");
	glGetUniformfv_func = (PFNGLGETUNIFORMFVPROC) GLUtils::getProcAddress("glGetUniformfv");
	glGetUniformiv_func = (PFNGLGETUNIFORMIVPROC) GLUtils::getProcAddress("glGetUniformiv");
	glLinkProgram_func = (PFNGLLINKPROGRAMPROC) GLUtils::getProcAddress("glLinkProgram");
	glShaderSource_func = (PFNGLSHADERSOURCEPROC) GLUtils::getProcAddress("glShaderSource");
	glUseProgram_func = (PFNGLUSEPROGRAMPROC) GLUtils::getProcAddress("glUseProgram");
	glUniform1f_func = (PFNGLUNIFORM1FPROC) GLUtils::getProcAddress("glUniform1f");
	glUniform2f_func = (PFNGLUNIFORM2FPROC) GLUtils::getProcAddress("glUniform2f");
	glUniform3f_func = (PFNGLUNIFORM3FPROC) GLUtils::getProcAddress("glUniform3f");
	glUniform4f_func = (PFNGLUNIFORM4FPROC) GLUtils::getProcAddress("glUniform4f");
	glUniform1i_func = (PFNGLUNIFORM1IPROC) GLUtils::getProcAddress("glUniform1i");
	glUniform2i_func = (PFNGLUNIFORM2IPROC) GLUtils::getProcAddress("glUniform2i");
	glUniform3i_func = (PFNGLUNIFORM3IPROC) GLUtils::getProcAddress("glUniform3i");
	glUniform4i_func = (PFNGLUNIFORM4IPROC) GLUtils::getProcAddress("glUniform4i");
	glUniform1fv_func = (PFNGLUNIFORM1FVPROC) GLUtils::getProcAddress("glUniform1fv");
	glUniform2fv_func = (PFNGLUNIFORM2FVPROC) GLUtils::getProcAddress("glUniform2fv");
	glUniform3fv_func = (PFNGLUNIFORM3FVPROC) GLUtils::getProcAddress("glUniform3fv");
	glUniform4fv_func = (PFNGLUNIFORM4FVPROC) GLUtils::getProcAddress("glUniform4fv");
	glUniform1iv_func = (PFNGLUNIFORM1IVPROC) GLUtils::getProcAddress("glUniform1iv");
	glUniform2iv_func = (PFNGLUNIFORM2IVPROC) GLUtils::getProcAddress("glUniform2iv");
	glUniform3iv_func = (PFNGLUNIFORM3IVPROC) GLUtils::getProcAddress("glUniform3iv");
	glUniform4iv_func = (PFNGLUNIFORM4IVPROC) GLUtils::getProcAddress("glUniform4iv");
	glUniformMatrix2fv_func = (PFNGLUNIFORMMATRIX2FVPROC) GLUtils::getProcAddress("glUniformMatrix2fv");
	glUniformMatrix3fv_func = (PFNGLUNIFORMMATRIX3FVPROC) GLUtils::getProcAddress("glUniformMatrix3fv");
	glUniformMatrix4fv_func = (PFNGLUNIFORMMATRIX4FVPROC) GLUtils::getProcAddress("glUniformMatrix4fv");
	glValidateProgram_func = (PFNGLVALIDATEPROGRAMPROC) GLUtils::getProcAddress("glValidateProgram");
}


void
ShaderAPITest::assert_test(const char *file, int line, int cond, const char *msg)
{
	if (!cond) {
		error = true;
		fprintf(stderr, "%s:%d assertion \"%s\" failed\n", file, line, msg);
	}
}

#undef assert
#define assert(x) assert_test(__FILE__, __LINE__, (x), #x)


void
ShaderAPITest::assert_no_error_test(const char *file, int line)
{
	GLenum err;

	err = glGetError();
	if (err != GL_NO_ERROR) {
		error = true;
		fprintf(stderr, "%s:%d received error %s\n",
				file, line, gluErrorString(err));
	}
}

#define assert_no_error() assert_no_error_test(__FILE__, __LINE__)


void
ShaderAPITest::assert_error_test(const char *file, int line, GLenum expect)
{
	GLenum err;

	err = glGetError();
	if (err != expect) {
		fprintf(stderr, "%s:%d expected %s but received %s\n",
				file, line, gluErrorString(expect), gluErrorString(err));
		error = true;
	}

	while (glGetError())
		; /* consume any following errors */
}

#define assert_error(err) assert_error_test(__FILE__, __LINE__, (err))


void
ShaderAPITest::check_status(GLuint id, GLenum pname,
			    void (APIENTRY *query)(GLuint, GLenum, GLint *),
			    void (APIENTRY *get_log)(GLuint, GLsizei, GLsizei *, GLchar *))
{
	GLint status;

	query(id, pname, &status);
	if (!status) {
		char info[65536];

		fprintf(stderr, "Compilation/link failure:\n");
		get_log(id, sizeof(info), NULL, info);
		fprintf(stderr, "%s\n", info);

		error = true;
	}
}


void
ShaderAPITest::check_compile_status(GLuint id)
{
	check_status(id, GL_COMPILE_STATUS, glGetShaderiv_func,
		     glGetShaderInfoLog_func);
}


void
ShaderAPITest::check_link_status(GLuint id)
{
	check_status(id, GL_LINK_STATUS, glGetProgramiv_func,
		     glGetProgramInfoLog_func);
}


GLuint
ShaderAPITest::make_shader(GLenum type, const char *src)
{
	GLuint id;

	assert_no_error();
	id = glCreateShader_func(type);
	glShaderSource_func(id, 1, &src, NULL);
	glCompileShader_func(id);
	check_compile_status(id);
	assert_no_error();
	return id;
}


GLuint
ShaderAPITest::make_program(const char *vs_src, const char *fs_src)
{
	GLuint id, vs, fs;

	assert_no_error();
	id = glCreateProgram_func();
	if (vs_src) {
		vs = make_shader(GL_VERTEX_SHADER, vs_src);
		glAttachShader_func(id, vs);
		glDeleteShader_func(vs);
	}
	if (fs_src) {
		fs = make_shader(GL_FRAGMENT_SHADER, fs_src);
		glAttachShader_func(id, fs);
		glDeleteShader_func(fs);
	}
	glLinkProgram_func(id);
	check_link_status(id);
	glUseProgram_func(id);
	glDeleteProgram_func(id);
	assert_no_error();
	return id;
}


void
ShaderAPITest::test_uniform_size_type1(const char *glslType, GLenum glType, const char *el)
{
	char buffer[1024];
	GLuint program;
	GLint active, i;

	//printf("  Running subtest %s\n", glslType);
	//fflush(stdout);
	sprintf(buffer, "#version 120\nuniform %s m[60];\nvoid main() { gl_Position[0] = m[59]%s; }\n",
			glslType, el);
	
	program = make_program(buffer, NULL);
	glGetProgramiv_func(program, GL_ACTIVE_UNIFORMS, &active);
	assert_no_error();
	for (i = 0; i < active; i++) {
		GLint size = -1;
		GLenum type = 0;
		glGetActiveUniform_func(program, i, sizeof(buffer), NULL,
								&size, &type, buffer);
		assert_no_error();
		assert(type == glType);
		assert(size == 60);
		if (strncmp(buffer, "m", 1) == 0)
			break;
	}
}


void
ShaderAPITest::test_uniform_size_type(void)
{
	test_uniform_size_type1("float", GL_FLOAT, "");
	test_uniform_size_type1("vec2", GL_FLOAT_VEC2, "[0]");
	test_uniform_size_type1("vec3", GL_FLOAT_VEC3, "[0]");
	test_uniform_size_type1("vec4", GL_FLOAT_VEC4, "[0]");

	test_uniform_size_type1("bool", GL_BOOL, " ? 1.0 : 0.0");
	test_uniform_size_type1("bvec2", GL_BOOL_VEC2, "[0] ? 1.0 : 0.0");
	test_uniform_size_type1("bvec3", GL_BOOL_VEC3, "[0] ? 1.0 : 0.0");
	test_uniform_size_type1("bvec4", GL_BOOL_VEC4, "[0] ? 1.0 : 0.0");

	test_uniform_size_type1("int", GL_INT, "");
	test_uniform_size_type1("ivec2", GL_INT_VEC2, "[0]");
	test_uniform_size_type1("ivec3", GL_INT_VEC3, "[0]");
	test_uniform_size_type1("ivec4", GL_INT_VEC4, "[0]");

	test_uniform_size_type1("mat2", GL_FLOAT_MAT2, "[0][0]");
	test_uniform_size_type1("mat3", GL_FLOAT_MAT3, "[0][0]");
	test_uniform_size_type1("mat4", GL_FLOAT_MAT4, "[0][0]");
	test_uniform_size_type1("mat2x3", GL_FLOAT_MAT2x3, "[0][0]");
	test_uniform_size_type1("mat2x4", GL_FLOAT_MAT2x4, "[0][0]");
	test_uniform_size_type1("mat3x2", GL_FLOAT_MAT3x2, "[0][0]");
	test_uniform_size_type1("mat3x4", GL_FLOAT_MAT3x4, "[0][0]");
	test_uniform_size_type1("mat4x2", GL_FLOAT_MAT4x2, "[0][0]");
	test_uniform_size_type1("mat4x3", GL_FLOAT_MAT4x3, "[0][0]");
}


void
ShaderAPITest::test_attrib_size_type1(const char *glslType, GLenum glType, const char *el)
{
	char buffer[1024];
	GLuint program;
	GLint active, i;

	//printf("  Running subtest %s\n", glslType);
	//fflush(stdout);
	sprintf(buffer, "#version 120\nattribute %s m;\nvoid main() { gl_Position[0] = m%s; }\n",
			glslType, el);

	program = make_program(buffer, NULL);
	glGetProgramiv_func(program, GL_ACTIVE_ATTRIBUTES, &active);
	assert_no_error();
	for (i = 0; i < active; i++) {
		GLint size = -1;
		GLenum type = 0;
		glGetActiveAttrib_func(program, i, sizeof(buffer), NULL,
							   &size, &type, buffer);
		assert_no_error();
		assert(type == glType);
		assert(size == 1);
		if (strncmp(buffer, "m", 1) == 0)
			break;
	}
	assert(i < active); /* Otherwise the compiler optimised it out */
}


void
ShaderAPITest::test_attrib_size_type(void)
{
	test_attrib_size_type1("float", GL_FLOAT, "");
	test_attrib_size_type1("vec2", GL_FLOAT_VEC2, "[0]");
	test_attrib_size_type1("vec3", GL_FLOAT_VEC3, "[0]");
	test_attrib_size_type1("vec4", GL_FLOAT_VEC4, "[0]");
	test_attrib_size_type1("mat2", GL_FLOAT_MAT2, "[0][0]");
	test_attrib_size_type1("mat3", GL_FLOAT_MAT3, "[0][0]");
	test_attrib_size_type1("mat4", GL_FLOAT_MAT4, "[0][0]");
	test_attrib_size_type1("mat2x3", GL_FLOAT_MAT2x3, "[0][0]");
	test_attrib_size_type1("mat2x4", GL_FLOAT_MAT2x4, "[0][0]");
	test_attrib_size_type1("mat3x2", GL_FLOAT_MAT3x2, "[0][0]");
	test_attrib_size_type1("mat3x4", GL_FLOAT_MAT3x4, "[0][0]");
	test_attrib_size_type1("mat4x2", GL_FLOAT_MAT4x2, "[0][0]");
	test_attrib_size_type1("mat4x3", GL_FLOAT_MAT4x3, "[0][0]");
}


void
ShaderAPITest::test_uniform_array_overflow(void)
{
	GLuint program;
	GLint location;
	GLfloat data[128];

	program = make_program("#version 120\nuniform vec2 x[10];\nvoid main() { gl_Position.xy = x[9]; }\n", NULL);
	location = glGetUniformLocation_func(program, "x");
	assert_no_error();
	glUniform2fv_func(location, 64, data);
	assert_no_error();
}


void
ShaderAPITest::test_uniform_scalar_count(void)
{
	GLuint program;
	GLint location;
	GLfloat data[128];

	program = make_program("#version 110\nuniform vec2 x;\nvoid main() { gl_Position.xy = x; }\n", NULL);
	location = glGetUniformLocation_func(program, "x");
	assert_no_error();
	glUniform2fv_func(location, 64, data);
	assert_error(GL_INVALID_OPERATION);
}


void
ShaderAPITest::test_uniform_query_matrix(void)
{
	GLuint program;
	GLfloat data[18];
	GLint i, r, c;
	GLint location;

	program = make_program("#version 110\nuniform mat3 m[2];\nvoid main() { gl_Position.xyz = m[1][2]; }\n", NULL);
	location = glGetUniformLocation_func(program, "m");
	for (i = 0; i < 9; i++)
		data[i] = i;
	for (i = 9; i < 18; i++)
		data[i] = 321.0;
	glUniformMatrix3fv_func(location, 1, GL_TRUE, data);

	for (i = 0; i < 18; i++)
		data[i] = 123.0;
	glGetUniformfv_func(program, location, data);
	for (c = 0; c < 3; c++)
		for (r = 0; r < 3; r++)
			assert(data[c * 3 + r] == r * 3 + c);
	for (i = 9; i < 18; i++)
		assert(data[i] == 123.0);
}


void
ShaderAPITest::test_uniform_neg_location(void)
{
	GLuint program;
	GLfloat data[4];

	program = make_program("#version 110\nvoid main() { gl_Position = vec4(1.0, 1.0, 1.0, 1.0); }\n", NULL);
	(void) program;
	assert_no_error();
	glUniform1i_func(-1, 1);
	assert_no_error();
	glUniform1i_func(-200, 1);
	assert_error(GL_INVALID_OPERATION);
	glUniformMatrix2fv_func(-1, 1, GL_FALSE, data);
	assert_no_error();
	glUniformMatrix2fv_func(-200, 1, GL_FALSE, data);
	assert_error(GL_INVALID_OPERATION);
}


void
ShaderAPITest::test_uniform_bool_conversion(void)
{
	GLuint program;
	GLint location;
	GLint value[16];  /* in case glGetUniformiv goes nuts on the stack */

	assert_no_error();
	program = make_program("uniform bool b;\nvoid main() { gl_Position.x = b ? 1.5 : 0.5; }\n", NULL);
	location = glGetUniformLocation_func(program, "b");
	assert(location != -1);
	assert_no_error();
	glUniform1i_func(location, 5);
	assert_no_error();
	glGetUniformiv_func(program, location, &value[0]);
	assert_no_error();
	assert(value[0] == 1);
}


void
ShaderAPITest::test_uniform_multiple_samplers(void)
{
   GLuint program;
   GLint location;
   GLint values[2] = {0, 1};

   assert_no_error();
   program = make_program(NULL, "uniform sampler2D s[2];\nvoid main() { gl_FragColor = texture2D(s[1], vec2(0.0, 0.0)); }\n");
   location = glGetUniformLocation_func(program, "s[0]");
   assert(location != -1);
   assert_no_error();
   glUniform1iv_func(location, 2, values);
   assert_no_error();
}


void
ShaderAPITest::run_tests(void)
{
	test_uniform_size_type();
	test_attrib_size_type();
	test_uniform_array_overflow();
	test_uniform_scalar_count();
	test_uniform_query_matrix();
	test_uniform_neg_location();
	test_uniform_bool_conversion();
	test_uniform_multiple_samplers();
}


void
ShaderAPITest::runOne(ShaderAPIResult &r, Window &w)
{
	(void) w;  // silence warning

	// error will be set to true if any of the assert functions below fail.
	error = false;

	get_ext_procs();

	run_tests();

	r.pass = !error;
}


void
ShaderAPITest::logOne(ShaderAPIResult &r)
{
	if (r.pass) {
		logPassFail(r);
		logConcise(r);
	}
	else {
		env->log << name << "FAIL\n";
	}
}


void
ShaderAPIResult::putresults(ostream &s) const
{
	if (pass) {
		s << "PASS\n";
	}
	else {
		s << "FAIL\n";
	}
}


bool
ShaderAPIResult::getresults(istream &s)
{
	char result[1000];
	s >> result;

	if (strcmp(result, "FAIL") == 0) {
		pass = false;
	}
	else {
		pass = true;
	}
	return s.good();
}


// We need OpenGL 2.0, 2.1 or 3.0
bool
ShaderAPITest::isApplicable() const
{
	if (GLUtils::getVersion() >= 2.0) {
		return true;
	}
	else {
		env->log << name
				 << ":  skipped.  Requires GL >= 2.0.\n";
		return false;
	}
}


// The test object itself:
ShaderAPITest shaderAPITest("shaderAPI", "window, rgb",
                            "",  // no extensions, but see isApplicable()
                            "Test GLSL shader-related API features.\n");



} // namespace GLEAN


