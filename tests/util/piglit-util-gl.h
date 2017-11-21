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
#ifndef __PIGLIT_UTIL_GL_H__
#define __PIGLIT_UTIL_GL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <errno.h>
#include <limits.h>

#include "piglit-util.h"

#include <piglit/gl_wrap.h>
#include <piglit/glut_wrap.h>

#define piglit_get_proc_address(x) piglit_dispatch_resolve_function(x)

#include "piglit-framework-gl.h"
#include "piglit-shader.h"

extern const uint8_t fdo_bitmap[];
extern const unsigned int fdo_bitmap_width;
extern const unsigned int fdo_bitmap_height;

extern bool piglit_is_core_profile;

/**
 * Determine if the API is OpenGL ES.
 */
bool piglit_is_gles(void);

/**
 * Determine if the API is OpenGL ES 3.x.
 */
bool piglit_is_gles3(void);

/**
 * \brief Get version of OpenGL or OpenGL ES API.
 *
 * Returned version is multiplied by 10 to make it an integer.  So for
 * example, if the GL version is 2.1, the return value is 21.
 */
int piglit_get_gl_version(void);

/**
 * \precondition name is not null
 */
bool piglit_is_extension_supported(const char *name);

/**
 * reinitialize the supported extension List.
 */
void piglit_gl_invalidate_extensions();

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
 * \brief Convert a string to a GL enum.
 *
 * For example, given "GL_INVALID_ENUM", return GL_INVALID_ENUM.
 *
 * abort() if the string is not recognized.
 */
GLenum piglit_get_gl_enum_from_name(const char *name);

GLenum
piglit_get_gl_memory_barrier_enum_from_name(const char *name);

/**
 * \brief Convert a GL primitive type enum value to a string.
 *
 * For example, given GL_POLYGON, return "GL_POLYGON".
 * We don't use piglit_get_gl_enum_name() for this because there are
 * other enums which alias the prim type enums (ex: GL_POINTS = GL_NONE);
 *
 * Return "(unrecognized enum)" if the enum is not recognized.
 */
const char *piglit_get_prim_name(GLenum prim);


/**
 * \brief Check for unexpected GL errors.
 *
 * If glGetError() returns an error other than \c expected_error, then
 * print a diagnostic and return GL_FALSE.  Otherwise return GL_TRUE.
 */
GLboolean
piglit_check_gl_error_(GLenum expected_error, const char *file, unsigned line);

#define piglit_check_gl_error(expected) \
 piglit_check_gl_error_((expected), __FILE__, __LINE__)

/**
 * \brief Drain all GL errors.
 *
 * Repeatly call glGetError and discard errors until it returns GL_NO_ERROR.
 */
void piglit_reset_gl_error(void);

void piglit_require_gl_version(int required_version_times_10);
void piglit_require_extension(const char *name);
void piglit_require_not_extension(const char *name);
unsigned piglit_num_components(GLenum format);
bool piglit_get_luminance_intensity_bits(GLenum internalformat, int *bits);
int piglit_probe_pixel_rgb_silent(int x, int y, const float* expected, float *out_probe);
int piglit_probe_pixel_rgba_silent(int x, int y, const float* expected, float *out_probe);
int piglit_probe_pixel_rgb(int x, int y, const float* expected);
int piglit_probe_pixel_rgba(int x, int y, const float* expected);
int piglit_probe_rect_r_ubyte(int x, int y, int w, int h, GLubyte expected);
int piglit_probe_rect_rgb(int x, int y, int w, int h, const float* expected);
int piglit_probe_rect_rgb_silent(int x, int y, int w, int h, const float *expected);
int piglit_probe_rect_rgba(int x, int y, int w, int h, const float* expected);
int piglit_probe_rect_rgba_int(int x, int y, int w, int h, const int* expected);
int piglit_probe_rect_rgba_uint(int x, int y, int w, int h, const unsigned int* expected);
void piglit_compute_probe_tolerance(GLenum format, float *tolerance);

/**
 * Compare two pixels.
 * \param x the x coordinate of the pixel being probed
 * \param y the y coordinate of the pixel being probed
 */
int piglit_compare_pixels(int x, int y, const float *expected, const float *probe,
			 const float *tolerance, int num_components);

