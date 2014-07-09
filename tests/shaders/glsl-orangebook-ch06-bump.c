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

/** @file glsl-orangebook-ch06-bump.c
 *
 * Tests that the Orange Book's chapter 6 shader for procedural bumpmapping
 * works correctly.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

/* Test size - note that the pixel probing below is very specific */
#define WIDTH 100
#define HEIGHT 100


static int bump_density_location, bump_size_location;
static int specular_factor_location, surface_color_location;
static int light_position_location, tangent_attrib;
static GLint prog;

enum piglit_result
piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	static const float surface_color[3] = {0.7, 0.6, 0.18};
	static const float test_specular[3] = {0.976, 0.894, 0.549};
	static const float test_nonspecular[3] = {0.572, 0.490, 0.145};
	static const float test_bump_dark[3] = {0.411, 0.352, 0.105};
	static const float test_bump_light[3] = {1.0, 0.961, 0.557};
	float light_position[3] = {-1.0, -1.0, 2.0};
	float w = WIDTH;
	float h = HEIGHT;
	float bump_x = w * 3 / 8;
	float bump_y = h * 3 / 8;
	float x, y;

	if (piglit_width < WIDTH || piglit_height < HEIGHT) {
		printf("window is too small.\n");
		return PIGLIT_SKIP;
	}

	piglit_ortho_projection(1, 1, GL_FALSE);

	glViewport(0, 0, WIDTH, HEIGHT);
	glClearColor(0.5, 0.5, 0.5, 0.5);
	glClear(GL_COLOR_BUFFER_BIT);

	glUniform1f(bump_density_location, 4);
	glUniform1f(bump_size_location, 0.15);
	glUniform1f(specular_factor_location, 0.5);
	glUniform3fv(surface_color_location, 1, surface_color);
	glUniform3fv(light_position_location, 1, light_position);

	glTranslatef(0, 0, -0.5);
	glNormal3f(0.0, 0.0, 1.0);
	glVertexAttrib3f(tangent_attrib, 1.0, 0.0, 0.0);
	for (x = 0.0; x < 1.0; x += 0.1) {
		for (y = 0.0; y < 1.0; y += 0.1) {
			piglit_draw_rect_tex(x, y, 0.1, 0.1,
					     x, y, 0.1, 0.1);
		}
	}

	/* Corners of the image: A highly specular point, and a
	 * non-specular point.
	 */
	pass &= piglit_probe_pixel_rgb(0, 0, test_specular);
	pass &= piglit_probe_pixel_rgb(WIDTH - 1, HEIGHT - 1,
				       test_nonspecular);

	/* Look at a bump -- does it have a lit part and an unlit part? */
	pass &= piglit_probe_pixel_rgb(bump_x + w / 16, bump_y + h / 16,
				       test_bump_dark);
	pass &= piglit_probe_pixel_rgb(bump_x - w / 16, bump_y - h / 16,
				       test_bump_light);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	GLint vs, fs;

	piglit_require_gl_version(20);

	vs = piglit_compile_shader(GL_VERTEX_SHADER,
				   "shaders/glsl-orangebook-ch06-bump.vert");
	fs = piglit_compile_shader(GL_FRAGMENT_SHADER,
				   "shaders/glsl-orangebook-ch06-bump.frag");

	prog = piglit_link_simple_program(vs, fs);

	glUseProgram(prog);

	bump_density_location = glGetUniformLocation(prog, "BumpDensity");
	bump_size_location = glGetUniformLocation(prog, "BumpSize");
	specular_factor_location = glGetUniformLocation(prog, "SpecularFactor");
	surface_color_location = glGetUniformLocation(prog, "SurfaceColor");
	light_position_location = glGetUniformLocation(prog, "LightPosition");
	assert(bump_density_location != -1);
	assert(bump_size_location != -1);
	assert(specular_factor_location != -1);
	assert(surface_color_location != -1);
	assert(light_position_location != -1);

	tangent_attrib = glGetAttribLocation(prog, "Tangent");
	assert(tangent_attrib != -1);
}
