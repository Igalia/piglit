/*
 * Copyright © 2015 Intel Corporation
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
 * \file fast-clear-blend.c
 *
 * Enables GL_FRAMEBUFFER_SRGB, clears the buffer to a color and then
 * blends it with a rectangle in another color before verifying the
 * result. This is mainly to test fast clears on SKL in the i965
 * driver because in that case fast clears can't be used with
 * GL_FRAMEBUFFER_SRGB so it internally needs to resolve the color
 * buffer.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 21;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static const char
vertex_source[] =
	"attribute vec4 piglit_vertex;\n"
	"\n"
	"void\n"
	"main()\n"
	"{\n"
	"	 gl_Position = piglit_vertex;\n"
	"}\n";

static const char
fragment_source[] =
	"uniform vec4 color;\n"
	"\n"
	"void\n"
	"main()\n"
	"{\n"
	"	 gl_FragColor = color;\n"
	"}\n";

static GLuint prog;
static GLuint fbo;
static GLint color_location;

static const float
clear_colors[][4] = {
	{ 0.0f, 0.0f, 0.0f, 0.0f },
	{ 1.0f, 1.0f, 1.0f, 1.0f },
	{ 0.0f, 0.0f, 1.0f, 0.0f },
	{ 1.0f, 0.0f, 0.0f, 1.0f },

	{ 0.25f, 0.5f, 0.75f, 1.0f },
	{ 0.75f, 0.5f, 0.25f, 0.0f },
	{ 0.5f, 0.25f, 0.75f, 0.5f },
};

static bool
probe_srgb_color(int x, int y, int w, int h,
		 const GLfloat *color)
{
	GLfloat srgb_color[4];
	int i;

	/* The value in the framebuffer is stored in SRGB space so we
	 * need to convert to that.
	 */
	for (i = 0; i < 3; i++)
		srgb_color[i] = piglit_linear_to_srgb(color[i]);
	srgb_color[3] = color[3];

	return piglit_probe_rect_rgba(x, y, w, h, srgb_color);
}

static bool
test_color(bool srgb_before_clear,
	   const GLfloat *clear_color)
{
	static const GLfloat rect_color[] = {
		0.0f, 0.75f, 1.0f, 0.5f
	};
	GLfloat expected_color[4];
	GLfloat fb_color;
	bool pass = true;
	int i;

	printf("Clear to %f,%f,%f,%f - SRGB enabled %s clear\n",
	       clear_color[0],
	       clear_color[1],
	       clear_color[2],
	       clear_color[3],
	       srgb_before_clear ? "before" : "after");

	if (srgb_before_clear)
		glEnable(GL_FRAMEBUFFER_SRGB);

	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glClearColor(clear_color[0],
		     clear_color[1],
		     clear_color[2],
		     clear_color[3]);
	glClear(GL_COLOR_BUFFER_BIT);

	if (!srgb_before_clear)
		glEnable(GL_FRAMEBUFFER_SRGB);

	glUseProgram(prog);
	glUniform4fv(color_location, 1, rect_color);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	/* Blend a rectangle into the right-hand half of the framebuffer */
	piglit_draw_rect(0.0f, -1.0f, 1.0f, 2.0f);

	glDisable(GL_BLEND);
	glDisable(GL_FRAMEBUFFER_SRGB);

	/* Sanity check that the blend didn't affect the left-hand
	 * side of the framebuffer where the rectangle wasn't drawn.
	 */
	if (srgb_before_clear) {
		pass = probe_srgb_color(0, 0,
					piglit_width / 2, piglit_height,
					clear_color) && pass;
	} else {
		pass = piglit_probe_rect_rgba(0, 0,
					      piglit_width / 2, piglit_height,
					      clear_color) && pass;
	}

	/* Calculate the expected blended color. The blending will be
	 * done in linear space so if GL_FRAMEBUFFER_SRGB was enabled
	 * before the clear then the SRGB conversions cancel out and
	 * don't affect the result. However if it wasn't enabled then
	 * the clear color will go through an SRGB→linear conversion
	 * before being used as one of the sources for the blend.
	 */
	for (i = 0; i < 4; i++) {
		if (i >= 3 || srgb_before_clear)
			fb_color = clear_color[i];
		else
			fb_color = piglit_srgb_to_linear(clear_color[i]);

		expected_color[i] = (fb_color * (1.0f - rect_color[3]) +
				     rect_color[i] * rect_color[3]);
	}

	pass = probe_srgb_color(piglit_width / 2, 0,
				piglit_width / 2, piglit_height,
				expected_color) && pass;

	/* Copy the test framebuffer into the winsys framebuffer so
	 * that something will be visible */
	glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
	glBlitFramebuffer(0, 0, piglit_width, piglit_height,
			  0, 0, piglit_width, piglit_height,
			  GL_COLOR_BUFFER_BIT,
			  GL_NEAREST);

	glBindFramebuffer(GL_FRAMEBUFFER, piglit_winsys_fbo);

	piglit_present_results();

	return pass;
}

enum piglit_result
piglit_display()
{
	bool pass = true;
	int i;

	for (i = 0; i < ARRAY_SIZE(clear_colors); i++) {
		pass = test_color(false, /*  srgb_before_clear */
				  clear_colors[i]) && pass;
		pass = test_color(true, /*  srgb_before_clear */
				  clear_colors[i]) && pass;
	}

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	GLuint rb;

	piglit_require_extension("GL_EXT_framebuffer_sRGB");
	piglit_require_extension("GL_ARB_framebuffer_sRGB");

	prog = piglit_build_simple_program(vertex_source, fragment_source);

	color_location = glGetUniformLocation(prog, "color");

	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glGenRenderbuffers(1, &rb);
	glBindRenderbuffer(GL_RENDERBUFFER, rb);
	glRenderbufferStorage(GL_RENDERBUFFER,
			      GL_SRGB8_ALPHA8,
			      piglit_width, piglit_height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER,
				  GL_COLOR_ATTACHMENT0,
				  GL_RENDERBUFFER,
				  rb);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) !=
	    GL_FRAMEBUFFER_COMPLETE) {
		fprintf(stderr, "FBO incomplete\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, piglit_winsys_fbo);
}
