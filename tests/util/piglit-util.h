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

#ifdef __cplusplus
extern "C" {
#endif

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
#include <stdarg.h>
#include <math.h>
#include <float.h>

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

#ifndef va_copy
#ifdef __va_copy
#define va_copy(dest, src) __va_copy((dest), (src))
#else
#define va_copy(dest, src) (dest) = (src)
#endif
#endif

#endif /* defined(_MSC_VER) */

#ifdef _WIN32
int asprintf(char **strp, const char *fmt, ...)
#ifdef __GNUC__
	__attribute__ ((format (printf, 2, 3)))
#endif
;
#endif /* _WIN32 */

// Trick from http://tdistler.com/2011/03/24/how-to-define-nan-not-a-number-on-windows
#ifndef INFINITY
#  define INFINITY (FLT_MAX + FLT_MAX)
#endif
#ifndef NAN
#  define NAN (INFINITY - INFINITY)
#endif

#ifdef _WIN32
#  define piglit_get_proc_address(x) wglGetProcAddress(x)
#else
#  define piglit_get_proc_address(x) glutGetProcAddress(x)
#endif

enum piglit_result {
	PIGLIT_PASS,
	PIGLIT_FAIL,
	PIGLIT_SKIP,
	PIGLIT_WARN
};

#include "piglit-framework.h"
#include "piglit-shader.h"
#include "piglit-transform-feedback.h"

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

#define CLAMP( X, MIN, MAX )  ( (X)<(MIN) ? (MIN) : ((X)>(MAX) ? (MAX) : (X)) )
#define MIN2(a, b) ((a) > (b) ? (b) : (a))
#define MAX2(a, b) ((a) > (b) ? (a) : (b))

extern const uint8_t fdo_bitmap[];
extern const unsigned int fdo_bitmap_width;
extern const unsigned int fdo_bitmap_height;

/**
 * Call glutInit() and, if EGLUT is used, also call glutInitAPIMask().
 */
void piglit_glutInit(int argc, char **argv);

/**
 * Determine if the API is OpenGL ES.
 */
bool piglit_is_gles();

/**
 * \brief Get version of OpenGL or OpenGL ES API.
 *
 * Returned version is multiplied by 10 to make it an integer.  So for
 * example, if the GL version is 2.1, the return value is 21.
 */
int piglit_get_gl_version();

/**
 * Determine if an extension is listed in an extension string
 *
 * \param haystack   List of all extensions to be searched
 * \param needle     Extension whose presens is to be detected
 *
 * \precondition \c haystack is not null
 *
 * \sa piglit_is_extension_supported, piglit_is_glx_extension_supported
 */
bool piglit_is_extension_in_string(const char *haystack, const char *needle);

/**
 * \precondition name is not null
 */
bool piglit_is_extension_supported(const char *name);

/**
 * \brief Convert a GL error to a string.
 *
 * For example, given GL_INVALID_ENUM, return "GL_INVALID_ENUM".
 *
 * Return "(unrecognized error)" if the enum is not recognized.
 */
const char* piglit_get_gl_error_name(GLenum error);

/**
 * \brief Convert a GL enum to a string.
 *
 * For example, given GL_INVALID_ENUM, return "GL_INVALID_ENUM".
 *
 * Return "(unrecognized enum)" if the enum is not recognized.
 */
const char *piglit_get_gl_enum_name(GLenum param);

/**
 * \brief Check for unexpected GL errors.
 *
 * If glGetError() returns an error other than \c expected_error, then
 * print a diagnostic and return GL_FALSE.  Otherwise return GL_TRUE.
 */
GLboolean
piglit_check_gl_error(GLenum expected_error);

/**
 * \brief Drain all GL errors.
 *
 * Repeatly call glGetError and discard errors until it returns GL_NO_ERROR.
 */
void piglit_reset_gl_error(void);

int FindLine(const char *program, int position);
void piglit_merge_result(enum piglit_result *all, enum piglit_result subtest);
void piglit_report_result(enum piglit_result result);
void piglit_require_gl_version(int required_version_times_10);
void piglit_require_extension(const char *name);
void piglit_require_not_extension(const char *name);
int piglit_probe_pixel_rgb_silent(int x, int y, const float* expected, float *out_probe);
int piglit_probe_pixel_rgba_silent(int x, int y, const float* expected, float *out_probe);
int piglit_probe_pixel_rgb(int x, int y, const float* expected);
int piglit_probe_pixel_rgba(int x, int y, const float* expected);
int piglit_probe_rect_rgb(int x, int y, int w, int h, const float* expected);
int piglit_probe_rect_rgb_silent(int x, int y, int w, int h, const float *expected);
int piglit_probe_rect_rgba(int x, int y, int w, int h, const float* expected);
int piglit_probe_rect_rgba_int(int x, int y, int w, int h, const int* expected);
int piglit_probe_rect_rgba_uint(int x, int y, int w, int h, const unsigned int* expected);
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
void piglit_frustum_projection(GLboolean push, double l, double r, double b,
			       double t, double n, double f);

GLuint piglit_checkerboard_texture(GLuint tex, unsigned level,
    unsigned width, unsigned height,
    unsigned horiz_square_size, unsigned vert_square_size,
    const float *black, const float *white);
GLuint piglit_rgbw_texture(GLenum format, int w, int h, GLboolean mip,
		    GLboolean alpha, GLenum basetype);
GLuint piglit_depth_texture(GLenum target, GLenum format, int w, int h, int d, GLboolean mip);
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

#ifndef HAVE_STRCHRNUL
char *strchrnul(const char *s, int c);
#endif

extern void piglit_set_rlimit(unsigned long lim);

static const GLint PIGLIT_ATTRIB_POS = 0;
static const GLint PIGLIT_ATTRIB_TEX = 1;

#ifdef __cplusplus
} /* end extern "C" */
#endif
