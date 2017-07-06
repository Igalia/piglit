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

#include "piglit-fbo.h"
using namespace piglit_util_fbo;

/**
 * \file polygon-stipple.cpp
 *
 * This test case just verifies the functionality of polygon stipple in
 * multisample FBO and assumes that MSAA accuracy test already passes.
 * Polygon stipple is expected to work exactly the same way on multisample
 * FBO as it works on a single sample FBO.
 *
 * This test operates by drawing a test pattern with GL_POLYGON_STIPPLE
 * enabled. Test pattern is first drawn in a single sample FBO to generate
 * a reference image in right half of default framebuffer.
 *
 * Draw the same test pattern in multisample buffer with GL_POLYGON_STIPPLE
 * enabled. Blit it in to left half of window system framebuffer.
 * This is the test image.
 *
 * Verify the accuracy of polygon stippling in multisample buffer by
 * comparing the two halves of default framebuffer.
 */

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 512;
	config.window_height = 256;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

const int pattern_width = 256; const int pattern_height = 256;

static Fbo ms_fbo, resolve_fbo;
static GLint num_samples;
static GLbitfield buffer_to_test;

static const float bg_color[4] =
	{0.0, 0.0, 1.0, 1.0};

static const float color[4][4] = {
	/* Red */
	{1.0, 0.0, 0.0, 1.0},
	/* Green */
	{0.0, 1.0, 0.0, 1.0},
	/* Yellow */
	{1.0, 1.0, 0.0, 1.0},
	/* Cyan */
	{0.0, 1.0, 1.0, 1.0} };

static GLint prog;
static GLint color_loc;
static GLint depth_loc;

static GLubyte stipple_pattern[] =
{
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xc0, 0x00, 0x00, 0x03, 0xc0, 0x00, 0x00, 0x03,
	0xcf, 0xff, 0xff, 0xf3, 0xcf, 0xff, 0xff, 0xf3,
	0xcc, 0x00, 0x00, 0x33, 0xcc, 0x00, 0x00, 0x33,
	0xcc, 0xff, 0xff, 0x33, 0xcc, 0xff, 0xff, 0x33,
	0xcc, 0xc0, 0x03, 0x33, 0xcc, 0xc0, 0x03, 0x33,
	0xcc, 0xcf, 0xf3, 0x33, 0xcc, 0xcf, 0xf3, 0x33,
	0xcc, 0xcf, 0xf3, 0x33, 0xcc, 0xcf, 0xf3, 0x33,
	0xcc, 0xcf, 0xf3, 0x33, 0xcc, 0xcf, 0xf3, 0x33,
	0xcc, 0xcf, 0xf3, 0x33, 0xcc, 0xcf, 0xf3, 0x33,
	0xcc, 0xc0, 0x03, 0x33, 0xcc, 0xc0, 0x03, 0x33,
	0xcc, 0xff, 0xff, 0x33, 0xcc, 0xff, 0xff, 0x33,
	0xcc, 0x00, 0x00, 0x33, 0xcc, 0x00, 0x00, 0x33,
	0xcf, 0xff, 0xff, 0xf3, 0xcf, 0xff, 0xff, 0xf3,
	0xc0, 0x00, 0x00, 0x03, 0xc0, 0x00, 0x00, 0x03,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
};

static const char *vert =
	"#version 120\n"
	"attribute vec2 pos;\n"
	"uniform float depth;\n"
	"void main()\n"
	"{\n"
	"  vec4 eye_pos = gl_ModelViewProjectionMatrix * vec4(pos, 0.0, 1.0);\n"
	"  gl_Position = vec4(eye_pos.xy, depth, 1.0);\n"
	"}\n";

static const char *frag =
	"#version 120\n"
	"uniform vec4 color;\n"
	"void main()\n"
	"{\n"
	"  gl_FragColor = color;\n"
	"}\n";

void
shader_compile()
{
	/* Compile program */
	GLint vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vert);
	GLint fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, frag);
	prog = piglit_link_simple_program(vs, fs);

	if (!piglit_link_check_status(prog)) {
		piglit_report_result(PIGLIT_FAIL);
	}

	glBindAttribLocation(prog, 0, "pos");
	glEnableVertexAttribArray(0);

	/* Set up uniforms */
	glUseProgram(prog);
	color_loc = glGetUniformLocation(prog, "color");
	depth_loc = glGetUniformLocation(prog, "depth");
}

