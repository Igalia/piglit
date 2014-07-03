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

#include "piglit-util-gl-common.h"
#include <ctype.h>

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

/**
 * An array of pointers to extension strings.
 *
 * Each extension is pointed to by a separate entry in the array.
 *
 * The end of the array is indicated by a NULL pointer.
 */
static const char **gl_extensions = NULL;

bool piglit_is_core_profile;

bool piglit_is_gles(void)
{
	const char *version_string = (const char *) glGetString(GL_VERSION);
	return strncmp("OpenGL ES", version_string, 9) == 0;
}

int piglit_get_gl_version(void)
{
	const char *version_string = (const char *) glGetString(GL_VERSION);
	int scanf_count;
	int major;
	int minor;

	/* skip to version number */
	while (!isdigit(*version_string) && *version_string != '\0')
		version_string++;

	/* Interpret version number */
	scanf_count = sscanf(version_string, "%i.%i", &major, &minor);
	if (scanf_count != 2) {
		printf("Unable to interpret GL_VERSION string: %s\n",
		       version_string);
		piglit_report_result(PIGLIT_FAIL);
	}
	return 10*major+minor;
}

static const char** gl_extension_array_from_getstring()
{
	const char *gl_extensions_string;
	gl_extensions_string = (const char *) glGetString(GL_EXTENSIONS);
	return piglit_split_string_to_array(gl_extensions_string, " ");
}

#if defined(PIGLIT_USE_OPENGL)
static const char** gl_extension_array_from_getstringi()
{
	const char **strings;
	int loop, num_extensions;

	glGetIntegerv(GL_NUM_EXTENSIONS, &num_extensions);
	strings = malloc (sizeof(char*) * (num_extensions + 1));
	assert (strings != NULL);

	for (loop = 0; loop < num_extensions; loop++) {
		strings[loop] = (const char*) glGetStringi(GL_EXTENSIONS, loop);
	}

	strings[loop] = NULL;

	return (const char**) strings;
}
#endif

static void initialize_piglit_extension_support(void)
{
	if (gl_extensions != NULL) {
		return;
	}

#if defined(PIGLIT_USE_OPENGL_ES1) || \
    defined(PIGLIT_USE_OPENGL_ES2) || \
    defined(PIGLIT_USE_OPENGL_ES3)
	gl_extensions = gl_extension_array_from_getstring();
#elif defined(PIGLIT_USE_OPENGL)
	if (piglit_get_gl_version() < 30) {
		gl_extensions = gl_extension_array_from_getstring();
	} else {
		gl_extensions = gl_extension_array_from_getstringi();
	}
#else
#error Need code implemented to read extensions
#endif
}

void piglit_gl_reinitialize_extensions()
{
	if (gl_extensions != NULL) {
		free(gl_extensions);
		gl_extensions = NULL;
	}
}

bool piglit_is_extension_supported(const char *name)
{
	initialize_piglit_extension_support();
	return piglit_is_extension_in_array(gl_extensions, name);
}

void piglit_require_gl_version(int required_version_times_10)
{
	if (piglit_is_gles() ||
	    piglit_get_gl_version() < required_version_times_10) {
		printf("Test requires GL version %g\n",
		       required_version_times_10 / 10.0);
		piglit_report_result(PIGLIT_SKIP);
	}
}

void piglit_require_extension(const char *name)
{
	if (!piglit_is_extension_supported(name)) {
		printf("Test requires %s\n", name);
		piglit_report_result(PIGLIT_SKIP);
	}
}

void piglit_require_not_extension(const char *name)
{
	if (piglit_is_extension_supported(name)) {
		piglit_report_result(PIGLIT_SKIP);
	}
}

const char* piglit_get_gl_error_name(GLenum error)
{
#define CASE(x) case x: return #x; 
    switch (error) {
    CASE(GL_INVALID_ENUM)
    CASE(GL_INVALID_OPERATION)
    CASE(GL_INVALID_VALUE)
    CASE(GL_NO_ERROR)
    CASE(GL_OUT_OF_MEMORY)
    /* enums that are not available everywhere */
#if defined(GL_STACK_OVERFLOW)
    CASE(GL_STACK_OVERFLOW)
#endif
#if defined(GL_STACK_UNDERFLOW)
    CASE(GL_STACK_UNDERFLOW)
#endif
#if defined(GL_INVALID_FRAMEBUFFER_OPERATION)
    CASE(GL_INVALID_FRAMEBUFFER_OPERATION)
#endif
    default:
        return "(unrecognized error)";
    }
#undef CASE
}

