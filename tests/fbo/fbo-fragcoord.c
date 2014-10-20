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
 */

/** @file glsl-fs-fragcoord.c
 *
 * Tests that gl_FragCoord produces the expected output in a fragment shader.
 */

#include "piglit-util-gl.h"

#define WIDTH 256
#define HEIGHT 256

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = WIDTH;
	config.window_height = HEIGHT;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static GLint prog;

static int
create_fbo(unsigned width, unsigned height, GLuint *out_tex)
{
	GLuint tex;
	GLuint fb;
	GLenum status;
	GLenum internal_format = GL_RGBA;

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height,
		     0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	glGenFramebuffersEXT(1, &fb);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
				  GL_TEXTURE_2D, tex, 0);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	status = glCheckFramebufferStatusEXT (GL_FRAMEBUFFER_EXT);
	if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
		if (status == GL_FRAMEBUFFER_UNSUPPORTED_EXT)
			printf("FBO with 0x%04x texture is unsupported\n",
			       internal_format);
		else
			fprintf(stderr, "FBO with 0x%04x texture is incomplete"
				" (0x%04x)\n",
				internal_format, status);

		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, piglit_winsys_fbo);
		glDeleteFramebuffersEXT(1, &fb);
		glDeleteTextures(1, &tex);
		piglit_report_result((status == GL_FRAMEBUFFER_UNSUPPORTED_EXT)
				     ? PIGLIT_SKIP : PIGLIT_FAIL);
	}

	*out_tex = tex;
	return fb;
}

enum piglit_result
piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	int x, y;
	GLuint fb, tex;

	/* Draw the shader to the fbo. */
	fb = create_fbo(WIDTH, HEIGHT, &tex);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);
	glViewport(0, 0, WIDTH, HEIGHT);

	glClearColor(1.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(prog);
	piglit_draw_rect(-1, -1, 2, 2);

	/* Draw the FBO to the screen. */
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, piglit_winsys_fbo);
	glViewport(0, 0, piglit_width, piglit_height);

	glClearColor(0.0, 0.0, 1.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glBindTexture(GL_TEXTURE_2D, tex);
	glEnable(GL_TEXTURE_2D);
	glUseProgram(0);
	piglit_draw_rect_tex(-1, -1, 2, 2,
			     0, 0, 1, 1);

	glDisable(GL_TEXTURE_2D);
	glDeleteTextures(1, &tex);
	glDeleteFramebuffersEXT(1, &fb);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	for (y = 0; y < piglit_height; y++) {
		for (x = 0; x < piglit_width; x++) {
			float color[3];

			color[0] = x / 256.0;
			color[1] = y / 256.0;
			color[2] = 0;

			pass &= piglit_probe_pixel_rgb(x, y, color);
			if (!pass)
				break;
		}
	}

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	GLint vs, fs;

	piglit_require_gl_version(20);
	piglit_require_extension("GL_EXT_framebuffer_object");

	vs = piglit_compile_shader(GL_VERTEX_SHADER,
				   "shaders/glsl-mvp.vert");
	fs = piglit_compile_shader(GL_FRAGMENT_SHADER,
				   "shaders/glsl-fs-fragcoord.frag");

	prog = piglit_link_simple_program(vs, fs);
}
