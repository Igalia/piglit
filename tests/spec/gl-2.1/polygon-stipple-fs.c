/*
 * Copyright 2016 Advanced Micro Devices, Inc.
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHOR(S) AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/*
 * Test that polygon stipple works and interacts reasonably with a simple
 * fragment shader.
 */

#include "piglit-util-gl.h"

#define STR(x) #x
#define STRINGIFY(x) STR(x)

#define TEX_WIDTH 64
#define TEX_HEIGHT 64

/* Test a window height that is not a multiple of 32!
 *
 * A minimum window size is required on Windows.
 */
#define WINDOW_WIDTH 160
#define WINDOW_HEIGHT 161

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 21;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

	config.window_width = WINDOW_WIDTH;
	config.window_height = WINDOW_HEIGHT;

	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

#define BLUE 0.45
#define ALPHA 0.82

/* Use a texture, uniforms, and immediate constants. */
static const char fragment_shader[] =
	"uniform sampler2D tex;\n"
	"uniform float b;\n"
	"void\n"
	"main()\n"
	"{\n"
	"   vec4 color = texture2D(tex, gl_TexCoord[0].xy);\n"
	"   gl_FragColor.xy = color.xy;\n"
	"   gl_FragColor.z = b;\n"
	"   gl_FragColor.w = float(" STRINGIFY(ALPHA) ");\n"
	"}\n";

static GLuint program;

static bool
test_stipple()
{
	GLuint uniform_tex, uniform_b;
	float *texture_img;
	float *expected_img;
	GLubyte *stipple;
	GLuint tex;
	unsigned i, x, y;
	bool pass = true;

	glPixelStorei(GL_UNPACK_LSB_FIRST, GL_TRUE);

	texture_img = malloc(TEX_WIDTH * TEX_HEIGHT * 4 * sizeof(float));
	expected_img = calloc(TEX_WIDTH * TEX_HEIGHT * 4, sizeof(float));
	stipple = malloc(32 * 32 / 8);

	for (i = 0; i < 32 * 32 / 8; ++i)
		stipple[i] = rand();

	i = 0;
	for (y = 0; y < TEX_HEIGHT; ++y) {
		for (x = 0; x < TEX_WIDTH; ++x, i += 4) {
			unsigned bit;
			bool active;

			texture_img[i + 0] = (float)rand() / RAND_MAX;
			texture_img[i + 1] = (float)rand() / RAND_MAX;
			texture_img[i + 2] = (float)rand() / RAND_MAX;
			texture_img[i + 3] = (float)rand() / RAND_MAX;

			bit = (y % 32) * 32 + x % 32;

			active = stipple[bit / 8] & (1 << (bit % 8));
			if (active) {
				expected_img[i + 0] = texture_img[i + 0];
				expected_img[i + 1] = texture_img[i + 1];
				expected_img[i + 2] = BLUE;
				expected_img[i + 3] = ALPHA;
			}
		}
	}

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, TEX_WIDTH, TEX_HEIGHT, 0,
		     GL_RGBA, GL_FLOAT, texture_img);

	glPolygonStipple(stipple);
	glEnable(GL_POLYGON_STIPPLE);

	glUseProgram(program);

	uniform_tex = glGetUniformLocation(program, "tex");
	glUniform1i(uniform_tex, 0);

	uniform_b = glGetUniformLocation(program, "b");
	glUniform1f(uniform_b, BLUE);

	piglit_draw_rect_tex(-1.0, -1.0,
			     2.0 * TEX_WIDTH / WINDOW_HEIGHT,
			     2.0 * TEX_HEIGHT / WINDOW_HEIGHT,
			     0, 0, 1.0, 1.0);

	glUseProgram(0);
	glDisable(GL_POLYGON_STIPPLE);
	glDeleteTextures(1, &tex);

	pass = piglit_probe_image_rgba(0, 0, TEX_WIDTH, TEX_HEIGHT, expected_img);

	free(stipple);
	free(expected_img);
	free(texture_img);

	return pass;
}

enum piglit_result
piglit_display(void)
{
	bool pass = true;

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	pass = test_stipple();

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	srand(0); /* reproducibility of the first error */

	program = piglit_build_simple_program(NULL, fragment_shader);
}
