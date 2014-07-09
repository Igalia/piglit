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
 */

/** \file array-depth-roundtrip.c
 *
 * Test that an array texture containing depth data works properly
 * when making a full "roundtrip" through both the GPU's rendering
 * pipeline and texturing operations.
 *
 * The test performs the following steps:
 *
 * - Create an array texture containing depth data.
 *
 * - Bind each slice of the array texture to a framebuffer, clear it,
 *   and render a quad to it.  A different depth value is used for
 *   each slice of the array.
 *
 * - Use a shader to read from each slice of the array texture and
 *   render to the window system framebuffer.
 *
 * - Verify that correct data was rendered to the window system
 *   framebuffer.
 */

#include "piglit-util-gl.h"


#define TEX_WIDTH 56
#define TEX_HEIGHT 56
#define NUM_TILES_ACROSS 4
#define NUM_TILES_DOWN 4
#define TEX_DEPTH (NUM_TILES_ACROSS * NUM_TILES_DOWN)


PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = TEX_WIDTH*NUM_TILES_ACROSS;
	config.window_height = TEX_HEIGHT*NUM_TILES_DOWN;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END


GLuint tex;
GLuint fb;
GLuint prog;
GLint samp_loc;
GLint proj_loc;
GLint tex_depth_loc;


const char *vs_text = \
	"#version 130\n"
	"uniform mat4 proj;\n"
	"uniform float tex_depth;\n"
	"out vec3 tex_coord;\n"
	"void main()\n"
	"{\n"
	"  gl_Position = proj * gl_Vertex;\n"
	"  tex_coord = vec3(gl_Vertex.xy, tex_depth);\n"
	"}\n";


const char *fs_text = \
	"#version 130\n"
	"uniform sampler2DArray samp;\n"
	"in vec3 tex_coord;\n"
	"void main()\n"
	"{\n"
	"  gl_FragColor = texture(samp, tex_coord);\n"
	"}\n";


void
piglit_init(int argc, char **argv)
{
	GLuint vs, fs;

	piglit_require_gl_version(30);

	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vs_text);
	fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, fs_text);
	prog = piglit_link_simple_program(vs, fs);
	samp_loc = glGetUniformLocation(prog, "samp");
	proj_loc = glGetUniformLocation(prog, "proj");
	tex_depth_loc = glGetUniformLocation(prog, "tex_depth");

	/* Create the array texture */
	glGenTextures(1, &tex);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, tex);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0 /* level */,
		     GL_DEPTH_COMPONENT, TEX_WIDTH, TEX_HEIGHT, TEX_DEPTH,
		     0 /* border */, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER,
			GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER,
			GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_MODE,
			GL_NONE);

	glGenFramebuffers(1, &fb);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);
}


enum piglit_result
piglit_display()
{
	int zoffset, x_tile, y_tile, i;
	float depth_value;
	float expected[3];
	bool pass = true;

	/* Bind each level of the array texture to the framebuffer,
	 * clear it, and render a quad to it, using a depth value that
	 * is different in each array slice.
	 */
	glBindFramebuffer(GL_FRAMEBUFFER, fb);
	glViewport(0, 0, TEX_WIDTH, TEX_HEIGHT);
	glUseProgram(0);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_ALWAYS);
	for (zoffset = 0; zoffset < TEX_DEPTH; ++zoffset) {
		glFramebufferTextureLayer(GL_FRAMEBUFFER,
					  GL_DEPTH_ATTACHMENT,
					  tex, 0 /* level */, zoffset);
		glClear(GL_DEPTH_BUFFER_BIT);
		depth_value = zoffset / (float) (TEX_DEPTH - 1);
		/* Adjust depth_value to [-1, 1] range to account for
		 * the fact that the pipeline translates from [-1, 1]
		 * to [0, 1].
		 */
		depth_value = depth_value * 2.0 - 1.0;
		piglit_draw_rect_z(depth_value,
				   -1, -1, 2, 2);
	}

	/* Use a shader to read from each slice of the array texture
	 * and render to the window system framebuffer.
	 */
	glBindFramebuffer(GL_FRAMEBUFFER, piglit_winsys_fbo);
	glViewport(0, 0, piglit_width, piglit_height);
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(prog);
	glUniform1i(samp_loc, 0);
	for (y_tile = 0; y_tile < NUM_TILES_DOWN; ++y_tile) {
		for (x_tile = 0; x_tile < NUM_TILES_ACROSS; ++x_tile) {
			float xscale = 2.0 / NUM_TILES_ACROSS;
			float yscale = 2.0 / NUM_TILES_DOWN;
			float proj[4][4] = {
				{ xscale, 0, 0, xscale * x_tile - 1 },
				{ 0, yscale, 0, yscale * y_tile - 1 },
				{ 0, 0, 0, 0 },
				{ 0, 0, 0, 1 }
			};
			zoffset = NUM_TILES_ACROSS * y_tile + x_tile;
			glUniformMatrix4fv(proj_loc, 1, GL_TRUE, &proj[0][0]);
			glUniform1f(tex_depth_loc, zoffset);
			piglit_draw_rect(0, 0, 1, 1);
		}
	}

	/* Verify that correct data was rendered. */
	for (y_tile = 0; y_tile < NUM_TILES_DOWN; ++y_tile) {
		for (x_tile = 0; x_tile < NUM_TILES_ACROSS; ++x_tile) {
			zoffset = NUM_TILES_ACROSS * y_tile + x_tile;
			printf("Probing array slice %d\n", zoffset);
			depth_value = zoffset / (float) (TEX_DEPTH - 1);
			for (i = 0; i < 3; ++i)
				expected[i] = depth_value;
			pass = piglit_probe_rect_rgb(x_tile * TEX_WIDTH,
						     y_tile * TEX_HEIGHT,
						     TEX_WIDTH, TEX_HEIGHT,
						     expected) && pass;
		}
	}

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
