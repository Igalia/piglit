/*
 * Copyright 2011 VMware, Inc.
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

/*
 * file copyteximage.c
 *
 * Test to verify functionality of  glCopyTexImage() with various texture
 * targets and texture internal formats.
 */


#include "piglit-util-gl-common.h"

#define IMAGE_SIZE 16
static const GLfloat fcolor[8][4] = {
	/* GL_RED */
	{0.5, 0.0, 0.0, 1.0},
	/* GL_RG */
	{0.5, 0.2, 0.0, 1.0},
	/* GL_RGB */
	{0.5, 0.2, 0.8, 1.0},
	/* GL_RGBA8, GL_RGBA16, GL_RGBA16F, GL_RGBA32F */
	{0.5, 0.2, 0.8, 0.4},
	/* GL_LUMINANCE */
	{0.5, 0.5, 0.5, 1},
	/* GL_LUMINANCE_ALPHA */
	{0.5, 0.5, 0.5, 0.4},
	/* GL_INTENSITY */
	{0.5, 0.5, 0.5, 0.5},
	/* GL_DEPTH_COMPONENT */
	{0.75, 0.75, 0.75, 1.0} };

static const struct {
	GLenum format;
	const GLvoid *expected;
} test_vectors[] = {
	{ GL_RED,		fcolor[0] },
	{ GL_RG,		fcolor[1] },

	/* Clamps the color values to [0, 1] */
	{ GL_RGB8,		fcolor[2] },
	{ GL_RGB16,		fcolor[2] },

	/* Don't clamp color values to [0, 1] */
	{ GL_RGB16F,		fcolor[2] },
	{ GL_RGB32F,		fcolor[2] },

	/* Clamps the color values to [0, 1] */
	{ GL_RGBA8,	  	fcolor[3] },
	{ GL_RGBA16,	  	fcolor[3] },

	/* Don't clamp color values to [0, 1] */
	{ GL_RGBA16F,	  	fcolor[3] },
	{ GL_RGBA32F,	  	fcolor[3] },

	{ GL_COMPRESSED_RED,	fcolor[0] },
	{ GL_COMPRESSED_RG,	fcolor[1] },
	{ GL_COMPRESSED_RGB,	fcolor[2] },
	{ GL_COMPRESSED_RGBA,	fcolor[3] },

	{ GL_LUMINANCE,	  	fcolor[4] },
	{ GL_LUMINANCE_ALPHA,	fcolor[5] },

	{ GL_INTENSITY,	  	fcolor[6] },

	{ GL_DEPTH_COMPONENT,  	fcolor[7] },
	{ GL_DEPTH_COMPONENT16, fcolor[7] },
	{ GL_DEPTH_COMPONENT24, fcolor[7] },
	{ GL_DEPTH_COMPONENT32F, fcolor[7] },
};

static const struct {
	GLenum target;
	const char *name;
	unsigned gl_version;
	const char *extension;
} target[] = {
	{GL_TEXTURE_1D,		"1D",		11, NULL},
	{GL_TEXTURE_2D,		"2D",		11, NULL},
	{GL_TEXTURE_3D,		"3D",		12, NULL},
	{GL_TEXTURE_CUBE_MAP,	"CUBE",		13, "GL_ARB_texture_cube_map"},
	{GL_TEXTURE_1D_ARRAY,	"1D_ARRAY",	30, "GL_EXT_texture_array"},
	{GL_TEXTURE_2D_ARRAY,	"2D_ARRAY",	30, "GL_EXT_texture_array"},
	{GL_TEXTURE_RECTANGLE,	"RECT",		31, "GL_ARB_texture_rectangle"}
};

static int test_target = -1;

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = IMAGE_SIZE*(ARRAY_SIZE(test_vectors)+1);
	config.window_height = IMAGE_SIZE;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DEPTH | PIGLIT_GL_VISUAL_ALPHA;

PIGLIT_GL_TEST_CONFIG_END

static const float texCoords_1d[2] = { 0.0, 1.0 };
static const float texCoords_2d[4][2] = {
	{ 0.0, 0.0 },
	{ 1.0, 0.0 },
	{ 1.0, 1.0 },
	{ 0.0, 1.0 } };

