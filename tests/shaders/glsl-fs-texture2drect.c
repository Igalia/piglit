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

/** @file glsl-fs-texture2drect.c
 *
 * Tests that we can access rectangular textures in the fragment shader.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static GLint prog;
float red[4]   = {1.0, 0.0, 0.0, 1.0};
float green[4] = {0.0, 1.0, 0.0, 1.0};
float blue[4]  = {0.0, 0.0, 1.0, 1.0};
float white[4] = {1.0, 1.0, 1.0, 1.0};
GLboolean proj3 = GL_FALSE, proj4 = GL_FALSE;

GLuint
rgbw_texture(GLenum format, int w, int h)
{
	GLfloat *data;
	int x, y;
	GLuint tex;

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_RECTANGLE, tex);
	glTexParameteri(GL_TEXTURE_RECTANGLE,
			GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_RECTANGLE,
			GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_RECTANGLE,
			GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE,
			GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	data = malloc(w * h * 4 * sizeof(GLfloat));

	for (y = 0; y < h; y++) {
		for (x = 0; x < w; x++) {
			const float *color;

			if (x < w / 2 && y < h / 2)
				color = red;
			else if (y < h / 2)
				color = green;
			else if (x < w / 2)
				color = blue;
			else
				color = white;

			memcpy(data + (y * w + x) * 4, color,
			       4 * sizeof(float));
		}
	}
	glTexImage2D(GL_TEXTURE_RECTANGLE, 0,
		     format,
		     w, h, 0,
		     GL_RGBA, GL_FLOAT, data);

	free(data);
	return tex;
}

enum piglit_result
piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	GLuint tex;
	int tx1 = piglit_width * 1 / 4;
	int tx2 = piglit_width * 3 / 4;
	int ty1 = piglit_height * 1 / 4;
	int ty2 = piglit_height * 3 / 4;
	float proj;

	/* Create the texture. */
	tex = rgbw_texture(GL_RGBA, 50, 25);

	glEnable(GL_TEXTURE_RECTANGLE);

	if (proj3 || proj4)
		proj = 2.0;
	else
		proj = 1.0;

	piglit_draw_rect_tex(-1, -1, 2, 2,
			     0, 0, 50 * proj, 25 * proj);

	pass = pass && piglit_probe_pixel_rgb(tx1, ty1, red);
	pass = pass && piglit_probe_pixel_rgb(tx2, ty1, green);
	pass = pass && piglit_probe_pixel_rgb(tx1, ty2, blue);
	pass = pass && piglit_probe_pixel_rgb(tx2, ty2, white);

	glDeleteTextures(1, &tex);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void piglit_init(int argc, char **argv)
{
	GLint vs, fs;
	int loc;
	char *fs_name;
	int i = 0;

	for (i = 0; i < argc; i++) {
		if (!strcmp(argv[i], "-proj3"))
			proj3 = GL_TRUE;
		if (!strcmp(argv[i], "-proj4"))
			proj4 = GL_TRUE;
	}

	piglit_require_gl_version(20);
	piglit_require_extension("GL_ARB_texture_rectangle");

	vs = piglit_compile_shader(GL_VERTEX_SHADER,
				   "shaders/glsl-tex-mvp.vert");
	if (proj4) {
		fs_name = "shaders/glsl-fs-texture2drect-proj4.frag";
	} else if (proj3) {
		fs_name = "shaders/glsl-fs-texture2drect-proj3.frag";
	} else {
		fs_name = "shaders/glsl-fs-texture2drect.frag";
	}
	fs = piglit_compile_shader(GL_FRAGMENT_SHADER, fs_name);

	prog = piglit_link_simple_program(vs, fs);
	if (!piglit_link_check_status(prog))
		piglit_report_result(PIGLIT_FAIL);

	glUseProgram(prog);

	loc = glGetUniformLocation(prog, "sampler");
	glUniform1i(loc, 0);
}
