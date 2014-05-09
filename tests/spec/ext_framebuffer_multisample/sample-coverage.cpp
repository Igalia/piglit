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

#include "piglit-test-pattern.h"
#include "piglit-fbo.h"
using namespace piglit_util_fbo;
using namespace piglit_util_test_pattern;

/**
 * \file sample-coverage.cpp
 *
 * Verify glSampleCoverage() with and without coverage mask invert.
 *
 * This test operates by drawing a test pattern to multisample_fbo with
 * GL_SAMPLE_COVERAGE disabled.
 *
 * Blit the multisample_fbo to top half of window system framebuffer. This
 * is used as reference image to visually compare the difference caused by
 * sample coverage value.
 *
 * Compute the expected color values based on the coverage value used to
 * draw the test pattern and status of coverage mask invert flag.
 *
 * Clear the multisample framebuffer to a unique color. Draw the
 * same test pattern in multisample buffer with GL_SAMPLE_COVERAGE enabled.
 * Resolve the multisample FBO by  blitting it to a single sample FBO. Blit
 * the resolve_fbo to bottom half of window system framebuffer. This is our
 * test image.
 *
 * Probe the rectangles in bottom half of window system framebuffer and
 * compare with expected color values. OpenGL 3.0 specification intends
 * to allow (but not require) the implementation to produce a dithering
 * effect when the coverage value is not a strict multiple of 1/num_samples.
 * We will skip computing expected values and probing for such rectangles.
 * They are drawn just to look for dithering by human inspection.
 *
 * This test can be executed in inverted / non-inverted modes using command
 * line options.
 *
 * Note: glSampleCoverage() takes effect in the graphics pipeline before
 * the point where the output of the fragment shader is split into the
 * various buffers. So it's very likely that if glSampleCoverage() works
 * properly for color buffers, it will work properly for depth and stencil
 * buffers too.
 *
 * Author: Anuj Phogat <anuj.phogat@gmail.com>
 */

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 512;
	config.window_height = 256;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

const int pattern_width = 512; const int pattern_height = 128;
static Fbo ms_fbo, resolve_fbo;
static GLbitfield buffer_to_test;

static bool coverage_invert = false;
static float *cov = NULL;
static float *color = NULL;
static float *expected = NULL;

static int num_samples;
static int num_rects;
static int prog;
static int color_loc;
static int depth_loc;

static const float bg_color[4] = { 0.4, 0.6, 0.0, 0.8 };

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
draw_pattern(bool sample_coverage)
{
	glUseProgram(prog);
	glClearColor(bg_color[0], bg_color[1],
		     bg_color[2], bg_color[3]);

	glClear(buffer_to_test);
	if (sample_coverage)
		glEnable (GL_SAMPLE_COVERAGE);

	unsigned int indices[6] = {0, 1, 2, 0, 2, 3};

	for (int i = 0; i < num_rects; ++i) {

		float vertex_data[4][2] = {
		{ 0.0f + i * (pattern_width / num_rects),   0.0f	   },
		{ 0.0f + i * (pattern_width / num_rects),   pattern_height },
		{ (i + 1.0f) * (pattern_width / num_rects), pattern_height },
		{ (i + 1.0f) * (pattern_width / num_rects), 0.0f	   } };

		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
				      sizeof(vertex_data[0]),
				      (void *) vertex_data);

		if(sample_coverage) {
			if(coverage_invert)
				glSampleCoverage (cov[i], GL_TRUE);
			else
				glSampleCoverage (cov[i], GL_FALSE);
		}

		glUniform4fv(color_loc, 1, (color + i * 4));
		glUniform1f(depth_loc, 0.0f);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT,
			       (void *) indices);
	}
	if(sample_coverage)
		glDisable (GL_SAMPLE_COVERAGE);
}

void
print_usage_and_exit(char *prog_name)
{
	printf("Usage: %s <num_samples> <mode> \n"
	       "  where <mode> is one of:\n"
	       "    inverted\n"
	       "    non-inverted\n",
	       prog_name);

	piglit_report_result(PIGLIT_FAIL);
}

