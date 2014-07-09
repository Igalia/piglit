/*
 * Copyright © 2011 Dave Airlie
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/**
 * \file fs-texelFetch-2D.c
 *
 * Tests the built-in function texelFetch() in the fragment shader.
 *
 * Creates a mipmapped 64x32 2D texture and draws a series of squares whose
 * color contains a texel fetched from each quadrant of the rgbw texture.
 */
#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 90;
	config.window_height = 150;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

const int tex_size = 64;

static int pos_location, lod_location;

static const char vert[] =
"#version 130\n"
"void main()\n"
"{\n"
"	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
"}\n";

static const char frag[] =
"#version 130\n"
"uniform ivec2 pos;\n"
"uniform int lod;\n"
"uniform sampler2D tex;\n"
"void main()\n"
"{\n"
"       vec4 texel = texelFetch(tex, pos, lod);\n"
"	gl_FragColor = texel;\n"
"}\n";

#ifdef _MSC_VER
#undef max
#endif
static float max(float x, float y) { return (x > y) ? x : y; }

enum piglit_result
piglit_display(void)
{
	int l, q;
	bool pass = true;
	float red[4]   = {1.0, 0.0, 0.0, 1.0};
	float green[4] = {0.0, 1.0, 0.0, 1.0};
	float blue[4]  = {0.0, 0.0, 1.0, 1.0};
	float white[4] = {1.0, 1.0, 1.0, 1.0};

	glClearColor(0.5, 0.5, 0.5, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	for (l = 0; (tex_size >> l) > 0; l++) {
		const int width = tex_size >> l;
		const int height = max(width / 2, 1);
		const int y = 10 + 20 * l;

		glUniform1i(lod_location, l);

		/* Draw 4 squares with a color sample for each quad */
		for (q = 0; q < 4; q++) {
			const int tex_x = (q / 2) * ((width / 2));
			const int tex_y = (q % 2) * ((height / 2));
			float *c;
			const int x = 10+20*q;

			if (q == 0) c = red;
			else if (q == 1) c = blue;
			else if (q == 2) c = green;
			else if (q == 3) c = white;

			glUniform2i(pos_location, tex_x, tex_y);
			piglit_draw_rect(x, y, 10, 10);

			if (width > 2) /* below 1 wide no test */
				pass &= piglit_probe_rect_rgba(x, y, 10, 10, c);
		}
	}

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	int prog;
	int tex_location;

	piglit_require_GLSL_version(130);

	glActiveTexture(GL_TEXTURE0);
	piglit_rgbw_texture(GL_RGBA, tex_size, tex_size / 2, true, false,
			    GL_UNSIGNED_NORMALIZED);

	piglit_ortho_projection(piglit_width, piglit_height, false);

	prog = piglit_build_simple_program(vert, frag);

	tex_location = glGetUniformLocation(prog, "tex");
	lod_location = glGetUniformLocation(prog, "lod");
	pos_location = glGetUniformLocation(prog, "pos");

	glUseProgram(prog);
	glUniform1i(tex_location, 0);
}
