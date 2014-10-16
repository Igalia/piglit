/*
 * Copyright © 2012 Intel Corporation
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
 *
 * Author: Neil Roberts <neil@linux.intel.com>
 */

/** @file ext_unpack_subimage.c
 *
 * Test GL_EXT_unpack_subimage.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_es_version = 20;

	config.window_width = 100;
	config.window_height = 100;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static GLboolean extension_supported;
static GLboolean pass;

static const GLubyte
tex_data[] = {
	/* Row 0. This is skipped if the extension is available,
	   otherwise it makes the top and bottom texel */
	0xff, 0x00, 0x00, 0xff, /* red */
	0x00, 0xff, 0x00, 0xff, /* green */
	/* Row 1. skipped */
	0x00, 0x00, 0x00, 0xff,
	0x00, 0x00, 0x00, 0xff,
	/* Row 2. skipped */
	0x00, 0x00, 0x00, 0xff,
	0x00, 0x00, 0x00, 0xff,
	/* Row 3. skipped */
	0x00, 0x00, 0x00, 0xff,
	0x00, 0x00, 0x00, 0xff,
	/* Row 4. This is the first row used if the extension is
	   available */
	/* The first pixel is skipped */
	0x00, 0x00, 0x00, 0xff,
	/* This texel becomes the top texel */
	0x00, 0x00, 0xff, 0xff, /* blue */
	/* Row 5. first texel not used */
	0x00, 0x00, 0x00, 0x00,
	/* second texel becomes the bottom texel */
	0x00, 0xff, 0xff, 0xff /* cyan */
};

static const char
vertex_shader[] =
	"attribute vec4 piglit_vertex;\n"
	"attribute vec4 piglit_texcoord;\n"
	"varying vec2 tex_coord;\n"
	"void main () {\n"
	"gl_Position = piglit_vertex;\n"
	"tex_coord = piglit_texcoord.xy;\n"
	"}\n";

static const char
fragment_shader[] =
	"uniform sampler2D tex; /* defaults to 0 */\n"
	"precision highp float;\n"
	"varying vec2 tex_coord;\n"
	"void main () {\n"
	"gl_FragColor = texture2D(tex, tex_coord);\n"
	"}\n";

enum piglit_result
piglit_display(void)
{
	GLuint tex;
	static const float red[] = { 1, 0, 0, 1 };
	static const float green[] = { 0, 1, 0, 1 };
	static const float blue[] = { 0, 0, 1, 1 };
	static const float cyan[] = { 0, 1, 1, 1 };
	GLuint program;
	GLenum expected_error;

	pass = GL_TRUE;

	extension_supported =
		piglit_is_extension_supported("GL_EXT_unpack_subimage") ||
		(piglit_is_gles() && piglit_get_gl_version() >= 3);

	expected_error = extension_supported ? GL_NO_ERROR : GL_INVALID_ENUM;

	if (!piglit_automatic) {
		if (extension_supported)
			printf("GL_EXT_unpack_subimage is supported\n");
		else
			printf("GL_EXT_unpack_subimage is not supported\n");
	}

	piglit_reset_gl_error();
	if (!piglit_automatic)
		printf("Trying GL_UNPACK_ROW_LENGTH\n");
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 2);
	pass = piglit_check_gl_error(expected_error) && pass;

	piglit_reset_gl_error();
	if (!piglit_automatic)
		printf("Trying GL_UNPACK_SKIP_PIXELS\n");
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, 1);
	pass = piglit_check_gl_error(expected_error) && pass;

	piglit_reset_gl_error();
	if (!piglit_automatic)
		printf("Trying GL_UNPACK_SKIP_ROWS\n");
	glPixelStorei(GL_UNPACK_SKIP_ROWS, 4);
	pass = piglit_check_gl_error(expected_error) && pass;

	glClear(GL_COLOR_BUFFER_BIT);

	/* Try creating a texture with the unpacking parameters we've set */
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D,
		     0, /* level */
		     GL_RGBA, /* internalFormat */
		     1, /* width */
		     2, /* height */
		     0, /* border */
		     GL_RGBA, /* format */
		     GL_UNSIGNED_BYTE, /* type */
		     tex_data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	program = piglit_build_simple_program(vertex_shader, fragment_shader);
	glUseProgram(program);

	piglit_draw_rect_tex(-1, -1, 2, 2,
			     0, 0, 1, 1);

	if (extension_supported) {
		pass &= piglit_probe_pixel_rgba(piglit_width / 2,
						piglit_height / 4,
						blue);
		pass &= piglit_probe_pixel_rgba(piglit_width / 2,
						piglit_height * 3 / 4,
						cyan);
	} else {
		pass &= piglit_probe_pixel_rgba(piglit_width / 2,
						piglit_height / 4,
						red);
		pass &= piglit_probe_pixel_rgba(piglit_width / 2,
						piglit_height * 3 / 4,
						green);
	}

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
}
