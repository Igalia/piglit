/*
 * Copyright 2014 VMware, Inc.
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

/* File: texelfetch.c
 *
 * Tests that texelFetch() really gets samples from different sample positions.
 * First, we draw a triangle into a MSAA texture/FBO.
 * Then, for each sample location, we draw a texture quad, using texelFetch()
 * to grab a particular sample.  We read back the colors to a temporary image.
 * Finally, we check that the colors in the temp images are different for at
 * least some of the pixels/samples.
 *
 * Brian Paul
 * July 2014
 */


#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

    config.supports_gl_compat_version = 30;
    config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;
    config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

/* Debug helpers */
#define MSAA 1
#define DEBUG_WITH_BLIT 0
#define DISPLAY_AVERAGE 0

#define MAX_SAMPLES 32

static const char *vs_src_draw =
	"#version 130 \n"
	"in vec4 piglit_vertex;\n"
	"void main() { \n"
	"   gl_Position = piglit_vertex; \n"
	"} \n";

static const char *fs_src_draw =
	"#version 130 \n"
	"out vec4 frag_color; \n"
	"void main() { \n"
	"   frag_color = vec4(0.9, 0.8, 0, 1); \n"
	"} \n";

static const char *vs_src_readback =
	"#version 130 \n"
	"in vec4 piglit_vertex; \n"
	"in vec2 piglit_texcoord;\n"
	"out vec2 coord_fs;\n"
	"void main() { \n"
	"   gl_Position = piglit_vertex; \n"
	"   coord_fs = piglit_texcoord; \n"
	"} \n";

static const char *fs_src_readback =
	"#version 130 \n"
	"#extension GL_ARB_texture_multisample : require\n"
#if MSAA
	"uniform sampler2DMS tex; \n"
#else
	"uniform sampler2D tex; \n"
#endif
	"uniform int samplePos; \n"
	"in vec2 coord_fs; \n"
	"out vec4 frag_color; \n"
	"void main() { \n"
#if MSAA
	"   frag_color = texelFetch(tex, ivec2(coord_fs), samplePos); \n"
#else
	"   frag_color = texture2D(tex, coord_fs / 31.0); \n"
#endif
	"} \n";

static GLuint tex;
static GLuint fbo;
static GLuint readback_prog, draw_prog;
static GLint sample_pos_uniform;
static int num_samples = 0;


enum piglit_result
piglit_display(void)
{
	static const GLfloat quad_verts[4][4] = {
		{  0.8,  0.1, 0, 1 },
		{  0.1,  1.0, 0, 1 },
		{ -0.1, -1.0, 0, 1 },
		{ -0.8, -0.1, 0, 1 },
	};
	unsigned i, j, num_diffs;
	GLfloat *images[MAX_SAMPLES], *average;

	for (i = 0; i < num_samples; i++) {
		images[i] = malloc(32 * 32 * 4 * sizeof(GLfloat));
	}
	average = malloc(32 * 32 * 4 * sizeof(GLfloat));

	glViewport(0, 0, 32, 32);

	/* Draw triangle into MSAA texture */
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glClearColor(0.5f, 0.5f, 0.5f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	piglit_check_gl_error(GL_NO_ERROR);

	glUseProgram(draw_prog);
	piglit_draw_rect_from_arrays(quad_verts, NULL, false, 1);

	piglit_check_gl_error(GL_NO_ERROR);

	/* Read back samples:
	 * Draw textured quad into main framebuffer using texture samples
	 * from the MSAA texture.  Then use glReadPixels to get the samples.
	 */
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClearColor(0.25, 0.25, 0.25, 0);
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(readback_prog);

#if DEBUG_WITH_BLIT
	/* Blit from MSAA texture/FBO to window */
	glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBlitFramebuffer(0, 0, 31, 31,
			  0, 0, 31, 31,
			  GL_COLOR_BUFFER_BIT, GL_NEAREST);
#else
	for (i = 0; i < num_samples; i++) {
		/* fetch the i-th sample */
		glUniform1i(sample_pos_uniform, i);

		piglit_draw_rect_tex(-1, -1, 2, 2, 0, 0, 31, 31);

		glReadPixels(0, 0, 32, 32, GL_RGBA, GL_FLOAT, images[i]);
	}
#endif

	piglit_check_gl_error(GL_NO_ERROR);

	/* compare sample images.
	 * There should be some differences.
	 */
	num_diffs = 0;
	for (j = 0; j < 32 * 32; j++) {
		for (i = 1; i < num_samples; i++) {
			/* compare samples at [j] */
			if (images[i][j*4+0] != images[0][j*4+0]) {
				num_diffs++;
				if (0)
				printf("diff at pixel %u: sample[%u]=%g vs"
				       " sample[%d]=%g\n",
				       j,
				       i, images[i][j*4+0],
				       0, images[0][j*4+0]);
			}
		}
	}

	if (num_diffs == 0) {
		fprintf(stderr,
			"There was no difference among the %d samples\n",
			num_samples);
		fflush(stderr);
	}

#if DISPLAY_AVERAGE
	/* "Resolve" the msaa image by computing the average of the samples. */
	for (j = 0; j < 32 * 32 * 4; j++) {
		float sum = 0.0f;
		for (i = 0; i < num_samples; i++) {
			sum += images[i][j];
		}
		average[j] = sum / num_samples;
	}

	glUseProgram(0);
	glDrawPixels(32, 32, GL_RGBA, GL_FLOAT, average);
#endif

	piglit_present_results();

	/* clean up */
	for (i = 0; i < num_samples; i++) {
		free(images[i]);
	}
	free(average);

	return num_diffs ? PIGLIT_PASS : PIGLIT_FAIL;
}


void
piglit_init(int argc, char **argv)
{
	GLuint max_samples;
	GLint tex_uniform;

#if MSAA
	piglit_require_extension("GL_ARB_texture_multisample");
#endif

	if (argc < 2) {
		fprintf(stderr, "Usage: %s <sample_count>\n", argv[0]);
		piglit_report_result(PIGLIT_SKIP);
	}
#if MSAA
	num_samples = strtoul(argv[1], NULL, 0);
	glGetIntegerv(GL_MAX_SAMPLES, (GLint *)&max_samples);
	if (num_samples > max_samples)
		piglit_report_result(PIGLIT_SKIP);
#else
	num_samples = 1;
#endif

	assert(num_samples <= MAX_SAMPLES);

	piglit_check_gl_error(GL_NO_ERROR);

	/* create MSAA tex and fbo */
	glGenTextures(1, &tex);
#if MSAA
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, tex);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, num_samples,
				GL_RGBA8,
				32, 32, GL_TRUE);
#else
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 
		     32, 32, 0,
		     GL_RGBA, GL_FLOAT, NULL);
#endif

	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
#if MSAA
			       GL_TEXTURE_2D_MULTISAMPLE,
#else
			       GL_TEXTURE_2D,
#endif
			       tex, 0);
	glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);

	assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) ==
	       GL_FRAMEBUFFER_COMPLETE);

	/* create sample readback shader */
	readback_prog = piglit_build_simple_program(vs_src_readback,
						    fs_src_readback);
	glUseProgram(readback_prog);
	tex_uniform = glGetUniformLocation(readback_prog, "tex");
	glUniform1i(tex_uniform, 0); /* unit 0 */
	sample_pos_uniform = glGetUniformLocation(readback_prog, "samplePos");

	/* create triangle drawing shader */
	draw_prog = piglit_build_simple_program(vs_src_draw, fs_src_draw);
	glUseProgram(draw_prog);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	glEnable(GL_MULTISAMPLE);
}
