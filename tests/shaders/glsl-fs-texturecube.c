/*
 * Copyright © 2010 Intel Corporation
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
 * Authors:
 *    Eric Anholt <eric@anholt.net>
 *
 */

/** @file glsl-fs-texturecube.c
 *
 * Tests that we can access cubemaps in the fragment shader.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static GLint prog;

static GLfloat colors[][3] = {
	{1.0, 1.0, 1.0},
	{1.0, 1.0, 0.0},
	{1.0, 0.0, 0.0},
	{1.0, 0.0, 1.0},
	{0.0, 0.0, 1.0},
	{0.0, 1.0, 1.0},
	{0.0, 1.0, 0.0},
};


static void
set_face_image(GLenum face, int size, int color)
{
	GLfloat *color1 = colors[color];
	GLfloat *color2 = colors[(color + 1) % ARRAY_SIZE(colors)];
	GLfloat *tex;
	int x, y;

	tex = malloc(size * size * 3 * sizeof(GLfloat));

	/* Set the texture for this face to one corner being color2 and the
	 * rest color1.  If the texture is 1x1, then it's all color1.
	 */
	for (y = 0; y < size; y++) {
		for (x = 0; x < size; x++) {
			GLfloat *chosen_color;

			if (y >= (size / 2) || x >= (size / 2))
				chosen_color = color1;
			else
				chosen_color = color2;

			tex[(y * size + x) * 3 + 0] = chosen_color[0];
			tex[(y * size + x) * 3 + 1] = chosen_color[1];
			tex[(y * size + x) * 3 + 2] = chosen_color[2];
		}
	}

	glTexImage2D(face, 0, GL_RGB, size, size, 0, GL_RGB, GL_FLOAT, tex);

	free(tex);
}


enum piglit_result
piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	GLuint tex;
	int face;

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	/* Create the texture. */
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_CUBE_MAP_ARB, tex);

	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB,
			GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB,
			GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB,
			GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB,
			GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	/* Fill in faces on each level */
	for (face = 0; face < 6; face++) {
		set_face_image(cube_face_targets[face], 32, face);
	}

	glEnable(GL_TEXTURE_CUBE_MAP_ARB);

	for (face = 0; face < 6; face++) {
		int x1 = piglit_width * face / 6;
		int x2 = piglit_width * (face + 1) / 6;
		int y1 = 0;
		int y2 = piglit_height;
		int tx1 = x1 + 1;
		int tx2 = x2 - 1;
		int ty1 = y1 + 1;
		int ty2 = y2 - 1;
		GLfloat *color1 = colors[face];
		GLfloat *color2 = colors[face + 1];

		glBegin(GL_QUADS);

		glTexCoord3fv(cube_face_texcoords[face][0]);
		glVertex2f(x1, y1);

		glTexCoord3fv(cube_face_texcoords[face][1]);
		glVertex2f(x2, y1);

		glTexCoord3fv(cube_face_texcoords[face][2]);
		glVertex2f(x2, y2);

		glTexCoord3fv(cube_face_texcoords[face][3]);
		glVertex2f(x1, y2);

		glEnd();

		pass = pass && piglit_probe_pixel_rgb(tx1, ty1, color2);
		pass = pass && piglit_probe_pixel_rgb(tx2, ty1, color1);
		pass = pass && piglit_probe_pixel_rgb(tx1, ty2, color1);
		pass = pass && piglit_probe_pixel_rgb(tx2, ty2, color1);
	}

	glDeleteTextures(1, &tex);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void piglit_init(int argc, char **argv)
{
	GLint vs, fs;
	int loc;
	GLboolean bias = GL_FALSE;
	char *fs_name;
	int i = 0;

	for (i = 0; i < argc; i++) {
		if (!strcmp(argv[i], "-bias"))
			bias = GL_TRUE;
	}

	piglit_require_gl_version(20);
	piglit_require_extension("GL_ARB_texture_cube_map");

	vs = piglit_compile_shader(GL_VERTEX_SHADER,
				   "shaders/glsl-tex-mvp.vert");
	if (bias) {
		fs_name = "shaders/glsl-fs-texturecube-bias.frag";
	} else {
		fs_name = "shaders/glsl-fs-texturecube.frag";
	}
	fs = piglit_compile_shader(GL_FRAGMENT_SHADER, fs_name);

	prog = piglit_link_simple_program(vs, fs);
	if (!piglit_link_check_status(prog))
		piglit_report_result(PIGLIT_FAIL);

	glUseProgram(prog);

	loc = glGetUniformLocation(prog, "sampler");
	glUniform1i(loc, 0);
}
