/*
 * Copyright © 2014 Ilia Mirkin
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
 * Author: Ilia Mirkin <imirkin@alum.mit.edu>
 */

/**
 * \file sampling-2d-array-as-cubemap.c
 * This tests that you can cast from a 2D Array texture to a Cubemap
 * texture and sample from the Cubemap view.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 30;
	config.supports_gl_es_version = 31;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

static const GLubyte green[] = {0, 255, 0, 255};
static const float greenf[] = {0, 1.0f, 0, 1.0f};
static const GLubyte red[] = {255, 0, 0, 255};

enum piglit_result
piglit_display(void)
{
	GLboolean pass;

	glViewport(0, 0, piglit_width, piglit_height);
	glClearColor(0.5, 0.5, 0.5, 0.5);
	glClear(GL_COLOR_BUFFER_BIT);

	piglit_draw_rect(-1, -1, 2, 2);

	pass = piglit_probe_rect_rgba(0, 0, piglit_width, piglit_height, greenf);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

#ifdef PIGLIT_USE_OPENGL
#define GLSL_VERSION "130"
#else
#define GLSL_VERSION "310 es"
#endif

static const char *vs =
	"#version " GLSL_VERSION "\n"
	"in vec4 piglit_vertex;\n"
	"void main() { \n"
	"	gl_Position = piglit_vertex;\n"
	"}\n";

static const char *fs =
	"#version " GLSL_VERSION "\n"
	"#ifdef GL_ES\n"
	"precision highp float;\n"
	"precision highp samplerCube;\n"
	"#endif\n"
	"uniform samplerCube tex;\n"
	"out vec4 color;\n"
	"void main() { \n"
	"	color = vec4(texture(tex, vec3(-1, 0, 0)).xyz, 1.0);\n"
	"}\n";

void
piglit_init(int argc, char **argv)
{
	int tex_loc_cube, prog_cube, l;
	GLuint tex_2DArray, tex_Cube;

#ifdef PIGLIT_USE_OPENGL
	piglit_require_extension("GL_ARB_texture_view");
#else
	piglit_require_extension("GL_OES_texture_view");
#endif

	/* setup shaders and program object for Cube rendering */
	prog_cube = piglit_build_simple_program(vs, fs);
	tex_loc_cube = glGetUniformLocation(prog_cube, "tex");

	glGenTextures(1, &tex_2DArray);
	glBindTexture(GL_TEXTURE_2D_ARRAY, tex_2DArray);

	glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA8, 1, 1, 8);

	/* load each array layer with red */
	for (l = 0; l < 8; l++) {
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, l,
				1, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, red);
	}
	/* make array layer 3 have green */
	glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 3,
			1, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, green);

	glGenTextures(1, &tex_Cube);
	/* the texture view starts at layer 2, so face 1 (-X) will have green */
	glTextureView(tex_Cube, GL_TEXTURE_CUBE_MAP, tex_2DArray, GL_RGBA8,
		      0, 1, 2, 6);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glBindTexture(GL_TEXTURE_CUBE_MAP, tex_Cube);

	glUseProgram(prog_cube);
	glUniform1i(tex_loc_cube, 0);
}
