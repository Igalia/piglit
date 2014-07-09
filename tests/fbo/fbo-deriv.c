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

/**
 * \file fbo-deriv.c
 *
 * Verify that the implementation produces correct values for the GLSL
 * dFdx() and dFdy() functions, both in fbos and in the default
 * framebuffer.
 *
 * Note: the reason that we test both fbos and the default framebuffer
 * in the same test is that some implementations (e.g. Mesa/i965) need
 * to compile the dFdy() function differently depending whether we are
 * rendering to an FBO or to the default framebuffer; testing both in
 * the same test allows us to verify that the implementation
 * recompiles the shader if necessary.
 *
 * This test draws a pair of squares in which dFdx and dFdy are
 * expected to both be 1.0.  It colors the rectangles red=0.5*dFdx and
 * green=0.5*dFdy, so the expected color is (0.5, 0.5, 0, 0).  The
 * left rectangle is drawn in the default framebuffer; the right
 * rectangle is drawn in an FBO and then blitted to the default
 * framebuffer.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 256;
	config.window_height = 128;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

static const int fbo_width = 128, fbo_height = 128;

static GLuint fbo;
static GLint prog;

static const char *vert =
	"void main()\n"
	"{\n"
	"  gl_Position = gl_Vertex;\n"
	"}\n";

static const char *frag =
	"void main()\n"
	"{\n"
	"  gl_FragColor = vec4(0.5*dFdx(gl_FragCoord.x),\n"
	"                      0.5*dFdy(gl_FragCoord.y), 0.0, 0.0);\n"
	"}\n";

static void
create_fbo()
{
	GLenum status;
	GLuint rb;

	glGenRenderbuffers(1, &rb);
	glBindRenderbuffer(GL_RENDERBUFFER, rb);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA, fbo_width, fbo_height);
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
	glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
				  GL_RENDERBUFFER, rb);
	status = glCheckFramebufferStatusEXT (GL_FRAMEBUFFER_EXT);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	switch (status) {
	case GL_FRAMEBUFFER_UNSUPPORTED:
		printf("Framebuffer unsupported\n");
		piglit_report_result(PIGLIT_SKIP);
		break;
	case GL_FRAMEBUFFER_COMPLETE:
		break;
	default:
		printf("Framebuffer incomplete\n");
		piglit_report_result(PIGLIT_FAIL);
		break;
	}
}

void
piglit_init(int argc, char **argv)
{
	GLint vs, fs;

	piglit_require_gl_version(20);
	piglit_require_extension("GL_ARB_framebuffer_object");

	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vert);
	fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, frag);
	prog = piglit_link_simple_program(vs, fs);

	create_fbo();
}

enum piglit_result
piglit_display(void)
{
	float expected[3] = { 0.5, 0.5, 0.0 };
	GLboolean pass = GL_TRUE;

	glUseProgram(prog);

	/* Draw a square to the left half of the window */
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
	glViewport(0, 0, piglit_width, piglit_height);
	piglit_draw_rect(-1, -1, 1, 2);

	/* Draw a square to the FBO */
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
	glViewport(0, 0, fbo_width, fbo_height);
	piglit_draw_rect(-1, -1, 2, 2);

	/* Blit the square from the FBO to the right half of the window */
	glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
	glBlitFramebuffer(0, 0, fbo_width, fbo_height,
			  piglit_width/2, 0, piglit_width, piglit_height,
			  GL_COLOR_BUFFER_BIT, GL_NEAREST);

	/* Check that both squares have the correct color */
	glBindFramebuffer(GL_READ_FRAMEBUFFER, piglit_winsys_fbo);
	pass = piglit_probe_rect_rgb(0, 0, piglit_width, piglit_height,
				     expected) && pass;

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
