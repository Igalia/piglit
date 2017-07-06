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

/** @file bufferstorage.c
 *
 * Tests that we can modify texture buffers using coherently mapped buffers.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_core_version = 31;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;
PIGLIT_GL_TEST_CONFIG_END

static const float green[4] = {0, 1, 0, 0};
static const float red[4] = {1, 0, 0, 0};
static float *map;

enum piglit_result
piglit_display(void)
{
	GLsync fence;
	bool pass = true;

	glViewport(0, 0, piglit_width, piglit_height);
	glClearColor(0.2, 0.2, 0.2, 0.2);
	glClear(GL_COLOR_BUFFER_BIT);

	memcpy(map, red, sizeof(red));
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	/* Wait for any previous rendering to finish before updating
	 * the texture buffer
	 */
	fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
	glClientWaitSync(fence, GL_SYNC_FLUSH_COMMANDS_BIT,
			 GL_TIMEOUT_IGNORED);

	memcpy(map, green, sizeof(green));
	glDrawArrays(GL_TRIANGLE_FAN, 4, 4);

	pass = piglit_probe_rect_rgba(
			0, 0, piglit_width / 2, piglit_height, red) && pass;
	pass = piglit_probe_rect_rgba(piglit_width / 2, 0,
				      piglit_width / 2, piglit_height,
				      green) && pass;

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
		"	gl_FragColor = texelFetch(s, 0);\n"
		"}\n";

	static const GLfloat verts[16] = {
		-1, -1,
		 0, -1,
		 0,  1,
		-1,  1,

		 0, -1,
		 1, -1,
		 1,  1,
		 0,  1
	};

	GLuint tex, tbo;
	GLuint prog;

	int vertex_location;
	GLuint vao, vbo;

	piglit_require_extension("GL_ARB_buffer_storage");

	prog = piglit_build_simple_program(vs_source, fs_source);
	glUseProgram(prog);
	vertex_location = glGetAttribLocation(prog, "piglit_vertex");

	glGenBuffers(1, &tbo);
	glBindBuffer(GL_TEXTURE_BUFFER, tbo);
	glBufferStorage(GL_TEXTURE_BUFFER, sizeof(red), NULL,
			GL_MAP_WRITE_BIT |
			GL_MAP_PERSISTENT_BIT |
			GL_MAP_COHERENT_BIT |
			GL_DYNAMIC_STORAGE_BIT);
	piglit_check_gl_error(GL_NO_ERROR);

	map = glMapBufferRange(GL_TEXTURE_BUFFER, 0, sizeof(red),
			       GL_MAP_WRITE_BIT |
			       GL_MAP_PERSISTENT_BIT |
			       GL_MAP_COHERENT_BIT);

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_BUFFER, tex);

	glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, tbo);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_READ);
	glVertexAttribPointer(vertex_location, 2, GL_FLOAT,
			      GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(vertex_location);
}
