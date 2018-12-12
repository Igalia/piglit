/*
 * Copyright (c) 2018 Andrii Simiklit
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
 *    Andrii Simiklit <asimiklit.work@gmail.com>
 *
 */

/**
 * Test what there no an assertion when we use upside down miptree and
 * GL_TEXTURE_MIN_FILTER is GL_LINEAR, base level is not 0
 * Bugzilla: https://bugs.freedesktop.org/show_bug.cgi?id=107987
 */

#include "piglit-util-gl.h"
#include <math.h>

#define TW 64
#define TH 64
#define MINIMUM(X, Y)  ((X) < (Y) ? (X) : (Y))
#define LEVELS (log2(MINIMUM(TW,TH)) + 1)

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

GLuint prog = 0;
GLuint texture = 0;
static const GLubyte fancy_pixel[4] = { 255, 128, 64, 32 };
static const char *fragShaderText =
	"uniform sampler2D tex;\n"
	"void main()\n"
	"{\n"
	"   gl_FragColor = texture2D(tex, gl_TexCoord[0].xy).rgba;\n"
	"}\n";

static void
get_rect_bounds(int *x, int *y, int *w, int *h)
{
	*x = 5;
	*y = 5;
	*w = piglit_width / 3 - 10;
	*h = piglit_height - 10;
}


static void
draw_rect()
{
	int x, y, w, h;
	get_rect_bounds(&x, &y, &w, &h);
	piglit_draw_rect_tex(x, y, w, h, 0, 0, 1, 1);
}


static GLboolean
probe_pos(const GLfloat expected[4])
{
	int x, y, w, h;
	get_rect_bounds(&x, &y, &w, &h);
	return piglit_probe_rect_rgba(x, y, w, h, expected);
}


enum piglit_result
piglit_display(void)
{
	GLint tex;
	GLboolean pass = GL_TRUE;
	unsigned level;
	const GLfloat oneby255 = (1.0 / 255.0);
	const GLfloat expected[4] = { oneby255 * fancy_pixel[0],
											oneby255 * fancy_pixel[1],
											oneby255 * fancy_pixel[2],
											oneby255 * fancy_pixel[3] };

	glClearColor(0.5f, 0.5f, 0.5f, 0.5f);
	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);
	glUseProgram(prog);
	tex = glGetUniformLocation(prog, "tex");
	glUniform1i(tex, 0);

	for(level = 1; level < LEVELS; level++) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, level);
		glClear(GL_COLOR_BUFFER_BIT);
		/* If we the draw_rect function doesn't cause crash/assertion
		* it means everything is okay and test will be marked
		* as pass
		*/
		draw_rect();
		/** Just in case we check it
		 */
		pass = probe_pos(expected) && pass;
	}

	glUseProgram(0);
	glDeleteProgram(prog);
	glDeleteTextures(1, &texture);
	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


static void
setup_texture(void)
{
	GLubyte img[TH][TW][4];
	int i, j, w, h;

	for (i = 0; i < TH; i++)
		for (j = 0; j < TW; j++)
			memcpy(&img[i][j], fancy_pixel, sizeof(fancy_pixel));

	/* make an abnormal upside down miptree
	 */
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	w = TW;
	h = TH;
	for (i = 0; w > 0 && h > 0; i++) {
		glTexImage2D(GL_TEXTURE_2D, LEVELS - 1 - i, GL_RGBA, w, h, 0,
			     GL_RGBA, GL_UNSIGNED_BYTE, img);
		w /= 2;
		h /= 2;
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}


void
piglit_init(int argc, char **argv)
{
	piglit_require_GLSL_version(110);
	prog = piglit_build_simple_program(NULL, fragShaderText);
	setup_texture();
}
