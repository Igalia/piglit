/* Copyright © 2020 Intel Corporation
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
 * \file clearbuffer-depth-cs-probe.c
 *
 * Verify clearing depth buffer with \c glClearBufferfv and
 * check its result by using compute shader
 *
 * \author Andrii Simiklit
 */
#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 33;
	config.supports_gl_core_version = 33;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

struct framebuffer
{
	GLuint fbo;
	GLuint depth;
};

static struct
{
	GLuint program;
	GLuint counter;
} cs_probe;

static void
cs_probe_init(float expected_depth);

static bool
cs_probe_check(GLuint texture);

static void
cs_probe_free();

static struct framebuffer
generate_fbo();

void piglit_init(int argc, char **argv)
{

	unsigned i;
	bool pass = true;
	static const float expected_depth[3] = {
		0.16f,
		0.77f,
		0.35f
	};

	piglit_require_extension("GL_ARB_compute_shader");
	piglit_require_extension("GL_ARB_shader_atomic_counters");
	piglit_require_extension("GL_ARB_explicit_uniform_location");

	for (i = 0; i < (ARRAY_SIZE(expected_depth) - 1); i++) {

		GLenum err;
		struct framebuffer fb = generate_fbo();

		cs_probe_init(expected_depth[i + 1]);

		glClearBufferfv(GL_DEPTH, 0, &expected_depth[i]);
		err = glGetError();
		if (err != GL_NO_ERROR) {
			fprintf(stderr,
				"First call to glClearBufferfv erroneously "
				"generated a GL error (%s, 0x%04x)\n",
				piglit_get_gl_error_name(err), err);
			pass = false;
		}

		/*
		 * After first clear the shader has to detect differences
		 * because shader expects other depth value
		 */
		pass = !cs_probe_check(fb.depth) && pass;

		glClearBufferfv(GL_DEPTH, 0, &expected_depth[i + 1]);
		err = glGetError();
		if (err != GL_NO_ERROR) {
			fprintf(stderr,
				"First call to glClearBufferfv erroneously "
				"generated a GL error (%s, 0x%04x)\n",
				piglit_get_gl_error_name(err), err);
			pass = false;
		}

		pass = cs_probe_check(fb.depth) && pass;

		cs_probe_free();

		glDeleteFramebuffers(1, &fb.fbo);
		glDeleteTextures(1, &fb.depth);
	}
	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

static struct framebuffer
generate_fbo()
{
	static const float default_depth = 0.2;
	struct framebuffer fb;

	glGenFramebuffers(1, &fb.fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fb.fbo);

	glGenTextures(1, &fb.depth);

	glBindTexture(GL_TEXTURE_2D, fb.depth);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT24, piglit_width, piglit_height);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, fb.depth, 0);

	/* If GL_ARB_ES2_compatibility is not supported, the GL
	 * expects the draw buffer and read buffer be disabled if
	 * there is no color buffer (to read or draw).
	 */
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	glClearDepth(default_depth);
	glClear(GL_DEPTH_BUFFER_BIT);
	glFinish();

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	return fb;
}


static bool
cs_probe_check(GLuint texture)
{
	static const unsigned initial_count = 0;
	unsigned differences = 0u;

	glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, cs_probe.counter);
	glBufferData(GL_ATOMIC_COUNTER_BUFFER,
		sizeof(unsigned), &initial_count, GL_DYNAMIC_DRAW);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);

	glUseProgram(cs_probe.program);
	glUniform1i(glGetUniformLocation(cs_probe.program, "source"), 0);

	glDispatchCompute(piglit_width, piglit_height, 1);

	glMemoryBarrier(GL_ALL_BARRIER_BITS);
	glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, cs_probe.counter);
	differences = *(unsigned*)glMapBufferRange(GL_ATOMIC_COUNTER_BUFFER, 0,
						sizeof(unsigned),
						GL_MAP_READ_BIT);
	glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);
	return differences == 0u;
}

static char*
generate_shader_src(float expected_depth)
{
	static const char *cs_probe_source =
		"#version 150\n"
		"#extension GL_ARB_compute_shader: require\n"
		"#extension GL_ARB_shader_atomic_counters: require\n"
		"#extension GL_ARB_explicit_uniform_location: require\n"
		"layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;\n"
		"layout(binding = 0) uniform atomic_uint differences;\n"
		"uniform sampler2D source;\n"
		"void main() {\n"
		"   ivec2 coord = ivec2(gl_GlobalInvocationID.xy);\n"
		"   if (abs(texelFetch(source, coord, 0).r - %f) > 0.001f)\n"
		"      atomicCounterIncrement(differences);\n"
		"}\n";

	char *shader_src;
	unsigned size;

	size = strlen(cs_probe_source) + 32;
	shader_src = malloc(size);
	snprintf(shader_src, size, cs_probe_source, expected_depth);
	return shader_src;
}

static void
cs_probe_init(float expected_depth)
{
	GLuint cs;
	char *source = generate_shader_src(expected_depth);

	cs = piglit_compile_shader_text(GL_COMPUTE_SHADER, source);
	cs_probe.program = glCreateProgram();
	glAttachShader(cs_probe.program, cs);
	glLinkProgram(cs_probe.program);

	glGenBuffers(1, &cs_probe.counter);

	glDeleteShader(cs);
	free(source);
}

static void
cs_probe_free()
{
	glDeleteProgram(cs_probe.program);
	glDeleteBuffers(1, &cs_probe.counter);
}

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}
