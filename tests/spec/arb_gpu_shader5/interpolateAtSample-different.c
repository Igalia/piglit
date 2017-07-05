/*
 * Copyright (c) 2015 Intel Corporation
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
 * @file interpolateAtSample-different.c
 *
 * Test ARB_gpu_shader5 interpolateAtSample builtin.
 *
 * A 1x1 multisample floating-point framebuffer is created with four
 * samples. A fragment is then rendered into the FBO four times, once
 * with interpolation at each different sample location. The
 * interpolation values are written into the framebuffer and read
 * back. They are then checked to ensure they are all different and
 * that they all lie within the fragment.
 *
 * interpolateAtSample can be called with const or non-const argument.
 * If ‘uniform’ is specified on the command line it will use a
 * non-const argument (via a uniform).
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_core_version = 32;

	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGB;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

#define N_SAMPLES 4

static const char
vertex_shader[] =
	"#version 150\n"
	"in vec2 piglit_vertex;\n"
	"in vec2 piglit_texcoord;\n"
	"out vec2 texcoord;\n"
	"\n"
	"void\n"
	"main()\n"
	"{\n"
	"        gl_Position = vec4(piglit_vertex, 0.0, 1.0);\n"
	"        texcoord = piglit_texcoord;\n"
	"}\n";

static const char
fragment_shader_version[] =
	"#version 150\n"
	"#extension GL_ARB_gpu_shader5 : require\n";

static const char
fragment_shader[] =
	/* Version header and define for sample_id is added outside of
	 * this string so that it can decide at runtime whether to use
	 * a uniform or a constant.
	 */
	"in vec2 texcoord;\n"
	"\n"
	"void\n"
	"main()\n"
	"{\n"
	"        gl_FragColor.rg = interpolateAtSample(texcoord, sample_id);\n"
	"        gl_FragColor.ba = vec2(0.0, 1.0);\n"
	"}\n";

enum piglit_result
piglit_display(void)
{
	/* not used */
	return PIGLIT_FAIL;
}

static void
create_framebuffer(int sample_count,
		   GLuint *fbo,
		   GLuint *rb)
{
	GLenum status;

	glGenFramebuffers(1, fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, *fbo);
	glGenRenderbuffers(1, rb);
	glBindRenderbuffer(GL_RENDERBUFFER, *rb);
	if (sample_count > 1)
		glRenderbufferStorageMultisample(GL_RENDERBUFFER,
						 sample_count, /* samples */
						 GL_RG32F,
						 1, 1 /* width/height */);
	else
		glRenderbufferStorage(GL_RENDERBUFFER,
				      GL_RG32F,
				      1, 1 /* width/height */);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER,
				  GL_COLOR_ATTACHMENT0,
				  GL_RENDERBUFFER,
				  *rb);

	status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		fprintf(stderr, "Multisample FBO incomplete\n");
		piglit_report_result(PIGLIT_SKIP);
	}
}

GLuint
create_program(int sample_id)
{
	char shader_source[sizeof fragment_shader +
			   sizeof fragment_shader_version +
			   512];

	strcpy(shader_source, fragment_shader_version);

	if (sample_id == -1) {
		strcat(shader_source, "uniform int sample_id;\n");
	} else {
		sprintf(shader_source + sizeof fragment_shader_version - 1,
			"#define sample_id %i\n",
			sample_id);
	}

	strcat(shader_source, fragment_shader);

	return piglit_build_simple_program(vertex_shader, shader_source);
}

void
piglit_init(int argc, char**argv)
{
	GLuint ms_fbo, ms_rb;
	GLuint ss_fbo, ss_rb;
	GLuint programs[N_SAMPLES];
	bool use_uniform = false;
	GLuint shader_id_location = -1;
	GLfloat results[N_SAMPLES][2];
	bool pass = true;
	int i, j;

	for (i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "uniform")) {
			use_uniform = true;
		} else {
			fprintf(stderr, "unknown argument \"%s\"\n", argv[i]);
			piglit_report_result(PIGLIT_FAIL);
		}
	}

	piglit_require_extension("GL_ARB_gpu_shader5");
	piglit_require_GLSL_version(150);

	if (use_uniform) {
		programs[0] = create_program(-1);
		glUseProgram(programs[0]);
		shader_id_location = glGetUniformLocation(programs[0],
							  "sample_id");
	} else {
		for (i = 0; i < N_SAMPLES; i++)
			programs[i] = create_program(i);
	}

	create_framebuffer(N_SAMPLES,
			   &ms_fbo, &ms_rb);
	create_framebuffer(1, /* sample_count */
			   &ss_fbo, &ss_rb);

	glViewport(0, 0, 1, 1);

	for (i = 0; i < N_SAMPLES; i++) {
		if (use_uniform)
			glUniform1i(shader_id_location, i);
		else
			glUseProgram(programs[i]);

		glBindFramebuffer(GL_FRAMEBUFFER, ms_fbo);
		glClear(GL_COLOR_BUFFER_BIT);
		piglit_draw_rect_tex(-1, -1, 2, 2,
				     0, 0, 1, 1);

		glBindFramebuffer(GL_READ_FRAMEBUFFER, ms_fbo);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, ss_fbo);

		glClear(GL_COLOR_BUFFER_BIT);
		glBlitFramebuffer(0, 0, /* srcX/Y0 */
				  1, 1, /* srcX/Y1 */
				  0, 0, /* dstX/Y0 */
				  1, 1, /* dstX/Y1 */
				  GL_COLOR_BUFFER_BIT,
				  GL_NEAREST);

		glBindFramebuffer(GL_FRAMEBUFFER, ss_fbo);

		glReadPixels(0, 0, /* x/y */
			     1, 1, /* width/height */
			     GL_RG, GL_FLOAT,
			     results[i]);
	}

	for (i = 0; i < N_SAMPLES; i++) {
		printf("value at sample %i = %f %f\n",
		       i,
		       results[i][0],
		       results[i][1]);
	}

	/* Check that the samples are within [0, 1] */
	for (i = 0; i < N_SAMPLES; i++) {
		if (results[i][0] < 0.0f || results[i][0] > 1.0f ||
		    results[i][1] < 0.0f || results[i][1] > 1.0f) {
			fprintf(stderr,
				"results for sample %i are out of range\n",
				i);
			pass = false;
		}
	}

	/* Check that all of the samples are different */
	for (i = 1; i < N_SAMPLES; i++) {
		for (j = 0; j < i; j++) {
			if (results[i][0] == results[j][0] &&
			    results[i][1] == results[j][1]) {
				fprintf(stderr,
					"samples %i and %i have the same "
					"value\n",
					i, j);
				pass = false;
				break;
			}
		}
	}

	glDeleteFramebuffers(1, &ms_fbo);
	glDeleteRenderbuffers(1, &ms_rb);
	glDeleteFramebuffers(1, &ss_fbo);
	glDeleteRenderbuffers(1, &ss_rb);

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