void
draw_pattern(void)
{
	float vertex_data[10][2] = {
		{ 0,			 0 },
		{ 0,			 pattern_height },
		{ pattern_width / 4,	 pattern_height },
		{ pattern_width / 4,	 0 },
		{ pattern_width / 2,	 pattern_height },
		{ pattern_width / 2,	 0 },
		{ 3 * pattern_width / 4, pattern_height },
		{ 3 * pattern_width / 4, 0 },
		{ pattern_width,	 pattern_height },
		{ pattern_width,	 0 },
	};

	unsigned int indices[24] = {0, 1, 2, 0, 2, 3,
				    3, 2, 4, 3, 4, 5,
				    5, 4, 6, 5, 6, 7,
				    7, 6, 8, 7, 8, 9};
	glUseProgram(prog);
	glClearColor(bg_color[0], bg_color[1],
		     bg_color[2], bg_color[3]);
	glClear(buffer_to_test);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_data[0]),
			      (void *) vertex_data);
	glUniform1f(depth_loc, 0.0);

	for (int i = 0; i < 4; ++i) {
		glUniform4fv(color_loc, 1, color[i]);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT,
			       (void *) (indices + 6 * i));
	}
}

bool
test_polygon_stipple()
{
	bool result = true;
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, ms_fbo.handle);
	draw_pattern();

	/* Blit ms_fbo to resolve_fbo to resolve multisample buffer */
	glBindFramebuffer(GL_READ_FRAMEBUFFER, ms_fbo.handle);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolve_fbo.handle);
	glBlitFramebuffer(0, 0, pattern_width, pattern_height,
			  0, 0, pattern_width, pattern_height,
			  buffer_to_test, GL_NEAREST);

	/* Blit resolve_fbo to the left half of window system framebuffer.
	 * This is the test image.
	 */
	glBindFramebuffer(GL_READ_FRAMEBUFFER, resolve_fbo.handle);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
	glBlitFramebuffer(0, 0, pattern_width, pattern_height,
			  0, 0, pattern_width, pattern_height,
			  buffer_to_test, GL_NEAREST);

	/* Check that the left and right halves of the screen match */
	glBindFramebuffer(GL_READ_FRAMEBUFFER, piglit_winsys_fbo);
	result = piglit_probe_rect_halves_equal_rgba(0, 0, piglit_width,
						     piglit_height)
		 && result;

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
	result = piglit_check_gl_error(GL_NO_ERROR) && result;
	return result;
}

void
print_usage_and_exit(char *prog_name)
{
	printf("Usage: %s <num_samples>\n", prog_name);
	piglit_report_result(PIGLIT_FAIL);
}

void
piglit_init(int argc, char **argv)
{
	if (argc < 2)
		print_usage_and_exit(argv[0]);
	{
		char *endptr = NULL;
		num_samples = strtol(argv[1], &endptr, 0);
		if (endptr != argv[1] + strlen(argv[1]))
			print_usage_and_exit(argv[0]);
	}

	piglit_require_gl_version(21);
	piglit_require_extension("GL_ARB_framebuffer_object");
	piglit_require_extension("GL_ARB_vertex_array_object");

	piglit_ortho_projection(pattern_width, pattern_height, GL_TRUE);

	/* Skip the test if num_samples > GL_MAX_SAMPLES */
	GLint max_samples;
	glGetIntegerv(GL_MAX_SAMPLES, &max_samples);
	if (num_samples > max_samples)
		piglit_report_result(PIGLIT_SKIP);

	ms_fbo.setup(FboConfig(num_samples, pattern_width, pattern_height));
	resolve_fbo.setup(FboConfig(0, pattern_width, pattern_height));

	buffer_to_test = GL_COLOR_BUFFER_BIT;
	shader_compile();
	glEnable(GL_POLYGON_STIPPLE);
	glPolygonStipple(stipple_pattern);

}

enum piglit_result
piglit_display()
{
	bool pass = true;
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	/* Draw test pattern in single sample resolve_fbo with GL_POLYGON_STIPPLE
	 * enabled.
	 */
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolve_fbo.handle);
	resolve_fbo.set_viewport();
	draw_pattern();

	/* Blit resolve_fbo to the right half of window system framebuffer. This
	 * is a reference image.
	 */
	glBindFramebuffer(GL_READ_FRAMEBUFFER, resolve_fbo.handle);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
	glBlitFramebuffer(0, 0, pattern_width, pattern_height,
			  pattern_width, 0, 2 * pattern_width, pattern_height,
			  buffer_to_test, GL_NEAREST);

	/* Test with multisample FBO and GL_POLYGON_STIPPLE enabled */
	pass = test_polygon_stipple() && pass;

	if (!piglit_automatic &&
	    buffer_to_test != GL_DEPTH_BUFFER_BIT)
		piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
