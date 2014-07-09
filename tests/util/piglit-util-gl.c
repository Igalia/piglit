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

#include "piglit-util-gl.h"
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

static void initialize_piglit_extension_support(void)
{
	if (gl_extensions != NULL) {
		return;
	}

	if (piglit_get_gl_version() < 30) {
		gl_extensions = gl_extension_array_from_getstring();
	} else {
		gl_extensions = gl_extension_array_from_getstringi();
	}
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
	case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
	case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
		*bw = *bh = 4;
		*bytes = 8;
		return true;
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

void
piglit_escape_exit_key(unsigned char key, int x, int y)
{
	(void) x;
	(void) y;
	switch (key) {
		case 27:
			exit(0);
			break;
	}
	if (!piglit_is_gles())
		piglit_post_redisplay();
}

/**
 * Convenience function to configure an abitrary orthogonal projection matrix
 */
void
piglit_gen_ortho_projection(double left, double right, double bottom,
			    double top, double near_val, double far_val,
			    GLboolean push)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if (push)
		glPushMatrix();

	if (piglit_is_gles())
		glOrthof(left, right, bottom, top, near_val, far_val);
	else
		glOrtho(left, right, bottom, top, near_val, far_val);

	glMatrixMode(GL_MODELVIEW);
	if (push)
		glPushMatrix();
	glLoadIdentity();
}

/**
 * Convenience function to configure projection matrix for window coordinates
 */
void
piglit_ortho_projection(int w, int h, GLboolean push)
{
        /* Set up projection matrix so we can just draw using window
         * coordinates.
         */
	piglit_gen_ortho_projection(0, w, 0, h, -1, 1, push);
}

/**
 * Convenience function to configure frustum projection.
 */
