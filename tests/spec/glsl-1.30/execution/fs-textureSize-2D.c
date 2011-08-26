/*
 * Copyright © 2011 Intel Corporation
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
 * \file fs-textureSize.c
 *
 * Tests the built-in function textureSize() in the fragment shader.
 *
 * Creates a mipmapped 64x32 2D texture and draws a series of squares whose
 * color contains the width (red) and height (green) of each mipmap level.
 */
#include "piglit-util.h"

int piglit_width = 150, piglit_height = 30;
int piglit_window_mode = GLUT_RGBA | GLUT_DOUBLE;

const int tex_size = 64;

static int lod_location;

static const char vert[] =
"#version 130\n"
"void main()\n"
"{\n"
"	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
"}\n";

static const char frag[] =
"#version 130\n"
"uniform int lod;\n"
"uniform sampler2D tex;\n"
"void main()\n"
"{\n"
"	ivec2 size = textureSize(tex, lod);\n"
"	gl_FragColor = vec4(0.01 * size, 0.0, 1.0);\n"
"}\n";

static float max(float x, float y) { return (x > y) ? x : y; }

enum piglit_result
piglit_display(void)
{
	int l;
	bool pass = true;

	glClearColor(0.5, 0.5, 0.5, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	/* Draw consecutive squares for each mipmap level */
	for (l = 0; (tex_size >> l) > 0; l++) {
		const int width = tex_size >> l;
		const int height = max(width / 2, 1);
		const float c[] = {0.01*width, 0.01*height, 0.0, 1.0};

		const int x = 10+20*l;

		piglit_Uniform1i(lod_location, l);
		piglit_draw_rect(x, 10, 10, 10);

		pass &= piglit_probe_rect_rgba(x, 10, 10, 10, c);
	}

	glutSwapBuffers();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	int vs, fs, prog;
	int tex_location;

	piglit_require_GLSL_version(130);

	glActiveTexture(GL_TEXTURE0);
	piglit_rgbw_texture(GL_RGBA, tex_size, tex_size / 2, true, false,
			    GL_UNSIGNED_NORMALIZED);

	piglit_ortho_projection(piglit_width, piglit_height, false);

	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vert);
	fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, frag);
	prog = piglit_link_simple_program(vs, fs);

	tex_location = piglit_GetUniformLocation(prog, "tex");
	lod_location = piglit_GetUniformLocation(prog, "lod");

	piglit_UseProgram(prog);
	piglit_Uniform1i(tex_location, 0);
}
