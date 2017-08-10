/*
 * Copyright © 2016 Intel Corporation
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
 * \file roundup-samples.c
 *
 * Tests that requesting an odd number of samples doesn't break
 * anything. The implementation should round this up to the next
 * supported value. Technically the implementation is probably allowed
 * to support the odd number of samples so it doesn't report this as a
 * failure.
 *
 * Bug #93957
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_core_version = 31;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

#define REQUEST_N_SAMPLES 3

const char *vs_source =
	"#version 140\n"
	"\n"
	"in vec4 piglit_vertex;\n"
	"\n"
	"void\n"
	"main()\n"
	"{\n"
	"        gl_Position = piglit_vertex;\n"
	"}\n";

const char *fs_source =
	"#version 140\n"
	"#extension GL_ARB_sample_shading : require\n"
	"#extension GL_ARB_shader_storage_buffer_object : require\n"
	"\n"
	"layout (std140, binding=0) buffer shader_data\n"
	"{\n"
	"        int num_samples;\n"
	"};\n"
	"\n"
	"void\n"
	"main()\n"
	"{\n"
	"        num_samples = gl_NumSamples;\n"
	"}\n";

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	GLuint fbo;
	GLint prog;
	GLuint ssbo;
	bool pass = true;
	GLint actual_n_samples, actual_sample_buffers;
	GLint shader_n_samples = 0;

	piglit_require_gl_version(31);
	piglit_require_extension("GL_ARB_framebuffer_no_attachments");
	piglit_require_extension("GL_ARB_shader_storage_buffer_object");
	piglit_require_extension("GL_ARB_sample_shading");

	/* Create fbo with no attachments. */
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	/* Setup default width, height and number of samples */
	glFramebufferParameteri(GL_FRAMEBUFFER,
				GL_FRAMEBUFFER_DEFAULT_WIDTH,
				1);
	glFramebufferParameteri(GL_FRAMEBUFFER,
				GL_FRAMEBUFFER_DEFAULT_HEIGHT,
				1);
	glFramebufferParameteri(GL_FRAMEBUFFER,
				GL_FRAMEBUFFER_DEFAULT_SAMPLES,
				REQUEST_N_SAMPLES);

	/* Check that fbo is marked complete. */
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) !=
	    GL_FRAMEBUFFER_COMPLETE)
		piglit_report_result(PIGLIT_SKIP);

	glGenBuffers(1, &ssbo);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);
	glBufferData(GL_SHADER_STORAGE_BUFFER,
		     sizeof (shader_n_samples),
		     &shader_n_samples,
		     GL_DYNAMIC_COPY);

	prog = piglit_build_simple_program(vs_source, fs_source);

	glUseProgram(prog);

	piglit_draw_rect(-1, -1, 2, 2);

	glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);

	glGetIntegerv(GL_SAMPLES, &actual_n_samples);
	glGetIntegerv(GL_SAMPLE_BUFFERS, &actual_sample_buffers);
	glGetBufferSubData(GL_SHADER_STORAGE_BUFFER,
			   0, /* offset */
			   sizeof (shader_n_samples),
			   &shader_n_samples);

	printf("Requested samples                : %i\n"
	       "glGetIntegerv(GL_SAMPLES)        : %i\n"
	       "glGetIntegerv(GL_SAMPLE_BUFFERS) : %i\n"
	       "gl_NumSamples from shader        : %u\n",
	       REQUEST_N_SAMPLES,
	       actual_n_samples,
	       actual_sample_buffers,
	       shader_n_samples);

	if (actual_n_samples < REQUEST_N_SAMPLES) {
		printf("FAIL: GL_SAMPLES is too small\n");
		pass = false;
	}

	if (actual_sample_buffers != 1) {
		printf("FAIL: GL_SAMPLE_BUFFERS should be 1\n");
		pass = false;
	}

	if (shader_n_samples != actual_n_samples) {
		printf("FAIL: GL_SAMPLES does not match gl_NumSamples\n");
		pass = false;
	}

	glDeleteBuffers(1, &ssbo);
	glDeleteFramebuffers(1, &fbo);
	glDeleteProgram(prog);

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
