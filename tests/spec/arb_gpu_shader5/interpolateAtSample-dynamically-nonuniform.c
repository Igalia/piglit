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
 * @file interpolateAtSample-dynamically-nonuniform.c
 *
 * Test ARB_gpu_shader5 interpolateAtSample builtin using dynamically
 * non-uniform sample IDs.
 *
 * A 2x2 multisample floating-point framebuffer is created with four
 * samples. The buffer is then filled with a single triangle four
 * times, once with interpolation at each different sample location.
 * The interpolation values are written into the framebuffer and read
 * back so that it will know the sample location of each sample for
 * each pixel. This process is then repeated but with each pixel
 * specifying the sample IDs in a different order so that the sample
 * ID will be dynamically non-uniform. The results are checked to
 * ensure that the sample locations are the same as the previous
 * render.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_core_version = 32;

	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGB;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

#define N_SAMPLES 4
#define FBO_WIDTH 2
#define FBO_HEIGHT 2

static const char
vertex_shader[] =
	"#version 150\n"
	"in vec2 piglit_vertex;\n"
	"out vec2 pos;\n"
	"\n"
	"void\n"
	"main()\n"
	"{\n"
	"        gl_Position = vec4(piglit_vertex, 0.0, 1.0);\n"
	"        pos = piglit_vertex;\n"
	"}\n";

static const char
fragment_shader_dynamically_uniform[] =
	"#version 150\n"
	"#extension GL_ARB_gpu_shader5 : require\n"
	"in vec2 pos;\n"
	"uniform int sample_id;\n"
	"\n"
	"void\n"
	"main()\n"
	"{\n"
	"        gl_FragColor.rg = interpolateAtSample(pos, sample_id);\n"
	"        gl_FragColor.ba = vec2(0.0, 1.0);\n"
	"}\n";

static const char
fragment_shader_dynamically_non_uniform[] =
	"#version 150\n"
	"#extension GL_ARB_gpu_shader5 : require\n"
	"in vec2 pos;\n"
	"uniform int sample_id;\n"
	"\n"
	"void\n"
	"main()\n"
	"{\n"
	"        int sid = sample_id ^ int(ceil(pos.x)) ^\n"
	"                  (int(ceil(pos.y)) << 1);\n"
	"        gl_FragColor.rg = interpolateAtSample(pos, sid);\n"
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
						 FBO_WIDTH, FBO_HEIGHT);
	else
		glRenderbufferStorage(GL_RENDERBUFFER,
				      GL_RG32F,
				      FBO_WIDTH, FBO_HEIGHT);
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

static void
get_samples(bool dynamically_uniform,
	    GLuint ms_fbo,
	    GLuint ss_fbo,
	    GLfloat results[N_SAMPLES][FBO_WIDTH * FBO_HEIGHT * 2])
{
	static const float verts[] = {
		-1.0f, -1.0f,
		8.0f, -1.0f,
		-1.0f, 8.0f
	};
	const char *fragment_source;
	GLuint vbo, vao;
	GLuint prog;
	GLuint shader_id_location;
	int attr;
	int i;

	if (dynamically_uniform)
		fragment_source = fragment_shader_dynamically_uniform;
	else
		fragment_source = fragment_shader_dynamically_non_uniform;

	prog = piglit_build_simple_program(vertex_shader, fragment_source);
	glUseProgram(prog);
	shader_id_location = glGetUniformLocation(prog, "sample_id");

	glViewport(0, 0, FBO_WIDTH, FBO_HEIGHT);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof verts, verts, GL_STATIC_DRAW);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	attr = glGetAttribLocation(prog, "piglit_vertex");
	glEnableVertexAttribArray(attr);
	glVertexAttribPointer(attr,
			      2, /* size */
			      GL_FLOAT,
			      GL_FALSE, /* normalized */
			      sizeof (GLfloat) * 2,
			      NULL /* pointer */);

	for (i = 0; i < N_SAMPLES; i++) {
		glUniform1i(shader_id_location, i);

		glBindFramebuffer(GL_FRAMEBUFFER, ms_fbo);
		glClear(GL_COLOR_BUFFER_BIT);

		glDrawArrays(GL_TRIANGLES, 0, 3);

		glBindFramebuffer(GL_READ_FRAMEBUFFER, ms_fbo);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, ss_fbo);

		glClear(GL_COLOR_BUFFER_BIT);
		glBlitFramebuffer(0, 0, /* srcX/Y0 */
				  FBO_WIDTH, FBO_HEIGHT, /* srcX/Y1 */
				  0, 0, /* dstX/Y0 */
				  FBO_WIDTH, FBO_HEIGHT, /* dstX/Y1 */
				  GL_COLOR_BUFFER_BIT,
				  GL_NEAREST);

		glBindFramebuffer(GL_FRAMEBUFFER, ss_fbo);

		glReadPixels(0, 0, /* x/y */
			     FBO_WIDTH, FBO_HEIGHT,
			     GL_RG, GL_FLOAT,
			     results[i]);
	}

	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);

	glDeleteProgram(prog);
}

static void
print_coords(int x, int y,
	     const GLfloat *results)
{
	printf(" %f,%f",
	       results[(x + y * FBO_WIDTH) * 2] + 1.0f - x,
	       results[(x + y * FBO_WIDTH) * 2 + 1] + 1.0f - y);
}

void
piglit_init(int argc, char**argv)
{
	GLuint ms_fbo, ms_rb;
	GLuint ss_fbo, ss_rb;
	GLfloat du_results[N_SAMPLES][FBO_WIDTH * FBO_HEIGHT * 2];
	GLfloat dnu_results[N_SAMPLES][FBO_WIDTH * FBO_HEIGHT * 2];
	bool pass = true;
	int x, y, i, j;

	piglit_require_extension("GL_ARB_gpu_shader5");
	piglit_require_GLSL_version(150);

	create_framebuffer(N_SAMPLES,
			   &ms_fbo, &ms_rb);
	create_framebuffer(1, /* sample_count */
			   &ss_fbo, &ss_rb);

	get_samples(true, ms_fbo, ss_fbo, du_results);
	get_samples(false, ms_fbo, ss_fbo, dnu_results);

	glDeleteFramebuffers(1, &ms_fbo);
	glDeleteRenderbuffers(1, &ms_rb);
	glDeleteFramebuffers(1, &ss_fbo);
	glDeleteRenderbuffers(1, &ss_rb);

	for (y = 0; y < FBO_HEIGHT; y++) {
		for (x = 0; x < FBO_WIDTH; x++) {
			printf("Dynamically uniform coords at     (%i,%i):",
			       x, y);
			for (i = 0; i < N_SAMPLES; i++)
				print_coords(x, y, du_results[i]);
			fputc('\n', stdout);
			printf("Dynamically non-uniform coords at (%i,%i):",
			       x, y);
			for (i = 0; i < N_SAMPLES; i++) {
				j = i ^ x ^ (y << 1);
				print_coords(x, y, dnu_results[j]);
				if (memcmp(&du_results[i]
					   [(y * FBO_WIDTH + x) * 2],
					   &dnu_results[j]
					   [(y * FBO_WIDTH + x) * 2],
					   sizeof (GLfloat) * 2))
					pass = false;

			}
			fputc('\n', stdout);
		}
	}

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

