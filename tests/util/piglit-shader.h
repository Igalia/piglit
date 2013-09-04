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

GLuint piglit_compile_shader(GLenum target, const char *filename);
GLuint piglit_compile_shader_text(GLenum target, const char *text);
GLboolean piglit_link_check_status(GLint prog);
GLboolean piglit_link_check_status_quiet(GLint prog);
GLint piglit_link_simple_program(GLint vs, GLint fs);
GLint piglit_build_simple_program(const char *vs_source, const char *fs_source);
GLuint piglit_build_simple_program_unlinked(const char *vs_source,
					    const char *fs_source);
GLint piglit_link_simple_program_multiple_shaders(GLint shader1, ...);
GLint piglit_build_simple_program_unlinked_multiple_shaders_v(GLenum target1,
							     const char*source1,
							     va_list ap);
GLint piglit_build_simple_program_unlinked_multiple_shaders(GLenum target1,
							   const char *source1,
							   ...);
GLint piglit_build_simple_program_multiple_shaders(GLenum target1,
						  const char *source1,
						  ...);

#if defined(PIGLIT_USE_OPENGL_ES1)
#define glAttachShader assert(!"glAttachShader does not exist in ES1")
#define glBindAttribLocation assert(!"glBindAttribLocation does not exist in ES1")
#define glCompileShader assert(!"glCompileShader does not exist in ES1")
#define glCreateProgram assert(!"glCreateProgram does not exist in ES1")
#define glCreateShader assert(!"glCreateShader does not exist in ES1")
#define glDeleteProgram assert(!"glDeleteProgram does not exist in ES1")
#define glDeleteShader assert(!"glDeleteShader does not exist in ES1")
#define glGetActiveUniform assert(!"glGetActiveUniform does not exist in ES1")
#define glGetAttribLocation assert(!"glGetAttribLocation does not exist in ES1")
#define glGetProgramInfoLog assert(!"glGetProgramInfoLog does not exist in ES1")
#define glGetProgramiv assert(!"glGetProgramiv does not exist in ES1")
#define glGetShaderInfoLog assert(!"glGetShaderInfoLog does not exist in ES1")
#define glGetShaderiv assert(!"glGetShaderiv does not exist in ES1")
#define glGetUniformLocation assert(!"glGetUniformLocation does not exist in ES1")
#define glLinkProgram assert(!"glLinkProgram does not exist in ES1")
#define glShaderSource assert(!"glShaderSource does not exist in ES1")
#define glUseProgram assert(!"glUseProgram does not exist in ES1")
#define glUniform1f assert(!"glUniform1f does not exist in ES1")
#define glUniform2f assert(!"glUniform2f does not exist in ES1")
#define glUniform3f assert(!"glUniform3f does not exist in ES1")
#define glUniform4f assert(!"glUniform4f does not exist in ES1")
#define glUniform1fv assert(!"glUniform1fv does not exist in ES1")
#define glUniform2fv assert(!"glUniform2fv does not exist in ES1")
#define glUniform3fv assert(!"glUniform3fv does not exist in ES1")
#define glUniform4fv assert(!"glUniform4fv does not exist in ES1")
#define glUniform1i assert(!"glUniform1i does not exist in ES1")
#define glUniform2iv assert(!"glUniform2iv does not exist in ES1")
#define glUniform3iv assert(!"glUniform3iv does not exist in ES1")
#define glUniform4iv assert(!"glUniform4iv does not exist in ES1")
#define glUniformMatrix2fv assert(!"glUniformMatrix2fv does not exist in ES1")
#define glUniformMatrix3fv assert(!"glUniformMatrix3fv does not exist in ES1")
#define glUniformMatrix4fv assert(!"glUniformMatrix4fv does not exist in ES1")
#define glGetUniformfv assert(!"glGetUniformfv does not exist in ES1")
#define glVertexAttribPointer assert(!"glVertexAttribPointer does not exist in ES1")
#define glEnableVertexAttribArray assert(!"glEnableVertexAttribArray does not exist in ES1")
#define glDisableVertexAttribArray assert(!"glDisableVertexAttribArray does not exist in ES1")
#define glUniformMatrix2x3fv assert(!"glUniformMatrix2x3fv does not exist in ES1")
#define glUniformMatrix2x4fv assert(!"glUniformMatrix2x4fv does not exist in ES1")
#define glUniformMatrix3x2fv assert(!"glUniformMatrix3x2fv does not exist in ES1")
#define glUniformMatrix3x4fv assert(!"glUniformMatrix3x4fv does not exist in ES1")
#define glUniformMatrix4x2fv assert(!"glUniformMatrix4x2fv does not exist in ES1")
#define glUniformMatrix4x3fv assert(!"glUniformMatrix4x3fv does not exist in ES1")
#endif

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