GLboolean
piglit_check_gl_error_(GLenum expected_error, const char *file, unsigned line)
{
	GLenum actual_error;

	actual_error = glGetError();
	if (actual_error == expected_error) {
		return GL_TRUE;
	}

	/*
	 * If the lookup of the error's name is successful, then print
	 *     Unexpected GL error: NAME 0xHEX
	 * Else, print
	 *     Unexpected GL error: 0xHEX
	 */
	printf("Unexpected GL error: %s 0x%x\n",
               piglit_get_gl_error_name(actual_error), actual_error);
        printf("(Error at %s:%u)\n", file, line);

	/* Print the expected error, but only if an error was really expected. */
	if (expected_error != GL_NO_ERROR) {
		printf("Expected GL error: %s 0x%x\n",
		piglit_get_gl_error_name(expected_error), expected_error);
        }

	return GL_FALSE;
}

void piglit_reset_gl_error(void)
{
	while (glGetError() != GL_NO_ERROR) {
		/* empty */
	}
}

/* These texture coordinates should have 1 or -1 in the major axis selecting
 * the face, and a nearly-1-or-negative-1 value in the other two coordinates
 * which will be used to produce the s,t values used to sample that face's
 * image.
 */
GLfloat cube_face_texcoords[6][4][3] = {
	{ /* GL_TEXTURE_CUBE_MAP_POSITIVE_X */
		{1.0,  0.99,  0.99},
		{1.0,  0.99, -0.99},
		{1.0, -0.99, -0.99},
		{1.0, -0.99,  0.99},
	},
	{ /* GL_TEXTURE_CUBE_MAP_NEGATIVE_X */
		{-1.0,  0.99, -0.99},
		{-1.0,  0.99,  0.99},
		{-1.0, -0.99,  0.99},
		{-1.0, -0.99, -0.99},
	},
	{ /* GL_TEXTURE_CUBE_MAP_POSITIVE_Y */
		{-0.99, 1.0, -0.99},
		{ 0.99, 1.0, -0.99},
		{ 0.99, 1.0,  0.99},
		{-0.99, 1.0,  0.99},
	},
	{ /* GL_TEXTURE_CUBE_MAP_NEGATIVE_Y */
		{-0.99, -1.0,  0.99},
		{-0.99, -1.0, -0.99},
		{ 0.99, -1.0, -0.99},
		{ 0.99, -1.0,  0.99},
	},
	{ /* GL_TEXTURE_CUBE_MAP_POSITIVE_Z */
		{-0.99,  0.99, 1.0},
		{-0.99, -0.99, 1.0},
		{ 0.99, -0.99, 1.0},
		{ 0.99,  0.99, 1.0},
	},
	{ /* GL_TEXTURE_CUBE_MAP_NEGATIVE_Z */
		{ 0.99,  0.99, -1.0},
		{-0.99,  0.99, -1.0},
		{-0.99, -0.99, -1.0},
		{ 0.99, -0.99, -1.0},
	},
};

const char *cube_face_names[6] = {
	"POSITIVE_X",
	"NEGATIVE_X",
	"POSITIVE_Y",
	"NEGATIVE_Y",
	"POSITIVE_Z",
	"NEGATIVE_Z",
};

const GLenum cube_face_targets[6] = {
	GL_TEXTURE_CUBE_MAP_POSITIVE_X,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
};

float piglit_tolerance[4] = { 0.01, 0.01, 0.01, 0.01 };

void
piglit_set_tolerance_for_bits(int rbits, int gbits, int bbits, int abits)
{
	int bits[4] = {rbits, gbits, bbits, abits};
	int i;

	for (i = 0; i < 4; i++) {
		if (bits[i] == 0) {
			/* With 0 bits of storage, we still want to
			 * validate expected results, (such as
			 * alpha==1.0 when targeting storage with no
			 * alpha bits). */
			piglit_tolerance[i] = 3.0 / (1 << 8);
		} else if (bits[i] == 1) {
			/* Don't try to validate channels when there's only 1
			 * bit of precision.
			 */
			piglit_tolerance[i] = 1.0;
		} else {
			piglit_tolerance[i] = 3.0 / (1 << bits[i]);
		}
	}
}

