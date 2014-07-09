/* Copyright © 2012 Intel Corporation
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

/** @file fetch-outside-bounds.c
 *
 * Tests that we can sample from outside of a texture buffer object
 * without crashing.
 *
 * From the GL_ARB_texture_buffer_object spec:
 *
 *     "When a buffer texture is accessed in a shader, the results of
 *      a texel fetch are undefined if the specified texel number is
 *      greater than or equal to the clamped number of texels in the
 *      texel array."
 *
 * we interpret this as allowing any result to come back, but not
 * terminate the program.  To test that, we glReadPixels the result
 * but don't test the values returned.
 */

#include "piglit-util-gl.h"

enum piglit_result
piglit_display(void)
{
	static const char *vs_source =
		"#version 140\n"
		"in vec4 vertex;\n"
		"void main()\n"
		"{\n"
		"	gl_Position = vertex;\n"
		"}\n";

	static const char *fs_source =
		"#version 140\n"
		"uniform samplerBuffer s;\n"
		"void main()\n"
		"{\n"
		"	gl_FragColor = texelFetch(s, 4096);\n"
		"}\n";

	GLuint tex, tbo, vbo;
	/* data stored in our TBO, not actually read by the shader. */
	static const uint8_t data[4] = {0x0, 0xff, 0x0, 0x0};
	uint8_t junk[4];
	static const GLfloat verts[8] = {
		-1, -1,
		 1, -1,
		 1,  1,
		-1,  1
	};
	int vertex_location;
	GLuint prog;

	prog = piglit_build_simple_program(vs_source, fs_source);
	glUseProgram(prog);

	vertex_location = glGetAttribLocation(prog, "vertex");

	glGenBuffers(1, &tbo);
	glBindBuffer(GL_TEXTURE_BUFFER, tbo);

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_BUFFER, tex);

	glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA8, tbo);
	glBufferData(GL_TEXTURE_BUFFER, sizeof(data), data, GL_STATIC_READ);

	if (piglit_get_gl_version() >= 31) {
		GLuint vao;
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
	}
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_READ);
	glVertexAttribPointer(vertex_location, 2, GL_FLOAT,
			      GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(vertex_location);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &junk);
	/* We don't verify the junk read, since it's undefined. */

	piglit_present_results();

	return PIGLIT_PASS;
}

void
piglit_init(int argc, char **argv)
{
	piglit_require_GLSL_version(140);
	if (piglit_get_gl_version() < 31)
		piglit_require_extension("GL_ARB_texture_buffer_object");
}

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 10;
	config.supports_gl_core_version = 31;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;
PIGLIT_GL_TEST_CONFIG_END