void
piglit_frustum_projection(GLboolean push, double l, double r, double b,
			  double t, double n, double f)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if (push)
		glPushMatrix();

	if (piglit_is_gles())
		glFrustumf(l, r, b, t, n, f);
	else
		glFrustum(l, r, b, t, n, f);

	glMatrixMode(GL_MODELVIEW);
	if (push)
		glPushMatrix();
	glLoadIdentity();
}

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
	bool use_fixed_function_attributes;

	bool gles = piglit_is_gles();
	int version = piglit_get_gl_version();

	if (gles) {
		use_fixed_function_attributes = (version < 20);
	}  else if (version >= 20 ||
		    piglit_is_extension_supported("GL_ARB_shader_objects")) {
		GLuint prog;

		glGetIntegerv(GL_CURRENT_PROGRAM, (GLint *) &prog);

		/* If there is a current program and that program has an
		 * active attribute named piglit_vertex, don't use the fixed
		 * function inputs.
		 */
		use_fixed_function_attributes = (prog == 0)
			|| glGetAttribLocation(prog, "piglit_vertex") == -1;
	} else {
		use_fixed_function_attributes = true;
	}

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
	} else {
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

/* Wrapper around glReadPixels that always returns floats; reads and converts
 * GL_UNSIGNED_BYTE on GLES.  If pixels == NULL, malloc a float array of the
 * appropriate size, otherwise use the one provided. */
static GLfloat *
piglit_read_pixels_float(GLint x, GLint y, GLsizei width, GLsizei height,
                         GLenum format, GLfloat *pixels)
{
	GLubyte *pixels_b;
	unsigned i, ncomponents;

	ncomponents = width * height * piglit_num_components(format);
	if (!pixels)
		pixels = malloc(ncomponents * sizeof(GLfloat));

	if (!piglit_is_gles()) {
		glReadPixels(x, y, width, height, format, GL_FLOAT, pixels);
		return pixels;
	}

	pixels_b = malloc(ncomponents * sizeof(GLubyte));
	glReadPixels(x, y, width, height, format, GL_UNSIGNED_BYTE, pixels_b);
	for (i = 0; i < ncomponents; i++)
		pixels[i] = pixels_b[i] / 255.0;
	free(pixels_b);
	return pixels;
}

int
piglit_probe_pixel_rgb_silent(int x, int y, const float* expected, float *out_probe)
{
	GLfloat probe[3];
	int i;
	GLboolean pass = GL_TRUE;

	piglit_read_pixels_float(x, y, 1, 1, GL_RGB, probe);

	for(i = 0; i < 3; ++i)
		if (fabs(probe[i] - expected[i]) > piglit_tolerance[i])
			pass = GL_FALSE;

	if (out_probe)
		memcpy(out_probe, probe, sizeof(probe));

	return pass;
}

int
piglit_probe_pixel_rgba_silent(int x, int y, const float* expected, float *out_probe)
{
	GLfloat probe[4];
	int i;
	GLboolean pass = GL_TRUE;

	piglit_read_pixels_float(x, y, 1, 1, GL_RGBA, probe);

	for(i = 0; i < 4; ++i)
		if (fabs(probe[i] - expected[i]) > piglit_tolerance[i])
			pass = GL_FALSE;

	if (out_probe)
		memcpy(out_probe, probe, sizeof(probe));

	return pass;
}

/**
 * Read a pixel from the given location and compare its RGB value to the
 * given expected values.
 *
 * Print a log message if the color value deviates from the expected value.
 * \return true if the color values match, false otherwise
 */
int
piglit_probe_pixel_rgb(int x, int y, const float* expected)
{
	GLfloat probe[3];
	int i;
	GLboolean pass = GL_TRUE;

	piglit_read_pixels_float(x, y, 1, 1, GL_RGB, probe);

	for(i = 0; i < 3; ++i)
		if (fabs(probe[i] - expected[i]) > piglit_tolerance[i])
			pass = GL_FALSE;

	if (pass)
		return 1;

	printf("Probe color at (%i,%i)\n", x, y);
	printf("  Expected: %f %f %f\n", expected[0], expected[1], expected[2]);
	printf("  Observed: %f %f %f\n", probe[0], probe[1], probe[2]);

	return 0;
}

/**
 * Read a pixel from the given location and compare its RGBA value to the
 * given expected values.
 *
 * Print a log message if the color value deviates from the expected value.
 * \return true if the color values match, false otherwise
 */
int
piglit_probe_pixel_rgba(int x, int y, const float* expected)
{
	GLfloat probe[4];
	int i;
	GLboolean pass = GL_TRUE;

	piglit_read_pixels_float(x, y, 1, 1, GL_RGBA, probe);

	for(i = 0; i < 4; ++i)
		if (fabs(probe[i] - expected[i]) > piglit_tolerance[i])
			pass = GL_FALSE;

	if (pass)
		return 1;

	printf("Probe color at (%i,%i)\n", x, y);
	printf("  Expected: %f %f %f %f\n", expected[0], expected[1], expected[2], expected[3]);
	printf("  Observed: %f %f %f %f\n", probe[0], probe[1], probe[2], probe[3]);

	return 0;
}

int
piglit_probe_rect_rgb_silent(int x, int y, int w, int h, const float *expected)
{
	int i, j, p;
	GLfloat *probe;
	GLfloat *pixels;

	pixels = piglit_read_pixels_float(x, y, w, h, GL_RGB, NULL);

	for (j = 0; j < h; j++) {
		for (i = 0; i < w; i++) {
			probe = &pixels[(j*w+i)*3];

			for (p = 0; p < 3; ++p) {
				if (fabs(probe[p] - expected[p]) >= piglit_tolerance[p]) {
					free(pixels);
					return 0;
				}
			}
		}
	}

	free(pixels);
	return 1;
}

int
piglit_probe_rect_rgb(int x, int y, int w, int h, const float *expected)
{
	int i, j, p;
	GLfloat *probe;
	GLfloat *pixels;

	pixels = piglit_read_pixels_float(x, y, w, h, GL_RGB, NULL);

	for (j = 0; j < h; j++) {
		for (i = 0; i < w; i++) {
			probe = &pixels[(j*w+i)*3];

			for (p = 0; p < 3; ++p) {
				if (fabs(probe[p] - expected[p]) >= piglit_tolerance[p]) {
					printf("Probe color at (%i,%i)\n", x+i, y+j);
					printf("  Expected: %f %f %f\n",
					       expected[0], expected[1], expected[2]);
					printf("  Observed: %f %f %f\n",
					       probe[0], probe[1], probe[2]);

					free(pixels);
					return 0;
				}
			}
		}
	}

	free(pixels);
	return 1;
}

int
piglit_probe_rect_rgba(int x, int y, int w, int h, const float *expected)
{
	int i, j, p;
	GLfloat *probe;
	GLfloat *pixels;

	pixels = piglit_read_pixels_float(x, y, w, h, GL_RGBA, NULL);

	for (j = 0; j < h; j++) {
		for (i = 0; i < w; i++) {
			probe = &pixels[(j*w+i)*4];

			for (p = 0; p < 4; ++p) {
				if (fabs(probe[p] - expected[p]) >= piglit_tolerance[p]) {
					printf("Probe color at (%i,%i)\n", x+i, y+j);
					printf("  Expected: %f %f %f %f\n",
					       expected[0], expected[1], expected[2], expected[3]);
					printf("  Observed: %f %f %f %f\n",
					       probe[0], probe[1], probe[2], probe[3]);

					free(pixels);
					return 0;
				}
			}
		}
	}

	free(pixels);
	return 1;
}

int
piglit_probe_rect_rgba_int(int x, int y, int w, int h, const int *expected)
{
	int i, j, p;
	GLint *probe;
	GLint *pixels = malloc(w*h*4*sizeof(int));

	glReadPixels(x, y, w, h, GL_RGBA_INTEGER, GL_INT, pixels);

	for (j = 0; j < h; j++) {
		for (i = 0; i < w; i++) {
			probe = &pixels[(j*w+i)*4];

			for (p = 0; p < 4; ++p) {
				if (abs(probe[p] - expected[p]) >= piglit_tolerance[p]) {
					printf("Probe color at (%d,%d)\n", x+i, y+j);
					printf("  Expected: %d %d %d %d\n",
					       expected[0], expected[1], expected[2], expected[3]);
					printf("  Observed: %d %d %d %d\n",
					       probe[0], probe[1], probe[2], probe[3]);

					free(pixels);
					return 0;
				}
			}
		}
	}

	free(pixels);
	return 1;
}

int
piglit_probe_rect_rgba_uint(int x, int y, int w, int h,
			    const unsigned int *expected)
{
	int i, j, p;
	GLuint *probe;
	GLuint *pixels = malloc(w*h*4*sizeof(unsigned int));

	glReadPixels(x, y, w, h, GL_RGBA_INTEGER, GL_UNSIGNED_INT, pixels);

	for (j = 0; j < h; j++) {
		for (i = 0; i < w; i++) {
			probe = &pixels[(j*w+i)*4];

			for (p = 0; p < 4; ++p) {
				if (abs((int) (probe[p] - expected[p])) >= piglit_tolerance[p]) {
					printf("Probe color at (%d,%d)\n", x+i, y+j);
					printf("  Expected: %u %u %u %u\n",
					       expected[0], expected[1], expected[2], expected[3]);
					printf("  Observed: %u %u %u %u\n",
					       probe[0], probe[1], probe[2], probe[3]);

					free(pixels);
					return 0;
				}
			}
		}
	}

	free(pixels);
	return 1;
}

static void
print_pixel_ubyte(const GLubyte *pixel, unsigned components)
{
	int p;
	for (p = 0; p < components; ++p)
		printf(" %u", pixel[p]);
}

static void
print_pixel_float(const float *pixel, unsigned components)
{
	int p;
	for (p = 0; p < components; ++p)
		printf(" %f", pixel[p]);
}

/**
 * Compute the appropriate tolerance for comparing images of the given
 * base format.
 */
void
piglit_compute_probe_tolerance(GLenum format, float *tolerance)
{
	int num_components, component;
	switch (format) {
	case GL_LUMINANCE_ALPHA:
		tolerance[0] = piglit_tolerance[0];
		tolerance[1] = piglit_tolerance[3];
		break;
	case GL_ALPHA:
		tolerance[0] = piglit_tolerance[3];
		break;
	default:
		num_components = piglit_num_components(format);
		for (component = 0; component < num_components; ++component)
			tolerance[component] = piglit_tolerance[component];
		break;
	}
}

/**
 * Compare two in-memory floating-point images.
 */
int
piglit_compare_images_color(int x, int y, int w, int h, int num_components,
			    const float *tolerance,
			    const float *expected_image,
			    const float *observed_image)
{
	int i, j, p;
	for (j = 0; j < h; j++) {
		for (i = 0; i < w; i++) {
			const float *expected =
				&expected_image[(j*w+i)*num_components];
			const float *probe =
				&observed_image[(j*w+i)*num_components];

			for (p = 0; p < num_components; ++p) {
				if (fabs(probe[p] - expected[p])
				    >= tolerance[p]) {
					printf("Probe at (%i,%i)\n", x+i, y+j);
					printf("  Expected:");
					print_pixel_float(expected, num_components);
					printf("\n  Observed:");
					print_pixel_float(probe, num_components);
					printf("\n");

					return 0;
				}
			}
		}
	}

	return 1;
}

/**
 * Compare the contents of the current read framebuffer with the given
 * in-memory floating-point image.
 */
int
piglit_probe_image_color(int x, int y, int w, int h, GLenum format,
			 const float *image)
{
	int c = piglit_num_components(format);
	GLfloat *pixels;
	float tolerance[4];
	int result;

	piglit_compute_probe_tolerance(format, tolerance);

	if (format == GL_INTENSITY) {
		/* GL_INTENSITY is not allowed for ReadPixels so
		 * substitute GL_LUMINANCE.
		 */
		format = GL_LUMINANCE;
	}

	pixels = piglit_read_pixels_float(x, y, w, h, format, NULL);

	result = piglit_compare_images_color(x, y, w, h, c, tolerance, image,
					     pixels);

	free(pixels);
	return result;
}

int
piglit_probe_image_rgb(int x, int y, int w, int h, const float *image)
{
	return piglit_probe_image_color(x, y, w, h, GL_RGB, image);
}

int
piglit_probe_image_rgba(int x, int y, int w, int h, const float *image)
{
	return piglit_probe_image_color(x, y, w, h, GL_RGBA, image);
}

/**
 * Compare two in-memory unsigned-byte images.
 */
int
piglit_compare_images_ubyte(int x, int y, int w, int h,
			    const GLubyte *expected_image,
			    const GLubyte *observed_image)
{
	int i, j;
	for (j = 0; j < h; j++) {
		for (i = 0; i < w; i++) {
			const GLubyte expected = expected_image[j*w+i];
			const GLubyte probe = observed_image[j*w+i];

			if (probe != expected) {
				printf("Probe at (%i,%i)\n", x+i, y+j);
				printf("  Expected: %d\n", expected);
				printf("  Observed: %d\n", probe);

				return 0;
			}
		}
	}

	return 1;
}

/**
 * Compare the contents of the current read framebuffer's stencil
 * buffer with the given in-memory byte image.
 */
int
piglit_probe_image_stencil(int x, int y, int w, int h,
			   const GLubyte *image)
{
	GLubyte *pixels = malloc(w*h*sizeof(GLubyte));
	int result;
	GLint old_pack_alignment;

	/* Temporarily set pack alignment to 1 so that glReadPixels
	 * won't put any padding at the end of the row.
	 */
	glGetIntegerv(GL_PACK_ALIGNMENT, &old_pack_alignment);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);

	glReadPixels(x, y, w, h, GL_STENCIL_INDEX, GL_UNSIGNED_BYTE, pixels);

	glPixelStorei(GL_PACK_ALIGNMENT, old_pack_alignment);

	result = piglit_compare_images_ubyte(x, y, w, h, image, pixels);

	free(pixels);
	return result;
}

