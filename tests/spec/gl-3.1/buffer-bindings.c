/*
 * Copyright 2017 VMware, Inc.
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/**
 * \file buffer-bindings.c
 *
 * Test glBindBuffer, glBufferSubData, etc. with various binding points.
 * The buffer <target> parameter passed to many buffer object functions
 * can be seen as a hint about what kind of data will be stored in the
 * buffer, but it can't be relied upon.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_core_version = 31;
	config.supports_gl_compat_version = 31;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;
PIGLIT_GL_TEST_CONFIG_END


static const GLfloat green[4] = {0, 1, 0, 1};
static GLint vertex_attrib;
static bool have_dsa;

static const GLenum buffer_targets[] = {
	GL_ARRAY_BUFFER,
	GL_ELEMENT_ARRAY_BUFFER_ARB,
	GL_PIXEL_PACK_BUFFER_EXT,
	GL_PIXEL_UNPACK_BUFFER_EXT,
	GL_COPY_READ_BUFFER,
	GL_COPY_WRITE_BUFFER,
	GL_TRANSFORM_FEEDBACK_BUFFER,
	GL_TEXTURE_BUFFER,
	GL_UNIFORM_BUFFER,
	GL_NONE   // To exercise DSA functions
};


static GLuint
create_vbo(GLenum target)
{
	static const GLfloat v[4][2] = {
		{ -1, -1 },
		{ -1,  1 },
		{  1, -1 },
		{  1,  1 }
	};
	GLuint buf;

	if (target == GL_NONE) {
		glCreateBuffers(1, &buf);
		glNamedBufferData(buf, sizeof(v), v, GL_STATIC_DRAW);
	}
	else {
		glGenBuffers(1, &buf);
		glBindBuffer(target, buf);
		glBufferData(target, sizeof(v), v, GL_STATIC_DRAW);
		glBindBuffer(target, 0);
	}

	return buf;
}


static bool
test_buffer(GLenum target)
{
	GLuint b = create_vbo(target);
	GLuint vao;
	bool p;

	glViewport(0, 0, piglit_width, piglit_height);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, b);
	glVertexAttribPointer(vertex_attrib, 2, GL_FLOAT, GL_FALSE,
			      2*sizeof(GLfloat), NULL);
	glEnableVertexAttribArray(vertex_attrib);

	glClear(GL_COLOR_BUFFER_BIT);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glDeleteBuffers(1, &b);

	p = piglit_probe_rect_rgba(0, 0, piglit_width, piglit_height, green);

	piglit_present_results();

	if (!p) {
		const char *s = target == GL_NONE ?
			"DSA" : piglit_get_gl_enum_name(target);
		printf("Test failed for buffer %s\n", s);
		return false;
	} else {
		return true;
	}
}


static bool
test_buffers(void)
{
	bool pass = true;
	int i;
	for (i = 0; i < ARRAY_SIZE(buffer_targets); i++) {
		if (buffer_targets[i] == GL_NONE && !have_dsa)
			continue;
		pass = test_buffer(buffer_targets[i]) && pass;
	}
	return pass;
}


static GLuint
make_program(void)
{
	static const char *vs_text =
		"#version 130 \n"
		"in vec4 vertex; \n"
		"uniform vec4 color; \n"
		"out vec4 vs_fs_color; \n"
		"void main() \n"
		"{ \n"
		"   gl_Position = vertex; \n"
		"   vs_fs_color = color; \n"
		"} \n";

	static const char *fs_text =
		"#version 130 \n"
		"in vec4 vs_fs_color; \n"
		"void main() \n"
		"{ \n"
		"   gl_FragColor = vs_fs_color; \n"
		"} \n";

	return piglit_build_simple_program(vs_text, fs_text);
}


void
piglit_init(int argc, char **argv)
{
	GLint color_uniform;
	GLuint program;

	have_dsa = piglit_is_extension_supported("GL_ARB_direct_state_access");

	program = make_program();
	glUseProgram(program);

	color_uniform = glGetUniformLocation(program, "color");
	glUniform4fv(color_uniform, 1, green);

	vertex_attrib = glGetAttribLocation(program, "vertex");
}


enum piglit_result
piglit_display(void)
{
	bool pass = test_buffers();
	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
