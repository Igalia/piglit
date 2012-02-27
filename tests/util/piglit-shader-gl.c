/*
 * Copyright © 2010 Intel Corporation
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
 */

#ifndef USE_OPENGL
#	error USE_OPENGL is undefined
#endif

#if defined(_MSC_VER)
#include <windows.h>
#endif

#include "piglit-util.h"

PFNGLATTACHSHADERPROC piglit_AttachShader = NULL;
PFNGLBINDATTRIBLOCATIONPROC piglit_BindAttribLocation = NULL;
PFNGLCOMPILESHADERPROC piglit_CompileShader = NULL;
PFNGLCREATEPROGRAMPROC piglit_CreateProgram = NULL;
PFNGLCREATESHADERPROC piglit_CreateShader = NULL;
PFNGLDELETEPROGRAMPROC piglit_DeleteProgram = NULL;
PFNGLDELETESHADERPROC piglit_DeleteShader = NULL;
PFNGLGETACTIVEUNIFORMPROC piglit_GetActiveUniform = NULL;
PFNGLGETATTRIBLOCATIONPROC piglit_GetAttribLocation = NULL;
PFNGLGETPROGRAMINFOLOGPROC piglit_GetProgramInfoLog = NULL;
PFNGLGETPROGRAMIVPROC piglit_GetProgramiv = NULL;
PFNGLGETSHADERINFOLOGPROC piglit_GetShaderInfoLog = NULL;
PFNGLGETSHADERIVPROC piglit_GetShaderiv = NULL;
PFNGLGETUNIFORMLOCATIONPROC piglit_GetUniformLocation = NULL;
PFNGLLINKPROGRAMPROC piglit_LinkProgram = NULL;
PFNGLSHADERSOURCEPROC piglit_ShaderSource = NULL;
PFNGLUSEPROGRAMPROC piglit_UseProgram = NULL;
PFNGLUNIFORM1FPROC piglit_Uniform1f = NULL;
PFNGLUNIFORM2FPROC piglit_Uniform2f = NULL;
PFNGLUNIFORM3FPROC piglit_Uniform3f = NULL;
PFNGLUNIFORM4FPROC piglit_Uniform4f = NULL;
PFNGLUNIFORM1FVPROC piglit_Uniform1fv = NULL;
PFNGLUNIFORM2FVPROC piglit_Uniform2fv = NULL;
PFNGLUNIFORM3FVPROC piglit_Uniform3fv = NULL;
PFNGLUNIFORM4FVPROC piglit_Uniform4fv = NULL;
PFNGLUNIFORM1IPROC piglit_Uniform1i = NULL;
PFNGLUNIFORM2IPROC piglit_Uniform2i = NULL;
PFNGLUNIFORM3IPROC piglit_Uniform3i = NULL;
PFNGLUNIFORM4IPROC piglit_Uniform4i = NULL;
PFNGLUNIFORM1IVPROC piglit_Uniform1iv = NULL;
PFNGLUNIFORM2IVPROC piglit_Uniform2iv = NULL;
PFNGLUNIFORM3IVPROC piglit_Uniform3iv = NULL;
PFNGLUNIFORM4IVPROC piglit_Uniform4iv = NULL;
PFNGLUNIFORM1UIPROC piglit_Uniform1ui = NULL;
PFNGLUNIFORM2UIPROC piglit_Uniform2ui = NULL;
PFNGLUNIFORM3UIPROC piglit_Uniform3ui = NULL;
PFNGLUNIFORM4UIPROC piglit_Uniform4ui = NULL;
PFNGLUNIFORM1UIVPROC piglit_Uniform1uiv = NULL;
PFNGLUNIFORM2UIVPROC piglit_Uniform2uiv = NULL;
PFNGLUNIFORM3UIVPROC piglit_Uniform3uiv = NULL;
PFNGLUNIFORM4UIVPROC piglit_Uniform4uiv = NULL;
PFNGLUNIFORMMATRIX2FVPROC piglit_UniformMatrix2fv = NULL;
PFNGLUNIFORMMATRIX3FVPROC piglit_UniformMatrix3fv = NULL;
PFNGLUNIFORMMATRIX4FVPROC piglit_UniformMatrix4fv = NULL;
PFNGLGETUNIFORMFVPROC piglit_GetUniformfv = NULL;
PFNGLVERTEXATTRIBPOINTERPROC piglit_VertexAttribPointer = NULL;
PFNGLENABLEVERTEXATTRIBARRAYPROC piglit_EnableVertexAttribArray = NULL;
PFNGLDISABLEVERTEXATTRIBARRAYPROC piglit_DisableVertexAttribArray = NULL;