int
piglit_probe_image_ubyte(int x, int y, int w, int h, GLenum format,
			const GLubyte *image)
{
	const int c = piglit_num_components(format);
	GLubyte *pixels = malloc(w * h * 4 * sizeof(GLubyte));
	int i, j, p;

	glReadPixels(x, y, w, h, format, GL_UNSIGNED_BYTE, pixels);

	for (j = 0; j < h; j++) {
		for (i = 0; i < w; i++) {
			const GLubyte *expected = &image[(j * w + i) * c];
			const GLubyte *probe = &pixels[(j * w + i) * c];

			for (p = 0; p < c; ++p) {
				if (probe[p] == expected[p])
					continue;

				printf("Probe at (%i,%i)\n", x + i, y + j);
				printf("  Expected:");
				print_pixel_ubyte(expected, c);
				printf("\n  Observed:");
				print_pixel_ubyte(probe, c);
				printf("\n");

				free(pixels);
				return 0;
			}
		}
	}

	free(pixels);
	return 1;
}

/**
 * Read a texel rectangle from the given location and compare its RGB value to
 * the given expected values.
 *
 * Print a log message if the color value deviates from the expected value.
 * \return true if the color values match, false otherwise
 */
int piglit_probe_texel_rect_rgb(int target, int level, int x, int y,
				int w, int h, const float* expected)
{
	GLfloat *buffer;
	GLfloat *probe;
	int i, j, p;
	GLint width;
	GLint height;

	glGetTexLevelParameteriv(target, level, GL_TEXTURE_WIDTH, &width);
	glGetTexLevelParameteriv(target, level, GL_TEXTURE_HEIGHT, &height);
	buffer = malloc(width * height * 3 * sizeof(GLfloat));

	glGetTexImage(target, level, GL_RGB, GL_FLOAT, buffer);

	assert(x >= 0);
	assert(y >= 0);
	assert(x+w <= width);
	assert(y+h <= height);

	for (j = y; j < y+h; ++j) {
		for (i = x; i < x+w; ++i) {
			probe = &buffer[(j * width + i) * 3];

			for (p = 0; p < 3; ++p) {
				if (fabs(probe[p] - expected[p]) >= piglit_tolerance[p]) {
					printf("Probe color at (%i,%i)\n", i, j);
					printf("  Expected: %f %f %f\n",
					       expected[0], expected[1], expected[2]);
					printf("  Observed: %f %f %f\n",
					       probe[0], probe[1], probe[2]);

					free(buffer);
					return 0;
				}
			}
		}
	}

	free(buffer);
	return 1;
}

