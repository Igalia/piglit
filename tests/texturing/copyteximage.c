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


#include "piglit-util-gl.h"

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

	{ GL_DEPTH24_STENCIL8,	fcolor[7] },
	{ GL_DEPTH32F_STENCIL8, fcolor[7] },
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
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA |
			       PIGLIT_GL_VISUAL_DEPTH | PIGLIT_GL_VISUAL_STENCIL;

PIGLIT_GL_TEST_CONFIG_END

static const char array1D_shader_text[] =
	"#extension GL_EXT_texture_array: require\n"
	"uniform sampler1DArray s;\n"
	"void main()\n"
	"{\n"
	"    gl_FragColor = texture1DArray(s, gl_TexCoord[0].xy);\n"
	"}\n"
	;

static const char array2D_shader_text[] =
	"#extension GL_EXT_texture_array: require\n"
	"uniform sampler2DArray s;\n"
	"void main()\n"
	"{\n"
	"    gl_FragColor = texture2DArray(s, gl_TexCoord[0].xyz);\n"
	"}\n"
	;


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
	case GL_DEPTH32F_STENCIL8:
	case GL_DEPTH24_STENCIL8:
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
	case GL_DEPTH_COMPONENT:
	case GL_DEPTH_COMPONENT16:
	case GL_DEPTH_COMPONENT24:
		return piglit_is_extension_supported("GL_ARB_depth_texture");
	case GL_DEPTH24_STENCIL8:
		return piglit_is_extension_supported("GL_EXT_packed_depth_stencil") &&
			piglit_is_extension_supported("GL_ARB_depth_texture");
	case GL_DEPTH_COMPONENT32F:
	case GL_DEPTH32F_STENCIL8:
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
	/* Array targets are only supported if GLSL is available.
	 */
	if ((target[i].target == GL_TEXTURE_1D_ARRAY
	     || target[i].target == GL_TEXTURE_2D_ARRAY)
	    && (piglit_get_gl_version() < 20
		&& !piglit_is_extension_supported("GL_ARB_fragment_shader")))
		return false;

	return piglit_get_gl_version() >= target[i].gl_version ||
		(target[i].extension &&
		 piglit_is_extension_supported(target[i].extension));
}

static GLenum get_format(GLenum format)
{
	if (format == GL_DEPTH32F_STENCIL8 ||
	    format == GL_DEPTH24_STENCIL8)
		return GL_DEPTH_STENCIL;
	else if (is_depth_format(format))
		return GL_DEPTH_COMPONENT;
	else
		return GL_RGBA;
}

