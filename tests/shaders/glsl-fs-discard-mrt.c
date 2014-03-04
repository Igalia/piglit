/*
 * Copyright © 2014 Intel Corporation
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
 */

/** @file glsl-fs-discard-mrt.c
 *
 * Tests that discarding fragments works correctly when rendering to
 * multiple render targets.
 */

#include "piglit-util-gl.h"

#define TEX_W 64
#define TEX_H 64
#define STRIPE_SIZE 25

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_DEPTH;

	config.window_width = TEX_W * 2 + 60;
	config.window_height = TEX_H + 40;

PIGLIT_GL_TEST_CONFIG_END

static char vs_source[] =
	"void main()\n"
	"{\n"
	"	gl_Position = gl_Vertex;\n"
	"}\n";

static char fs_source[] =
	"#extension GL_ARB_explicit_attrib_location : enable\n"
	"\n"
	"layout(location = 0) out vec4 fragcolor_0;\n"
	"layout(location = 1) out vec4 fragcolor_1;\n"
	"\n"
	"void main()\n"
	"{\n"
	"	if (gl_FragCoord.x > 25.0)\n"
	"		discard;\n"
	"	fragcolor_0 = vec4(0.0, 1.0, 0.0, 0.0); /* green */\n"
	"	fragcolor_1 = vec4(1.0, 0.0, 1.0, 0.0); /* magenta */\n"
	"}\n";

static GLuint tex[2];

static void
make_and_bind_fbo()
{
	int i;
	GLuint fbo;
	static const GLenum buffers[] = {
		GL_COLOR_ATTACHMENT0,
		GL_COLOR_ATTACHMENT1
	};

	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	glGenTextures(2, tex);

	for (i = 0; i < 2; i++) {
		glBindTexture(GL_TEXTURE_2D, tex[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, TEX_W, TEX_H, 0,
			     GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
				GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
				GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i,
				       GL_TEXTURE_2D, tex[i], 0);
	}
	glDrawBuffers(2, buffers);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) !=
	    GL_FRAMEBUFFER_COMPLETE) {
		printf("Framebuffer not complete.\n");
		piglit_report_result(PIGLIT_SKIP);
	}
}

static void
draw_to_fbo()
{
	GLint prog;

	prog = piglit_build_simple_program(vs_source, fs_source);
	glUseProgram(prog);

	glClearColor(0, 0, 1, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	piglit_draw_rect(-1, -1, 2, 2);

	assert(glGetError() == 0);

	glDeleteProgram(prog);
}

static bool
draw_fbo_to_screen_and_test(void)
{
	int i;
	bool pass = true;
	float green[4] = {0, 1, 0, 0};
	float blue[4] = {0, 0, 1, 0};
	float magenta[4] = {1, 0, 1, 0};

	glUseProgram(0);
	glBindFramebuffer(GL_FRAMEBUFFER, piglit_winsys_fbo);

	glClearColor(0.5, 0.5, 0.5, 0.5);
	glClear(GL_COLOR_BUFFER_BIT);

	for (i = 0; i < 2; i++) {
		int start_x = 20 + (TEX_W + 20) * i;
		int start_y = 20;

		piglit_ortho_projection(piglit_width, piglit_height, false);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, tex[i]);

		piglit_draw_rect_tex(start_x, start_y,
				     TEX_W, TEX_H,
				     0, 0,
				     1, 1);

		pass = piglit_probe_rect_rgba(start_x,
					      start_y,
					      STRIPE_SIZE,
					      TEX_H,
					      i ? magenta : green) && pass;
		pass = piglit_probe_rect_rgba(start_x + STRIPE_SIZE,
					      start_y,
					      TEX_W - STRIPE_SIZE,
					      TEX_H,
					      blue) && pass;
	}

	return pass;
}

enum piglit_result
piglit_display(void)
{
	bool pass = true;

	make_and_bind_fbo();
	draw_to_fbo();
	pass = draw_fbo_to_screen_and_test();

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	GLint max_buffers;

	piglit_require_GLSL();
	piglit_require_extension("GL_ARB_explicit_attrib_location");
	piglit_require_extension("GL_EXT_framebuffer_object");

	glGetIntegerv(GL_MAX_DRAW_BUFFERS, &max_buffers);
	if (max_buffers == 1) {
		fprintf(stderr, "Test requires 2 draw buffers\n");
		piglit_report_result(PIGLIT_SKIP);
	}
}

