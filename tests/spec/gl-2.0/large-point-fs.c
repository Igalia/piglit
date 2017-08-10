/*
 * Copyright 2015 VMware, Inc.
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
 * Test drawing large points with a fragment shader.
 * Brian Paul
 * July 2015
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 20;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;
PIGLIT_GL_TEST_CONFIG_END


static const char *vertShaderText =
	"#version 120 \n"
	"uniform vec4 color_bias; \n"
	"varying vec4 color_bias_varying; \n"
	" \n"
	"void main() \n"
	"{ \n"
	"   gl_Position = ftransform(); \n"
	"   color_bias_varying = color_bias; \n"
	"   gl_FrontColor = gl_Color; \n"
	"} \n";

static const char *fragShaderText =
	"varying vec4 color_bias_varying; \n"
	"uniform vec4 color_scale; \n"
	"varying vec4 color;\n"
	"void main()\n"
	"{ \n"
	"   gl_FragColor = gl_Color * color_scale + color_bias_varying; \n"
	"} \n";


static int color_scale_uniform, color_bias_uniform;


enum piglit_result
piglit_display(void)
{
	const float colors[4][4] = {
		{ 0.5, 0, 1, 1 },
		{ 0, .5, 1, 1 },
		{ 0.0, 0, 0.5, 0.5 },
		{ 0.25, 0, 0.25, 0.25 }
	};
	const float scale[4] = { 2.0, 3.0, 0.0, 0.0 };
	const float bias[4] = { 0, 0, 0.5, 0.5 };
	float expected[4][4];
	bool pass = true;
	GLfloat size, probeSize;
	int i;

	/* compute expected colors */
	for (i = 0; i < 4; i++) {
		expected[i][0] = MIN2(1.0, colors[i][0] * scale[0] + bias[0]);
		expected[i][1] = MIN2(1.0, colors[i][1] * scale[1] + bias[1]);
		expected[i][2] = MIN2(1.0, colors[i][2] * scale[2] + bias[2]);
		expected[i][3] = MIN2(1.0, colors[i][3] * scale[3] + bias[3]);
	}

	glGetFloatv(GL_POINT_SIZE_MAX, &size);
        if (size < 3.0) {
           /* legal, but unusual */
           printf("Max point size is %g pixel(s)\n", size);
           fflush(stdout);
           return PIGLIT_SKIP;
        }

	size = MIN2(30, size);
	probeSize = size - 2; /* to accomodate small rasterization errors */

	glUniform4fv(color_scale_uniform, 1, scale);
	glUniform4fv(color_bias_uniform, 1, bias);

	glPointSize(size);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, piglit_width, 0, piglit_height, -1, 1);

	glClear(GL_COLOR_BUFFER_BIT);

	/* draw row of four lage points, each with different color */
	glBegin(GL_POINTS);
	for (i = 0; i < 4; i++) {
		float x = size / 2 + i * size;
		float y = size / 2;
		glColor4fv(colors[i]);
		glVertex2f(x, y);
	}
	glEnd();

	/* check results */
	for (i = 0; i < 4; i++) {
		int x = i * size + 1;
		int y = 1;

		if (!piglit_probe_rect_rgba(x, y,
					    probeSize, probeSize, expected[i]))
			pass = false;
	}
	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


void
piglit_init(int argc, char **argv)
{
	GLuint prog;

	piglit_require_gl_version(20);

	prog = piglit_build_simple_program(vertShaderText, fragShaderText);
	if (!prog) {
		printf("Failed to compile/link program\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	glUseProgram(prog);

	color_bias_uniform = glGetUniformLocation(prog, "color_bias");
	color_scale_uniform = glGetUniformLocation(prog, "color_scale");
}
