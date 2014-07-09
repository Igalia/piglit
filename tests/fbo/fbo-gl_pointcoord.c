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
 *
 *
 */

/**
 * \file pointcoord.c
 * Verify that applications can use point coordinate correctly with FBO.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

static const char vs_text[] =
	"#version 120 \n"
	"void main()\n"
	"{\n"
	"   gl_Position = gl_Vertex;\n"
	"}\n";

static const char fs_text[] =
	"#version 120 \n"
	"void main()\n"
	"{\n"
	"   gl_FragColor = vec4(gl_PointCoord.x, gl_PointCoord.y, 0.0, 1.0);\n"
	"}\n";

static GLuint prog;
static GLuint fb, rb;
static GLuint testPoint_x, testPoint_y;
static GLuint PointSize;

static const int rb_size = 100;

static const float green[] = { 0.0, 1.0, 0.0, 1.0 };
static const float black[] = { 0.0, 0.0, 0.0, 1.0 };

enum piglit_result
piglit_display(void)
{
	GLboolean pass = GL_TRUE;

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fb);

	glViewport(0, 0, rb_size, rb_size);

	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	glBegin(GL_POINTS);
	glVertex2f(0.0, 0.0);
	glEnd();

	glBindFramebuffer(GL_READ_FRAMEBUFFER, fb);

	testPoint_x = ( rb_size - PointSize ) / 2;
	testPoint_y = ( rb_size - PointSize ) / 2;
	pass = piglit_probe_pixel_rgb(0, 0, black) && pass;
	pass = piglit_probe_pixel_rgb(testPoint_x, testPoint_y, green) && pass;

	testPoint_y = rb_size - testPoint_y;
	pass = piglit_probe_pixel_rgb(testPoint_x, testPoint_y + 1, black) && pass;

	/* Draw the point out if want to have a look. */
	if (!piglit_automatic){
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
		glClear(GL_COLOR_BUFFER_BIT);
		glBlitFramebuffer(0, 0, rb_size, rb_size,
				  0, 0, rb_size, rb_size,
				  GL_COLOR_BUFFER_BIT, GL_NEAREST);
		glFlush();
	}


	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	GLuint vs, fs;
	int point_size_limits[2];

	piglit_require_extension("GL_ARB_point_sprite");
	piglit_require_extension("GL_ARB_framebuffer_object");

	glGetIntegerv(GL_ALIASED_POINT_SIZE_RANGE, point_size_limits);

	PointSize = point_size_limits[1] >= 64 ? 64 : point_size_limits[1];
	glEnable(GL_POINT_SPRITE_ARB);
	glPointSize(PointSize);

	piglit_require_vertex_shader();
	piglit_require_fragment_shader();

	fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, fs_text);
	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vs_text);
	prog = piglit_link_simple_program(vs, fs);

	glUseProgram(prog);

	glGenRenderbuffers(1, &rb);
	glBindRenderbuffer(GL_RENDERBUFFER, rb);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA, rb_size, rb_size);

	glGenFramebuffers(1, &fb);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fb);
	glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
				  GL_RENDERBUFFER, rb);
}
