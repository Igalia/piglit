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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

/** @file subdata-sync.c
 *
 * Tests that glBufferSubData() synchronizes correctly with TBO rendering.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 10;
	config.supports_gl_core_version = 31;

	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;
PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
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
		"void main()\n"
		"{\n"
		"	gl_FragColor = texelFetch(s, 0);\n"
		"}\n";

	bool pass = true;
	GLuint tex, bo;
	GLuint prog;
	float green[] = {0, 1, 0, 0};
	float blue[] =  {0, 0, 1, 0};
	uint8_t g_rgba8[] = {0x00, 0xff, 0x00, 0x00};
	uint8_t b_rgba8[] = {0x00, 0x00, 0xff, 0x00};

	prog = piglit_build_simple_program(vs_source, fs_source);
	glUseProgram(prog);

	glGenBuffers(1, &bo);
	glBindBuffer(GL_TEXTURE_BUFFER, bo);
	/* Make the buffer bigger than the data to trigger the driver
	 * code path we want.
	 */
	glBufferData(GL_TEXTURE_BUFFER, 4096, NULL, GL_STREAM_DRAW);
	glBufferSubData(GL_TEXTURE_BUFFER, 0, sizeof(g_rgba8), g_rgba8);

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_BUFFER, tex);
	glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA8, bo);

	piglit_draw_rect(-1, -1, 1, 2);
	glBufferSubData(GL_TEXTURE_BUFFER, 0, sizeof(b_rgba8), b_rgba8);
	piglit_draw_rect(0, -1, 1, 2);

	pass = pass && piglit_probe_rect_rgba(0, 0,
					      piglit_width / 2, piglit_height,
					      green);
	pass = pass && piglit_probe_rect_rgba(piglit_width / 2, 0,
					      piglit_width / 2, piglit_height,
					      blue);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	piglit_require_GLSL_version(140);
	if (piglit_get_gl_version() < 31)
		piglit_require_extension("GL_ARB_texture_buffer_object");
}
