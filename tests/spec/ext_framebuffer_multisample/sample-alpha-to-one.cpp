/*
 * Copyright © 2012 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
static float coverage[4];
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

#include "common.h"

/**
 * \file sample-alpha-to-one.cpp
 *
 * This test operates by computing the expected color values when
 * GL_SAMPLE_ALPHA_TO_ONE is disabled.
 *
 * Draw a test pattern with GL_SAMPLE_ALPHA_TO_ONE disabled and blit it to
 * the right half of window system framebuffer. Probe the right half of
 * framebuffer and compare with expected values.
 *
 * Compute the expected color values when GL_SAMPLE_ALPHA_TO_ONE is enabled.
 * Draws the same test pattern for the second time in multisample buffer with
 * GL_SAMPLE_ALPHA_TO_ONE enabled. Blits it in to left half of window system
 * framebuffer.
 *
 * Probe the left half of window syetem framebuffer and compare with expected
 * color values.
 *
 * Author: Anuj Phogat <anuj.phogat@gmail.com>
 */

PIGLIT_GL_TEST_MAIN(512 /*window_width*/,
		    256 /*window_height*/,
		    GLUT_DOUBLE | GLUT_RGBA | GLUT_ALPHA)

const int pattern_width = 256; const int pattern_height = 256;
static Fbo ms_fbo;
static GLint num_samples;
static float expected[4][4];
static GLbitfield buffer_to_test;

static const float bg_color[4] =
	{0.0, 0.0, 1.0, 0.8};

static const float color[4][4] = {
	/* Red */
	{1.0, 0.0, 0.0, 0.0},
	/* Green */
	{0.0, 1.0, 0.0, 0.25},
	/* Yellow */
	{1.0, 1.0, 0.0, 0.75},
	/* Cyan */
	{0.0, 1.0, 1.0, 1.0} };

static GLint prog;
static GLint color_loc;
static GLint depth_loc;

static const char *vert =
	"#version 130\n"
	"in vec2 pos;\n"
	"uniform float depth;\n"
	"void main()\n"
	"{\n"
	"  vec4 eye_pos = gl_ModelViewProjectionMatrix * vec4(pos, 0.0, 1.0);\n"
	"  gl_Position = vec4(eye_pos.xy, depth, 1.0);\n"
	"}\n";

static const char *frag =
	"#version 130\n"
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
draw_pattern(bool sample_alpha_to_one)
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

	if (sample_alpha_to_one)
		glEnable(GL_SAMPLE_ALPHA_TO_ONE);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_data[0]),
			      (void *) vertex_data);

	for (int i = 0; i < 4; ++i) {
		glUniform4fv(color_loc, 1, color[i]);
		glUniform1f(depth_loc, 0.0);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT,
			       (void *) (indices + 6 * i));
	}
	if (sample_alpha_to_one)
		glDisable (GL_SAMPLE_ALPHA_TO_ONE);
}

void
print_usage_and_exit(char *prog_name)
{
	printf("Usage: %s <num_samples>\n", prog_name);
	piglit_report_result(PIGLIT_FAIL);
}

void
compute_expected(void)
{

	/* Page 242 (page 258 of the PDF) of the OpenGL 3.0 spec says:
	 * Next, if SAMPLE ALPHA TO ONE is enabled, each alpha value is
	 * replaced by the maximum representable alpha value. Otherwise,
	 * the alpha values are not changed.
	 */
	for(int i = 0; i < 4; i++) {
		expected[i][0] = color[i][0];
		expected[i][1] = color[i][1];
		expected[i][2] = color[i][2];
		expected[i][3] = 1.0 ;
	}
}

bool probe_framebuffer_color(GLint x_offset, const float *expected)
{
	bool result = true;

	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	for(int i = 0; i< 4; i++) {
		result = piglit_probe_rect_rgba(x_offset + i * (pattern_width / 4),
						0,
						pattern_width / 4,
						pattern_height,
						expected + 4 * i)
			 && result;
	}
	return result;
}

bool
test_sample_alpha_to_one(void)
{
	bool result = true;
	compute_expected();
	/* Draw test pattern in multisample ms_fbo with GL_SAMPLE_COVERAGE
	 * enabled
	 */
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, ms_fbo.handle);
	draw_pattern(true);

	/* Blit ms_fbo to the left half of window system framebuffer. This
	 * is the test image.
	 */
	glBindFramebuffer(GL_READ_FRAMEBUFFER, ms_fbo.handle);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBlitFramebuffer(0, 0, pattern_width, pattern_height,
			  0, 0, pattern_width, pattern_height,
			  buffer_to_test, GL_NEAREST);

	/* Probe the left half of default framebuffer and compare to the
	 * expected values */
	if (buffer_to_test == GL_COLOR_BUFFER_BIT)
		result = probe_framebuffer_color(0, expected[0]) && result;

	result = piglit_check_gl_error(GL_NO_ERROR) && result;
	return result;
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

	piglit_require_gl_version(30);
	piglit_ortho_projection(pattern_width, pattern_height, GL_TRUE);

	/* Skip the test if num_samples > GL_MAX_SAMPLES */
	GLint max_samples;
	glGetIntegerv(GL_MAX_SAMPLES, &max_samples);

	if (num_samples > max_samples)
		piglit_report_result(PIGLIT_SKIP);

	ms_fbo.setup(FboConfig(num_samples, pattern_width, pattern_height));

	if (!piglit_check_gl_error(GL_NO_ERROR)) {
                printf("Error setting up frame buffer objects\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	buffer_to_test = GL_COLOR_BUFFER_BIT;
	shader_compile();
}

enum piglit_result
piglit_display()
{
	bool pass = true;
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	/* Draw test pattern in  multisample ms_fbo with GL_SAMPLE_ALPHA_TO_ONE
	 * disabled.
	 */
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, ms_fbo.handle);
	ms_fbo.set_viewport();
	draw_pattern(false);

	/* Blit ms_fbo to the right half of window system framebuffer. This
	 * is a reference image to see the visual difference when compared
	 * to the test image.
	 */
	glBindFramebuffer(GL_READ_FRAMEBUFFER, ms_fbo.handle);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBlitFramebuffer(0, 0, pattern_width, pattern_height,
			  pattern_width, 0, 2 * pattern_width, pattern_height,
			  buffer_to_test, GL_NEAREST);
	/* Probe the right half of default framebuffer and compare to the
	 * expected values */
	pass = probe_framebuffer_color(pattern_width, color[0]) && pass;

	/* Now test multisample fbo with GL_SAMPLE_ALPHA_TO_ONE enabled */
	pass = test_sample_alpha_to_one() && pass;

	if (!piglit_automatic)
		piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