/**
 * Read a texel from the given location and compare its RGB value to the
 * given expected values.
 *
 * Print a log message if the color value deviates from the expected value.
 * \return true if the color values match, false otherwise
 */
int piglit_probe_texel_rgb(int target, int level, int x, int y,
			   const float *expected)
{
	return piglit_probe_texel_rect_rgb(target, level, x, y, 1, 1, expected);
}

/**
 * Read a texel rectangle from the given location and compare its RGBA value to
 * the given expected values.
 *
 * Print a log message if the color value deviates from the expected value.
 * \return true if the color values match, false otherwise
 */
int piglit_probe_texel_rect_rgba(int target, int level, int x, int y,
				 int w, int h, const float* expected)
{
	GLfloat *buffer;
	GLfloat *probe;
	int i, j, p;
	GLint width;
	GLint height;

	glGetTexLevelParameteriv(target, level, GL_TEXTURE_WIDTH, &width);
	glGetTexLevelParameteriv(target, level, GL_TEXTURE_HEIGHT, &height);
	buffer = malloc(width * height * 4 * sizeof(GLfloat));

	glGetTexImage(target, level, GL_RGBA, GL_FLOAT, buffer);

	assert(x >= 0);
	assert(y >= 0);
	assert(x+w <= width);
	assert(y+h <= height);

	for (j = y; j < y+h; ++j) {
		for (i = x; i < x+w; ++i) {
			probe = &buffer[(j * width + i) * 4];

			for (p = 0; p < 4; ++p) {
				if (fabs(probe[p] - expected[p]) >= piglit_tolerance[p]) {
					printf("Probe color at (%i,%i)\n", i, j);
					printf("  Expected: %f %f %f %f\n",
					       expected[0], expected[1], expected[2], expected[3]);
					printf("  Observed: %f %f %f %f\n",
					       probe[0], probe[1], probe[2], probe[3]);

					free(buffer);
					return 0;
				}
			}
		}
	}

	free(buffer);
	return 1;
}

/**
 * Read a texel from the given location and compare its RGBA value to the
 * given expected values.
 *
 * Print a log message if the color value deviates from the expected value.
 * \return true if the color values match, false otherwise
 */
int piglit_probe_texel_rgba(int target, int level, int x, int y,
			    const float* expected)
{
	return piglit_probe_texel_rect_rgba(target, level, x, y, 1, 1,
					    expected);
}

/**
 * Read a texel rectangle from the given location and compare its RGBA value to
 * the given expected values.
 *
 * Print a log message if the color value deviates from the expected value.
 * \return true if the color values match, false otherwise
 */
int piglit_probe_texel_volume_rgba(int target, int level, int x, int y, int z,
				 int w, int h, int d, const float* expected)
{
	GLfloat *buffer;
	GLfloat *probe;
	int i, j, k, p;
	GLint width;
	GLint height;
	GLint depth;

	glGetTexLevelParameteriv(target, level, GL_TEXTURE_WIDTH, &width);
	glGetTexLevelParameteriv(target, level, GL_TEXTURE_HEIGHT, &height);
	glGetTexLevelParameteriv(target, level, GL_TEXTURE_DEPTH, &depth);
	buffer = malloc(width * height * depth * 4 * sizeof(GLfloat));

	glGetTexImage(target, level, GL_RGBA, GL_FLOAT, buffer);

	assert(x >= 0);
	assert(y >= 0);
	assert(d >= 0);
	assert(x+w <= width);
	assert(y+h <= height);
	assert(z+d <= depth);

	for (k = z; k < z+d; ++k) {
		for (j = y; j < y+h; ++j) {
			for (i = x; i < x+w; ++i) {
				probe = &buffer[(k * width * height + j * width + i) * 4];

				for (p = 0; p < 4; ++p) {
					if (fabs(probe[p] - expected[p]) >= piglit_tolerance[p]) {
						printf("Probe color at (%i,%i,%i)\n", i, j, k);
						printf("  Expected: %f %f %f %f\n",
						       expected[0], expected[1], expected[2], expected[3]);
						printf("  Observed: %f %f %f %f\n",
						       probe[0], probe[1], probe[2], probe[3]);

						free(buffer);
						return 0;
					}
				}
			}
		}
	}

	free(buffer);
	return 1;
}

/**
 * Read a pixel from the given location and compare its depth value to the
 * given expected value.
 *
 * Print a log message if the depth value deviates from the expected value.
 * \return true if the depth value matches, false otherwise
 */
int piglit_probe_pixel_depth(int x, int y, float expected)
{
	GLfloat probe;
	GLfloat delta;

	glReadPixels(x, y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &probe);

	delta = probe - expected;
	if (fabs(delta) < 0.01)
		return 1;

	printf("Probe depth at (%i,%i)\n", x, y);
	printf("  Expected: %f\n", expected);
	printf("  Observed: %f\n", probe);

	return 0;
}

