/*
 * Copyright (c) The Piglit project 2007
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.  IN NO EVENT SHALL
 * VA LINUX SYSTEM, IBM AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once

/**
 * Null parameters are ignored.
 *
 * \param es Is it GLSL ES?
 */
void piglit_get_glsl_version(bool *es, int* major, int* minor);

GLuint piglit_compile_shader(GLenum target, char *filename);
GLuint piglit_compile_shader_text(GLenum target, const char *text);
GLboolean piglit_link_check_status(GLint prog);
GLboolean piglit_link_check_status_quiet(GLint prog);
GLint piglit_link_simple_program(GLint vs, GLint fs);

/**
 * \name Portable wrappers for GLSL functions\
 *
 * \note
 * One of \c piglit_require_GLSL, \c piglit_require_vertex_shader, or
 * \c piglit_require_fragment_shader must be called before using these
 * wrappers.
 */
/*@{*/
#if defined(USE_OPENGL_ES2)
#define piglit_AttachShader glAttachShader
#define piglit_BindAttribLocation glBindAttribLocation
#define piglit_CompileShader glCompileShader
#define piglit_CreateProgram glCreateProgram
#define piglit_CreateShader glCreateShader
#define piglit_DeleteProgram glDeleteProgram
#define piglit_DeleteShader glDeleteShader
#define piglit_GetActiveUniform glGetActiveUniform
#define piglit_GetAttribLocation glGetAttribLocation
#define piglit_GetProgramInfoLog glGetProgramInfoLog
#define piglit_GetProgramiv glGetProgramiv
#define piglit_GetShaderInfoLog glGetShaderInfoLog
#define piglit_GetShaderiv glGetShaderiv
#define piglit_GetUniformLocation glGetUniformLocation
#define piglit_LinkProgram glLinkProgram
#define piglit_ShaderSource glShaderSource
#define piglit_UseProgram glUseProgram
#define piglit_Uniform1f glUniform1f
#define piglit_Uniform2f glUniform2f
#define piglit_Uniform3f glUniform3f
#define piglit_Uniform4f glUniform4f
#define piglit_Uniform1fv glUniform1fv
#define piglit_Uniform2fv glUniform2fv
#define piglit_Uniform3fv glUniform3fv
#define piglit_Uniform4fv glUniform4fv
#define piglit_Uniform1i glUniform1i
#define piglit_Uniform2iv glUniform2iv
#define piglit_Uniform3iv glUniform3iv
#define piglit_Uniform4iv glUniform4iv
#define piglit_UniformMatrix2fv glUniformMatrix2fv
#define piglit_UniformMatrix3fv glUniformMatrix3fv
#define piglit_UniformMatrix4fv glUniformMatrix4fv
#define piglit_GetUniformfv glGetUniformfv
#define piglit_VertexAttribPointer glVertexAttribPointer
#define piglit_EnableVertexAttribArray glEnableVertexAttribArray
#define piglit_DisableVertexAttribArray glDisableVertexAttribArray
#define piglit_UniformMatrix2x3fv assert(!"glUniformMatrix2x3fv does not exist in ES")
#define piglit_UniformMatrix2x4fv assert(!"glUniformMatrix2x4fv does not exist in ES")
#define piglit_UniformMatrix3x2fv assert(!"glUniformMatrix3x2fv does not exist in ES")
#define piglit_UniformMatrix3x4fv assert(!"glUniformMatrix3x4fv does not exist in ES")
#define piglit_UniformMatrix4x2fv assert(!"glUniformMatrix4x2fv does not exist in ES")
#define piglit_UniformMatrix4x3fv assert(!"glUniformMatrix4x3fv does not exist in ES")
#else
extern PFNGLATTACHSHADERPROC piglit_AttachShader;
extern PFNGLBINDATTRIBLOCATIONPROC piglit_BindAttribLocation;
extern PFNGLCOMPILESHADERPROC piglit_CompileShader;
extern PFNGLCREATEPROGRAMPROC piglit_CreateProgram;
extern PFNGLCREATESHADERPROC piglit_CreateShader;
extern PFNGLDELETEPROGRAMPROC piglit_DeleteProgram;
extern PFNGLDELETESHADERPROC piglit_DeleteShader;
extern PFNGLGETACTIVEUNIFORMPROC piglit_GetActiveUniform;
extern PFNGLGETATTRIBLOCATIONPROC piglit_GetAttribLocation;
extern PFNGLGETPROGRAMINFOLOGPROC piglit_GetProgramInfoLog;
extern PFNGLGETPROGRAMIVPROC piglit_GetProgramiv;
extern PFNGLGETSHADERINFOLOGPROC piglit_GetShaderInfoLog;
extern PFNGLGETSHADERIVPROC piglit_GetShaderiv;
extern PFNGLGETUNIFORMLOCATIONPROC piglit_GetUniformLocation;
extern PFNGLLINKPROGRAMPROC piglit_LinkProgram;
extern PFNGLSHADERSOURCEPROC piglit_ShaderSource;
extern PFNGLUSEPROGRAMPROC piglit_UseProgram;
extern PFNGLUNIFORM1FPROC piglit_Uniform1f;
extern PFNGLUNIFORM2FPROC piglit_Uniform2f;
extern PFNGLUNIFORM3FPROC piglit_Uniform3f;
extern PFNGLUNIFORM4FPROC piglit_Uniform4f;
extern PFNGLUNIFORM1FVPROC piglit_Uniform1fv;
extern PFNGLUNIFORM2FVPROC piglit_Uniform2fv;
extern PFNGLUNIFORM3FVPROC piglit_Uniform3fv;
extern PFNGLUNIFORM4FVPROC piglit_Uniform4fv;
extern PFNGLUNIFORM1IPROC piglit_Uniform1i;
extern PFNGLUNIFORM2IPROC piglit_Uniform2i;
extern PFNGLUNIFORM3IPROC piglit_Uniform3i;
extern PFNGLUNIFORM4IPROC piglit_Uniform4i;
extern PFNGLUNIFORM1IVPROC piglit_Uniform1iv;
extern PFNGLUNIFORM2IVPROC piglit_Uniform2iv;
extern PFNGLUNIFORM3IVPROC piglit_Uniform3iv;
extern PFNGLUNIFORM4IVPROC piglit_Uniform4iv;
extern PFNGLUNIFORM1UIPROC piglit_Uniform1ui;
extern PFNGLUNIFORM2UIPROC piglit_Uniform2ui;
extern PFNGLUNIFORM3UIPROC piglit_Uniform3ui;
extern PFNGLUNIFORM4UIPROC piglit_Uniform4ui;
extern PFNGLUNIFORM1UIVPROC piglit_Uniform1uiv;
extern PFNGLUNIFORM2UIVPROC piglit_Uniform2uiv;
extern PFNGLUNIFORM3UIVPROC piglit_Uniform3uiv;
extern PFNGLUNIFORM4UIVPROC piglit_Uniform4uiv;
extern PFNGLUNIFORMMATRIX2FVPROC piglit_UniformMatrix2fv;
extern PFNGLUNIFORMMATRIX3FVPROC piglit_UniformMatrix3fv;
extern PFNGLUNIFORMMATRIX4FVPROC piglit_UniformMatrix4fv;
extern PFNGLGETUNIFORMFVPROC piglit_GetUniformfv;
extern PFNGLVERTEXATTRIBPOINTERPROC piglit_VertexAttribPointer;
extern PFNGLENABLEVERTEXATTRIBARRAYPROC piglit_EnableVertexAttribArray;
extern PFNGLDISABLEVERTEXATTRIBARRAYPROC piglit_DisableVertexAttribArray;
extern PFNGLUNIFORMMATRIX2X3FVPROC piglit_UniformMatrix2x3fv;
extern PFNGLUNIFORMMATRIX2X4FVPROC piglit_UniformMatrix2x4fv;
extern PFNGLUNIFORMMATRIX3X2FVPROC piglit_UniformMatrix3x2fv;
extern PFNGLUNIFORMMATRIX3X4FVPROC piglit_UniformMatrix3x4fv;
extern PFNGLUNIFORMMATRIX4X2FVPROC piglit_UniformMatrix4x2fv;
extern PFNGLUNIFORMMATRIX4X3FVPROC piglit_UniformMatrix4x3fv;
#endif
/*@}*/

/**
 * Require a specific version of GLSL.
 *
 * \param version Integer version, for example 130
 */
extern void piglit_require_GLSL_version(int version);
/** Require any version of GLSL */
extern void piglit_require_GLSL(void);
extern void piglit_require_fragment_shader(void);
extern void piglit_require_vertex_shader(void);
