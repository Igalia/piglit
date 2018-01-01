/* Copyright © 2015 Ilia Mirkin
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

/** @file max-size.c
 *
 * Tests that we can sample a maximally-sized texture buffer.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_core_version = 31;
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
		"#version 140\n"
		"in vec4 piglit_vertex;\n"
		"void main()\n"
		"{\n"
		"	gl_Position = piglit_vertex;\n"
		"}\n";

	static const char *fs_source =
		"#version 140\n"
		"uniform samplerBuffer s;\n"
		"uniform int offset;\n"
		"void main()\n"
		"{\n"
		"	gl_FragColor = texelFetch(s, offset);\n"
		"}\n";

	GLuint tex, tbo;
	static const uint8_t data[4] = {0x00, 0xff, 0x00, 0x00};
	GLuint prog;
	GLint max;
	GLenum err;

	prog = piglit_build_simple_program(vs_source, fs_source);
	glUseProgram(prog);

	glGenBuffers(1, &tbo);
	glBindBuffer(GL_TEXTURE_BUFFER, tbo);

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_BUFFER, tex);

	glGetIntegerv(GL_MAX_TEXTURE_BUFFER_SIZE, &max);

	printf("MAX_TEXTURE_BUFFER_SIZE: %d\n", max);
	if (max >= 512 * 1024 * 1024) {
		/* Buffer sizes >= 2G are a bit dicey, ideally this
		 * test would try various formats, including GL_R8.
		 */
		printf("MAX_TEXTURE_BUFFER_SIZE >= 512M, "
		       "testing with size 512M-1\n");
		max = 512 * 1024 * 1024 - 1;
	}

	glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA8, tbo);
	glBufferData(GL_TEXTURE_BUFFER,
		     max * sizeof(data), NULL, GL_STATIC_READ);
	err = glGetError();
	if (err == GL_OUT_OF_MEMORY) {
		printf("couldn't allocate buffer due to OOM, skipping.\n");
		piglit_report_result(PIGLIT_SKIP);
	}
	glBufferSubData(GL_TEXTURE_BUFFER,
			(max - 1) * sizeof(data), sizeof(data), data);

	glUniform1i(glGetUniformLocation(prog, "offset"), max - 1);
}