int piglit_probe_rect_depth(int x, int y, int w, int h, float expected)
{
	int i, j;
	GLfloat *probe;
	GLfloat *pixels = malloc(w*h*sizeof(float));

	glReadPixels(x, y, w, h, GL_DEPTH_COMPONENT, GL_FLOAT, pixels);

	for (j = 0; j < h; j++) {
		for (i = 0; i < w; i++) {
			probe = &pixels[j*w+i];

			if (fabs(*probe - expected) >= 0.01) {
				printf("Probe depth at (%i,%i)\n", x+i, y+j);
				printf("  Expected: %f\n", expected);
				printf("  Observed: %f\n", *probe);

				free(pixels);
				return 0;
			}
		}
	}

	free(pixels);
	return 1;
}

int piglit_probe_pixel_stencil(int x, int y, unsigned expected)
{
	GLuint probe;
	glReadPixels(x, y, 1, 1, GL_STENCIL_INDEX, GL_UNSIGNED_INT, &probe);

	if (probe == expected)
		return 1;

	printf("Probe stencil at (%i, %i)\n", x, y);
	printf("  Expected: %u\n", expected);
	printf("  Observed: %u\n", probe);

	return 0;
}

int piglit_probe_rect_stencil(int x, int y, int w, int h, unsigned expected)
{
	int i, j;
	GLuint *pixels = malloc(w*h*sizeof(GLuint));

	glReadPixels(x, y, w, h, GL_STENCIL_INDEX, GL_UNSIGNED_INT, pixels);

	for (j = 0; j < h; j++) {
		for (i = 0; i < w; i++) {
			GLuint probe = pixels[j * w + i];
			if (probe != expected) {
				printf("Probe stencil at (%i, %i)\n", x + i, y + j);
				printf("  Expected: %u\n", expected);
				printf("  Observed: %u\n", probe);
				free(pixels);
				return 0;
			}
		}
	}

	free(pixels);
	return 1;
}

bool piglit_probe_buffer(GLuint buf, GLenum target, const char *label,
		         unsigned n, unsigned num_components,
			 const float *expected)
{
	float *ptr;
	unsigned i;
	bool status = true;

	glBindBuffer(target, buf);
	ptr = glMapBuffer(target, GL_READ_ONLY);

	for (i = 0; i < n * num_components; i++) {
		if (fabs(ptr[i] - expected[i % num_components]) > 0.01) {
			printf("%s[%i]: %f, Expected: %f\n", label, i, ptr[i],
				expected[i % num_components]);
			status = false;
		}
	}

	glUnmapBuffer(target);

	return status;
}

GLint piglit_ARBfp_pass_through = 0;

int piglit_use_fragment_program(void)
{
	static const char source[] =
		"!!ARBfp1.0\n"
		"MOV	result.color, fragment.color;\n"
		"END\n"
		;

	piglit_dispatch_default_init(PIGLIT_DISPATCH_GL);
	if (!piglit_is_extension_supported("GL_ARB_fragment_program"))
		return 0;

	piglit_ARBfp_pass_through =
		piglit_compile_program(GL_FRAGMENT_PROGRAM_ARB, source);

	return (piglit_ARBfp_pass_through != 0);
}

void piglit_require_fragment_program(void)
{
	if (!piglit_use_fragment_program()) {
		printf("GL_ARB_fragment_program not supported.\n");
		piglit_report_result(PIGLIT_SKIP);
	}
}

int piglit_use_vertex_program(void)
{
	piglit_dispatch_default_init(PIGLIT_DISPATCH_GL);
	return piglit_is_extension_supported("GL_ARB_vertex_program");
}

void piglit_require_vertex_program(void)
{
	if (!piglit_use_vertex_program()) {
		printf("GL_ARB_vertex_program not supported.\n");
		piglit_report_result(PIGLIT_SKIP);
	}
}

