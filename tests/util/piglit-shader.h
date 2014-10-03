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
GLuint piglit_compile_shader_text_nothrow(GLenum target, const char *text);
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

extern GLboolean piglit_program_pipeline_check_status(GLuint pipeline);
extern GLboolean piglit_program_pipeline_check_status_quiet(GLuint pipeline);

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
