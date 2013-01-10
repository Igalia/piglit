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


/**
 * An array of pointers to extension strings.
 *
 * Each extension is pointed to by a separate entry in the array.
 *
 * The end of the array is indicated by a NULL pointer.
 */
static const char **gl_extensions = NULL;

bool piglit_is_gles(void)
{
	const char *version_string = (const char *) glGetString(GL_VERSION);
	return strncmp("OpenGL ES ", version_string, 10) == 0;
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
		exit(1);
	}
	return 10*major+minor;
}

static const char** split_string(const char *string)
{
	char **strings, *string_copy;
	int i, length, max_words;

	length = strlen(string);
	max_words = length / 2;
	strings = malloc ((sizeof(char*) * (max_words + 1)) +
	                  (sizeof(char) * (length + 1)));
	assert (strings != NULL);

	string_copy = (char*) &strings[max_words + 1];
	strcpy(string_copy, string);

	strings[0] = strtok(string_copy, " ");
	for (i = 0; strings[i] != NULL; ++i)
		strings[i + 1] = strtok(NULL, " ");

	return (const char**) strings;
}

static const char** gl_extension_array_from_getstring()
{
	const char *gl_extensions_string;
	gl_extensions_string = (const char *) glGetString(GL_EXTENSIONS);
	return split_string(gl_extensions_string);
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
		exit(1);
	}
}

void piglit_require_extension(const char *name)
{
	if (!piglit_is_extension_supported(name)) {
		printf("Test requires %s\n", name);
		piglit_report_result(PIGLIT_SKIP);
		exit(1);
	}
}

void piglit_require_not_extension(const char *name)
{
	if (piglit_is_extension_supported(name)) {
		piglit_report_result(PIGLIT_SKIP);
		exit(1);
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
		if (bits[i] < 2) {
			/* Don't try to validate channels when there's only 1
			 * bit of precision (or none).
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
					printf("Probe at (%i,%i)\n", x+i, x+j);
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
