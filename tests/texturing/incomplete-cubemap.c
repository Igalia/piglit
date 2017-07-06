/*
 * Copyright © 2016 Intel Corporation
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

/**
 * Tests that a cube map texture which doesn't have the same size or
 * format for all of the faces isn't considered complete.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 20;
	config.supports_gl_core_version = 31;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

static const GLenum
faces[6] = {
	GL_TEXTURE_CUBE_MAP_POSITIVE_X,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
};

enum test_attribute {
	TEST_ATTRIBUTE_SIZE,
	TEST_ATTRIBUTE_FORMAT
};

static enum test_attribute
test_attribute;

/* Incomplete textures return 0,0,0,1 when sampled in GLSL */
static const float
expected_color[] = {
	0.0f, 0.0f, 0.0f, 1.0f
};

static const char
vertex_source[] =
	"attribute vec2 piglit_vertex;\n"
	"\n"
	"void\n"
	"main()\n"
	"{\n"
	"        gl_Position = vec4(piglit_vertex, 0.0, 1.0);\n"
	"}\n";

static const char
fragment_source[] =
	"uniform samplerCube tex;\n"
	"\n"
	"void\n"
	"main()\n"
	"{\n"
	"        gl_FragColor = textureCube(tex, vec3(0.0));\n"
	"}\n";

static void
make_image(GLenum target,
	   int size,
	   GLenum internal_format)
{
	GLubyte *data = malloc(size * size * 4), *p = data;
	int i;

	for (i = 0; i < size * size; i++) {
		/* Red texture because it should be incomplete so if
		 * it is displayed then it is a failure.
		 */
		*(p++) = 0xff;
		*(p++) = 0x00;
		*(p++) = 0x00;
		*(p++) = 0xff;
	}

	glTexImage2D(target,
		     0, /* level */
		     internal_format,
		     size, size,
		     0, /* border */
		     GL_RGBA,
		     GL_UNSIGNED_BYTE,
		     data);

	free(data);
}

enum piglit_result
piglit_display(void)
{
	GLuint tex;
	int face;
	GLenum internal_format;
	int size;
	bool pass;

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_CUBE_MAP, tex);

	for (face = 0; face < ARRAY_SIZE (faces); face++) {
		internal_format = GL_RGBA;
		size = 4;

		if (face == 3) {
			switch (test_attribute) {
			case TEST_ATTRIBUTE_SIZE:
				size = 8;
				break;
			case TEST_ATTRIBUTE_FORMAT:
				internal_format = GL_RGB;
				break;
			}
		}

		make_image(faces[face], size, internal_format);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP,
			GL_TEXTURE_MIN_FILTER,
			GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP,
			GL_TEXTURE_MAG_FILTER,
			GL_NEAREST);

	piglit_draw_rect(-1, -1, 2, 2);

	glDeleteTextures(1, &tex);

	pass = piglit_probe_rect_rgba(0, 0,
				      piglit_width, piglit_height,
				      expected_color);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

static void
show_usage(void)
{
	fprintf(stderr, "usage: incomplete-cubemap <size|format>\n");
	piglit_report_result(PIGLIT_FAIL);
}

void
piglit_init(int argc, char **argv)
{
	GLuint program;

	if (argc != 2)
		show_usage();

	if (!strcmp(argv[1], "size"))
		test_attribute = TEST_ATTRIBUTE_SIZE;
	else if (!strcmp(argv[1], "format"))
		test_attribute = TEST_ATTRIBUTE_FORMAT;
	else
		show_usage();

	program = piglit_build_simple_program(vertex_source, fragment_source);
	glUseProgram(program);
}
