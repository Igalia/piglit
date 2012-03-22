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


#include "piglit-util.h"

#define IMAGE_SIZE 16

int piglit_window_mode = GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_ALPHA;

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

static const GLenum target[] = {
	GL_TEXTURE_1D,
	GL_TEXTURE_2D,
	GL_TEXTURE_CUBE_MAP };

int piglit_width = IMAGE_SIZE * (ARRAY_SIZE(test_vectors) + 1);
int piglit_height = IMAGE_SIZE;

static const float texCoords_1d[2] = { 0.0, 1.0 };
static const float texCoords_2d[4][2] = {
	{ 0.0, 0.0 },
	{ 1.0, 0.0 },
	{ 1.0, 1.0 },
	{ 0.0, 1.0 } };

GLboolean
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

enum piglit_result
piglit_display(void)
{
	GLfloat buf_fcolor[IMAGE_SIZE][IMAGE_SIZE][4];
	GLuint tex;
	GLboolean pass = GL_TRUE;
	GLenum format;
	const GLfloat *expected;
	int i, j;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	/* Image data setup */
	for (i = 0; i < IMAGE_SIZE; i++) {
		for (j = 0; j < IMAGE_SIZE; j++) {
			buf_fcolor[i][j][0] = 0.5;
			buf_fcolor[i][j][1] = 0.2;
			buf_fcolor[i][j][2] = 0.8;
			buf_fcolor[i][j][3] = 0.4;
		}
	}

	/* Do glCopyPixels and draw a textured rectangle for each format
	 * and each texture target
	 */
	for (j = 0; j < ARRAY_SIZE(target); j++) {

		/* Draw a pixel rectangle with float color data. As per OpenGL 3.0
		 * specification integer formats are not allowed in glDrawPixels
		 */
		glDrawPixels(IMAGE_SIZE, IMAGE_SIZE, GL_RGBA,
			     GL_FLOAT, buf_fcolor);

		/* Texture setup */
		glGenTextures(1, &tex);
		glBindTexture(target[j], tex);
		glTexParameteri(target[j],
				GL_TEXTURE_MIN_FILTER,
				GL_NEAREST);
		glTexParameteri(target[j],
				GL_TEXTURE_MAG_FILTER,
				GL_NEAREST);
		glTexParameteri(target[j],
				GL_GENERATE_MIPMAP,
				GL_FALSE);

		glEnableClientState(GL_TEXTURE_COORD_ARRAY);

		for (i = 0; i < ARRAY_SIZE(test_vectors); i++) {
			GLint x = IMAGE_SIZE * (i + 1);
			GLint y = 0;
			GLfloat vertices_1d[2][2] = { {x, y},
						      {x + IMAGE_SIZE, y} };

			format = test_vectors[i].format;
			expected = (const float*)test_vectors[i].expected;

			if(!piglit_automatic)
				printf("Texture target = %s, Internal"
				       " format = %s\n",
				       piglit_get_gl_enum_name(target[j]),
				       piglit_get_gl_enum_name(format));

			if (((format == GL_RGBA16F ||
			      format == GL_RGBA32F) &&
			     !piglit_is_extension_supported(
			     "GL_ARB_texture_float")) ||

			    ((format == GL_RG) &&
			     !piglit_is_extension_supported(
			     "GL_ARB_texture_rg"))) {
				if (!piglit_automatic)
					printf("Internal format = %s skipped\n",
					       piglit_get_gl_enum_name(format));
				   continue;
			}

			/* To avoid failures not related to this test case,
			 * loosen up the tolerence for compressed texture
			 * formats
			 */
			if (is_compressed_format(format))
				piglit_set_tolerance_for_bits(7, 7, 7, 7);
			else
				piglit_set_tolerance_for_bits(8, 8, 8, 8);

			switch(target[j]) {

			case GL_TEXTURE_1D:
				glCopyTexImage1D(GL_TEXTURE_1D, 0,
						 format,
						 0, 0, IMAGE_SIZE, 0);
				pass = piglit_check_gl_error(GL_NO_ERROR)
				       && pass;

				glEnable(target[j]);
				glEnableClientState(GL_VERTEX_ARRAY);
				glTexCoordPointer(1, GL_FLOAT, 0, texCoords_1d);
				glVertexPointer(2, GL_FLOAT, 0, vertices_1d);

				glDrawArrays(GL_LINES, 0, 2);
				pass = piglit_probe_pixel_rgba(x, 0, expected)
				       && pass;
				pass = piglit_probe_pixel_rgba(x + IMAGE_SIZE - 1,
							       0, expected)
				       && pass;
				break;

			case GL_TEXTURE_2D:
				glCopyTexImage2D(GL_TEXTURE_2D, 0, format, 0, 0,
						 IMAGE_SIZE, IMAGE_SIZE, 0);
				pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

				glEnable(target[j]);
				glTexCoordPointer(2, GL_FLOAT, 0, texCoords_2d);

				piglit_draw_rect(x, y, IMAGE_SIZE, IMAGE_SIZE);
				pass = piglit_probe_rect_rgba(x, y, IMAGE_SIZE,
							      IMAGE_SIZE,
							      expected)
				       && pass;
				break;

			case GL_TEXTURE_CUBE_MAP:
				glCopyTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X,
						 0, format, 0, 0,
						 IMAGE_SIZE, IMAGE_SIZE, 0);
				glCopyTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
						 0, format, 0, 0,
						 IMAGE_SIZE, IMAGE_SIZE, 0);
				glCopyTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
						 0, format, 0, 0,
						 IMAGE_SIZE, IMAGE_SIZE, 0);
				glCopyTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
						 0, format, 0, 0,
						 IMAGE_SIZE, IMAGE_SIZE, 0);
				glCopyTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
						 0, format, 0, 0,
						 IMAGE_SIZE, IMAGE_SIZE, 0);
				glCopyTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
						 0, format, 0, 0,
						 IMAGE_SIZE, IMAGE_SIZE, 0);
				pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

				glEnable(target[j]);

				/* Draw a rect with +X cubemap face as texture */
				glTexCoordPointer(3, GL_FLOAT, 0,
						  cube_face_texcoords[0]);
				piglit_draw_rect(x, y, IMAGE_SIZE, IMAGE_SIZE);
				pass = piglit_probe_rect_rgba(x, y,
				       IMAGE_SIZE, IMAGE_SIZE, expected)
				       && pass;

				/* Draw a rect with +Z cubemap face as texture */
				glTexCoordPointer(3, GL_FLOAT, 0,
						  cube_face_texcoords[2]);
				piglit_draw_rect(x, y, IMAGE_SIZE, IMAGE_SIZE);
				pass = piglit_probe_rect_rgba(x, y,
				       IMAGE_SIZE, IMAGE_SIZE, expected)
				       && pass;
				break;
			}
			glDisable(target[j]);
		}
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDeleteTextures(1, &tex);
	}
	if (!piglit_automatic)
		piglit_present_results();
	return (pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

void
piglit_init(int argc, char **argv)
{
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClearDepth(0.75);
	piglit_ortho_projection(piglit_width, piglit_height, GL_TRUE);
}