static const float texCoords_3d[4][4][3] = {
	{
		{ 0.0, 0.0, 0.0 },
		{ 1.0, 0.0, 0.0 },
		{ 1.0, 1.0, 0.0 },
		{ 0.0, 1.0, 0.0 }
	},
	{
		{ 0.0, 0.0, 0.25 },
		{ 1.0, 0.0, 0.25 },
		{ 1.0, 1.0, 0.25 },
		{ 0.0, 1.0, 0.25 }
	},
	{
		{ 0.0, 0.0, 0.5 },
		{ 1.0, 0.0, 0.5 },
		{ 1.0, 1.0, 0.5 },
		{ 0.0, 1.0, 0.5 }
	},
	{
		{ 0.0, 0.0, 0.75 },
		{ 1.0, 0.0, 0.75 },
		{ 1.0, 1.0, 0.75 },
		{ 0.0, 1.0, 0.75 }
	}
};

static const float texCoords_1d_array[16][4][2] = {
	{
		{ 0.0, 0 },
		{ 1.0, 0 },
		{ 1.0, 0 },
		{ 0.0, 0 }
	},
	{
		{ 0.0, 1 },
		{ 1.0, 1 },
		{ 1.0, 1 },
		{ 0.0, 1 }
	},
	{
		{ 0.0, 2 },
		{ 1.0, 2 },
		{ 1.0, 2 },
		{ 0.0, 2 }
	},
	{
		{ 0.0, 3 },
		{ 1.0, 3 },
		{ 1.0, 3 },
		{ 0.0, 3 }
	},
	{
		{ 0.0, 4 },
		{ 1.0, 4 },
		{ 1.0, 4 },
		{ 0.0, 4 }
	},
	{
		{ 0.0, 5 },
		{ 1.0, 5 },
		{ 1.0, 5 },
		{ 0.0, 5 }
	},
	{
		{ 0.0, 6 },
		{ 1.0, 6 },
		{ 1.0, 6 },
		{ 0.0, 6 }
	},
	{
		{ 0.0, 7 },
		{ 1.0, 7 },
		{ 1.0, 7 },
		{ 0.0, 7 }
	},
	{
		{ 0.0, 8 },
		{ 1.0, 8 },
		{ 1.0, 8 },
		{ 0.0, 8 }
	},
	{
		{ 0.0, 9 },
		{ 1.0, 9 },
		{ 1.0, 9 },
		{ 0.0, 9 }
	},
	{
		{ 0.0, 10 },
		{ 1.0, 10 },
		{ 1.0, 10 },
		{ 0.0, 10 }
	},
	{
		{ 0.0, 11 },
		{ 1.0, 11 },
		{ 1.0, 11 },
		{ 0.0, 11 }
	},
	{
		{ 0.0, 12 },
		{ 1.0, 12 },
		{ 1.0, 12 },
		{ 0.0, 12 }
	},
	{
		{ 0.0, 13 },
		{ 1.0, 13 },
		{ 1.0, 13 },
		{ 0.0, 13 }
	},
	{
		{ 0.0, 14 },
		{ 1.0, 14 },
		{ 1.0, 14 },
		{ 0.0, 14 }
	},
	{
		{ 0.0, 15 },
		{ 1.0, 15 },
		{ 1.0, 15 },
		{ 0.0, 15 }
	}
};

static const float texCoords_2d_array[4][4][3] = {
	{
		{ 0.0, 0.0, 0 },
		{ 1.0, 0.0, 0 },
		{ 1.0, 1.0, 0 },
		{ 0.0, 1.0, 0 }
	},
	{
		{ 0.0, 0.0, 1 },
		{ 1.0, 0.0, 1 },
		{ 1.0, 1.0, 1 },
		{ 0.0, 1.0, 1 }
	},
	{
		{ 0.0, 0.0, 2 },
		{ 1.0, 0.0, 2 },
		{ 1.0, 1.0, 2 },
		{ 0.0, 1.0, 2 }
	},
	{
		{ 0.0, 0.0, 3 },
		{ 1.0, 0.0, 3 },
		{ 1.0, 1.0, 3 },
		{ 0.0, 1.0, 3 }
	}
};

