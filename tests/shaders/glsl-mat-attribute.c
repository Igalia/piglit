/*
 * Copyright © 2010 Intel Corporation
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

/**
 * \file glsl-mat-attribute.c
 * Test shaders that use matrix attributes.
 *
 * \author Ian Romanick <ian.d.romanick@intel.com>
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_DEPTH;

PIGLIT_GL_TEST_CONFIG_END

static GLint prog[4];

static const float pos[4 * 4 * 2] = {
	10.0, 10.0, 20.0, 10.0, 20.0, 20.0, 10.0, 20.0,
	30.0, 10.0, 40.0, 10.0, 40.0, 20.0, 30.0, 20.0,
	50.0, 10.0, 60.0, 10.0, 60.0, 20.0, 50.0, 20.0,
	70.0, 10.0, 80.0, 10.0, 80.0, 20.0, 70.0, 20.0,
};

static const float color_mat_col1[4 * 4 * 4] = {
	40.0, 20.0, 10.0, 5.0,
	60.0, 40.0, 20.0, 10.0,
	80.0, 60.0, 30.0, 15.0,
	100.0, 80.0, 40.0, 20.0,

	999.0, 0.0, 0.0, 999.0,
	999.0, 0.0, 0.0, 999.0,
	999.0, 0.0, 0.0, 999.0,
	999.0, 0.0, 0.0, 999.0,

	999.0, 0.0, 0.0, 999.0,
	999.0, 0.0, 0.0, 999.0,
	999.0, 0.0, 0.0, 999.0,
	999.0, 0.0, 0.0, 999.0,

	999.0, 0.0, 0.0, 999.0,
	999.0, 0.0, 0.0, 999.0,
	999.0, 0.0, 0.0, 999.0,
	999.0, 0.0, 0.0, 999.0,
};

static const float color_mat_col2[4 * 4 * 4] = {
	999.0, 0.0, 0.0, 999.0,
	999.0, 0.0, 0.0, 999.0,
	999.0, 0.0, 0.0, 999.0,
	999.0, 0.0, 0.0, 999.0,

	40.0, 20.0, 10.0, 5.0,
	60.0, 40.0, 20.0, 10.0,
	80.0, 60.0, 30.0, 15.0,
	100.0, 80.0, 40.0, 20.0,

	999.0, 0.0, 0.0, 999.0,
	999.0, 0.0, 0.0, 999.0,
	999.0, 0.0, 0.0, 999.0,
	999.0, 0.0, 0.0, 999.0,

	999.0, 0.0, 0.0, 999.0,
	999.0, 0.0, 0.0, 999.0,
	999.0, 0.0, 0.0, 999.0,
	999.0, 0.0, 0.0, 999.0,
};

static const float color_mat_col3[4 * 4 * 4] = {
	999.0, 0.0, 0.0, 999.0,
	999.0, 0.0, 0.0, 999.0,
	999.0, 0.0, 0.0, 999.0,
	999.0, 0.0, 0.0, 999.0,

	999.0, 0.0, 0.0, 999.0,
	999.0, 0.0, 0.0, 999.0,
	999.0, 0.0, 0.0, 999.0,
	999.0, 0.0, 0.0, 999.0,

	40.0, 20.0, 10.0, 5.0,
	60.0, 40.0, 20.0, 10.0,
	80.0, 60.0, 30.0, 15.0,
	100.0, 80.0, 40.0, 20.0,

	999.0, 0.0, 0.0, 999.0,
	999.0, 0.0, 0.0, 999.0,
	999.0, 0.0, 0.0, 999.0,
	999.0, 0.0, 0.0, 999.0,
};

static const float color_mat_col4[4 * 4 * 4] = {
	999.0, 0.0, 0.0, 999.0,
	999.0, 0.0, 0.0, 999.0,
	999.0, 0.0, 0.0, 999.0,
	999.0, 0.0, 0.0, 999.0,

	999.0, 0.0, 0.0, 999.0,
	999.0, 0.0, 0.0, 999.0,
	999.0, 0.0, 0.0, 999.0,
	999.0, 0.0, 0.0, 999.0,

	999.0, 0.0, 0.0, 999.0,
	999.0, 0.0, 0.0, 999.0,
	999.0, 0.0, 0.0, 999.0,
	999.0, 0.0, 0.0, 999.0,

	40.0, 20.0, 10.0, 5.0,
	60.0, 40.0, 20.0, 10.0,
	80.0, 60.0, 30.0, 15.0,
	100.0, 80.0, 40.0, 20.0,
};

static const float normalization[4 * 4 * 4] = {
	1.0 / 40.0, 1.0 / 20.0, 1.0 / 10.0, 1.0 / 5.0,
	1.0 / 60.0, 1.0 / 40.0, 1.0 / 20.0, 1.0 / 10.0,
	1.0 / 80.0, 1.0 / 60.0, 1.0 / 30.0, 1.0 / 15.0,
	1.0 / 100.0, 1.0 / 80.0, 1.0 / 40.0, 1.0 / 20.0,

	1.0 / 40.0, 1.0 / 20.0, 1.0 / 10.0, 1.0 / 5.0,
	1.0 / 60.0, 1.0 / 40.0, 1.0 / 20.0, 1.0 / 10.0,
	1.0 / 80.0, 1.0 / 60.0, 1.0 / 30.0, 1.0 / 15.0,
	1.0 / 100.0, 1.0 / 80.0, 1.0 / 40.0, 1.0 / 20.0,

	1.0 / 40.0, 1.0 / 20.0, 1.0 / 10.0, 1.0 / 5.0,
	1.0 / 60.0, 1.0 / 40.0, 1.0 / 20.0, 1.0 / 10.0,
	1.0 / 80.0, 1.0 / 60.0, 1.0 / 30.0, 1.0 / 15.0,
	1.0 / 100.0, 1.0 / 80.0, 1.0 / 40.0, 1.0 / 20.0,

	1.0 / 40.0, 1.0 / 20.0, 1.0 / 10.0, 1.0 / 5.0,
	1.0 / 60.0, 1.0 / 40.0, 1.0 / 20.0, 1.0 / 10.0,
	1.0 / 80.0, 1.0 / 60.0, 1.0 / 30.0, 1.0 / 15.0,
	1.0 / 100.0, 1.0 / 80.0, 1.0 / 40.0, 1.0 / 20.0,
};

enum piglit_result piglit_display(void)
{
	static const float white[4] = {
		1.0, 1.0, 1.0, 1.0,
	};
	static const float b[] = {
		1.0, 1.0, -1.0, 1.0, -1.0, -1.0, 1.0, -1.0
	};
	GLboolean pass = GL_TRUE;
	unsigned i;
	unsigned j;

	glClear(GL_COLOR_BUFFER_BIT);

	for (i = 0; i < 4; i++) {
		glUseProgram(prog[i]);
		glDrawArrays(GL_QUADS, i * 4, 4);

		for (j = 0; j < 8; j += 2) {
			const unsigned k = 8 * i;
			if (!piglit_probe_pixel_rgba(pos[k + j + 0] + b[j + 0],
						     pos[k + j + 1] + b[j + 1],
						     white))
				pass = GL_FALSE;
		}
	}

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void piglit_init(int argc, char **argv)
{
	static const char * const vs_text =
		"attribute mat4 color;\n"
		"attribute vec4 normalization;\n"
		"void main()\n"
		"{\n"
		"   gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
		"   gl_FrontColor = color[IDX] * normalization;\n"
		"}\n"
	;

	unsigned i;

	piglit_require_gl_version(20);

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	for (i = 0; i < 4; i++) {
		char buf[64];
		const char *sources[2];
		GLint stat;
		GLuint sh;
		GLboolean ok;

		sprintf(buf, "#define IDX %d\n", i);

		sh = glCreateShader(GL_VERTEX_SHADER);

		sources[0] = buf;
		sources[1] = vs_text;
		glShaderSource(sh, 2, sources, NULL);
		glCompileShader(sh);

		glGetShaderiv(sh, GL_COMPILE_STATUS, &stat);
		if (!stat) {
			printf("error compiling vertex shader!\n");
			exit(1);
		}

		prog[i] = glCreateProgram();
		glAttachShader(prog[i], sh);

		/* Since "color" is a mat4, it will occupy 4 attribute
		 * locations.
		 */
		glBindAttribLocation(prog[i], 1, "color");
		glBindAttribLocation(prog[i], 5, "normalization");

		glLinkProgram(prog[i]);
		ok = piglit_link_check_status(prog[i]);
		if (!ok)
			piglit_report_result(PIGLIT_FAIL);
	}

	glClearColor(0.3, 0.3, 0.3, 0.0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat),
			      pos);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat),
			      color_mat_col1);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat),
			      color_mat_col2);
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat),
			      color_mat_col3);
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat),
			      color_mat_col4);
	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat),
			      normalization);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);
	glEnableVertexAttribArray(4);
	glEnableVertexAttribArray(5);
}
