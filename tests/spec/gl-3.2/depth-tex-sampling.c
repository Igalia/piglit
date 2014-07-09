/*
 * Copyright (c) 2013 VMware, Inc.
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

/*
 * The section 3.8.7 (page 160) of the GL 3.2 core specification says:
 *
 * "Depth textures and the depth components of depth/stencil textures can
 * be treated as RED textures during texture filtering and application
 * (see section 3.8.15). The initial state for depth and depth/stencil
 * textures treats them as RED textures."
 *
 * Brian Paul
 * 5 Dec 2013
 */


#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_core_version = 32;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;
PIGLIT_GL_TEST_CONFIG_END


#define TEX_SIZE 64

static GLuint tex, prog, vao;


static GLuint
make_depth_texture(void)
{
	GLfloat texels[TEX_SIZE][TEX_SIZE];
	GLuint tex, i, j;
	GLenum format = GL_DEPTH_COMPONENT;

	/* Z = 0 at bottom, Z = 1 at top */
	for (i = 0; i < TEX_SIZE; i++) {
		for (j = 0; j < TEX_SIZE; j++) {
			float z = i / (float) (TEX_SIZE - 1);
			texels[i][j] = z;
		}
	}

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexImage2D(GL_TEXTURE_2D, 0, format,
		     TEX_SIZE, TEX_SIZE, 0,
		     format, GL_FLOAT, texels);

	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		piglit_report_result(PIGLIT_FAIL);
	}

	/* this call should generate an error in the core profile */
	glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);
	if (!piglit_check_gl_error(GL_INVALID_ENUM)) {
		piglit_report_result(PIGLIT_FAIL);
	}

	return tex;
}


static GLuint
make_shader_program(void)
{
	static const char *vs_text =
		"#version 150\n"
		"in vec4 pos_in;\n"
		"in vec2 texcoord_in;\n"
		"smooth out vec2 texcoord;\n"
		"void main()\n"
		"{\n"
		"   gl_Position = pos_in;\n"
		"   texcoord = texcoord_in;\n"
		"}\n";
	static const char *fs_text =
		"#version 150\n"
		"uniform sampler2D tex;\n"
		"smooth in vec2 texcoord;\n"
		"out vec4 color;\n"
		"void main()\n"
		"{\n"
		"   color = texture(tex, texcoord);\n"
		"   // enabling the next line fixes NVIDIA failure\n"
		"   // color = vec4(vec3(color.x), 1.0);\n"
		"}\n";

	GLuint prog;
	GLint u;

	prog = piglit_build_simple_program(vs_text, fs_text);
	glUseProgram(prog);

	u = glGetUniformLocation(prog, "tex");
	glUniform1i(u, 0);  /* bind tex unit 0, just to be safe */

	glBindAttribLocation(prog, 0, "pos_in");
	glBindAttribLocation(prog, 1, "texcoord_in");

	glLinkProgram(prog);

	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		piglit_report_result(PIGLIT_FAIL);
	}

	return prog;
}


static GLuint
make_vao(void)
{
	static const float pos_tc[4][4] = {
		{ -1.0, -1.0,  0.0, 0.0 },
		{  1.0, -1.0,  1.0, 0.0 },
		{  1.0,  1.0,  1.0, 1.0 },
		{ -1.0,  1.0,  0.0, 1.0 }
	};
	const int stride = sizeof(pos_tc[0]);
	GLuint vbo, vao;

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(pos_tc), pos_tc, GL_STATIC_DRAW);
	piglit_check_gl_error(GL_NO_ERROR);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, (void *) 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void *) 8);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		piglit_report_result(PIGLIT_FAIL);
	}

	return vbo;
}


void
piglit_init(int argc, char **argv)
{
	tex = make_depth_texture();
	prog = make_shader_program();
	vao = make_vao();
}


enum piglit_result
piglit_display(void)
{
	static const float black[4] = { 0.0, 0.0, 0.0, 1.0 };
	static const float red50[4] = { 0.5, 0.0, 0.0, 1.0 };
	static const float red100[4] = { 1.0, 0.0, 0.0, 1.0 };
	bool pass = true;

	glViewport(0, 0, piglit_width, piglit_height);

	/* This should draw a red gradient ranging from black
	 * at the bottom of the window to full red at the top.
	 */
	glClearColor(0.2, 0.2, 0.8, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	if (!piglit_probe_pixel_rgba(0, 0, black))
		pass = false;

	if (!piglit_probe_pixel_rgba(0, piglit_height/2, red50))
		pass = false;

	if (!piglit_probe_pixel_rgba(0, piglit_height-1, red100))
		pass = false;

	piglit_present_results();

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