static const float texCoords_rect[4][2] = {
	{ 0.0, 0.0 },
	{ IMAGE_SIZE-1, 0.0 },
	{ IMAGE_SIZE-1, IMAGE_SIZE-1 },
	{ 0.0, IMAGE_SIZE-1 } };

static GLboolean
is_compressed_format(GLenum format)
{
   switch (format) {
   case GL_COMPRESSED_RED:
   case GL_COMPRESSED_RG:
   case GL_COMPRESSED_RGB:
   case GL_COMPRESSED_RGBA:
      return GL_TRUE;
   default:
      return GL_FALSE;
   }
}

static bool
is_depth_format(GLenum format)
{
	switch (format) {
	case GL_DEPTH_COMPONENT:
	case GL_DEPTH_COMPONENT16:
	case GL_DEPTH_COMPONENT24:
	case GL_DEPTH_COMPONENT32F:
		return true;
	default:
		return false;
	}
}

/** is the given texture internal format supported? */
static bool
supported_format(GLenum format)
{
	switch (format) {
	case GL_RGBA16F:
	case GL_RGBA32F:
	case GL_RGB16F:
	case GL_RGB32F:
		return piglit_is_extension_supported("GL_ARB_texture_float");
	case GL_RED:
	case GL_RG:
	case GL_COMPRESSED_RED:
	case GL_COMPRESSED_RG:
		return piglit_is_extension_supported("GL_ARB_texture_rg");
	case GL_DEPTH_COMPONENT32F:
		return piglit_is_extension_supported("GL_ARB_depth_buffer_float");
	default:
		return true;
	}
}

/** is the texture format allowed for the texture target? */
static bool
supported_target_format(GLenum target, GLenum format)
{
	if (is_depth_format(format) && target == GL_TEXTURE_3D) {
		return false;
	}
	return true;
}

static bool
supported_target(unsigned i)
{
	return piglit_get_gl_version() >= target[i].gl_version ||
		(target[i].extension &&
		 piglit_is_extension_supported(target[i].extension));
}

static GLenum get_format(GLenum format)
{
	if (is_depth_format(format))
		return GL_DEPTH_COMPONENT;
	else
		return GL_RGBA;
}

static void draw_pixels(float scale)
{
	GLfloat buf_fcolor[IMAGE_SIZE][IMAGE_SIZE][4];
	int i, j;

	/* Image data setup */
	for (i = 0; i < IMAGE_SIZE; i++) {
		for (j = 0; j < IMAGE_SIZE; j++) {
			buf_fcolor[i][j][0] = 0.5 * scale;
			buf_fcolor[i][j][1] = 0.2 * scale;
			buf_fcolor[i][j][2] = 0.8 * scale;
			buf_fcolor[i][j][3] = 0.4 * scale;
		}
	}

	glDrawPixels(IMAGE_SIZE, IMAGE_SIZE, GL_RGBA,
		     GL_FLOAT, buf_fcolor);
}

static void draw_depth(float scale)
{
	glClearDepth(0.75 * scale);
	glClear(GL_DEPTH_BUFFER_BIT);
}

static void draw(GLenum format, float scale)
{
	if (is_depth_format(format))
		draw_depth(scale);
	else
		draw_pixels(scale);
}

static GLboolean probe_rect(int x, int y, int w, int h,
			    const float *expected, float scale)
{
	float expected_scaled[4];
	unsigned i;

	for (i = 0; i < 4; i++) {
		if (expected[i] == 1.0 || expected[i] == 0.0)
			expected_scaled[i] = expected[i];
		else
			expected_scaled[i] = expected[i] * scale;
	}
	return piglit_probe_rect_rgba(x, y, w, h, expected_scaled);
}


/**
 * Test a specific texture target and format combination.
 */