GLuint piglit_compile_program(GLenum target, const char* text)
{
	GLuint program;
	GLint errorPos;

	glGenProgramsARB(1, &program);
	glBindProgramARB(target, program);
	glProgramStringARB(
			target,
			GL_PROGRAM_FORMAT_ASCII_ARB,
			strlen(text),
			(const GLubyte *)text);
	glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &errorPos);
	if (glGetError() != GL_NO_ERROR || errorPos != -1) {
		int l = piglit_find_line(text, errorPos);
		int a;

		fprintf(stderr, "Compiler Error (pos=%d line=%d): %s\n",
			errorPos, l,
			(char *) glGetString(GL_PROGRAM_ERROR_STRING_ARB));

		for (a=-10; a<10; a++)
		{
			if (errorPos+a < 0)
				continue;
			if (errorPos+a >= strlen(text))
				break;
			fprintf(stderr, "%c", text[errorPos+a]);
		}
		fprintf(stderr, "\nin program:\n%s", text);
		piglit_report_result(PIGLIT_FAIL);
	}
	if (!glIsProgramARB(program)) {
		fprintf(stderr, "glIsProgramARB failed\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	return program;
}

/**
 * Convenience function to draw a triangle.
 */
GLvoid
piglit_draw_triangle(float x1, float y1, float x2, float y2,
		     float x3, float y3)
{
	piglit_draw_triangle_z(0.0, x1, y1, x2, y2, x3, y3);
}

/**
 * Convenience function to draw a triangle at a given depth.
 */
GLvoid
piglit_draw_triangle_z(float z, float x1, float y1, float x2, float y2,
		     float x3, float y3)
{
	float verts[3][4];

	verts[0][0] = x1;
	verts[0][1] = y1;
	verts[0][2] = z;
	verts[0][3] = 1.0;
	verts[1][0] = x2;
	verts[1][1] = y2;
	verts[1][2] = z;
	verts[1][3] = 1.0;
	verts[2][0] = x3;
	verts[2][1] = y3;
	verts[2][2] = z;
	verts[2][3] = 1.0;

	glVertexPointer(4, GL_FLOAT, 0, verts);
	glEnableClientState(GL_VERTEX_ARRAY);

	glDrawArrays(GL_TRIANGLES, 0, 3);

	glDisableClientState(GL_VERTEX_ARRAY);
}

/**
 * Generate a checkerboard texture
 *
 * \param tex                Name of the texture to be used.  If \c tex is
 *                           zero, a new texture name will be generated.
 * \param level              Mipmap level the checkerboard should be written to
 * \param width              Width of the texture image
 * \param height             Height of the texture image
 * \param horiz_square_size  Size of each checkerboard tile along the X axis
 * \param vert_square_size   Size of each checkerboard tile along the Y axis
 * \param black              RGBA color to be used for "black" tiles
 * \param white              RGBA color to be used for "white" tiles
 *
 * A texture with alternating black and white squares in a checkerboard
 * pattern is generated.  The texture data is written to LOD \c level of
 * the texture \c tex.
 *
 * If \c tex is zero, a new texture created.  This texture will have several
 * texture parameters set to non-default values:
 *
 *  - Min and mag filter will be set to \c GL_NEAREST.
 *  - For GL:
 *    - S and T wrap modes will be set to \c GL_CLAMP_TO_BORDER.
 *    - Border color will be set to { 1.0, 0.0, 0.0, 1.0 }.
 *  - For GLES:
 *    - S and T wrap modes will be set to \c GL_CLAMP_TO_EDGE.
 *
 * \return
 * Name of the texture.  In addition, this texture will be bound to the
 * \c GL_TEXTURE_2D target of the currently active texture unit.
 */
GLuint
piglit_checkerboard_texture(GLuint tex, unsigned level,
			    unsigned width, unsigned height,
			    unsigned horiz_square_size,
			    unsigned vert_square_size,
			    const float *black, const float *white)
{
	static const GLfloat border_color[4] = { 1.0, 0.0, 0.0, 1.0 };
	unsigned i;
	unsigned j;
	void *tex_data;
	char *texel;
	unsigned pixel_size;
	GLubyte black_b[4], white_b[4];
	const void *black_data, *white_data;

	if (piglit_is_gles()) {
		pixel_size = 4 * sizeof(GLubyte);
		for (i = 0; i < 4; i++) {
			black_b[i] = black[i] * 255;
			white_b[i] = white[i] * 255;
		}
		black_data = black_b;
		white_data = white_b;
	} else {
		pixel_size = 4 * sizeof(float);
		black_data = black;
		white_data = white;
	}
	texel = tex_data = malloc(width * height * pixel_size);

	for (i = 0; i < height; i++) {
		const unsigned row = i / vert_square_size;

		for (j = 0; j < width; j++) {
			const unsigned col = j / horiz_square_size;

			if ((row ^ col) & 1) {
				memcpy(texel, white_data, pixel_size);
			} else {
				memcpy(texel, black_data, pixel_size);
			}

			texel += pixel_size;
		}
	}

	if (tex == 0) {
		glGenTextures(1, &tex);

		glBindTexture(GL_TEXTURE_2D, tex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
				GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
				GL_NEAREST);
		if (piglit_is_gles()) {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
					GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
					GL_CLAMP_TO_EDGE);
		} else {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
					GL_CLAMP_TO_BORDER);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
					GL_CLAMP_TO_BORDER);
			glTexParameterfv(GL_TEXTURE_2D,
					 GL_TEXTURE_BORDER_COLOR,
					 border_color);
		}
	} else {
		glBindTexture(GL_TEXTURE_2D, tex);
	}

	glTexImage2D(GL_TEXTURE_2D, level, GL_RGBA, width, height, 0, GL_RGBA,
		     piglit_is_gles() ? GL_UNSIGNED_BYTE : GL_FLOAT, tex_data);

	return tex;
}

/**
 * Generates a 8x8 mipmapped texture whose layers contain solid r, g, b, and w.
 */
GLuint
piglit_miptree_texture()
{
	GLfloat *data;
	int size, i, level;
	GLuint tex;
	const float color_wheel[4][4] = {
		{1, 0, 0, 1}, /* red */
		{0, 1, 0, 1}, /* green */
		{0, 0, 1, 1}, /* blue */
		{1, 1, 1, 1}, /* white */
	};

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
			GL_NEAREST_MIPMAP_NEAREST);

	for (level = 0; level < 4; ++level) {
		size = 8 >> level;

		data = malloc(size*size*4*sizeof(GLfloat));
		for (i = 0; i < size * size; ++i) {
			memcpy(data + 4 * i, color_wheel[level],
			       4 * sizeof(GLfloat));
		}
		glTexImage2D(GL_TEXTURE_2D, level, GL_RGBA,
			     size, size, 0, GL_RGBA, GL_FLOAT, data);
		free(data);
	}
	return tex;
}

/**
 * Generates an image of the given size with quadrants of red, green,
 * blue and white.
 * Note that for compressed teximages, where the blocking would be
 * problematic, we assign the whole layers at w == 4 to red, w == 2 to
 * green, and w == 1 to blue.
 *
 * \param internalFormat  either GL_RGBA or a specific compressed format
 * \param w  the width in texels
 * \param h  the height in texels
 * \param alpha  if TRUE, use varied alpha values, else all alphas = 1
 * \param basetype  either GL_UNSIGNED_NORMALIZED, GL_SIGNED_NORMALIZED
 *                  or GL_FLOAT
 */