PFNGLUNIFORMMATRIX2X3FVPROC piglit_UniformMatrix2x3fv = NULL;
PFNGLUNIFORMMATRIX2X4FVPROC piglit_UniformMatrix2x4fv = NULL;
PFNGLUNIFORMMATRIX3X2FVPROC piglit_UniformMatrix3x2fv = NULL;
PFNGLUNIFORMMATRIX3X4FVPROC piglit_UniformMatrix3x4fv = NULL;
PFNGLUNIFORMMATRIX4X2FVPROC piglit_UniformMatrix4x2fv = NULL;
PFNGLUNIFORMMATRIX4X3FVPROC piglit_UniformMatrix4x3fv = NULL;

static void
init_functions_from_core(void)
{
	piglit_AttachShader = glAttachShader;
	piglit_BindAttribLocation = glBindAttribLocation;
	piglit_CompileShader = glCompileShader;
	piglit_CreateProgram = glCreateProgram;
	piglit_CreateShader = glCreateShader;
	piglit_DeleteProgram = glDeleteProgram;
	piglit_DeleteShader = glDeleteShader;
	piglit_GetActiveUniform = glGetActiveUniform;
	piglit_GetAttribLocation = glGetAttribLocation;
	piglit_GetProgramInfoLog = glGetProgramInfoLog;
	piglit_GetProgramiv = glGetProgramiv;
	piglit_GetShaderInfoLog = glGetShaderInfoLog;
	piglit_GetShaderiv = glGetShaderiv;
	piglit_GetUniformLocation = glGetUniformLocation;
	piglit_LinkProgram = glLinkProgram;
	piglit_ShaderSource = glShaderSource;
	piglit_UseProgram = glUseProgram;
	piglit_Uniform1f = glUniform1f;
	piglit_Uniform2f = glUniform2f;
	piglit_Uniform3f = glUniform3f;
	piglit_Uniform4f = glUniform4f;
	piglit_Uniform1fv = glUniform1fv;
	piglit_Uniform2fv = glUniform2fv;
	piglit_Uniform3fv = glUniform3fv;
	piglit_Uniform4fv = glUniform4fv;
	piglit_Uniform1i = glUniform1i;
	piglit_Uniform2i = glUniform2i;
	piglit_Uniform3i = glUniform3i;
	piglit_Uniform4i = glUniform4i;
	piglit_Uniform1iv = glUniform1iv;
	piglit_Uniform2iv = glUniform2iv;
	piglit_Uniform3iv = glUniform3iv;
	piglit_Uniform4iv = glUniform4iv;
	piglit_Uniform1ui = glUniform1ui;
	piglit_Uniform2ui = glUniform2ui;
	piglit_Uniform3ui = glUniform3ui;
	piglit_Uniform4ui = glUniform4ui;
	piglit_Uniform1uiv = glUniform1uiv;
	piglit_Uniform2uiv = glUniform2uiv;
	piglit_Uniform3uiv = glUniform3uiv;
	piglit_Uniform4uiv = glUniform4uiv;
	piglit_UniformMatrix2fv = glUniformMatrix2fv;
	piglit_UniformMatrix3fv = glUniformMatrix3fv;
	piglit_UniformMatrix4fv = glUniformMatrix4fv;
	piglit_GetUniformfv = glGetUniformfv;
	piglit_VertexAttribPointer = glVertexAttribPointer;
	piglit_EnableVertexAttribArray = glEnableVertexAttribArray;
	piglit_DisableVertexAttribArray = glDisableVertexAttribArray;

	/* These are part of OpenGL 2.1.
	 */
	piglit_UniformMatrix2x3fv = glUniformMatrix2x3fv;
	piglit_UniformMatrix2x4fv = glUniformMatrix2x4fv;
	piglit_UniformMatrix3x2fv = glUniformMatrix3x2fv;
	piglit_UniformMatrix3x4fv = glUniformMatrix3x4fv;
	piglit_UniformMatrix4x2fv = glUniformMatrix4x2fv;
	piglit_UniformMatrix4x3fv = glUniformMatrix4x3fv;
}