static GLboolean
test_target_and_format(GLint x, GLint y, GLenum target, GLenum format,
		       const GLfloat *expected)
{
	GLboolean pass = GL_TRUE;
	GLuint k;

	if (!piglit_automatic)
		printf("Texture target = %s, Internal format = %s",
		       piglit_get_gl_enum_name(target),
		       piglit_get_gl_enum_name(format));

	if (!supported_format(format) ||
	    !supported_target_format(target, format)) {
		if (!piglit_automatic)
			printf(" - skipped\n");
		return GL_TRUE; /* not a failure */
	} else {
		if (!piglit_automatic)
			printf("\n");
	}

	/* To avoid failures not related to this test case,
	 * loosen up the tolerence for compressed texture
	 * formats
	 */
	if (is_compressed_format(format))
		piglit_set_tolerance_for_bits(5, 5, 5, 5);
	else
		piglit_set_tolerance_for_bits(8, 8, 8, 8);

	switch(target) {

	case GL_TEXTURE_1D:
		draw(format, 1.0);
		glCopyTexImage1D(GL_TEXTURE_1D, 0,
				 format,
				 0, 0, IMAGE_SIZE, 0);
		pass = piglit_check_gl_error(GL_NO_ERROR)
			&& pass;

		glEnable(target);
		glTexCoordPointer(2, GL_FLOAT, 0, texCoords_2d);
		piglit_draw_rect(x, y, IMAGE_SIZE, IMAGE_SIZE);
		pass = piglit_probe_rect_rgba(x, y, IMAGE_SIZE,
					      IMAGE_SIZE,
					      expected)
			&& pass;
		break;

	case GL_TEXTURE_2D:
		draw(format, 1.0);
		glCopyTexImage2D(GL_TEXTURE_2D, 0, format, 0, 0,
				 IMAGE_SIZE, IMAGE_SIZE, 0);
		pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

		glEnable(target);
		glTexCoordPointer(2, GL_FLOAT, 0, texCoords_2d);

		piglit_draw_rect(x, y, IMAGE_SIZE, IMAGE_SIZE);
		pass = piglit_probe_rect_rgba(x, y, IMAGE_SIZE,
					      IMAGE_SIZE,
					      expected)
			&& pass;
		break;

	case GL_TEXTURE_3D:
		glTexImage3D(GL_TEXTURE_3D, 0, format, IMAGE_SIZE, IMAGE_SIZE, 4,
			     0, GL_RGBA, GL_FLOAT, NULL);

		for (k = 0; k < 4; k++) {
			draw(format, 1.0 - k*0.2);
			glCopyTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, k,
					    0, 0, IMAGE_SIZE, IMAGE_SIZE);
		}

		pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

		glEnable(target);

		for (k = 0; k < 4; k++) {
			glTexCoordPointer(3, GL_FLOAT, 0, texCoords_3d[k]);
			piglit_draw_rect(x, y, IMAGE_SIZE, IMAGE_SIZE);
			pass = probe_rect(x, y, IMAGE_SIZE, IMAGE_SIZE,
					  expected, 1.0 - k*0.2) && pass;
		}
		break;

	case GL_TEXTURE_CUBE_MAP:
		for (k = 0; k < 6; k++) {
			draw(format, 1.0 - k*0.15);
			glCopyTexImage2D(cube_face_targets[k],
					 0, format, 0, 0,
					 IMAGE_SIZE, IMAGE_SIZE, 0);
		}

		pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

		glEnable(target);

		for (k = 0; k < 6; k++) {
			glTexCoordPointer(3, GL_FLOAT, 0,
					  cube_face_texcoords[k]);
			piglit_draw_rect(x, y, IMAGE_SIZE, IMAGE_SIZE);
			pass = probe_rect(x, y, IMAGE_SIZE, IMAGE_SIZE,
					  expected, 1.0 - k*0.15) && pass;
		}
		break;

	case GL_TEXTURE_1D_ARRAY:
		glTexImage2D(GL_TEXTURE_1D_ARRAY, 0, format, IMAGE_SIZE, 16,
			     0, get_format(format), GL_FLOAT, NULL);

		for (k = 0; k < 4; k++) {
			draw(format, 1.0 - 0.2*k);
			glCopyTexSubImage2D(GL_TEXTURE_1D_ARRAY, 0, 0, 4*k,
					    0, 0, IMAGE_SIZE, 4);
		}

		pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

		glEnable(target);

		for (k = 0; k < 16; k++) {
			glTexCoordPointer(2, GL_FLOAT, 0, texCoords_1d_array[k]);
			piglit_draw_rect(x, y, IMAGE_SIZE, IMAGE_SIZE);
			pass = probe_rect(x, y, IMAGE_SIZE, IMAGE_SIZE,
					  expected, 1.0 - 0.2*(k/4)) && pass;
		}
		break;

	case GL_TEXTURE_2D_ARRAY:
		glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, format, IMAGE_SIZE, IMAGE_SIZE, 4,
			     0, get_format(format), GL_FLOAT, NULL);

		for (k = 0; k < 4; k++) {
			draw(format, 1.0 - k*0.2);
			glCopyTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, k,
					    0, 0, IMAGE_SIZE, IMAGE_SIZE);
		}

		pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

		glEnable(target);

		for (k = 0; k < 4; k++) {
			glTexCoordPointer(3, GL_FLOAT, 0, texCoords_2d_array[k]);
			piglit_draw_rect(x, y, IMAGE_SIZE, IMAGE_SIZE);
			pass = probe_rect(x, y, IMAGE_SIZE, IMAGE_SIZE,
					  expected, 1.0 - k*0.2) && pass;
		}
		break;

	case GL_TEXTURE_RECTANGLE:
		draw(format, 1.0);
		glCopyTexImage2D(GL_TEXTURE_RECTANGLE, 0, format, 0, 0,
				 IMAGE_SIZE, IMAGE_SIZE, 0);
		pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
		
		glEnable(target);
		glTexCoordPointer(2, GL_FLOAT, 0, texCoords_rect);

		piglit_draw_rect(x, y, IMAGE_SIZE, IMAGE_SIZE);
		pass = piglit_probe_rect_rgba(x, y, IMAGE_SIZE,
					      IMAGE_SIZE,
					      expected)
			&& pass;
		break;
	}

	glDisable(target);

	return pass;
}