GLfloat *
piglit_rgbw_image(GLenum internalFormat, int w, int h,
		  GLboolean alpha, GLenum basetype)
{
	float red[4]   = {1.0, 0.0, 0.0, 0.0};
	float green[4] = {0.0, 1.0, 0.0, 0.25};
	float blue[4]  = {0.0, 0.0, 1.0, 0.5};
	float white[4] = {1.0, 1.0, 1.0, 1.0};
	GLfloat *data;
	int x, y;

	if (!alpha) {
		red[3] = 1.0;
		green[3] = 1.0;
		blue[3] = 1.0;
		white[3] = 1.0;
	}

	switch (basetype) {
	case GL_UNSIGNED_NORMALIZED:
		break;

	case GL_SIGNED_NORMALIZED:
		for (x = 0; x < 4; x++) {
			red[x] = red[x] * 2 - 1;
			green[x] = green[x] * 2 - 1;
			blue[x] = blue[x] * 2 - 1;
			white[x] = white[x] * 2 - 1;
		}
		break;

	case GL_FLOAT:
		for (x = 0; x < 4; x++) {
			red[x] = red[x] * 10 - 5;
			green[x] = green[x] * 10 - 5;
			blue[x] = blue[x] * 10 - 5;
			white[x] = white[x] * 10 - 5;
		}
		break;

	default:
		assert(0);
	}

	data = malloc(w * h * 4 * sizeof(GLfloat));

	for (y = 0; y < h; y++) {
		for (x = 0; x < w; x++) {
			const int size = w > h ? w : h;
			const float *color;

			if (x < w / 2 && y < h / 2)
				color = red;
			else if (y < h / 2)
				color = green;
			else if (x < w / 2)
				color = blue;
			else
				color = white;

			switch (internalFormat) {
			case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
			case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
			case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
			case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
			case GL_COMPRESSED_RGB_FXT1_3DFX:
			case GL_COMPRESSED_RGBA_FXT1_3DFX:
			case GL_COMPRESSED_RED_RGTC1:
			case GL_COMPRESSED_SIGNED_RED_RGTC1:
			case GL_COMPRESSED_RG_RGTC2:
			case GL_COMPRESSED_SIGNED_RG_RGTC2:
			case GL_COMPRESSED_SRGB_S3TC_DXT1_EXT:
			case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT:
			case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT:
			case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT:
				if (size == 4)
					color = red;
				else if (size == 2)
					color = green;
				else if (size == 1)
					color = blue;
				break;
			default:
				break;
			}

			memcpy(data + (y * w + x) * 4, color,
			       4 * sizeof(float));
		}
	}

	return data;
}

static GLubyte *
piglit_rgbw_image_ubyte(int w, int h, GLboolean alpha)
{
	GLubyte red[4]   = {255, 0, 0, 0};
	GLubyte green[4] = {0, 255, 0, 64};
	GLubyte blue[4]  = {0, 0, 255, 128};
	GLubyte white[4] = {255, 255, 255, 255};
	GLubyte *data;
	int x, y;

	if (!alpha) {
		red[3] = 255;
		green[3] = 255;
		blue[3] = 255;
		white[3] = 255;
	}

	data = malloc(w * h * 4 * sizeof(GLubyte));

	for (y = 0; y < h; y++) {
		for (x = 0; x < w; x++) {
			const GLubyte *color;

			if (x < w / 2 && y < h / 2)
				color = red;
			else if (y < h / 2)
				color = green;
			else if (x < w / 2)
				color = blue;
			else
				color = white;

			memcpy(data + (y * w + x) * 4, color,
			       4 * sizeof(GLubyte));
		}
	}

	return data;
}

/**
 * Generates a texture with the given internalFormat, w, h with a
 * teximage of r, g, b, w quadrants.
 *
 * Note that for compressed teximages, where the blocking would be
 * problematic, we assign the whole layers at w == 4 to red, w == 2 to
 * green, and w == 1 to blue.
 */
GLuint
piglit_rgbw_texture(GLenum internalFormat, int w, int h, GLboolean mip,
		    GLboolean alpha, GLenum basetype)
{
	int size, level;
	GLuint tex;
	GLenum teximage_type;

	switch (basetype) {
	case GL_UNSIGNED_NORMALIZED:
	case GL_SIGNED_NORMALIZED:
	case GL_FLOAT:
		teximage_type = GL_FLOAT;
		break;
	case GL_UNSIGNED_BYTE:
		teximage_type = GL_UNSIGNED_BYTE;
		break;
	default:
		assert(0);
	}

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	if (mip) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
				GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
				GL_LINEAR_MIPMAP_NEAREST);
	} else {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
				GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
				GL_NEAREST);
	}

	for (level = 0, size = w > h ? w : h; size > 0; level++, size >>= 1) {
		void *data;

		if (teximage_type == GL_UNSIGNED_BYTE)
			data = piglit_rgbw_image_ubyte(w, h, alpha);
		else
			data = piglit_rgbw_image(internalFormat, w, h,
			                         alpha, basetype);

		glTexImage2D(GL_TEXTURE_2D, level,
			     internalFormat,
			     w, h, 0,
			     GL_RGBA, teximage_type, data);
		free(data);

		if (!mip)
			break;

		if (w > 1)
			w >>= 1;
		if (h > 1)
			h >>= 1;
	}

	return tex;
}

/**
 * Create a depth texture.  The depth texture will be a gradient which varies
 * from 0.0 at the left side to 1.0 at the right side.  For a 2D array texture,
 * all the texture layers will have the same gradient.
 *
 * \param target  either GL_TEXTURE_1D, GL_TEXTURE_2D, GL_TEXTURE_1D_ARRAY,
 *                GL_TEXTURE_2D_ARRAY or GL_TEXTURE_RECTANGLE.
 * \param internalformat  either GL_DEPTH_STENCIL, GL_DEPTH_COMPONENT,
 *                        GL_DEPTH24_STENCIL8_EXT or GL_DEPTH32F_STENCIL8.
 * \param w, h, d  level 0 image width, height and depth
 * \param mip  if true, create a full mipmap.  Else, create single-level texture.
 * \return the new texture object id
 */