static void
init_functions_from_extension(void)
{
	piglit_AttachShader = glAttachObjectARB;
	piglit_BindAttribLocation = glBindAttribLocationARB;
	piglit_CompileShader = glCompileShaderARB;
	piglit_CreateProgram = glCreateProgramObjectARB;
	piglit_CreateShader = glCreateShaderObjectARB;
	piglit_DeleteProgram = glDeleteObjectARB;
	piglit_DeleteShader = glDeleteObjectARB;
	piglit_GetActiveUniform = glGetActiveUniformARB;
	piglit_GetAttribLocation = glGetAttribLocationARB;
	piglit_GetProgramInfoLog = glGetInfoLogARB;
	piglit_GetProgramiv = glGetObjectParameterivARB;
	piglit_GetShaderInfoLog = glGetInfoLogARB;
	piglit_GetShaderiv = glGetObjectParameterivARB;
	piglit_GetUniformLocation = glGetUniformLocationARB;
	piglit_LinkProgram = glLinkProgramARB;
	piglit_ShaderSource = glShaderSourceARB;
	piglit_UseProgram = glUseProgramObjectARB;
	piglit_Uniform1f = glUniform1fARB;
	piglit_Uniform2f = glUniform2fARB;
	piglit_Uniform3f = glUniform3fARB;
	piglit_Uniform4f = glUniform4fARB;
	piglit_Uniform1fv = glUniform1fvARB;
	piglit_Uniform2fv = glUniform2fvARB;
	piglit_Uniform3fv = glUniform3fvARB;
	piglit_Uniform4fv = glUniform4fvARB;
	piglit_Uniform1i = glUniform1iARB;
	piglit_Uniform2iv = glUniform2ivARB;
	piglit_Uniform3iv = glUniform3ivARB;
	piglit_Uniform4iv = glUniform4ivARB;
	piglit_UniformMatrix2fv = glUniformMatrix2fvARB;
	piglit_UniformMatrix3fv = glUniformMatrix3fvARB;
	piglit_UniformMatrix4fv = glUniformMatrix4fvARB;
	piglit_GetUniformfv = glGetUniformfvARB;
	piglit_VertexAttribPointer = glVertexAttribPointerARB;
	piglit_EnableVertexAttribArray = glEnableVertexAttribArrayARB;
	piglit_DisableVertexAttribArray = glDisableVertexAttribArrayARB;
}

void
piglit_require_GLSL(void)
{
	if (piglit_get_gl_version() >= 20) {
		init_functions_from_core();
	} else if (GLEW_ARB_shader_objects && GLEW_ARB_shading_language_100) {
		init_functions_from_extension();
	} else {
		printf("GLSL not supported.\n");
		piglit_report_result(PIGLIT_SKIP);
		exit(1);
	}
}

void
piglit_require_GLSL_version(int version)
{
	bool es;
	int major, minor;

	piglit_require_GLSL();

	piglit_get_glsl_version(&es, &major, &minor);

	if (es || 100 * major + minor < version) {
		printf("GLSL %d.%d not supported.\n",
		       version / 100, version % 100);
		piglit_report_result(PIGLIT_SKIP);
		exit(1);
	}
}

void
piglit_require_vertex_shader(void)
{
	if (piglit_get_gl_version() >= 20) {
		init_functions_from_core();
	} else if (GLEW_ARB_shader_objects && GLEW_ARB_vertex_shader) {
		init_functions_from_extension();
	} else {
		printf("GLSL vertex shaders are not supported.\n");
		piglit_report_result(PIGLIT_SKIP);
		exit(1);
	}
}

void
piglit_require_fragment_shader(void)
{
	if (piglit_get_gl_version() >= 20) {
		init_functions_from_core();
	} else if (GLEW_ARB_shader_objects && GLEW_ARB_fragment_shader) {
		init_functions_from_extension();
	} else {
		printf("GLSL vertex shaders are not supported.\n");
		piglit_report_result(PIGLIT_SKIP);
		exit(1);
	}
}
