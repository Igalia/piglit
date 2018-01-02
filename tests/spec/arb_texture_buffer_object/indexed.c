/* Copyright © 2015 Ilia Mirkin
 * Copyright © 2017 VMware, Inc.
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

/** @file indexed.c
 *
 * Tests that we can sample texture buffers with sampler indexing.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_core_version = 32;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;
PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	static const float green[4] = {0, 1, 0, 0};
	bool pass;

	glViewport(0, 0, piglit_width, piglit_height);
	glClearColor(0.2, 0.2, 0.2, 0.2);
	glClear(GL_COLOR_BUFFER_BIT);

	piglit_draw_rect(-1, -1, 2, 2);

	pass = piglit_probe_rect_rgba(0, 0, piglit_width, piglit_height, green);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	static const char *vs_source =
		"#version 150\n"
		"in vec4 piglit_vertex;\n"
		"void main()\n"
		"{\n"
		"	gl_Position = piglit_vertex;\n"
		"}\n";

	static const char *fs_source =
		"#version 150\n"
		"#extension GL_ARB_gpu_shader5: require\n"
		"uniform samplerBuffer s[2];\n"
		"uniform int offset;\n"
		"uniform int index = 1;\n"
		"void main()\n"
		"{\n"
		"	gl_FragColor = texelFetch(s[index], offset);\n"
		"}\n";

	GLuint tex[2], tbo[2];
	GLint indices[2] = { 0, 1 };
	static const uint8_t datag[4] = {0x00, 0xff, 0x00, 0x00};
	static const uint8_t datar[4] = {0xff, 0x00, 0x00, 0x00};
	GLuint prog;
	GLint size = 4;
	piglit_require_extension("GL_ARB_gpu_shader5");

	prog = piglit_build_simple_program(vs_source, fs_source);
	glUseProgram(prog);

	glGenBuffers(2, tbo);
	glGenTextures(2, tex);
	glBindBuffer(GL_TEXTURE_BUFFER, tbo[0]);
	glBindTexture(GL_TEXTURE_BUFFER, tex[0]);
	glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA8, tbo[0]);
	glBufferData(GL_TEXTURE_BUFFER,
		     size * sizeof(datar), NULL, GL_STATIC_READ);
	glBufferSubData(GL_TEXTURE_BUFFER,
			(size - 1) * sizeof(datar), sizeof(datar), datar);

	glActiveTexture(GL_TEXTURE1);
	glBindBuffer(GL_TEXTURE_BUFFER, tbo[1]);
	glBindTexture(GL_TEXTURE_BUFFER, tex[1]);
	glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA8, tbo[1]);
	glBufferData(GL_TEXTURE_BUFFER,
		     size * sizeof(datag), NULL, GL_STATIC_READ);
	glBufferSubData(GL_TEXTURE_BUFFER,
			(size - 1) * sizeof(datag), sizeof(datag), datag);

	glUniform1i(glGetUniformLocation(prog, "offset"), size - 1);
	glUniform1iv(glGetUniformLocation(prog, "s"), 2, indices);
}