GLuint
piglit_depth_texture(GLenum target, GLenum internalformat, int w, int h, int d, GLboolean mip)
{
	void *data;
	float *f = NULL, *f2 = NULL;
	unsigned int  *i = NULL;
	int size, x, y, level, layer;
	GLuint tex;
	GLenum type, format;

	glGenTextures(1, &tex);
	glBindTexture(target, tex);
	glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	if (mip) {
		glTexParameteri(target, GL_TEXTURE_MAG_FILTER,
				GL_LINEAR);
		glTexParameteri(target, GL_TEXTURE_MIN_FILTER,
				GL_LINEAR_MIPMAP_NEAREST);
	} else {
		glTexParameteri(target, GL_TEXTURE_MAG_FILTER,
				GL_NEAREST);
		glTexParameteri(target, GL_TEXTURE_MIN_FILTER,
				GL_NEAREST);
	}
	data = malloc(w * h * 4 * sizeof(GLfloat));

	if (internalformat == GL_DEPTH_STENCIL_EXT ||
	    internalformat == GL_DEPTH24_STENCIL8_EXT) {
		format = GL_DEPTH_STENCIL_EXT;
		type = GL_UNSIGNED_INT_24_8_EXT;
		i = data;
	} else if (internalformat == GL_DEPTH32F_STENCIL8) {
		format = GL_DEPTH_STENCIL;
		type = GL_FLOAT_32_UNSIGNED_INT_24_8_REV;
		f2 = data;
	} else {
		format = GL_DEPTH_COMPONENT;
		type = GL_FLOAT;
		f = data;
	}

	for (level = 0, size = w > h ? w : h; size > 0; level++, size >>= 1) {
		for (y = 0; y < h; y++) {
			for (x = 0; x < w; x++) {
				float val = (float)(x) / (w - 1);
				if (f)
					f[y * w + x] = val;
				else if (f2)
					f2[(y * w + x)*2] = val;
				else
					i[y * w + x] = 0xffffff00 * val;
			}
		}

		switch (target) {
		case GL_TEXTURE_1D:
			glTexImage1D(target, level,
				     internalformat,
				     w, 0,
				     format, type, data);
			break;

		case GL_TEXTURE_1D_ARRAY:
		case GL_TEXTURE_2D:
		case GL_TEXTURE_RECTANGLE:
			glTexImage2D(target, level,
				     internalformat,
				     w, h, 0,
				     format, type, data);
			break;

		case GL_TEXTURE_2D_ARRAY:
			glTexImage3D(target, level,
				     internalformat,
				     w, h, d, 0,
				     format, type, NULL);
			for (layer = 0; layer < d; layer++) {
				glTexSubImage3D(target, level,
						0, 0, layer, w, h, 1,
						format, type, data);
			}
			break;

		default:
			assert(0);
		}

		if (!mip)
			break;

		if (w > 1)
			w >>= 1;
		if (h > 1)
			h >>= 1;
	}
	free(data);
	return tex;
}

/**
 * Require transform feedback.
 *
 * Transform feedback may either be provided by GL 3.0 or
 * EXT_transform_feedback.
 */
void
piglit_require_transform_feedback(void)
{
	if (!(piglit_get_gl_version() >= 30 ||
	      piglit_is_extension_supported("GL_EXT_transform_feedback"))) {
		printf("Transform feedback not supported.\n");
		piglit_report_result(PIGLIT_SKIP);
	}
}

/**
 * Convert the image into a format that can be easily understood by
 * visual inspection, and display it on the screen.
 *
 * Luminance and intensity values are converted to a grayscale value.
 * Alpha values are visualized by blending the image with a grayscale
 * checkerboard.
 *
 * Pass image_count = 0 to disable drawing multiple images to window
 * system framebuffer.
 */
void
piglit_visualize_image(float *img, GLenum base_internal_format,
		       int image_width, int image_height,
		       int image_count, bool rhs)
{
	int x, y;
	float checker;
	unsigned components = piglit_num_components(base_internal_format);
	float *visualization =
		(float *) malloc(sizeof(float)*3*image_width*image_height);
	for (y = 0; y < image_height; ++y) {
		for ( x = 0; x < image_width; ++x) {
			float r = 0, g = 0, b = 0, a = 1;
			float *pixel =
				&img[(y * image_width + x) * components];
			switch (base_internal_format) {
			case GL_ALPHA:
				a = pixel[0];
				break;
			case GL_RGBA:
				a = pixel[3];
				/* Fall through */
			case GL_RGB:
				b = pixel[2];
				/* Fall through */
			case GL_RG:
				g = pixel[1];
				/* Fall through */
			case GL_RED:
				r = pixel[0];
				break;
			case GL_LUMINANCE_ALPHA:
				a = pixel[1];
				/* Fall through */
			case GL_INTENSITY:
			case GL_LUMINANCE:
				r = pixel[0];
				g = pixel[0];
				b = pixel[0];
				break;
			}
			checker = ((x ^ y) & 0x10) ? 0.75 : 0.25;
			r = r * a + checker * (1 - a);
			g = g * a + checker * (1 - a);
			b = b * a + checker * (1 - a);
			visualization[(y * image_width + x) * 3] = r;
			visualization[(y * image_width + x) * 3 + 1] = g;
			visualization[(y * image_width + x) * 3 + 2] = b;
		}
	}
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
	glUseProgram(0);

	/* To simultaneously display multiple images on window system
	 * framebuffer.
	 */
	if(image_count) {
		/* Use glWindowPos to directly update x, y coordinates of
		 * current raster position without getting transformed by
		 * modelview projection matrix and viewport-to-window
		 * transform.
		 */
		glWindowPos2f(rhs ? image_width : 0,
			      (image_count - 1) * image_height);
	}
	else {
		glRasterPos2f(rhs ? 0 : -1, -1);
	}
	glDrawPixels(image_width, image_height, GL_RGB, GL_FLOAT,
		     visualization);
	free(visualization);
}

/**
 * Convert from sRGB color space to linear color space, using the
 * formula from the GL 3.0 spec, section 4.1.8 (sRGB Texture Color
 * Conversion).
 */
float
piglit_srgb_to_linear(float x)
{
        if (x <= 0.0405)
                return x / 12.92;
        else
                return pow((x + 0.055) / 1.055, 2.4);
}

/* Convert from linear color space to sRGB color space. */
float
piglit_linear_to_srgb(float x)
{
   if (x < 0.0f)
      return 0.0f;
   else if (x < 0.0031308f)
      return 12.92f * x;
   else if (x < 1.0f)
      return 1.055f * powf(x, 0.41666f) - 0.055f;
   else
      return 1.0f;
}
