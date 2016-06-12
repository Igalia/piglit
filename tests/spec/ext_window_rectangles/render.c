/*
 * Copyright (C) 2016 Ilia Mirkin
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

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 30;
	config.supports_gl_es_version = 30;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

GLint prog, color;
GLuint fb;

enum piglit_result
piglit_display(void)
{
	static const float blue[4] = {0, 0, 1, 1};
	static const float green[4] = {0, 1, 0, 1};
	static const int rect[4 * 8] = {
	  0, 0, 1, 1,
	  2, 0, 1, 1,
	  4, 0, 1, 1,
	  1, 1, 1, 1,
	  3, 1, 1, 1,
	  5, 1, 1, 1,
	  0, 2, 1, 1,
	  2, 2, 1, 1,
	};

	bool pass = true;

	glBindFramebuffer(GL_FRAMEBUFFER, fb);
	glUseProgram(prog);

	glViewport(0, 0, 20, 20);

	glClearColor(0, 0, 1, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	glWindowRectanglesEXT(GL_EXCLUSIVE_EXT, 8, rect);
	glUniform4fv(color, 1, green);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	for (int i = 0; i < 20; i++) {
		for (int j = 0; j < 20; j++) {
			// excluded
			if ((i == 0 && (j == 0 || j == 2 || j == 4)) ||
			    (i == 1 && (j == 1 || j == 3 || j == 5)) ||
			    (i == 2 && (j == 0 || j == 2)))
				pass &= piglit_probe_pixel_rgba(j, i, blue);
			else
				pass &= piglit_probe_pixel_rgba(j, i, green);
		}
	}

	glClearColor(0, 0, 1, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	glWindowRectanglesEXT(GL_INCLUSIVE_EXT, 8, rect);
	glUniform4fv(color, 1, green);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	for (int i = 0; i < 20; i++) {
		for (int j = 0; j < 20; j++) {
			// included
			if ((i == 0 && (j == 0 || j == 2 || j == 4)) ||
			    (i == 1 && (j == 1 || j == 3 || j == 5)) ||
			    (i == 2 && (j == 0 || j == 2)))
				pass &= piglit_probe_pixel_rgba(j, i, green);
			else
				pass &= piglit_probe_pixel_rgba(j, i, blue);
		}
	}

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
	glBlitFramebuffer(0, 0, 20, 20,
			  0, 0, piglit_width, piglit_height,
			  GL_COLOR_BUFFER_BIT, GL_NEAREST);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	static const float verts[4][4] = {
		/* x   y   z   w */
		{ -1, -1, 1.0, 1 },
		{  1, -1, 1.0, 1 },
		{ -1,  1, 0.1, 1 },
		{  1,  1, 0.1, 1 }
	};

	GLuint bo, rb;

	piglit_require_extension("GL_EXT_window_rectangles");

	prog = piglit_build_simple_program(
#ifdef PIGLIT_USE_OPENGL
			"#version 130\n"
#else
			"#version 300 es\n"
			"precision highp float;\n"
#endif
			"in vec4 piglit_vertex;\n"
			"void main() { gl_Position = piglit_vertex; }\n",

#ifdef PIGLIT_USE_OPENGL
			"#version 130\n"
#else
			"#version 300 es\n"
			"precision highp float;\n"
#endif
			"out vec4 col;\n"
			"uniform vec4 color;\n"
			"void main() { col = color; }\n");
	color = glGetUniformLocation(prog, "color");

	glEnableVertexAttribArray(0);
	glGenBuffers(1, &bo);
	glBindBuffer(GL_ARRAY_BUFFER, bo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
	glVertexAttribPointer(glGetAttribLocation(prog, "piglit_vertex"),
			      4, GL_FLOAT, GL_FALSE, 0, (GLvoid const *)0);

	glGenFramebuffers(1, &fb);
	glBindFramebuffer(GL_FRAMEBUFFER, fb);
	glGenRenderbuffers(1, &rb);
	glBindRenderbuffer(GL_RENDERBUFFER, rb);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, 20, 20);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
				  GL_RENDERBUFFER, rb);
}