void
compute_expected(void)
{
	int i, j;

	/* Sample coverage doesn't affect single sample FBO */
	if(num_samples == 0) {
		for(i = 0; i < 4; i++)
			expected[i] = color[i];
		return;
	}

	float *coverage = new float[num_rects];

	if(coverage_invert) {
		for (i = 0; i < num_rects; i++)
			coverage[i] = 1 - cov[i];
	}
	else
		for (i = 0; i < num_rects; i++)
			coverage[i] = cov[i];

	/* Coverage value decides the number of samples in multisample buffer
	 * covered by an incoming fragment, which will then receive the fragment
	 * data. When the multisample buffer is resolved it will be blended
	 * with the background color which will be written to the remaining
	 * samples.
	 * Page 254 (page 270 of the PDF) of the OpenGL 3.0 spec says:
	 * "The method of combination is not specified, though a simple average
	 * computed independently for each color component is recommended."
	 */
	if(buffer_to_test == GL_COLOR_BUFFER_BIT) {
		for (i = 0; i < num_rects; i++) {

			float samples_used = coverage[i] * num_samples;
			/* Exepected color values are computed only for integer
			 * number of samples_used
			 */
			if(samples_used == (int)samples_used) {

				for(j =0; j < 4; j++)
					expected[i * 4 + j] =
					color[i * 4 + j] * coverage[i] +
					bg_color[j] * (1 - coverage[i]);
			}
		}
	}

	delete [] coverage;
}

bool
probe_framebuffer_color(void)
{
	bool result = true;

	float *coverage = new float[num_rects];

	if(coverage_invert) {
		for (int i = 0; i < num_rects; i++)
			coverage[i] = 1 - cov[i];
	}
	else
		for (int i = 0; i < num_rects; i++)
			coverage[i] = cov[i];

	glBindFramebuffer(GL_READ_FRAMEBUFFER, piglit_winsys_fbo);

	for (int i = 0; i < num_rects; i++) {
		float samples_used = coverage[i] * num_samples;

		/* Only probe rectangles with coverage value which is a strict
		 *  multiple of 1 / num_samples.
		 */
		if(samples_used == (int)samples_used) {
			result = piglit_probe_rect_rgba(
				 i * (pattern_width / num_rects),
				 0,
				 pattern_width / num_rects,
				 pattern_height,
				 expected + i * 4)
				 && result;
		}
	}

	delete [] coverage;

	return result;
}

bool
test_sample_coverage(void)
{
	bool result = true;
	compute_expected();

	/* Now draw test pattern in multisample ms_fbo with GL_SAMPLE_COVERAGE
	 * enabled
	 */
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, ms_fbo.handle);
	draw_pattern(true /* sample_coverage */);

	/* Blit ms_fbo to resolve_fbo to resolve multisample buffer */
	glBindFramebuffer(GL_READ_FRAMEBUFFER, ms_fbo.handle);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolve_fbo.handle);
	glBlitFramebuffer(0, 0, pattern_width, pattern_height,
			  0, 0, pattern_width, pattern_height,
			  buffer_to_test, GL_NEAREST);

	/* Blit resolve_fbo to the bottom half of window system framebuffer.
	 * This is the test image.
	 */
	glBindFramebuffer(GL_READ_FRAMEBUFFER, resolve_fbo.handle);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
	glBlitFramebuffer(0, 0, pattern_width, pattern_height,
			  0, 0, pattern_width, pattern_height,
			  buffer_to_test, GL_NEAREST);

	/* Probe the bottom half of default framebuffer and compare to the
	 * expected values */
	if (buffer_to_test == GL_COLOR_BUFFER_BIT)
		result = probe_framebuffer_color() && result;

	result = piglit_check_gl_error(GL_NO_ERROR) && result;
	return result;
}

