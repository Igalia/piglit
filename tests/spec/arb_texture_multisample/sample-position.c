/*
 * Copyright © The Piglit Project 2013
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
 *
 * Authors: Christoph Bumiller
 */
#include "piglit-util-gl.h"

/* File: sample-position.c
 *
 * Tests whether the sample positions reported by the driver via
 * glGetMultisamplefv correspond to the actually positions used when rendering.
 *
 * This test draws creates a 1x1 multisample texture and renders a triangle
 * covering all of the render target starting at a specific x/y offset from
 * the left/bottom to test the x/y coordinate.
 * After each such draw, the value of all the samples is recorded into a buffer
 * via transform feedback, and the offset is increased slightly.
 * In the end, the buffer is mapped to check if the correct samples were
 * covered.
 */

PIGLIT_GL_TEST_CONFIG_BEGIN

    config.supports_gl_compat_version = 30;
    config.window_visual = PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

#define NUM_STEPS   32
#define MAX_SAMPLES 32

const float inc = 2.0f / (float)NUM_STEPS;

static GLuint buf;
static GLuint tex;
static GLuint fbo;
static GLuint prog_rd, prog_wr;

static int samples = 0;

static void
draw_ms_triangle(int step, int axis)
{
	float p = -1.0f + inc * step;
	float x = axis ? -1.0f : p;
	float y = axis ? p : -1.0f;

	float tri[3][2] = { { x, y }, { x + 4.0f, y }, { x, y + 4.0f } };

	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(prog_wr);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, tri);
	glEnableVertexAttribArray(0);

	glDrawArrays(GL_TRIANGLES, 0, 3);

	glDisableVertexAttribArray(0);
}

static void
read_samples(int step, int axis)
{
	int index = step + axis * NUM_STEPS;

	glBindFramebuffer(GL_FRAMEBUFFER, piglit_winsys_fbo);

	glUseProgram(prog_rd);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, tex);

	glBindBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, 0, buf,
			  MAX_SAMPLES * sizeof(float) * index,
			  MAX_SAMPLES * sizeof(float));

	glBeginTransformFeedback(GL_POINTS);
	glDrawArrays(GL_POINTS, 0, 1);
	glEndTransformFeedback();
}

enum piglit_result
piglit_display(void)
{
	int pos_determined[MAX_SAMPLES][2];
	float pos_expected[MAX_SAMPLES][2];
	float pos_observed[MAX_SAMPLES][2];
	float *res;
	int i, s;
	enum piglit_result result = PIGLIT_PASS;

	memset(pos_determined, 0, sizeof(pos_determined));

	for (s = 0; s < MAX_SAMPLES; ++s)
		pos_observed[s][0] = pos_observed[s][1] = -16384.0f;

	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	for (i = 0; i < samples; ++i)
		glGetMultisamplefv(GL_SAMPLE_POSITION, i, &pos_expected[i][0]);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	for (i = 0; i < NUM_STEPS; ++i) {
		draw_ms_triangle(i, 0);
		read_samples(i, 0);
	}
	for (i = 0; i < NUM_STEPS; ++i) {
		draw_ms_triangle(i, 1);
		read_samples(i, 1);
	}

	/* Determine sample positions from observed coverage:
	 */
	res = glMapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, GL_READ_ONLY);

	for (i = 0; i < NUM_STEPS; ++i) {
		/* x/y offset of previous step == position of samples that
		 * are not covered anymore in this step:
		 */
		float p = (float)(i - 1) / (float)NUM_STEPS;

		int base_x = MAX_SAMPLES * i;
		int base_y = MAX_SAMPLES * (i + NUM_STEPS);

		for (s = 0; s < samples; ++s) {
			if (!pos_determined[s][0] && res[base_x + s] != 1.0f) {
				pos_determined[s][0] = 1;
				pos_observed[s][0] = p;
			}
			if (!pos_determined[s][1] && res[base_y + s] != 1.0f) {
				pos_determined[s][1] = 1;
				pos_observed[s][1] = p;
			}
		}
	}
	glUnmapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER);

	/* Compare observed sample positions with reported ones:
	 */
	for (s = 0; s < samples; ++s) {
		if (pos_observed[s][0] != pos_expected[s][0]) {
			result = PIGLIT_FAIL;
			fprintf(stderr, "sample %i position incorrect (x axis): "
				"observed %f, expected %f\n", s,
				pos_observed[s][0], pos_expected[s][0]);
		}
		if (pos_observed[s][1] != pos_expected[s][1]) {
			result = PIGLIT_FAIL;
			fprintf(stderr, "sample %i position incorrect (y axis): "
				"observed %f, expected %f\n", s,
				pos_observed[s][1], pos_expected[s][1]);
		}
	}

	return result;
}

static const char *vsSourceWr = "#version 130 \n"
	"attribute vec2 pos;\n"
	"void main() { \n"
	"   gl_Position = vec4(pos.x, pos.y, 0.0, 1.0); \n"
	"} \n";

static const char *vsSourceRd = "#version 130 \n"
	"#extension GL_ARB_texture_multisample : require\n"
	"uniform sampler2DMS tex; \n"
	"out float sample[32]; \n" /* 32 == MAX_SAMPLES */
	"void main() { \n"
	"   int i; \n"
	"   for (i = 0; i < 32; ++i) \n"
	"      sample[i] = texelFetch(tex, ivec2(0, 0), i).g; \n"
	"   gl_Position = vec4(0.0); \n"
	"} \n";

static const char *fsSource = "#version 130 \n"
	"void main() { \n"
	"   gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0); \n"
	"} \n";

void
piglit_init(int argc, char **argv)
{
	static const char *varyings[] = { "sample" };
	GLuint vs, fs;
	GLuint max_samples;

	piglit_require_extension("GL_ARB_texture_multisample");
	piglit_require_extension("GL_EXT_transform_feedback");

	if (argc < 2) {
		fprintf(stderr, "%s <sample_count>\n", argv[0]);
		piglit_report_result(PIGLIT_FAIL);
	}
	samples = strtoul(argv[1], NULL, 0);

	glGetIntegerv(GL_MAX_SAMPLES, (GLint *)&max_samples);
	if (samples > max_samples)
		piglit_report_result(PIGLIT_SKIP);

	fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, fsSource);
	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vsSourceRd);
	prog_rd = piglit_link_simple_program(vs, fs);
	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vsSourceWr);
	prog_wr = piglit_link_simple_program(vs, fs);

	glTransformFeedbackVaryings(prog_rd,
				    1, varyings, GL_INTERLEAVED_ATTRIBS);
	glLinkProgram(prog_rd);
	glGenBuffers(1, &buf);
	glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, buf);
	glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER,
		     MAX_SAMPLES * sizeof(float) * NUM_STEPS * 2, NULL,
		     GL_STREAM_DRAW);

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, tex);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples,
				GL_RGBA8,
				1, 1, GL_TRUE);
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			       GL_TEXTURE_2D_MULTISAMPLE, tex, 0);

	glUseProgram(prog_rd);
	glUniform1i(glGetUniformLocation(prog_rd, "tex"), 0);

	assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) ==
	       GL_FRAMEBUFFER_COMPLETE);

	glViewport(0, 0, 1, 1);

	glEnable(GL_MULTISAMPLE);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);
}