typedef union { GLfloat f; GLint i; } fi_type;

/**
 * Convert a 4-byte float to a 2-byte half float.
 * Based on code from:
 * http://www.opengl.org/discussion_boards/ubb/Forum3/HTML/008786.html
 *
 * Taken over from Mesa.
 */
unsigned short
piglit_half_from_float(float val)
{
	const fi_type fi = {val};
	const int flt_m = fi.i & 0x7fffff;
	const int flt_e = (fi.i >> 23) & 0xff;
	const int flt_s = (fi.i >> 31) & 0x1;
	int s, e, m = 0;
	unsigned short result;

	/* sign bit */
	s = flt_s;

	/* handle special cases */
	if ((flt_e == 0) && (flt_m == 0)) {
		/* zero */
		/* m = 0; - already set */
		e = 0;
	}
	else if ((flt_e == 0) && (flt_m != 0)) {
		/* denorm -- denorm float maps to 0 half */
		/* m = 0; - already set */
		e = 0;
	}
	else if ((flt_e == 0xff) && (flt_m == 0)) {
		/* infinity */
		/* m = 0; - already set */
		e = 31;
	}
	else if ((flt_e == 0xff) && (flt_m != 0)) {
		/* NaN */
		m = 1;
		e = 31;
	}
	else {
		/* regular number */
		const int new_exp = flt_e - 127;
		if (new_exp < -24) {
			/* this maps to 0 */
			/* m = 0; - already set */
			e = 0;
		}
		else if (new_exp < -14) {
			/* this maps to a denorm */
			/* 2^-exp_val*/
			unsigned int exp_val = (unsigned int) (-14 - new_exp);

			e = 0;
			switch (exp_val) {
			case 0:
				/* m = 0; - already set */
				break;
			case 1: m = 512 + (flt_m >> 14); break;
			case 2: m = 256 + (flt_m >> 15); break;
			case 3: m = 128 + (flt_m >> 16); break;
			case 4: m = 64 + (flt_m >> 17); break;
			case 5: m = 32 + (flt_m >> 18); break;
			case 6: m = 16 + (flt_m >> 19); break;
			case 7: m = 8 + (flt_m >> 20); break;
			case 8: m = 4 + (flt_m >> 21); break;
			case 9: m = 2 + (flt_m >> 22); break;
			case 10: m = 1; break;
			}
		}
		else if (new_exp > 15) {
			/* map this value to infinity */
			/* m = 0; - already set */
			e = 31;
		}
		else {
			/* regular */
			e = new_exp + 15;
			m = flt_m >> 13;
		}
	}

	result = (s << 15) | (e << 10) | m;
	return result;
}

