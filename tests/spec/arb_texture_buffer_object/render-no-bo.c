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

/** @file render-no-bo.c
 *
 * Tests that we can sample from the default texture buffer or a
 * texture buffer with no buffer bound, and not crash.
 *
 * From the GL_ARB_texture_buffer_object spec:
 *
 *     "If no buffer object is bound to the buffer texture, the
 *      results of the texel access are undefined."
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

	GLuint tex, vbo;
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

	/* First, draw with no texture buffer bound (so using the
	 * default texture buffer object)
	 */
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &junk);

	/* Now, do it again with a texture buffer that doesn't have
	 * any buffer bound yet.
	 */
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_BUFFER, tex);

	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &junk);

	glDeleteTextures(1, &tex);

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
