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

// tapi2.h:  Test OpenGL 2.x API functions/features
// Brian Paul  9 March 2007

#define GL_GLEXT_PROTOTYPES

#include <stdlib.h>
#include <cstring>
#include <cassert>
#include <math.h>
#include "tapi2.h"


namespace GLEAN {


static PFNGLATTACHSHADERPROC glAttachShader_func = NULL;
static PFNGLBINDATTRIBLOCATIONPROC glBindAttribLocation_func = NULL;
static PFNGLCOMPILESHADERPROC glCompileShader_func = NULL;
static PFNGLCREATEPROGRAMPROC glCreateProgram_func = NULL;
static PFNGLCREATESHADERPROC glCreateShader_func = NULL;
static PFNGLDELETEPROGRAMPROC glDeleteProgram_func = NULL;
static PFNGLDELETESHADERPROC glDeleteShader_func = NULL;
static PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray_func = NULL;
static PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray_func = NULL;
static PFNGLGETATTACHEDSHADERSPROC glGetAttachedShaders_func = NULL;
static PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation_func = NULL;
static PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog_func = NULL;
static PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog_func = NULL;
static PFNGLGETSHADERIVPROC glGetShaderiv_func = NULL;
static PFNGLGETPROGRAMIVPROC glGetProgramiv_func = NULL;
static PFNGLGETSHADERSOURCEPROC glGetShaderSource_func = NULL;
static PFNGLGETUNIFORMFVPROC glGetUniformfv_func = NULL;
static PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation_func = NULL;
static PFNGLISPROGRAMPROC glIsProgram_func = NULL;
static PFNGLISSHADERPROC glIsShader_func = NULL;
static PFNGLLINKPROGRAMPROC glLinkProgram_func = NULL;
static PFNGLSHADERSOURCEPROC glShaderSource_func = NULL;
static PFNGLUNIFORM1FVPROC glUniform1fv_func = NULL;
static PFNGLUNIFORM2FVPROC glUniform2fv_func = NULL;
static PFNGLUNIFORM3FVPROC glUniform3fv_func = NULL;
static PFNGLUNIFORM4FVPROC glUniform4fv_func = NULL;
static PFNGLUNIFORM1FPROC glUniform1f_func = NULL;
static PFNGLUNIFORM2FPROC glUniform2f_func = NULL;
static PFNGLUNIFORM3FPROC glUniform3f_func = NULL;
static PFNGLUNIFORM4FPROC glUniform4f_func = NULL;
static PFNGLUNIFORM1IPROC glUniform1i_func = NULL;
static PFNGLUNIFORM2IPROC glUniform2i_func = NULL;
static PFNGLUNIFORM3IPROC glUniform3i_func = NULL;
static PFNGLUNIFORM4IPROC glUniform4i_func = NULL;
static PFNGLUNIFORM1IVPROC glUniform1iv_func = NULL;
static PFNGLUNIFORM2IVPROC glUniform2iv_func = NULL;
static PFNGLUNIFORM3IVPROC glUniform3iv_func = NULL;
static PFNGLUNIFORM4IVPROC glUniform4iv_func = NULL;
static PFNGLUNIFORMMATRIX2FVPROC glUniformMatrix2fv_func = NULL;
static PFNGLUNIFORMMATRIX3FVPROC glUniformMatrix3fv_func = NULL;
static PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv_func = NULL;
static PFNGLUSEPROGRAMPROC glUseProgram_func = NULL;
static PFNGLVALIDATEPROGRAMPROC glValidateProgram_func = NULL;

static PFNGLVERTEXATTRIB1FPROC glVertexAttrib1f_func = NULL;
static PFNGLVERTEXATTRIB2FPROC glVertexAttrib2f_func = NULL;
static PFNGLVERTEXATTRIB3FPROC glVertexAttrib3f_func = NULL;
static PFNGLVERTEXATTRIB4FPROC glVertexAttrib4f_func = NULL;
static PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer_func = NULL;

static PFNGLSTENCILOPSEPARATEPROC glStencilOpSeparate_func = NULL;
static PFNGLSTENCILFUNCSEPARATEPROC glStencilFuncSeparate_func = NULL;
static PFNGLSTENCILMASKSEPARATEPROC glStencilMaskSeparate_func = NULL;

static PFNGLBLENDEQUATIONSEPARATEPROC glBlendEquationSeparate_func = NULL;
static PFNGLDRAWBUFFERSPROC glDrawBuffers_func = NULL;



// Get ptrs to 2.0 API functions.
// \param errorFunc  returns name of API function in case of error
// \return true for success, false for error
bool
API2Test::getFunctions_2_0(const char **errorFunc)
{
#define GET(PTR, TYPE, STR)                          \
	PTR  = (TYPE) GLUtils::getProcAddress(STR);  \
	if (!PTR) {				     \
		*errorFunc = STR;                    \
		return false;                        \
        }

	// shading language
	GET(glAttachShader_func, PFNGLATTACHSHADERPROC, "glAttachShader");
	GET(glBindAttribLocation_func, PFNGLBINDATTRIBLOCATIONPROC, "glBindAttribLocation");
	GET(glCompileShader_func, PFNGLCOMPILESHADERPROC, "glCompileShader");
	GET(glCreateProgram_func, PFNGLCREATEPROGRAMPROC, "glCreateProgram");
	GET(glCreateShader_func, PFNGLCREATESHADERPROC, "glCreateShader");
	GET(glDeleteProgram_func, PFNGLDELETEPROGRAMPROC, "glDeleteProgram");
	GET(glDeleteShader_func, PFNGLDELETESHADERPROC, "glDeleteShader");
	GET(glDisableVertexAttribArray_func, PFNGLDISABLEVERTEXATTRIBARRAYPROC, "glDisableVertexAttribArray");
	GET(glEnableVertexAttribArray_func, PFNGLENABLEVERTEXATTRIBARRAYPROC, "glEnableVertexAttribArray");
	GET(glGetAttachedShaders_func, PFNGLGETATTACHEDSHADERSPROC, "glGetAttachedShaders");
	GET(glGetAttribLocation_func, PFNGLGETATTRIBLOCATIONPROC, "glGetAttribLocation");
	GET(glGetProgramInfoLog_func, PFNGLGETPROGRAMINFOLOGPROC, "glGetProgramInfoLog");
	GET(glGetShaderInfoLog_func, PFNGLGETSHADERINFOLOGPROC, "glGetShaderInfoLog");
	GET(glGetProgramiv_func, PFNGLGETPROGRAMIVPROC, "glGetProgramiv");
	GET(glGetShaderiv_func, PFNGLGETSHADERIVPROC, "glGetShaderiv");
	GET(glGetShaderSource_func, PFNGLGETSHADERSOURCEPROC, "glGetShaderSource");
	GET(glGetUniformLocation_func, PFNGLGETUNIFORMLOCATIONPROC, "glGetUniformLocation");
	GET(glGetUniformfv_func, PFNGLGETUNIFORMFVPROC, "glGetUniformfv");
	GET(glIsProgram_func, PFNGLISPROGRAMPROC, "glIsProgram");
	GET(glIsShader_func, PFNGLISSHADERPROC, "glIsShader");
	GET(glLinkProgram_func, PFNGLLINKPROGRAMPROC, "glLinkProgram");
	GET(glShaderSource_func, PFNGLSHADERSOURCEPROC, "glShaderSource");
	GET(glUniform1f_func, PFNGLUNIFORM1FPROC, "glUniform1f");
	GET(glUniform2f_func, PFNGLUNIFORM2FPROC, "glUniform2f");
	GET(glUniform3f_func, PFNGLUNIFORM3FPROC, "glUniform3f");
	GET(glUniform4f_func, PFNGLUNIFORM4FPROC, "glUniform4f");
	GET(glUniform1fv_func, PFNGLUNIFORM1FVPROC, "glUniform1fv");
	GET(glUniform2fv_func, PFNGLUNIFORM2FVPROC, "glUniform2fv");
	GET(glUniform3fv_func, PFNGLUNIFORM3FVPROC, "glUniform3fv");
	GET(glUniform4fv_func, PFNGLUNIFORM3FVPROC, "glUniform4fv");
	GET(glUniform1i_func, PFNGLUNIFORM1IPROC, "glUniform1i");
	GET(glUniform2i_func, PFNGLUNIFORM2IPROC, "glUniform2i");
	GET(glUniform3i_func, PFNGLUNIFORM3IPROC, "glUniform3i");
	GET(glUniform4i_func, PFNGLUNIFORM4IPROC, "glUniform4i");
	GET(glUniform1iv_func, PFNGLUNIFORM1IVPROC, "glUniform1iv");
	GET(glUniform2iv_func, PFNGLUNIFORM2IVPROC, "glUniform2iv");
	GET(glUniform3iv_func, PFNGLUNIFORM3IVPROC, "glUniform3iv");
	GET(glUniform4iv_func, PFNGLUNIFORM4IVPROC, "glUniform4iv");
	GET(glUniformMatrix2fv_func, PFNGLUNIFORMMATRIX2FVPROC, "glUniformMatrix2fv");
	GET(glUniformMatrix3fv_func, PFNGLUNIFORMMATRIX3FVPROC, "glUniformMatrix3fv");
	GET(glUniformMatrix4fv_func, PFNGLUNIFORMMATRIX4FVPROC, "glUniformMatrix4fv");
	GET(glUseProgram_func, PFNGLUSEPROGRAMPROC, "glUseProgram");
	GET(glValidateProgram_func, PFNGLVALIDATEPROGRAMPROC, "glValidateProgram");
	GET(glVertexAttrib1f_func, PFNGLVERTEXATTRIB1FPROC, "glVertexAttrib1f");
	GET(glVertexAttrib2f_func, PFNGLVERTEXATTRIB2FPROC, "glVertexAttrib2f");
	GET(glVertexAttrib3f_func, PFNGLVERTEXATTRIB3FPROC, "glVertexAttrib3f");
	GET(glVertexAttrib4f_func, PFNGLVERTEXATTRIB4FPROC, "glVertexAttrib4f");
	GET(glVertexAttribPointer_func, PFNGLVERTEXATTRIBPOINTERPROC, "glVertexAttribPointer");

	// stencil
	GET(glStencilOpSeparate_func, PFNGLSTENCILOPSEPARATEPROC, "glStencilOpSeparate");
	GET(glStencilFuncSeparate_func, PFNGLSTENCILFUNCSEPARATEPROC, "glStencilFuncSeparate");
	GET(glStencilMaskSeparate_func, PFNGLSTENCILMASKSEPARATEPROC, "glStencilMaskSeparate");

	// misc
	GET(glBlendEquationSeparate_func, PFNGLBLENDEQUATIONSEPARATEPROC, "glBlendEquationSeparate");
	GET(glDrawBuffers_func, PFNGLDRAWBUFFERSPROC, "glDrawBuffers");

	return true;
#undef GET
}


bool
API2Test::setup(void)
{
	// check that we have OpenGL 2.0
	if (GLUtils::getVersion() < 2.0) {
		//env->log << "OpenGL 2.0 not supported\n";
		return false;
	}

	const char *errorFunc;
	if (!getFunctions_2_0(&errorFunc)) {
		env->log << "Unable to get pointer to OpenGL 2.0 function '"
			 << errorFunc
			 << "'\n";
		return false;
	}

	GLenum err = glGetError();
	assert(!err);  // should be OK

	// setup vertex transform (we'll draw a quad in middle of window)
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-4.0, 4.0, -4.0, 4.0, 0.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glDrawBuffer(GL_FRONT);
	glReadBuffer(GL_FRONT); 

	// compute error tolerances (may need fine-tuning)
	int bufferBits[5];
	glGetIntegerv(GL_RED_BITS, &bufferBits[0]);
	glGetIntegerv(GL_GREEN_BITS, &bufferBits[1]);
	glGetIntegerv(GL_BLUE_BITS, &bufferBits[2]);
	glGetIntegerv(GL_ALPHA_BITS, &bufferBits[3]);
	glGetIntegerv(GL_DEPTH_BITS, &bufferBits[4]);

	tolerance[0] = 2.0 / (1 << bufferBits[0]);
	tolerance[1] = 2.0 / (1 << bufferBits[1]);
	tolerance[2] = 2.0 / (1 << bufferBits[2]);
	if (bufferBits[3])
		tolerance[3] = 2.0 / (1 << bufferBits[3]);
	else
		tolerance[3] = 1.0;
	if (bufferBits[4])
		tolerance[4] = 16.0 / (1 << bufferBits[4]);
	else
		tolerance[4] = 1.0;

	return true;
}


void
API2Test::reportFailure(const char *msg, int line) const
{
	env->log << "FAILURE: " << msg << " (at tapi2.cpp:" << line << ")\n";
}


void
API2Test::reportFailure(const char *msg, GLenum target, int line) const
{
	env->log << "FAILURE: " << msg;
	if (target == GL_FRAGMENT_SHADER)
		env->log << " (fragment)";
	else
		env->log << " (vertex)";
	env->log << " (at tapi2.cpp:" << line << ")\n";
}



#define REPORT_FAILURE(MSG) reportFailure(MSG, __LINE__)
#define REPORT_FAILURE_T(MSG, TARGET) reportFailure(MSG, TARGET, __LINE__)


// Compare actual and expected colors
bool
API2Test::equalColors(const GLfloat act[4], const GLfloat exp[4]) const
{
	if ((fabsf(act[0] - exp[0]) > tolerance[0]) ||
            (fabsf(act[1] - exp[1]) > tolerance[1]) ||
            (fabsf(act[2] - exp[2]) > tolerance[2]) ||
            (fabsf(act[3] - exp[3]) > tolerance[3]))
		return false;
	else
		return true;
}


// Render test quad w/ current shader program, return RGBA color of quad
void
API2Test::renderQuad(GLfloat *pixel) const
{
	const GLfloat r = 0.62; // XXX draw 16x16 pixel quad

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBegin(GL_POLYGON);
	glTexCoord2f(0, 0);  glVertex2f(-r, -r);
	glTexCoord2f(1, 0);  glVertex2f( r, -r);
	glTexCoord2f(1, 1);  glVertex2f( r,  r);
	glTexCoord2f(0, 1);  glVertex2f(-r,  r);
	glEnd();

	// read a pixel from lower-left corner of rendered quad
	glReadPixels(windowSize / 2 - 2, windowSize / 2 - 2, 1, 1,
		     GL_RGBA, GL_FLOAT, pixel);
}


// As above, but use vertex arrays
// \param attr  which vertex attribute array to put colors into
// \param value  4-component valut to put into the attribute array
// \param pixel  returns the rendered color obtained with glReadPixels
void
API2Test::renderQuadWithArrays(GLint attr, const GLfloat value[4],
			       GLfloat *pixel) const
{
	const GLfloat r = 0.62; // XXX draw 16x16 pixel quad
	static const GLfloat vertcoords[4][3] = {
		{ -r, -r, 0 }, {  r, -r, 0 }, {  r,  r, 0 }, { -r,  r, 0 }
	};
	GLfloat values[4][4];
	GLint i;
	for (i = 0; i < 4; i++) {
		values[i][0] = value[0];
		values[i][1] = value[1];
		values[i][2] = value[2];
		values[i][3] = value[3];
	};

	glVertexPointer(3, GL_FLOAT, 0, vertcoords);
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexAttribPointer_func(attr, 4, GL_FLOAT, GL_FALSE, 0, values);
	glEnableVertexAttribArray_func(attr);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDrawArrays(GL_POLYGON, 0, 4);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableVertexAttribArray_func(attr);

	// read a pixel from lower-left corner of rendered quad
	glReadPixels(windowSize / 2 - 2, windowSize / 2 - 2, 1, 1,
		     GL_RGBA, GL_FLOAT, pixel);
}


GLuint
API2Test::loadAndCompileShader(GLenum target, const char *text)
{
	GLint stat, val;
	GLuint shader = glCreateShader_func(target);
	if (!shader) {
		REPORT_FAILURE("glCreateShader failed (fragment)");
		return 0;
	}
	glShaderSource_func(shader, 1,
			    (const GLchar **) &text, NULL);
	glCompileShader_func(shader);
	glGetShaderiv_func(shader, GL_COMPILE_STATUS, &stat);
	if (!stat) {
		REPORT_FAILURE_T("glShaderSource or glCompileShader failed", target);
		return 0;
	}
        if (!glIsShader_func(shader)) {
		REPORT_FAILURE("glIsShader failed (fragment)");
		return false;
	}
	glGetShaderiv_func(shader, GL_SHADER_TYPE, &val);
	if (val != (GLint) target) {
		REPORT_FAILURE_T("glGetShaderiv(GL_SHADER_TYPE) failed", target);
		return 0;
	}
	glGetShaderiv_func(shader, GL_COMPILE_STATUS, &val);
	if (val != GL_TRUE) {
		REPORT_FAILURE_T("glGetShaderiv(GL_COMPILE_STATUS) failed", target);
		return 0;
	}
	glGetShaderiv_func(shader, GL_SHADER_SOURCE_LENGTH, &val);
        // Note: some OpenGLs return a 1-char shorter length than strlen(text)
	if (abs(val - (int) strlen(text)) > 1) {
		REPORT_FAILURE_T("glGetShaderiv(GL_SHADER_SOURCE_LENGTH) failed", target);
		return 0;
	}
	return shader;
}


GLuint
API2Test::createProgram(GLuint vertShader, GLuint fragShader)
{
	GLuint program = glCreateProgram_func();
	if (vertShader)
		glAttachShader_func(program, vertShader);
	if (fragShader)
		glAttachShader_func(program, fragShader);
	glLinkProgram_func(program);
	return program;
}


bool
API2Test::testShaderObjectFuncs(void)
{
	static const char *vertShaderText =
		"void main() { \n"
		"   gl_Position = ftransform(); \n"
		"} \n";
	static const char *fragShaderText =
		"void main() { \n"
		"   gl_FragColor = vec4(1.0, 0.5, 0.25, 0.0); \n"
		"} \n";
	GLuint vertShader, fragShader, program;
	GLint stat, val, err;

	vertShader = loadAndCompileShader(GL_VERTEX_SHADER, vertShaderText);
	if (!vertShader)
		return false;
	fragShader = loadAndCompileShader(GL_FRAGMENT_SHADER, fragShaderText);
	if (!fragShader)
		return false;


	program = createProgram(vertShader, fragShader);
	if (!program) {
		REPORT_FAILURE("glCreateProgram failed");
		return false;
	}
	glGetProgramiv_func(program, GL_LINK_STATUS, &stat);
	if (!stat) {
		REPORT_FAILURE("glLinkProgram failed");
		return false;
	}
	glUseProgram_func(program);

	glGetIntegerv(GL_CURRENT_PROGRAM, &val);
	if (val != (GLint) program) {
		REPORT_FAILURE("glGetInteger(GL_CURRENT_PROGRAM) failed");
		return false;
	}

        err = glGetError();
        if (err) {
		REPORT_FAILURE("OpenGL error detected in testShaderFuncs");
		return false;
	}

        if (!glIsProgram_func(program)) {
		REPORT_FAILURE("glIsProgram failed");
		return false;
	}

	GLuint objects[2];
	GLsizei count;
	glGetProgramiv_func(program, GL_ATTACHED_SHADERS, &val);
	if (val != 2) {
		REPORT_FAILURE("glGetProgramiv(GL_ATTACHED_SHADERS) failed");
		return false;
	}
	glGetAttachedShaders_func(program, 2, &count, objects);
	if (count != 2) {
		REPORT_FAILURE("glGetAttachedShaders failed (wrong count)");
		return false;
	}
	if (objects[0] != vertShader && objects[1] != vertShader) {
		REPORT_FAILURE("glGetAttachedShaders failed (vertex shader missing)");
		return false;
	}
	if (objects[0] != fragShader && objects[1] != fragShader) {
		REPORT_FAILURE("glGetAttachedShaders failed (fragment shader missing)");
		return false;
	}

	glValidateProgram_func(program);
	glGetProgramiv_func(program, GL_VALIDATE_STATUS, &stat);
	if (!stat) {
		REPORT_FAILURE("glValidateProgram failed");
		return false;
	}

	// Delete vertex shader
	glDeleteShader_func(vertShader);
	if (!glIsShader_func(vertShader)) {
		// the shader is still attached so the handle should be valid
		REPORT_FAILURE("glIsShader(deleted shader) failed");
		return false;
	}
	glGetShaderiv_func(vertShader, GL_DELETE_STATUS, &stat);
	if (stat != GL_TRUE) {
		REPORT_FAILURE("Incorrect shader delete status");
		return false;
	}

	// Delete fragment shader
	glDeleteShader_func(fragShader);

	// Delete program object
	glDeleteProgram_func(program);
	if (!glIsProgram_func(program)) {
		// the program is still in use so the handle should be valid
		REPORT_FAILURE("glIsProgram(deleted program) failed");
		return false;
	}
	glGetProgramiv_func(program, GL_DELETE_STATUS, &stat);
	if (stat != GL_TRUE) {
		REPORT_FAILURE("Incorrect program delete status");
		return false;
	}

	// now unbind the program
	glUseProgram_func(0);
	stat = glIsProgram_func(program);
	if (stat) {
		// the program and handle should have really been deleted now
		REPORT_FAILURE("glIsProgram(deleted program) failed");
		return false;
	}

	glGetProgramiv_func(program, GL_DELETE_STATUS, &stat);
	err = glGetError();
	if (!err) {
		// the program and handle should have been deleted now
		// so glGetProgramiv() should have generated an error
		REPORT_FAILURE("glGetProgramiv(deleted program) failed");
		return false;
	}

	return true;
}


bool
API2Test::testUniformfFuncs(void)
{
	static const char *fragShaderText =
		"uniform float uf1; \n"
		"uniform vec2 uf2; \n"
		"uniform vec3 uf3; \n"
		"uniform vec4 uf4; \n"
		"void main() { \n"
		"   gl_FragColor = vec4(uf1, uf2.y, uf3.z, uf4.w); \n"
		"} \n";
	GLuint fragShader, program;
	GLint uf1, uf2, uf3, uf4;
	GLfloat value[4];

	fragShader = loadAndCompileShader(GL_FRAGMENT_SHADER, fragShaderText);
	if (!fragShader) {
		return false;
	}
	program = createProgram(0, fragShader);
	if (!program) {
		REPORT_FAILURE("glCreateProgram (uniform test) failed");
		return false;
	}
	glUseProgram_func(program);

	uf1 = glGetUniformLocation_func(program, "uf1");
	if (uf1 < 0) {
		REPORT_FAILURE("glGetUniform \"uf1\" failed");
		return false;
	}
	uf2 = glGetUniformLocation_func(program, "uf2");
	if (uf2 < 0) {
		REPORT_FAILURE("glGetUniform \"uf2\" failed");
		return false;
	}
	uf3 = glGetUniformLocation_func(program, "uf3");
	if (uf3 < 0) {
		REPORT_FAILURE("glGetUniform \"uf3\" failed");
		return false;
	}
	uf4 = glGetUniformLocation_func(program, "uf4");
	if (uf4 < 0) {
		REPORT_FAILURE("glGetUniform \"uf4\" failed");
		return false;
	}


	GLfloat pixel[4], expected[4];

	// Test glUniform[1234]f()
	expected[0] = 0.1;
	expected[1] = 0.2;
	expected[2] = 0.3;
	expected[3] = 0.4;
	glUniform1f_func(uf1, expected[0]);
	glUniform2f_func(uf2, 0.0, expected[1]);
	glUniform3f_func(uf3, 0.0, 0.0, expected[2]);
	glUniform4f_func(uf4, 0.0, 0.0, 0.0, expected[3]);
	renderQuad(pixel);
	if (!equalColors(pixel, expected)) {
		REPORT_FAILURE("glUniform[1234]f failed");
		//printf("found:    %f %f %f %f\n", pixel[0], pixel[1], pixel[2], pixel[3]);
		//printf("expected: %f %f %f %f\n", expected[0], expected[1], expected[2], expected[3]);

		return false;
	}

	// Test glUniform[1234]fv()
	GLfloat u[4];
	expected[0] = 0.9;
	expected[1] = 0.8;
	expected[2] = 0.7;
	expected[3] = 0.6;
	u[0] = expected[0];
	glUniform1fv_func(uf1, 1, u);
	u[0] = 0.0;  u[1] = expected[1];
	glUniform2fv_func(uf2, 1, u);
	u[0] = 0.0;  u[1] = 0.0;  u[2] = expected[2];
	glUniform3fv_func(uf3, 1, u);
	u[0] = 0.0;  u[1] = 0.0;  u[2] = 0.0;  u[3] = expected[3];
	glUniform4fv_func(uf4, 1, u);
	renderQuad(pixel);
	if (!equalColors(pixel, expected)) {
		REPORT_FAILURE("glUniform[1234]f failed");
		return false;
	}

	// Test glGetUniformfv
	glUniform4fv_func(uf4, 1, expected);
	glGetUniformfv_func(program, uf4, value);
	if (value[0] != expected[0] ||
	    value[1] != expected[1] ||
	    value[2] != expected[2] ||
	    value[3] != expected[3]) {
		REPORT_FAILURE("glGetUniformfv failed");
		return false;
	}

	return true;
}


bool
API2Test::testUniformiFuncs(void)
{
	static const char *fragShaderText =
		"uniform int ui1; \n"
		"uniform ivec2 ui2; \n"
		"uniform ivec3 ui3; \n"
		"uniform ivec4 ui4; \n"
		"void main() { \n"
		"   gl_FragColor = vec4(ui1, ui2.y, ui3.z, ui4.w) * 0.1; \n"
		"} \n";
	GLuint fragShader, program;
	GLint ui1, ui2, ui3, ui4;

	fragShader = loadAndCompileShader(GL_FRAGMENT_SHADER, fragShaderText);
	if (!fragShader) {
		return false;
	}
	program = createProgram(0, fragShader);
	if (!program) {
		REPORT_FAILURE("glCreateProgram (uniform test) failed");
		return false;
	}
	glUseProgram_func(program);

	ui1 = glGetUniformLocation_func(program, "ui1");
	if (ui1 < 0) {
		REPORT_FAILURE("glGetUniform \"ui1\" failed");
		return false;
	}
	ui2 = glGetUniformLocation_func(program, "ui2");
	if (ui2 < 0) {
		REPORT_FAILURE("glGetUniform \"ui2\" failed");
		return false;
	}
	ui3 = glGetUniformLocation_func(program, "ui3");
	if (ui3 < 0) {
		REPORT_FAILURE("glGetUniform \"ui3\" failed");
		return false;
	}
	ui4 = glGetUniformLocation_func(program, "ui4");
	if (ui4 < 0) {
		REPORT_FAILURE("glGetUniform \"ui4\" failed");
		return false;
	}

	GLfloat pixel[4], expected[4];
	GLint expectedInt[4];

	// Test glUniform[1234]i()
	expectedInt[0] = 1;
	expectedInt[1] = 2;
	expectedInt[2] = 3;
	expectedInt[3] = 4;
	expected[0] = 0.1;
	expected[1] = 0.2;
	expected[2] = 0.3;
	expected[3] = 0.4;
	glUniform1i_func(ui1, expectedInt[0]);
	glUniform2i_func(ui2, 0, expectedInt[1]);
	glUniform3i_func(ui3, 0, 0, expectedInt[2]);
	glUniform4i_func(ui4, 0, 0, 0, expectedInt[3]);
	renderQuad(pixel);
	if (!equalColors(pixel, expected)) {
		REPORT_FAILURE("glUniform[1234]i failed");
		//printf("%f %f %f %f\n", pixel[0], pixel[1], pixel[2], pixel[3]);
		return false;
	}

	// Test glUniform[1234]iv()
	GLint u[4];
	expectedInt[0] = 9;
	expectedInt[1] = 8;
	expectedInt[2] = 7;
	expectedInt[3] = 6;
	expected[0] = 0.9;
	expected[1] = 0.8;
	expected[2] = 0.7;
	expected[3] = 0.6;
	u[0] = expectedInt[0];
	glUniform1iv_func(ui1, 1, u);
	u[0] = 0;  u[1] = expectedInt[1];
	glUniform2iv_func(ui2, 1, u);
	u[0] = 0;  u[1] = 0;  u[2] = expectedInt[2];
	glUniform3iv_func(ui3, 1, u);
	u[0] = 0;  u[1] = 0;  u[2] = 0;  u[3] = expectedInt[3];
	glUniform4iv_func(ui4, 1, u);
	renderQuad(pixel);
	if (!equalColors(pixel, expected)) {
		REPORT_FAILURE("glUniform[1234]i failed");
#if 0
		printf("Expected color %f %f %f %f\n",
                       expected[0], expected[1], expected[2], expected[3]);
		printf("Found color %f %f %f %f\n",
                       pixel[0], pixel[1], pixel[2], pixel[3]);
#endif
		return false;
	}

	return true;
}


bool
API2Test::testShaderAttribs(void)
{
	static const char *vertShaderText =
		"attribute vec4 generic; \n"
		"void main() { \n"
		"   gl_Position = ftransform(); \n"
		"   gl_FrontColor = generic; \n"
		"} \n";
	GLuint vertShader, program;

	vertShader = loadAndCompileShader(GL_VERTEX_SHADER, vertShaderText);
	if (!vertShader) {
		return false;
	}
	program = createProgram(vertShader, 0);
	if (!program) {
		REPORT_FAILURE("glCreateProgram (uniform test) failed");
		return false;
	}
	glUseProgram_func(program);

	static const GLfloat testColors[3][4] = {
		{ 1.0, 0.5, 0.25, 0.0 },
		{ 0.0, 0.1, 0.2,  0.3 },
		{ 0.5, 0.6, 0.7,  0.8 },
	};

	// let compiler allocate the attribute location
	const GLint attr = glGetAttribLocation_func(program, "generic");
	if (attr < 0) {
		REPORT_FAILURE("glGetAttribLocation failed");
		return false;
	}
	for (int i = 0; i < 3; i++) {
		GLfloat pixel[4];
		renderQuadWithArrays(attr, testColors[i], pixel);
		if (!equalColors(pixel, testColors[i])) {
#if 0
                   printf("Expected color %f %f %f\n",
                          testColors[i][0],
                          testColors[i][1],
                          testColors[i][2]);
                   printf("Found color %f %f %f\n",
                          pixel[0], pixel[1], pixel[2]);
#endif
			REPORT_FAILURE("Vertex array test failed");
			return false;
		}
	}

	// Test explicit attribute binding.
	const GLint bindAttr = 6;  // XXX a non-colliding alias
	glBindAttribLocation_func(program, bindAttr, "generic");
	glLinkProgram_func(program);
	GLint loc = glGetAttribLocation_func(program, "generic");
	if (loc != bindAttr) {
		REPORT_FAILURE("glBindAttribLocation failed");
		return false;
	}
	for (int i = 0; i < 3; i++) {
		GLfloat pixel[4];
		renderQuadWithArrays(bindAttr, testColors[i], pixel);
		if (!equalColors(pixel, testColors[i])) {
			REPORT_FAILURE("Vertex array test failed (2)");
			return false;
		}
	}

	return true;
}

#define CLAMP( X, MIN, MAX )  ( (X)<(MIN) ? (MIN) : ((X)>(MAX) ? (MAX) : (X)) )

bool
API2Test::testStencilFuncSeparate(void)
{
	GLint val;
	GLint stencilBits, stencilMax;

	glGetIntegerv(GL_STENCIL_BITS, &stencilBits);
	stencilMax = (1 << stencilBits) - 1;

	glStencilFuncSeparate_func(GL_FRONT, GL_LEQUAL, 12, 0xf);
	glStencilFuncSeparate_func(GL_BACK, GL_GEQUAL, 13, 0xe);

	glGetIntegerv(GL_STENCIL_BACK_FUNC, &val);
	if (val != GL_GEQUAL) {
		REPORT_FAILURE("GL_STENCIL_BACK_FUNC query returned wrong value");
		return false;
	}

	glGetIntegerv(GL_STENCIL_FUNC, &val);
	if (val != GL_LEQUAL) {
		REPORT_FAILURE("GL_STENCIL_FUNC (front) query returned wrong value");
		return false;
	}

	glGetIntegerv(GL_STENCIL_BACK_REF, &val);
	if (val != CLAMP(13, 0, stencilMax)) {
		REPORT_FAILURE("GL_STENCIL_BACK_REF query returned wrong value");
		return false;
	}

	glGetIntegerv(GL_STENCIL_REF, &val);
	if (val != CLAMP(12, 0, stencilMax)) {
		REPORT_FAILURE("GL_STENCIL_REF (front) query returned wrong value");
		return false;
	}

	glGetIntegerv(GL_STENCIL_BACK_VALUE_MASK, &val);
	if (val != 0xe) {
		REPORT_FAILURE("GL_STENCIL_BACK_VALUE_MASK query returned wrong value");
		return false;
	}

	glGetIntegerv(GL_STENCIL_VALUE_MASK, &val);
	if (val != 0xf) {
		REPORT_FAILURE("GL_STENCIL_VALUE_MASK (front) query returned wrong value");
		return false;
	}

	return true;
}


bool
API2Test::testStencilOpSeparate(void)
{
	GLint val;

	// face, fail, zfail, zpass
	glStencilOpSeparate_func(GL_FRONT, GL_INVERT, GL_ZERO, GL_INCR);
	glStencilOpSeparate_func(GL_BACK, GL_INCR, GL_KEEP, GL_REPLACE);

	glGetIntegerv(GL_STENCIL_BACK_FAIL, &val);
	if (val != GL_INCR) {
		REPORT_FAILURE("GL_STENCIL_BACK_FAIL query returned wrong value");
		return false;
	}

	glGetIntegerv(GL_STENCIL_FAIL, &val);
	if (val != GL_INVERT) {
		REPORT_FAILURE("GL_STENCIL_FAIL (front) query returned wrong value");
		return false;
	}

	glGetIntegerv(GL_STENCIL_BACK_PASS_DEPTH_FAIL, &val);
	if (val != GL_KEEP) {
		REPORT_FAILURE("GL_STENCIL_BACK_PASS_DEPTH_FAIL query returned wrong value");
		return false;
	}

	glGetIntegerv(GL_STENCIL_PASS_DEPTH_FAIL, &val);
	if (val != GL_ZERO) {
		REPORT_FAILURE("GL_STENCIL_PASS_DEPTH_FAIL (front) query returned wrong value");
		return false;
	}

	glGetIntegerv(GL_STENCIL_BACK_PASS_DEPTH_PASS, &val);
	if (val != GL_REPLACE) {
		REPORT_FAILURE("GL_STENCIL_BACK_PASS_DEPTH_PASS query returned wrong value");
		return false;
	}

	glGetIntegerv(GL_STENCIL_PASS_DEPTH_PASS, &val);
	if (val != GL_INCR) {
		REPORT_FAILURE("GL_STENCIL_PASS_DEPTH_PASS (front) query returned wrong value");
		return false;
	}

	return true;
}


bool
API2Test::testStencilMaskSeparate(void)
{
	GLint val;

	// face, fail, zfail, zpass
	glStencilMaskSeparate_func(GL_BACK, 0xa);
	glStencilMaskSeparate_func(GL_FRONT, 0xb);

	glGetIntegerv(GL_STENCIL_BACK_WRITEMASK, &val);
	if (val != 0xa) {
		REPORT_FAILURE("GL_STENCIL_BACK_WRITEMASK query returned wrong value");
		return false;
	}

	glGetIntegerv(GL_STENCIL_WRITEMASK, &val);
	if (val != 0xb) {
		REPORT_FAILURE("GL_STENCIL_WRITEMASK (front) query returned wrong value");
		return false;
	}

	return true;
}


bool
API2Test::testBlendEquationSeparate(void)
{
	GLint val;

	glBlendEquationSeparate_func(GL_MAX, GL_FUNC_SUBTRACT);

	glGetIntegerv(GL_BLEND_EQUATION, &val);
	if (val != GL_MAX) {
		REPORT_FAILURE("GL_BLEND_EQUATION (rgb) query returned wrong value");
		return false;
	}

	glGetIntegerv(GL_BLEND_EQUATION_ALPHA, &val);
	if (val != GL_FUNC_SUBTRACT) {
		REPORT_FAILURE("GL_BLEND_EQUATION (rgb) query returned wrong value");
		return false;
	}

	return true;
}


bool
API2Test::testDrawBuffers(void)
{
	const int MAX = 2;
	GLint maxBuf = -1, i, n, val;
	GLenum buffers[MAX], err;
	GLint initDrawBuffer;

	glGetIntegerv(GL_DRAW_BUFFER, &initDrawBuffer);

	glGetIntegerv(GL_MAX_DRAW_BUFFERS, &maxBuf);
	if (maxBuf < 1) {
		REPORT_FAILURE("GL_MAX_DRAW_BUFFERS query failed");
		return false;
	}

	n = maxBuf < MAX ? maxBuf : MAX;
	assert(n > 0);
	for (i = 0; i < n; i++) {
		buffers[i] = (i & 1) ? GL_FRONT_LEFT : GL_BACK_LEFT;
	}
	glDrawBuffers_func(n, buffers);

	for (i = 0; i < n; i++) {
		glGetIntegerv(GL_DRAW_BUFFER0 + i, &val);
		if (val != (GLint) buffers[i]) {
			REPORT_FAILURE("glDrawBuffers failed");
			return false;
		}
	}

	// restore
	glDrawBuffer(initDrawBuffer);

	err = glGetError();
	if (err) {
		REPORT_FAILURE("glDrawBuffers generrated an OpenGL error");
		return false;
	}

	return true;
}


// Run all the subtests, incrementing numPassed, numFailed
void
API2Test::runSubTests(MultiTestResult &r)
{
	static TestFunc funcs[] = {
		&GLEAN::API2Test::testStencilFuncSeparate,
		&GLEAN::API2Test::testStencilOpSeparate,
		&GLEAN::API2Test::testStencilMaskSeparate,
		&GLEAN::API2Test::testBlendEquationSeparate,
		&GLEAN::API2Test::testDrawBuffers,
		&GLEAN::API2Test::testShaderObjectFuncs,
		&GLEAN::API2Test::testUniformfFuncs,
		&GLEAN::API2Test::testUniformiFuncs,
		&GLEAN::API2Test::testShaderAttribs,
		NULL
	};

	for (int i = 0; funcs[i]; i++)
		if ((this->*funcs[i])())
			r.numPassed++;
		else
			r.numFailed++;
}


void
API2Test::runOne(MultiTestResult &r, Window &w)
{
	(void) w;

	if (!setup()) {
		r.pass = false;
		return;
	}

	runSubTests(r);

	r.pass = (r.numFailed == 0);
}


// The test object itself:
API2Test api2Test("api2", "window, rgb, z, db",
		  "",  // no extension filter (we'll test for version 2.x during setup)
		  "API2 test: check that OpenGL 2.x API functions work.\n"
		  );



} // namespace GLEAN
