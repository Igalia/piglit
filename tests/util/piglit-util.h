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

#include "config.h"

#if defined(_WIN32)
#include <windows.h>
#endif

#if defined(_MSC_VER)
typedef __int32 int32_t;
typedef __int64 int64_t;
typedef unsigned __int8 uint8_t;
typedef unsigned __int32 uint32_t;
typedef unsigned __int64 uint64_t;
#define bool BOOL
#define true 1
#define false 0
#else
#include <stdint.h>
#include <stdbool.h>
#endif

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <piglit/gl_wrap.h>
#include <piglit/glut_wrap.h>

#if defined(_MSC_VER)

#define snprintf sprintf_s

static __inline double
round(double x) {
	return x >= 0.0 ? floor(x + 0.5) : ceil(x - 0.5);
}

static __inline float
roundf(float x) {
	return x >= 0.0f ? floorf(x + 0.5f) : ceilf(x - 0.5f);
}

#define piglit_get_proc_address(x) wglGetProcAddress(x)

#else /* !defined(_MSC_VER) */

#define piglit_get_proc_address(x) glutGetProcAddress(x)

#endif /* !defined(_MSC_VER) */

enum piglit_result {
	PIGLIT_PASS,
	PIGLIT_FAIL,
	PIGLIT_SKIP,
	PIGLIT_WARN
};

#include "piglit-framework.h"

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

extern const uint8_t fdo_bitmap[];
extern const unsigned int fdo_bitmap_width;
extern const unsigned int fdo_bitmap_height;

/**
 * Call glutInit() and, if EGLUT is used, also call glutInitAPIMask().
 */
void piglit_glutInit(int argc, char **argv);

/**
 * \brief Get version of OpenGL API.
 *
 * Null parameters are ignored.
 *
 * \param es Is the API OpenGL or OpenGL ES?
 */
void piglit_get_gl_version(bool *es, int* major, int* minor);

/**
 * Null parameters are ignored.
 *
 * \param es Is it GLSL ES?
 */
void piglit_get_glsl_version(bool *es, int* major, int* minor);

/**
 * \precondition name is not null
 */
bool piglit_is_extension_supported(const char *name);

int FindLine(const char *program, int position);
void piglit_report_result(enum piglit_result result);
void piglit_require_extension(const char *name);
void piglit_require_not_extension(const char *name);
int piglit_probe_pixel_rgb_silent(int x, int y, const float* expected, float *out_probe);
int piglit_probe_pixel_rgba_silent(int x, int y, const float* expected, float *out_probe);
int piglit_probe_pixel_rgb(int x, int y, const float* expected);
int piglit_probe_pixel_rgba(int x, int y, const float* expected);
int piglit_probe_rect_rgb(int x, int y, int w, int h, const float* expected);
int piglit_probe_rect_rgba(int x, int y, int w, int h, const float* expected);
int piglit_probe_image_rgb(int x, int y, int w, int h, const float *image);
int piglit_probe_image_rgba(int x, int y, int w, int h, const float *image);
int piglit_probe_texel_rect_rgb(int target, int level, int x, int y,
				int w, int h, const float *expected);
int piglit_probe_texel_rgb(int target, int level, int x, int y,
			   const float* expected);
int piglit_probe_texel_rect_rgba(int target, int level, int x, int y,
				 int w, int h, const float *expected);
int piglit_probe_texel_rgba(int target, int level, int x, int y,
			    const float* expected);
int piglit_probe_pixel_depth(int x, int y, float expected);
int piglit_probe_rect_depth(int x, int y, int w, int h, float expected);
int piglit_probe_pixel_stencil(int x, int y, unsigned expected);
int piglit_probe_rect_stencil(int x, int y, int w, int h, unsigned expected);
int piglit_probe_rect_halves_equal_rgba(int x, int y, int w, int h);

int piglit_use_fragment_program(void);
int piglit_use_vertex_program(void);
void piglit_require_fragment_program(void);
void piglit_require_vertex_program(void);
GLuint piglit_compile_program(GLenum target, const char* text);
GLuint piglit_compile_shader(GLenum target, char *filename);
GLuint piglit_compile_shader_text(GLenum target, const char *text);
GLboolean piglit_link_check_status(GLint prog);
GLboolean piglit_link_check_status_quiet(GLint prog);
GLint piglit_link_simple_program(GLint vs, GLint fs);
GLvoid piglit_draw_rect(float x, float y, float w, float h);
GLvoid piglit_draw_rect_z(float z, float x, float y, float w, float h);
GLvoid piglit_draw_rect_tex(float x, float y, float w, float h,
                            float tx, float ty, float tw, float th);
GLvoid piglit_draw_rect_back(float x, float y, float w, float h);

void piglit_escape_exit_key(unsigned char key, int x, int y);

char *piglit_load_text_file(const char *file_name, unsigned *size);

void piglit_gen_ortho_projection(double left, double right, double bottom,
				 double top, double near_val, double far_val,
				 GLboolean push);
void piglit_ortho_projection(int w, int h, GLboolean push);

GLuint piglit_checkerboard_texture(GLuint tex, unsigned level,
    unsigned width, unsigned height,
    unsigned horiz_square_size, unsigned vert_square_size,
    const float *black, const float *white);
GLuint piglit_rgbw_texture(GLenum format, int w, int h, GLboolean mip,
		    GLboolean alpha, GLenum basetype);
GLuint piglit_depth_texture(GLenum format, int w, int h, GLboolean mip);
extern float piglit_tolerance[4];
void piglit_set_tolerance_for_bits(int rbits, int gbits, int bbits, int abits);

extern GLfloat cube_face_texcoords[6][4][3];
extern const char *cube_face_names[6];
extern const GLenum cube_face_targets[6];

/**
 * Common vertex program code to perform a model-view-project matrix transform
 */
#define PIGLIT_VERTEX_PROGRAM_MVP_TRANSFORM		\
	"ATTRIB	iPos = vertex.position;\n"		\
	"OUTPUT	oPos = result.position;\n"		\
	"PARAM	mvp[4] = { state.matrix.mvp };\n"	\
	"DP4	oPos.x, mvp[0], iPos;\n"		\
	"DP4	oPos.y, mvp[1], iPos;\n"		\
	"DP4	oPos.z, mvp[2], iPos;\n"		\
	"DP4	oPos.w, mvp[3], iPos;\n"

/**
 * Handle to a generic fragment program that passes the input color to output
 *
 * \note
 * Either \c piglit_use_fragment_program or \c piglit_require_fragment_program
 * must be called before using this program handle.
 */
extern GLint piglit_ARBfp_pass_through;

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
extern PFNGLUNIFORM2IVPROC piglit_Uniform2iv;
extern PFNGLUNIFORM3IVPROC piglit_Uniform3iv;
extern PFNGLUNIFORM4IVPROC piglit_Uniform4iv;
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

extern void piglit_require_GLSL(void);
extern void piglit_require_fragment_shader(void);
extern void piglit_require_vertex_shader(void);

#ifndef HAVE_STRCHRNUL
char *strchrnul(const char *s, int c);
#endif

static const GLint PIGLIT_ATTRIB_POS = 0;
static const GLint PIGLIT_ATTRIB_TEX = 1;