/**
 * Compare two adjacent in-memory floating-point images.
 * Adjacent means: y1 == y2 && x1 == x2 - w;
 *
 * \param w the width of the rectangle containing both images
 * \param h the height of the rectangle containing both images
 * \param images : the start of the buffer containing the observed image on
 *		the left and the expected image on the right
 */
int piglit_compare_image_halves_color(int w, int h, int num_components,
			    const float *tolerance,
			    const float *expected_observed_image);
int piglit_compare_images_color(int x, int y, int w, int h, int num_components,
				const float *tolerance,
				const float *expected_image,
				const float *observed_image);
int piglit_probe_image_color(int x, int y, int w, int h, GLenum format, const float *image);
int piglit_probe_image_rgb(int x, int y, int w, int h, const float *image);
int piglit_probe_image_rgba(int x, int y, int w, int h, const float *image);
int piglit_compare_images_ubyte(int x, int y, int w, int h,
				const GLubyte *expected_image,
				const GLubyte *observed_image);
int piglit_equal_images_update_rgba8(const GLubyte *expected_original,
				     const GLubyte *expected_updated,
				     const GLubyte *observed,
				     unsigned w, unsigned h, unsigned d,
				     unsigned ux, unsigned uy, unsigned uz,
				     unsigned uw, unsigned uh, unsigned ud,
				     unsigned bits);
int piglit_probe_image_stencil(int x, int y, int w, int h, const GLubyte *image);
int piglit_probe_image_ubyte(int x, int y, int w, int h, GLenum format,
			     const GLubyte *image);
int piglit_probe_texel_rect_rgb(int target, int level, int x, int y,
				int w, int h, const float *expected);
int piglit_probe_texel_rgb(int target, int level, int x, int y,
			   const float* expected);
int piglit_probe_texel_rect_rgba(int target, int level, int x, int y,
				 int w, int h, const float *expected);
int piglit_probe_texel_rgba(int target, int level, int x, int y,
			    const float* expected);
int piglit_probe_texel_volume_rgba(int target, int level, int x, int y, int z,
				 int w, int h, int d, const float *expected);
int piglit_probe_pixel_depth(int x, int y, float expected);
int piglit_probe_rect_depth(int x, int y, int w, int h, float expected);
int piglit_probe_pixel_stencil(int x, int y, unsigned expected);
int piglit_probe_rect_stencil(int x, int y, int w, int h, unsigned expected);
int piglit_probe_rect_halves_equal_rgba(int x, int y, int w, int h);

/**
 * \brief Check if two rectangles are equivalent
 *
 * Given the coordinates of two rectangles, check that the two are equal.
 * The first rectangle is what's observed, whereas the second rectangle is
 * what's expected.
 */
int piglit_probe_rects_equal(int x1, int y1, int x2, int y2,
			int w, int h, GLenum format);

bool piglit_probe_buffer(GLuint buf, GLenum target, const char *label,
			 unsigned n, unsigned num_components,
			 const float *expected);
bool piglit_probe_buffer_doubles(GLuint buf, GLenum target, const char *label,
				 unsigned n, unsigned num_components,
				 const double *expected);
int piglit_use_fragment_program(void);
int piglit_use_vertex_program(void);
void piglit_require_fragment_program(void);
void piglit_require_vertex_program(void);
GLuint piglit_compile_program(GLenum target, const char* text);
GLvoid piglit_draw_triangle(float x1, float y1, float x2, float y2,
			    float x3, float y3);
GLvoid piglit_draw_triangle_z(float z, float x1, float y1, float x2, float y2,
			      float x3, float y3);
GLvoid piglit_draw_rect_custom(float x, float y, float w, float h,
			       bool use_patches, unsigned instance_count);
GLvoid piglit_draw_rect(float x, float y, float w, float h);
GLvoid piglit_draw_rect_z(float z, float x, float y, float w, float h);
GLvoid piglit_draw_rect_tex(float x, float y, float w, float h,
                            float tx, float ty, float tw, float th);
GLvoid piglit_draw_rect_back(float x, float y, float w, float h);
void piglit_draw_rect_from_arrays(const void *verts, const void *tex,
				  bool use_patches, unsigned instance_count);