static GLenum get_type(GLenum format)
{
	switch (format) {
	case GL_DEPTH24_STENCIL8:
		return GL_UNSIGNED_INT_24_8;
	case GL_DEPTH32F_STENCIL8:
		return GL_FLOAT_32_UNSIGNED_INT_24_8_REV;
	default:
		return GL_FLOAT;
	}
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
 * Convenience function to draw an axis-aligned rectangle with 3 dimensional
 * texture coordinates where the third coordinate is constant.
 */
static GLvoid
draw_rect_tex_3d(float x, float y, float w, float h,
                 float tx, float ty, float tz, float tw, float th)
{
	float verts[4][4];
	float tex[4][3];

	verts[0][0] = x;
	verts[0][1] = y;
	verts[0][2] = 0.0;
	verts[0][3] = 1.0;
	tex[0][0] = tx;
	tex[0][1] = ty;
	tex[0][2] = tz;
	verts[1][0] = x + w;
	verts[1][1] = y;
	verts[1][2] = 0.0;
	verts[1][3] = 1.0;
	tex[1][0] = tx + tw;
	tex[1][1] = ty;
	tex[1][2] = tz;
	verts[2][0] = x;
	verts[2][1] = y + h;
	verts[2][2] = 0.0;
	verts[2][3] = 1.0;
	tex[2][0] = tx;
	tex[2][1] = ty + th;
	tex[2][2] = tz;
	verts[3][0] = x + w;
	verts[3][1] = y + h;
	verts[3][2] = 0.0;
	verts[3][3] = 1.0;
	tex[3][0] = tx + tw;
	tex[3][1] = ty + th;
	tex[3][2] = tz;

	glVertexPointer(4, GL_FLOAT, 0, verts);
	glTexCoordPointer(3, GL_FLOAT, 0, tex);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}


/**
 * Convenience function to draw an axis-aligned rectangle textured with one face
 * of a cube map.
 */
static GLvoid
draw_rect_tex_cube_face(float x, float y, float w, float h, int face)
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
	verts[2][0] = x + w;
	verts[2][1] = y + h;
	verts[2][2] = 0.0;
	verts[2][3] = 1.0;
	verts[3][0] = x;
	verts[3][1] = y + h;
	verts[3][2] = 0.0;
	verts[3][3] = 1.0;

	glVertexPointer(4, GL_FLOAT, 0, verts);
	glTexCoordPointer(3, GL_FLOAT, 0, cube_face_texcoords[face]);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glDrawArrays(GL_QUADS, 0, 4);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
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
	GLuint prog = 0;

	printf("Texture target = %s, Internal format = %s",
	       piglit_get_gl_enum_name(target),
	       piglit_get_gl_enum_name(format));

	if (!supported_format(format) ||
	    !supported_target_format(target, format)) {
		printf(" - skipped\n");
		return GL_TRUE; /* not a failure */
	} else {
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
		piglit_draw_rect_tex(x, y, IMAGE_SIZE, IMAGE_SIZE,
				      0, 0, 1, 1);
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
		piglit_draw_rect_tex(x, y, IMAGE_SIZE, IMAGE_SIZE,
				      0, 0, 1, 1);
		pass = piglit_probe_rect_rgba(x, y, IMAGE_SIZE,
					      IMAGE_SIZE,
					      expected)
			&& pass;
		break;

	case GL_TEXTURE_3D:
		glTexImage3D(GL_TEXTURE_3D, 0, format, IMAGE_SIZE, IMAGE_SIZE, 4,
			     0, GL_RGBA, get_type(format), NULL);

		for (k = 0; k < 4; k++) {
			draw(format, 1.0 - k*0.2);
			glCopyTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, k,
					    0, 0, IMAGE_SIZE, IMAGE_SIZE);
		}

		pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

		glEnable(target);

		for (k = 0; k < 4; k++) {
			const float tz = (k + 0.5) * 0.25;
			draw_rect_tex_3d(x, y, IMAGE_SIZE, IMAGE_SIZE,
					 0, 0, tz, 1, 1);
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
			draw_rect_tex_cube_face(x, y, IMAGE_SIZE, IMAGE_SIZE, k);
			pass = probe_rect(x, y, IMAGE_SIZE, IMAGE_SIZE,
					  expected, 1.0 - k*0.15) && pass;
		}
		break;

	case GL_TEXTURE_1D_ARRAY:
		glTexImage2D(GL_TEXTURE_1D_ARRAY, 0, format, IMAGE_SIZE, 16,
			     0, get_format(format), get_type(format), NULL);

		for (k = 0; k < 4; k++) {
			draw(format, 1.0 - 0.2*k);
			glCopyTexSubImage2D(GL_TEXTURE_1D_ARRAY, 0, 0, 4*k,
					    0, 0, IMAGE_SIZE, 4);
		}

		pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

		prog = piglit_build_simple_program(NULL, array1D_shader_text);
		glUseProgram(prog);
		glDeleteProgram(prog);

		for (k = 0; k < 16; k++) {
			piglit_draw_rect_tex(x, y, IMAGE_SIZE, IMAGE_SIZE,
					      0, k, 1, 0);
			pass = probe_rect(x, y, IMAGE_SIZE, IMAGE_SIZE,
					  expected, 1.0 - 0.2*(k/4)) && pass;
		}
		break;

	case GL_TEXTURE_2D_ARRAY:
		glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, format, IMAGE_SIZE, IMAGE_SIZE, 4,
			     0, get_format(format), get_type(format), NULL);

		for (k = 0; k < 4; k++) {
			draw(format, 1.0 - k*0.2);
			glCopyTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, k,
					    0, 0, IMAGE_SIZE, IMAGE_SIZE);
		}

		pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

		prog = piglit_build_simple_program(NULL, array2D_shader_text);
		glUseProgram(prog);
		glDeleteProgram(prog);

		for (k = 0; k < 4; k++) {
			draw_rect_tex_3d(x, y, IMAGE_SIZE, IMAGE_SIZE,
					 0, 0, k, 1, 1);
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
		piglit_draw_rect_tex(x, y, IMAGE_SIZE, IMAGE_SIZE,
				      0, 0, IMAGE_SIZE - 1, IMAGE_SIZE - 1);
		pass = piglit_probe_rect_rgba(x, y, IMAGE_SIZE,
					      IMAGE_SIZE,
					      expected)
			&& pass;
		break;
	}

	/* If a GLSL program is in use, then the preceeding code should not
	 * have called glEnable(target).  In that case, this code should not
	 * disable it.  For some targets, like GL_TEXTURE_1D_ARRAY,
	 * glDisable(target) will generate an error.
	 */
	if (prog == 0)
		glDisable(target);
	else
		glUseProgram(0);

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

	glEnableClientState(GL_VERTEX_ARRAY);
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