int
piglit_probe_rect_halves_equal_rgba(int x, int y, int w, int h)
{
	int i, j, p;
	GLfloat probe1[4];
	GLfloat probe2[4];
	GLubyte *pixels = malloc(w*h*4*sizeof(GLubyte));

	glReadPixels(x, y, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

	for (j = 0; j < h; j++) {
		for (i = 0; i < w / 2; i++) {
			GLubyte *pixel1 = &pixels[4 * (j * w + i)];
			GLubyte *pixel2 = &pixels[4 * (j * w + w / 2 + i)];

			for (p = 0; p < 4; ++p) {
				probe1[p] = pixel1[p] / 255.0f;
				probe2[p] = pixel2[p] / 255.0f;
			}

			for (p = 0; p < 4; ++p) {
				if (fabs(probe1[p] - probe2[p]) >= piglit_tolerance[p]) {
					printf("Probe color at (%i,%i)\n", x+i, x+j);
					printf("  Left: %f %f %f %f\n",
					       probe1[0], probe1[1], probe1[2], probe1[3]);
					printf("  Right: %f %f %f %f\n",
					       probe2[0], probe2[1], probe2[2], probe2[3]);

					free(pixels);
					return 0;
				}
			}
		}
	}

	free(pixels);
	return 1;
}


/**
 * Return block size info for a specific texture compression format.
 * \param  bw returns the block width, in pixels
 * \param  bh returns the block height, in pixels
 * \param  return number of bytes per block
 * \return true if format is a known compressed format, false otherwise
 */
bool
piglit_get_compressed_block_size(GLenum format,
				 unsigned *bw, unsigned *bh, unsigned *bytes)
{
	switch (format) {
#if defined(PIGLIT_USE_OPENGL) || defined(PIGLIT_USE_OPENGL_ES2)
	case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
	case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
		*bw = *bh = 4;
		*bytes = 8;
		return true;
#endif
#if defined(PIGLIT_USE_OPENGL)
	case GL_COMPRESSED_SRGB_S3TC_DXT1_EXT:
	case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT:
	case GL_COMPRESSED_RED_RGTC1:
	case GL_COMPRESSED_SIGNED_RED_RGTC1:
	case GL_COMPRESSED_LUMINANCE_LATC1_EXT:
	case GL_COMPRESSED_SIGNED_LUMINANCE_LATC1_EXT:
		*bw = *bh = 4;
		*bytes = 8;
		return true;
	case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
	case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
	case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT:
	case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT:
	case GL_COMPRESSED_RG_RGTC2:
	case GL_COMPRESSED_SIGNED_RG_RGTC2:
	case GL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT:
	case GL_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2_EXT:
		*bw = *bh = 4;
		*bytes = 16;
		return true;
	case GL_COMPRESSED_RGB_FXT1_3DFX:
	case GL_COMPRESSED_RGBA_FXT1_3DFX:
		*bw = 8;
		*bh = 4;
		*bytes = 16;
		return true;
#endif
	default:
		/* return something rather than uninitialized values */
		*bw = *bh = *bytes = 1;
		return false;
	}
}


/**
 * Compute size (in bytes) neede to store an image in the given compressed
 * format.
 */
unsigned
piglit_compressed_image_size(GLenum format, unsigned width, unsigned height)
{
	unsigned bw, bh, bytes;
	bool b = piglit_get_compressed_block_size(format, &bw, &bh, &bytes);
	assert(b);
	return ((width + bw - 1) / bw) * ((height + bh - 1) / bh) * bytes;
}


/**
 * Return offset (in bytes) to the given texel in a compressed image.
 * Note the x and y must be multiples of the compressed block size.
 */
unsigned
piglit_compressed_pixel_offset(GLenum format, unsigned width,
			       unsigned x, unsigned y)
{
	unsigned bw, bh, bytes, offset;
	bool b = piglit_get_compressed_block_size(format, &bw, &bh, &bytes);

	assert(b);

	assert(x % bw == 0);
	assert(y % bh == 0);
	assert(width % bw == 0);

	offset = (width / bw * bytes * y / bh) + (x / bw * bytes);

	return offset;
}


#ifndef PIGLIT_USE_OPENGL_ES1
/**
 * Convenience function to configure a shader uniform variable as an
 * arbitrary orthogonal projection matrix.
 */
void
piglit_gen_ortho_uniform(GLint location, double l, double r, double b,
			 double t, double n, double f)
{
	const GLfloat values[4][4] = {
		{ 2/(r-l),    0,        0,    -(r+l)/(r-l) },
		{    0,    2/(t-b),     0,    -(t+b)/(t-b) },
		{    0,       0,    -2/(f-n), -(f+n)/(f-n) },
		{    0,       0,        0,          1      }
	};
	glUniformMatrix4fv(location, 1, GL_TRUE, (const GLfloat *)values);
}


/**
 * Convenience function to configure a shader uniform variable as a
 * projection matrix for window coordinates.
 */
void
piglit_ortho_uniform(GLint location, int w, int h)
{
        /* Set up projection matrix so we can just draw using window
         * coordinates.
         */
	piglit_gen_ortho_uniform(location, 0, w, 0, h, -1, 1);
}
#endif


unsigned
required_gl_version_from_glsl_version(unsigned glsl_version)
{
	switch (glsl_version) {
	case 110: return 20;
	case 120: return 21;
	/* GLSL 1.30 is naturally matched with GL3,
	 * but is usefully supportable on GL2.1 if
	 * EXT_gpu_shader4 is also supported.
	 */
	case 130: return 21;
	case 140: return 31;
	case 150: return 32;
	case 330: return 33;
	case 400: return 40;
	case 410: return 41;
	case 420: return 42;
	case 430: return 43;
	default: return 0;
	}
}

/**
 * Call glDrawArrays.  verts is expected to be
 *
 *   float verts[4][4];
 *
 * if not NULL; tex is expected to be
 *
 *   float tex[4][2];
 *
 * if not NULL.
 */
void
piglit_draw_rect_from_arrays(const void *verts, const void *tex)
{
#if defined(PIGLIT_USE_OPENGL_ES1)
	const bool use_fixed_function_attributes = true;
#elif defined(PIGLIT_USE_OPENGL_ES2) || defined(PIGLIT_USE_OPENGL_ES3)
	const bool use_fixed_function_attributes = false;
#elif defined(PIGLIT_USE_OPENGL)
	bool use_fixed_function_attributes = true;
#else
#error "don't know how to draw arrays"
#endif

#if defined(PIGLIT_USE_OPENGL)
	if (piglit_get_gl_version() >= 20
	    || piglit_is_extension_supported("GL_ARB_shader_objects")) {
		GLuint prog;

		glGetIntegerv(GL_CURRENT_PROGRAM, (GLint *) &prog);

		/* If there is a current program and that program has an
		 * active attribute named piglit_vertex, don't use the fixed
		 * function inputs.
		 */
		use_fixed_function_attributes = prog == 0
			|| glGetAttribLocation(prog, "piglit_vertex") == -1;
	}
#endif

#if defined(PIGLIT_USE_OPENGL_ES1) || defined(PIGLIT_USE_OPENGL)
	if (use_fixed_function_attributes) {
		if (verts) {
			glVertexPointer(4, GL_FLOAT, 0, verts);
			glEnableClientState(GL_VERTEX_ARRAY);
		}

		if (tex) {
			glTexCoordPointer(2, GL_FLOAT, 0, tex);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		}

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		if (verts)
			glDisableClientState(GL_VERTEX_ARRAY);
		if (tex)
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	}
#endif
#if defined(PIGLIT_USE_OPENGL_ES2) || defined(PIGLIT_USE_OPENGL_ES3) \
	|| defined(PIGLIT_USE_OPENGL)
	if (!use_fixed_function_attributes) {
		GLuint buf = 0;
		GLuint old_buf = 0;
		GLuint vao = 0;
		GLuint old_vao = 0;

		/* Vertex array objects were added in both OpenGL 3.0 and
		 * OpenGL ES 3.0.  The use of VAOs is required in desktop
		 * OpenGL 3.1 (without GL_ARB_compatibility) and all desktop
		 * OpenGL core profiles.  If the functionality is supported,
		 * just use it.
		 */
		if (piglit_get_gl_version() >= 30
		    || piglit_is_extension_supported("GL_OES_vertex_array_object")
		    || piglit_is_extension_supported("GL_ARB_vertex_array_object")) {
			glGetIntegerv(GL_VERTEX_ARRAY_BINDING,
				      (GLint *) &old_vao);
			glGenVertexArrays(1, &vao);
			glBindVertexArray(vao);
		}

		/* Assume that VBOs are supported in any implementation that
		 * uses shaders.
		 */
		glGetIntegerv(GL_ARRAY_BUFFER_BINDING,
			      (GLint *) &old_buf);
		glGenBuffers(1, &buf);
		glBindBuffer(GL_ARRAY_BUFFER, buf);
		glBufferData(GL_ARRAY_BUFFER,
			     (sizeof(GLfloat) * 4 * 4)
			     + (sizeof(GLfloat) * 4 * 2),
			     NULL,
			     GL_STATIC_DRAW);

		if (verts) {
			glBufferSubData(GL_ARRAY_BUFFER,
					0,
					sizeof(GLfloat) * 4 * 4,
					verts);
			glVertexAttribPointer(PIGLIT_ATTRIB_POS, 4, GL_FLOAT,
					      GL_FALSE, 0,
					      BUFFER_OFFSET(0));
			glEnableVertexAttribArray(PIGLIT_ATTRIB_POS);
		}

		if (tex) {
			glBufferSubData(GL_ARRAY_BUFFER,
					sizeof(GLfloat) * 4 * 4,
					sizeof(GLfloat) * 4 * 2,
					tex);
			glVertexAttribPointer(PIGLIT_ATTRIB_TEX, 2, GL_FLOAT,
					      GL_FALSE, 0,
					      BUFFER_OFFSET(sizeof(GLfloat) * 4 * 4));
			glEnableVertexAttribArray(PIGLIT_ATTRIB_TEX);
		}

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		if (verts)
			glDisableVertexAttribArray(PIGLIT_ATTRIB_POS);
		if (tex)
			glDisableVertexAttribArray(PIGLIT_ATTRIB_TEX);

		glBindBuffer(GL_ARRAY_BUFFER, old_buf);
		glDeleteBuffers(1, &buf);

		if (vao != 0) {
			glBindVertexArray(old_vao);
			glDeleteVertexArrays(1, &vao);
		}
	}
#endif
}

/**
 * Convenience function to draw an axis-aligned rectangle.
 */
GLvoid
piglit_draw_rect(float x, float y, float w, float h)
{
	float verts[4][4];

	verts[0][0] = x;
	verts[0][1] = y;
	verts[0][2] = 0.0;
	verts[0][3] = 1.0;
	verts[1][0] = x + w;
	verts[1][1] = y;
	verts[1][2] = 0.0;
	verts[1][3] = 1.0;
	verts[2][0] = x;
	verts[2][1] = y + h;
	verts[2][2] = 0.0;
	verts[2][3] = 1.0;
	verts[3][0] = x + w;
	verts[3][1] = y + h;
	verts[3][2] = 0.0;
	verts[3][3] = 1.0;

	piglit_draw_rect_from_arrays(verts, NULL);
}

/**
 * Convenience function to draw an axis-aligned rectangle.
 */
GLvoid
piglit_draw_rect_z(float z, float x, float y, float w, float h)
{
	float verts[4][4];

	verts[0][0] = x;
	verts[0][1] = y;
	verts[0][2] = z;
	verts[0][3] = 1.0;
	verts[1][0] = x + w;
	verts[1][1] = y;
	verts[1][2] = z;
	verts[1][3] = 1.0;
	verts[2][0] = x;
	verts[2][1] = y + h;
	verts[2][2] = z;
	verts[2][3] = 1.0;
	verts[3][0] = x + w;
	verts[3][1] = y + h;
	verts[3][2] = z;
	verts[3][3] = 1.0;

	piglit_draw_rect_from_arrays(verts, NULL);
}

/**
 * Convenience function to draw an axis-aligned rectangle
 * with texture coordinates.
 */
GLvoid
piglit_draw_rect_tex(float x, float y, float w, float h,
                     float tx, float ty, float tw, float th)
{
	float verts[4][4];
	float tex[4][2];

	verts[0][0] = x;
	verts[0][1] = y;
	verts[0][2] = 0.0;
	verts[0][3] = 1.0;
	tex[0][0] = tx;
	tex[0][1] = ty;
	verts[1][0] = x + w;
	verts[1][1] = y;
	verts[1][2] = 0.0;
	verts[1][3] = 1.0;
	tex[1][0] = tx + tw;
	tex[1][1] = ty;
	verts[2][0] = x;
	verts[2][1] = y + h;
	verts[2][2] = 0.0;
	verts[2][3] = 1.0;
	tex[2][0] = tx;
	tex[2][1] = ty + th;
	verts[3][0] = x + w;
	verts[3][1] = y + h;
	verts[3][2] = 0.0;
	verts[3][3] = 1.0;
	tex[3][0] = tx + tw;
	tex[3][1] = ty + th;

	piglit_draw_rect_from_arrays(verts, tex);
}

unsigned
piglit_num_components(GLenum base_format)
{
	switch (base_format) {
	case GL_ALPHA:
	case GL_DEPTH_COMPONENT:
	case GL_INTENSITY:
	case GL_LUMINANCE:
	case GL_RED:
		return 1;
	case GL_DEPTH_STENCIL:
	case GL_LUMINANCE_ALPHA:
	case GL_RG:
		return 2;
	case GL_RGB:
		return 3;
	case GL_RGBA:
		return 4;
	default:
		printf("Unknown num_components for %s\n",
		       piglit_get_gl_enum_name(base_format));
		piglit_report_result(PIGLIT_FAIL);
		return 0;
	}
}

/* This function only handles formats not supported by the OpenGL framebuffer
 * size queries, which only support querying the R,G,B,A sizes.
 *
 * The function doesn't change the bits for formats it doesn't handle.
 *
 * The returned number of bits is an approximation but should be no less than
 * the actual number of bits for the format chosen by OpenGL.
 *
 * The combination of the OpenGL framebuffer queries size and calling
 * this function without checking the return value should give you reasonable
 * values for any format.
 */
bool
piglit_get_luminance_intensity_bits(GLenum internalformat, int *bits)
{
	switch (internalformat) {
	case GL_LUMINANCE4:
		bits[0] = bits[1] = bits[2] = 4;
		bits[3] = 0;
		return true;

	case GL_LUMINANCE:
	case GL_LUMINANCE_SNORM:
	case GL_LUMINANCE8:
	case GL_LUMINANCE8_SNORM:
	case GL_LUMINANCE8I_EXT:
	case GL_LUMINANCE8UI_EXT:
		bits[0] = bits[1] = bits[2] = 8;
		bits[3] = 0;
		return true;

	case GL_LUMINANCE12:
		bits[0] = bits[1] = bits[2] = 12;
		bits[3] = 0;
		return true;

	case GL_LUMINANCE16:
	case GL_LUMINANCE16_SNORM:
	case GL_LUMINANCE16I_EXT:
	case GL_LUMINANCE16UI_EXT:
	case GL_LUMINANCE16F_ARB:
		bits[0] = bits[1] = bits[2] = 16;
		bits[3] = 0;
		return true;

	case GL_LUMINANCE32I_EXT:
	case GL_LUMINANCE32UI_EXT:
	case GL_LUMINANCE32F_ARB:
		bits[0] = bits[1] = bits[2] = 32;
		bits[3] = 0;
		return true;

	case GL_LUMINANCE4_ALPHA4:
	case GL_INTENSITY4:
		bits[0] = bits[1] = bits[2] = bits[3] = 4;
		return true;

	case GL_LUMINANCE_ALPHA:
	case GL_LUMINANCE_ALPHA_SNORM:
	case GL_LUMINANCE8_ALPHA8:
	case GL_LUMINANCE8_ALPHA8_SNORM:
	case GL_LUMINANCE_ALPHA8I_EXT:
	case GL_LUMINANCE_ALPHA8UI_EXT:
	case GL_INTENSITY:
	case GL_INTENSITY_SNORM:
	case GL_INTENSITY8:
	case GL_INTENSITY8_SNORM:
	case GL_INTENSITY8I_EXT:
	case GL_INTENSITY8UI_EXT:
		bits[0] = bits[1] = bits[2] = bits[3] = 8;
		return true;

	case GL_LUMINANCE12_ALPHA12:
	case GL_INTENSITY12:
		bits[0] = bits[1] = bits[2] = bits[3] = 12;
		return true;

	case GL_LUMINANCE16_ALPHA16:
	case GL_LUMINANCE16_ALPHA16_SNORM:
	case GL_LUMINANCE_ALPHA16I_EXT:
	case GL_LUMINANCE_ALPHA16UI_EXT:
	case GL_LUMINANCE_ALPHA16F_ARB:
	case GL_INTENSITY16:
	case GL_INTENSITY16_SNORM:
	case GL_INTENSITY16I_EXT:
	case GL_INTENSITY16UI_EXT:
	case GL_INTENSITY16F_ARB:
		bits[0] = bits[1] = bits[2] = bits[3] = 16;
		return true;

	case GL_LUMINANCE_ALPHA32I_EXT:
	case GL_LUMINANCE_ALPHA32UI_EXT:
	case GL_LUMINANCE_ALPHA32F_ARB:
	case GL_INTENSITY32I_EXT:
	case GL_INTENSITY32UI_EXT:
	case GL_INTENSITY32F_ARB:
		bits[0] = bits[1] = bits[2] = bits[3] = 32;
		return true;
	}
	return false;
}