unsigned short piglit_half_from_float(float val);

/**
 * Wrapper for piglit_half_from_float() which allows using an exact
 * hex bit pattern to generate a half float value.
 */
static inline unsigned short
strtohf_hex(const char *nptr, char **endptr)
{
	/* skip spaces and tabs */
	while (*nptr == ' ' || *nptr == '\t')
		nptr++;

	if (strncmp(nptr, "0x", 2) == 0) {
		uint32_t u = strtoul(nptr, endptr, 16);
		if (u > USHRT_MAX) {
			errno = ERANGE;
			return USHRT_MAX;
		} else {
			return u;
		}
	} else {
		return piglit_half_from_float(strtod_inf(nptr, endptr));
	}
}

void piglit_escape_exit_key(unsigned char key, int x, int y);

void piglit_gen_ortho_projection(double left, double right, double bottom,
				 double top, double near_val, double far_val,
				 GLboolean push);
void piglit_ortho_projection(int w, int h, GLboolean push);
void piglit_frustum_projection(GLboolean push, double l, double r, double b,
			       double t, double n, double f);
void piglit_gen_ortho_uniform(GLint location, double left, double right,
			      double bottom, double top, double near_val,
			      double far_val);
void piglit_ortho_uniform(GLint location, int w, int h);

GLuint
piglit_quads_texture(GLuint tex, unsigned level,
		     unsigned width, unsigned height,
		     unsigned horiz_square_size,
		     unsigned vert_square_size,
		     const float *bl, const float *br,
		     const float *tl, const float *tr);
GLuint piglit_checkerboard_texture(GLuint tex, unsigned level,
    unsigned width, unsigned height,
    unsigned horiz_square_size, unsigned vert_square_size,
    const float *black, const float *white);
GLuint piglit_miptree_texture(void);
GLfloat *piglit_rgbw_image(GLenum internalFormat, int w, int h,
                           GLboolean alpha, GLenum basetype);
GLubyte *piglit_rgbw_image_ubyte(int w, int h, GLboolean alpha);
GLuint piglit_rgbw_texture(GLenum internalFormat, int w, int h, GLboolean mip,
		    GLboolean alpha, GLenum basetype);
GLuint piglit_rgbw_texture_1d(void);
GLuint piglit_rgbw_texture_3d(void);
GLuint piglit_integer_texture(GLenum internalFormat, int w, int h, int b, int a);
GLuint piglit_depth_texture(GLenum target, GLenum format, int w, int h, int d, GLboolean mip);
GLuint piglit_array_texture(GLenum target, GLenum format, int w, int h, int d, GLboolean mip);
GLuint piglit_multisample_texture(GLenum target, GLuint tex,
				  GLenum internalFormat,
				  unsigned width, unsigned height,
				  unsigned depth, unsigned samples,
				  GLenum format, GLenum type, const void *data);
extern float piglit_tolerance[4];
void piglit_set_tolerance_for_bits(int rbits, int gbits, int bbits, int abits);
extern void piglit_require_transform_feedback(void);

bool
piglit_get_compressed_block_size(GLenum format,
				 unsigned *bw, unsigned *bh, unsigned *bytes);

unsigned
piglit_compressed_image_size(GLenum format, unsigned width, unsigned height);

unsigned
piglit_compressed_pixel_offset(GLenum format, unsigned width,
			       unsigned x, unsigned y);

void
piglit_visualize_image(float *img, GLenum base_internal_format,
		       int image_width, int image_height,
		       int image_count, bool rhs);

float piglit_srgb_to_linear(float x);
float piglit_linear_to_srgb(float x);

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

static const GLint PIGLIT_ATTRIB_POS = 0;
static const GLint PIGLIT_ATTRIB_TEX = 1;

/**
 * Given a GLSL version number, return the lowest-numbered GL version
 * that is guaranteed to support it.
 */
unsigned
required_gl_version_from_glsl_version(unsigned glsl_version);

void
piglit_write_png(const char *filename, GLenum base_format,
                 int width, int height, GLubyte *data, bool flip_y);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif /* __PIGLIT_UTIL_GL_H__ */