void
allocate_data_arrays(void)
{
	/* Draw 2N + 1 rectangles for N samples, each with a unique color
	 * and coverage value
	 */
	num_rects = 2 * num_samples + 1;

	/* Allocate data arrays based on number of samples used */
	color = (float *) malloc(num_rects * 4 * sizeof(float));
	cov = (float *) malloc(num_rects * sizeof(float));
	expected = (float *) malloc(num_rects * 4 * sizeof(float));

	for(int i = 0; i < num_rects; i++) {
		color[i * 4 + 0] = (sin((float)(i * 4 + 0)) + 1) / 2;
		color[i * 4 + 1] = (sin((float)(i * 4 + 1)) + 1) / 2;
		color[i * 4 + 2] = (sin((float)(i * 4 + 2)) + 1) / 2;
		color[i * 4 + 3] = (sin((float)(i * 4 + 3)) + 1) / 2;

		cov[i] = i * (1.0 / (2 * num_samples));
	}
}

void
free_data_arrays(void)
{
	if(color != NULL) {
		free(color);
		color = NULL;
	}
	if(cov != NULL) {
		free(cov);
		cov = NULL;
	}
	if(expected != NULL) {
		free(expected);
		expected = NULL;
	}
}

void
piglit_init(int argc, char **argv)
{
	int samples;
	if (argc < 3)
		print_usage_and_exit(argv[0]);
	{
		char *endptr = NULL;
		samples = strtol(argv[1], &endptr, 0);
		if (endptr != argv[1] + strlen(argv[1]))
			print_usage_and_exit(argv[0]);
	}

	for (int i = 2; i < argc; ++i) {
		if (strcmp(argv[i], "inverted") == 0)
			coverage_invert = true;
		else if (strcmp(argv[i], "non-inverted") == 0)
			coverage_invert = false;
	}

	piglit_require_gl_version(21);
	piglit_require_extension("GL_ARB_framebuffer_object");
	piglit_require_extension("GL_ARB_vertex_array_object");

	piglit_ortho_projection(pattern_width, pattern_height, GL_TRUE);

	/* Skip the test if samples > GL_MAX_SAMPLES */
	GLint max_samples;
	glGetIntegerv(GL_MAX_SAMPLES, &max_samples);

	if (samples > max_samples)
		piglit_report_result(PIGLIT_SKIP);

	/* Setup frame buffer objects with required configuration */
	ms_fbo.setup(FboConfig(samples, pattern_width, pattern_height));
	resolve_fbo.setup(FboConfig(0, pattern_width, pattern_height));

	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		printf("Error setting up frame buffer objects\n");
			piglit_report_result(PIGLIT_FAIL);
	}

	/* Query the number of samples used in ms_fbo. OpenGL implementation
	 * may create FBO with more samples per pixel than what is requested.
	 */
	glBindRenderbuffer(GL_RENDERBUFFER, ms_fbo.color_rb[0]);
	glGetRenderbufferParameteriv(GL_RENDERBUFFER,
				     GL_RENDERBUFFER_SAMPLES,
				     &num_samples);

	buffer_to_test = GL_COLOR_BUFFER_BIT;
	shader_compile();
}

enum piglit_result
piglit_display()
{
	bool pass = true;
	allocate_data_arrays();

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(buffer_to_test);

	/* Draw test pattern in  multisample ms_fbo with GL_SAMPLE_COVERAGE
	 * disabled.
	 */
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, ms_fbo.handle);
	ms_fbo.set_viewport();
	draw_pattern(false /* sample_coverage */);

	/* Blit ms_fbo to the top half of window system framebuffer. This
	 * is our reference image to visually compare the effect of MSAA with
	 * sample coverage.
	 */
	glBindFramebuffer(GL_READ_FRAMEBUFFER, ms_fbo.handle);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
	glBlitFramebuffer(0, 0,
			  pattern_width, pattern_height,
			  0, pattern_height,
			  pattern_width, 2 * pattern_height,
			  buffer_to_test, GL_NEAREST);

	pass = test_sample_coverage() && pass;

	/* Free the memory allocated for data arrays */
	free_data_arrays();

	if (!piglit_automatic &&
	    buffer_to_test != GL_DEPTH_BUFFER_BIT)
		piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
