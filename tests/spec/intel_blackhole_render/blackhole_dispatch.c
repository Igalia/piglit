/*
 * Copyright © 2018 Intel Corporation
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

/** @file blackhole_draw.c
 *
 * Verifies that with GL_INTEL_black_render enabled, dispatch operations have no
 * effect.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

#if defined(PIGLIT_USE_OPENGL)
	config.supports_gl_core_version = 42;
#elif defined(PIGLIT_USE_OPENGL_ES2) || defined(PIGLIT_USE_OPENGL_ES3)
	config.supports_gl_es_version = 20;
#endif
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

#define SIZE_X (4)

static const char *compute_shader =
	"#version 430\n"
	"layout (local_size_x = 1) in;"
	"uniform float value;"
	"\n"
	"layout (std430, binding = 0) buffer OutBuf { float output_values[]; };\n"
	"\n"
	"void main()\n"
	"{\n"
	"    uint pos = gl_GlobalInvocationID.x;\n"
	"    output_values[pos] = value;\n"
	"}\n";

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	enum piglit_result result = PIGLIT_PASS;
	GLuint data_bo = 0;
	GLfloat *data_buf;
	GLint ok = 1;
	GLint prog = 0;
	GLuint shader;
	const float one = 1.0f;

	piglit_require_extension("GL_ARB_compute_shader");

	data_buf = calloc(SIZE_X, sizeof(*data_buf));
	glGenBuffers(1, &data_bo);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, data_bo);
	glBufferData(GL_SHADER_STORAGE_BUFFER,
		     sizeof(float) * SIZE_X,
		     data_buf, GL_STATIC_DRAW);
	free(data_buf);

	shader = glCreateShader(GL_COMPUTE_SHADER);

	glShaderSource(shader, 1,
		       (const GLchar **) &compute_shader,
		       NULL);

	glCompileShader(shader);

	glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
	assert(ok);

	prog = glCreateProgram();

	glAttachShader(prog, shader);

	glLinkProgram(prog);

	glGetProgramiv(prog, GL_LINK_STATUS, &ok);
	assert(ok);

	glUseProgram(prog);

	assert(!glIsEnabled(GL_BLACKHOLE_RENDER_INTEL));

	glMemoryBarrier(GL_ALL_BARRIER_BITS);
	glUniform1f(glGetUniformLocation(prog, "value"), 1.0f);
	glDispatchCompute(SIZE_X, 1, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	if (!piglit_probe_buffer(data_bo, GL_SHADER_STORAGE_BUFFER, "output_values",
				 SIZE_X, 1, &one))
		result = PIGLIT_FAIL;

	glEnable(GL_BLACKHOLE_RENDER_INTEL);
	assert(glIsEnabled(GL_BLACKHOLE_RENDER_INTEL));

	glMemoryBarrier(GL_ALL_BARRIER_BITS);
	glUniform1f(glGetUniformLocation(prog, "value"), 2.0f);
	glDispatchCompute(SIZE_X, 1, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	if (!piglit_probe_buffer(data_bo, GL_SHADER_STORAGE_BUFFER, "output_values",
				 SIZE_X, 1, &one))
		result = PIGLIT_FAIL;

	piglit_report_result(result);
}
