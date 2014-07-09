/*
 * Copyright © 2013 Intel Corporation
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

/** \file
 *
 * Verify that alpha blending works properly in multisample
 * framebuffers even if it is preceded by normal (non-blended)
 * rendering.
 *
 * In the fix for https://bugs.freedesktop.org/show_bug.cgi?id=53077,
 * Mesa's i965 driver must convert a compressed multisampled buffer to
 * an uncompressed buffer the first time it notices that alpha
 * blending is being performed on the buffer.  This test verifies that
 * the conversion happens correctly.  It specifically exercises pixels
 * that are in the following states at the time of conversion:
 *
 * - Clear
 * - Fully covered
 * - Partially covered, partially clear
 * - Partially covered by one color, partially by another color
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 30;
	config.supports_gl_core_version = 31;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

const char *vs_text =
	"#version 130\n"
	"in vec4 piglit_vertex;\n"
	"void main()\n"
	"{\n"
	"  gl_Position = piglit_vertex;\n"
	"}\n";

const char *fs_text =
	"#version 130\n"
	"uniform vec4 color;\n"
	"void main()\n"
	"{\n"
	"  gl_FragColor = color;\n"
	"}\n";

static GLuint prog, singlesampled_fbo;
static GLint color_loc;
static int num_samples;


static void
print_usage_and_exit(const char *prog_name)
{
	printf("Usage: %s <num_samples>\n", prog_name);
	piglit_report_result(PIGLIT_FAIL);
}


void
piglit_init(int argc, char **argv)
{
	GLuint rb;
	char *endptr;
	GLint max_samples;

	if (argc != 2)
		print_usage_and_exit(argv[0]);
	endptr = NULL;
	num_samples = strtol(argv[1], &endptr, 0);
	if (endptr != argv[1] + strlen(argv[1]))
		print_usage_and_exit(argv[0]);

	/* Test assumes that glSampleCoverage(0.5) will yield exactly
	 * 50% blending; this only works if num_samples is even and
	 * greater than zero.
	 */
	if (num_samples <= 0 || num_samples % 2 != 0) {
		printf("num_samples must be even and greater than zero.\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	/* Skip the test if num_samples > GL_MAX_SAMPLES */
	glGetIntegerv(GL_MAX_SAMPLES, &max_samples);
	if (num_samples > max_samples) {
		printf("num_samples = %d requested, but only %d supported.\n",
		       num_samples, max_samples);
		piglit_report_result(PIGLIT_SKIP);
	}

	prog = piglit_build_simple_program(vs_text, fs_text);
	color_loc = glGetUniformLocation(prog, "color");

	/* Create the single-sampled fbo.  We only need to create this
	 * once, since it isn't subject to the bugfix.
	 */
	glGenFramebuffers(1, &singlesampled_fbo);
	glGenRenderbuffers(1, &rb);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, singlesampled_fbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rb);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER,
					 0 /* samples */,
					 GL_RGBA8 /* internalformat */,
					 piglit_width, piglit_height);
	glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
				  GL_RENDERBUFFER, rb);
	if (glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER)
	    != GL_FRAMEBUFFER_COMPLETE) {
		printf("Single-sampled framebuffer incomplete\n");
		piglit_report_result(PIGLIT_FAIL);
	}
}


enum piglit_result
piglit_display(void)
{
	GLuint msaa_fbo, rb;
	bool pass = true;
	GLfloat expected_ul[] = { 0.25, 0.0,  0.25, 0.75 };
	GLfloat expected_ur[] = { 0.0,  0.25, 0.25, 0.75 };
	GLfloat expected_ll[] = { 0.5,  0.0,  0.0,  0.75 };
	GLfloat expected_lr[] = { 0.0,  0.5,  0.0,  0.75 };

	/* Create a multisampled framebuffer.  We need to do this here
	 * (rather than in piglit_init()) because the bugfix we are
	 * verifying only converts any given buffer once; we want to
	 * make sure we trigger the bugfix for every call to
	 * piglit_display().
	 */
	glGenFramebuffers(1, &msaa_fbo);
	glGenRenderbuffers(1, &rb);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, msaa_fbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rb);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER,
					 num_samples,
					 GL_RGBA8 /* internalformat */,
					 piglit_width, piglit_height);
	glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
				  GL_RENDERBUFFER, rb);
	if (glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER)
	    != GL_FRAMEBUFFER_COMPLETE) {
		printf("MSAA framebuffer incomplete\n");
		pass = false;
	}

	/* Clear the framebuffer to red. */
	glClearColor(1, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	/* Paint the right half of the framebuffer green. */
	glUseProgram(prog);
	glDisable(GL_SAMPLE_COVERAGE);
	glDisable(GL_BLEND);
	glUniform4f(color_loc, 0, 1, 0, 1);
	piglit_draw_rect(0, -1, 1, 2);

	/* Paint the top half of the framebuffer blue, using 50%
	 * sample coverage.
	 */
	glEnable(GL_SAMPLE_COVERAGE);
	glSampleCoverage(0.5, GL_FALSE);
	glUniform4f(color_loc, 0, 0, 1, 1);
	piglit_draw_rect(-1, 0, 2, 1);

	/* Paint black over the entire framebuffer, using 50% alpha
	 * blending.
	 */
	glDisable(GL_SAMPLE_COVERAGE);
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glUniform4f(color_loc, 0, 0, 0, 0.5);
	piglit_draw_rect(-1, -1, 2, 2);

	/* Blit to the single-sampled fbo to force a multisample
	 * resolve.  Note that we don't blit directly to the screen
	 * because the screen may be using SRGB, which might trigger
	 * the driver to do something other than linear averaging when
	 * resolving the samples.
	 */
	glBindFramebuffer(GL_READ_FRAMEBUFFER, msaa_fbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, singlesampled_fbo);
	glBlitFramebuffer(0, 0, piglit_width, piglit_height,
			  0, 0, piglit_width, piglit_height,
			  GL_COLOR_BUFFER_BIT, GL_NEAREST);

	/* Blit to the screen for ease in diagnosing failures. */
	glBindFramebuffer(GL_READ_FRAMEBUFFER, singlesampled_fbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
	glBlitFramebuffer(0, 0, piglit_width, piglit_height,
			  0, 0, piglit_width, piglit_height,
			  GL_COLOR_BUFFER_BIT, GL_NEAREST);

	/* Check that the color is correct in each quadrant. */
	pass = piglit_probe_rect_rgba(0, 0, piglit_width / 2,
				      piglit_height / 2, expected_ll) && pass;
	pass = piglit_probe_rect_rgba(piglit_width - piglit_width / 2, 0,
				      piglit_width / 2, piglit_height / 2,
				      expected_lr) && pass;
	pass = piglit_probe_rect_rgba(0, piglit_height - piglit_height / 2,
				      piglit_width / 2, piglit_height / 2,
				      expected_ul) && pass;
	pass = piglit_probe_rect_rgba(piglit_width - piglit_width / 2,
				      piglit_height - piglit_height / 2,
				      piglit_width / 2, piglit_height / 2,
				      expected_ur) && pass;

	piglit_present_results();

	/* Clean up */
	glDeleteRenderbuffers(1, &rb);
	glDeleteFramebuffers(1, &msaa_fbo);

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