static GLuint
create_texture(GLenum target)
{
	GLuint tex;

	glGenTextures(1, &tex);
	glBindTexture(target, tex);
	glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(target,	GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(target, GL_GENERATE_MIPMAP, GL_FALSE);

	return tex;
}


enum piglit_result
piglit_display(void)
{
	GLuint tex;
	GLboolean pass = GL_TRUE;
	const GLfloat *expected;
	int i, j;

	glClear(GL_COLOR_BUFFER_BIT);

	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	/* Do glCopyPixels and draw a textured rectangle for each format
	 * and each texture target
	 */
	for (j = 0; j < ARRAY_SIZE(target); j++) {
		if (test_target != -1 && test_target != j)
			continue;
		if (!supported_target(j))
			continue;

		printf("Testing %s\n", piglit_get_gl_enum_name(target[j].target));

		if (target[j].target == GL_TEXTURE_1D_ARRAY) {
			printf("NOTE: We use glCopyTexSubImage2D to set 4 texture layers at once.\n");
		}

		tex = create_texture(target[j].target);

		for (i = 0; i < ARRAY_SIZE(test_vectors); i++) {
			GLint x = IMAGE_SIZE * (i + 1);
			GLint y = 0;
			expected = (const float*)test_vectors[i].expected;

			if (!test_target_and_format(x, y, target[j].target,
						    test_vectors[i].format,
						    expected)) {
				pass = GL_FALSE;
			}
		}

		glDeleteTextures(1, &tex);
	}
	if (!piglit_automatic)
		piglit_present_results();
	return (pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

void
piglit_init(int argc, char **argv)
{
	if (argc == 2) {
		unsigned i;
		for (i = 0; i < ARRAY_SIZE(target); i++) {
			if (strcmp(target[i].name, argv[1]) == 0) {
				test_target = i;

				if (!supported_target(i)) {
					printf("Test requires OpenGL %1.1f", target[i].gl_version * 0.1);
					if (target[i].extension)
						printf(" or %s", target[i].extension);
					printf(".\n");
					piglit_report_result(PIGLIT_SKIP);
				}
				break;
			}
		}
	}

	glClearColor(0.0, 0.0, 0.0, 1.0);
	piglit_ortho_projection(piglit_width, piglit_height, GL_TRUE);
}
