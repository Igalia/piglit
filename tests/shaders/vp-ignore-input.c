/*
 * Copyright © 2009 Intel Corporation
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

// author: Ben Holmes

/*
 * draws using a vertex program that ignores inputs and instead just
 * writes a constant to gl_Position.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 400;
	config.window_height = 300;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static GLint prog;
static GLint fs;
static GLint vs;


static GLfloat vertices[12] = {150.0, 125.0, 0.0,
                                150.0, 175.0, 0.0,
                                100.0, 125.0, 0.0,
                                100.0, 175.0, 0.0};

static const char *vertShaderText =
	"void main()\n"
	"{ \n"
	"	gl_Position = vec4(100, 50, 0, 0);\n"
	"} \n";

static const char *fragShaderText =
	"void main()\n"
	"{ \n"
        "       gl_FragColor = vec4(0.0, 1.0, 1.0, 1.0);\n"
	"} \n";

static void compileLinkProg(void);

void
piglit_init(int argc, char **argv)
{
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0, 400, 0, 300, -1, 1);

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

	glClearColor(0.2, 0.2, 0.2, 1.0);

	piglit_require_gl_version(20);

	compileLinkProg();
}

static void
compileLinkProg(void)
{
	GLint stat;

	vs = glCreateShader(GL_VERTEX_SHADER);
	fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(vs, 1, (const GLchar **) &vertShaderText, NULL);
	glShaderSource(fs, 1, (const GLchar **) &fragShaderText, NULL);
	glCompileShader(vs);
	glGetShaderiv(vs, GL_COMPILE_STATUS, &stat);
	if (!stat) {
                printf("error compiling vertex shader!\n");
                exit(1);
        }
	glCompileShader(fs);
	glGetShaderiv(fs, GL_COMPILE_STATUS, &stat);
	if (!stat) {
		printf("error compiling fragment shader!\n");
		exit(1);
	}

	prog = glCreateProgram();
	glAttachShader(prog, vs);
	glAttachShader(prog, fs);
	glLinkProgram(prog);
	glUseProgram(prog);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat),
                                vertices);

        glEnableVertexAttribArray(0);
}


enum piglit_result
piglit_display(void)
{
	glClear(GL_COLOR_BUFFER_BIT);

	glDrawArrays(GL_POINTS, 0, 4);

	glPushMatrix();
	glTranslatef(75.0, 0.0, 0.0);

	glDrawArrays(GL_POINTS, 0, 4);

	glPopMatrix();

	glFinish();
        piglit_present_results();

	return PIGLIT_PASS;
}
