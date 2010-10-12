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
 *
 * Authors:
 *    Ben Holmes <shranzel@hotmail.com>
 */

/*
 * tests for bug fdo 23746. The bug prevents glUseProgram from working when
 * called within a display list.
 */

#include "piglit-util.h"

int piglit_width = 400, piglit_height = 300;
int piglit_window_mode = GLUT_RGB | GLUT_DOUBLE;

static GLint progr;
static GLint progg;
static GLint fsr;
static GLint fsg;
static GLint vs;
static GLuint list;

static GLfloat vertices[12] = {150.0, 125.0, 0.0,
				150.0, 175.0, 0.0,
				100.0, 125.0, 0.0,
				100.0, 175.0, 0.0};

static const char *vertShaderText =
	"void main()\n"
	"{ \n"
	"	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
	"} \n";

static const char *fragShaderTextRed =
	"void main()\n"
	"{ \n"
	"	gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
	"} \n";

static const char *fragShaderTextGreen =
	"void main()\n"
	"{ \n"
	"	gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);\n"
	"} \n";

static void
compileLinkProg(void)
{
	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vertShaderText);
	fsr = piglit_compile_shader_text(GL_FRAGMENT_SHADER, fragShaderTextRed);
	fsg =
	  piglit_compile_shader_text(GL_FRAGMENT_SHADER, fragShaderTextGreen);

	progr = piglit_link_simple_program(vs, fsr);
	progg = piglit_link_simple_program(vs, fsg);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat),
				vertices);

	glEnableVertexAttribArray(0);

	list = glGenLists(1);
	glNewList(list, GL_COMPILE);
		glUseProgram(progg);
	glEndList();

}

void
piglit_init(int argc, char **argv)
{
	if (!GLEW_VERSION_2_0) {
		printf("Requires OpenGL 2.0\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	glClearColor(0.2, 0.2, 0.2, 1.0);

	compileLinkProg();
}

enum piglit_result
piglit_display(void)
{
	GLfloat green[3] = {0.0, 1.0, 0.0};
	GLboolean pass = GL_TRUE;

	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(progr);
	glCallList(list);

	glDrawArrays(GL_TRIANGLE_STRIP,0,4);

	pass = piglit_probe_pixel_rgb(125, 150, green);

	glFinish();
	glutSwapBuffers();

	return pass ? PIGLIT_SUCCESS : PIGLIT_FAILURE;
}
