/*
 * Copyright © 2019 Intel Corporation
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

/** @file re-init.c
 *
 * Test checks that TBO re-init works correctly
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_core_version = 31;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;
PIGLIT_GL_TEST_CONFIG_END

#define NUMBER_OF_COLORS 4
#define NUMBER_OF_TBO NUMBER_OF_COLORS
/***
 * Reinit TBO with different data several
 * times just to make sure
 ***/
#define NUMBER_OF_TBO_REINIT 12

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

struct texbo {
	GLuint tex;
	GLuint bo;
};

struct texbo create_tbo();
void init_tbo_data(struct texbo *tbo, const unsigned char *color);
void destroy_tbo(struct texbo *tbo);

enum piglit_result
piglit_display(void)
{
	bool pass;
	GLuint prog;
	struct texbo tbo_array[NUMBER_OF_TBO];

	const unsigned char pink[] = { 255, 0, 128, 255 };
	const unsigned char colors[NUMBER_OF_COLORS][4] = {
		{ 255,   0,   0, 255 },
		{   0, 255,   0, 255 },
		{   0,   0, 255, 255 },
		{ 255, 255, 255, 255 }
	};

	for (int i = 0; i < NUMBER_OF_TBO; i++) {
		tbo_array[i] = create_tbo();
	}

	prog = piglit_build_simple_program(vs_source, fs_source);
	glUseProgram(prog);

	glUniform1i(glGetUniformLocation(prog, "s"), 0);

	const float line_width = 2.0f / NUMBER_OF_TBO;
	//first init using pink color and draw with it.
	for (int i = 0; i < NUMBER_OF_TBO; i++) {
		init_tbo_data(&tbo_array[i], pink);
		glUniform1i(glGetUniformLocation(prog, "offset"), i);
		glBindTexture(GL_TEXTURE_BUFFER, tbo_array[i].tex);
		piglit_draw_rect(i * line_width - 1.0f,
						-1.0f, line_width, 2.0f);
	}
	//change colors and draw
	for (int tbo_reinit = 0; tbo_reinit < NUMBER_OF_TBO_REINIT; tbo_reinit++) {
		for (int i = 0; i < NUMBER_OF_TBO; i++) {
			const int idx = (i + tbo_reinit) % NUMBER_OF_COLORS;
			init_tbo_data(&tbo_array[i], colors[idx]);
			glUniform1i(glGetUniformLocation(prog, "offset"), i);
			glBindTexture(GL_TEXTURE_BUFFER, tbo_array[i].tex);
			piglit_draw_rect(i * line_width - 1.0f,
							-1.0f, line_width, 2.0f);
		}
	}

	glFinish();
	pass = piglit_check_gl_error(GL_NO_ERROR);

	int piglit_line_width = piglit_width * (line_width / 2);
	for (int i = 0; i < NUMBER_OF_TBO; i++) {
		const int idx = (i + (NUMBER_OF_TBO_REINIT - 1)) % NUMBER_OF_COLORS;
		const unsigned char *expected = colors[idx];
		const float color[] = {
			expected[0] / 255.0f,
			expected[1] / 255.0f,
			expected[2] / 255.0f,
			expected[3] / 255.0f
		};
		pass = pass && piglit_probe_rect_rgba(piglit_line_width * i,
												0, piglit_line_width,
												piglit_height, color);
	}

	piglit_present_results();

	for (int i = 0; i < NUMBER_OF_TBO; i++) {
		destroy_tbo(&tbo_array[i]);
	}
	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
}

struct texbo create_tbo()
{
	struct texbo tbo;
	glGenBuffers(1, &tbo.bo);
	glBindBuffer(GL_TEXTURE_BUFFER, tbo.bo);

	glGenTextures(1, &tbo.tex);
	glBindTexture(GL_TEXTURE_BUFFER, tbo.tex);
	glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA8, tbo.bo);

	glBindBuffer(GL_TEXTURE_BUFFER, 0);
	glBindTexture(GL_TEXTURE_BUFFER, 0);
	return tbo;
}

void init_tbo_data(struct texbo *tbo, const unsigned char *color)
{
	glBindBuffer(GL_TEXTURE_BUFFER, tbo->bo);
	const unsigned components = 4;//RGBA8
	const unsigned component_size = 1;//RGBA8
	const unsigned total_size = NUMBER_OF_TBO
								* components
								* component_size;
	unsigned char *data =
			(unsigned char *)malloc(total_size);
	for (int i = 0; i < NUMBER_OF_TBO; i++) {
		data[i * components + 0] = color[0];
		data[i * components + 1] = color[1];
		data[i * components + 2] = color[2];
		data[i * components + 3] = color[3];
	}
	//invalidate/alloc the buffer and init it by the data
	glBufferData(GL_TEXTURE_BUFFER, total_size, NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_TEXTURE_BUFFER, 0, total_size, data);
	glBindBuffer(GL_TEXTURE_BUFFER, 0);
	free(data);
}

void destroy_tbo(struct texbo *tbo)
{
	glDeleteBuffers(1, &tbo->bo);
	glDeleteTextures(1, &tbo->tex);
}