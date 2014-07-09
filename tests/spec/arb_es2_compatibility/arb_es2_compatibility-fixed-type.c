/*
 * Copyright © 2011 Marek Olšák <maraeo@gmail.com>
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
 */

#include "piglit-util-gl.h"
#include <stdarg.h>

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 250;
	config.window_height = 250;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static unsigned verts[4*4*4];

static unsigned opos[8] = {
	50,  50,
	150, 50,
	50,  150,
	150, 150
};

static float ocol[16] = {
	0.1, 0.5, 0.9, 0.4,
	0.3, 0.95, 0.1, 0.6,
	0.8, 0.45, 0, 0,
	0.6, 0.6, 0.35, 0.15
};

static unsigned vpos[8] = {
	0,  0,
	50, 0,
	50, 50,
	0,  50
};

static const char *vertShaderText =
	"void main()\n"
	"{ \n"
	"	gl_Position = gl_ModelViewProjectionMatrix * floor(gl_Vertex);\n"
	"	gl_FrontColor = fract(gl_Vertex);\n"
	"} \n";

static void expect_error(GLenum expect, const char * where, ...)
{
	GLenum error = glGetError();
	if (error != expect) {
		va_list va;

		fprintf(stderr, "Expected OpenGL error 0x%04x, got 0x%04x\nat: ", expect, error);

		va_start(va, where);
		vfprintf(stderr, where, va);
		va_end(va);
		fprintf(stderr, "\n");

		piglit_report_result(PIGLIT_FAIL);
	}
}

void
piglit_init(int argc, char **argv)
{
	GLuint prog;
	int i,j;

	piglit_require_gl_version(20);

	piglit_require_extension("GL_ARB_ES2_compatibility");
	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	glClearColor(0.2, 0.2, 0.2, 0.2);

	prog = piglit_build_simple_program(vertShaderText, NULL);

	glUseProgram(prog);

	glVertexPointer(4, GL_FIXED, 0, verts);
	expect_error(GL_INVALID_ENUM, "glVertexPointer should not accept GL_FIXED.");
	glNormalPointer(GL_FIXED, 0, verts);
	expect_error(GL_INVALID_ENUM, "glNormalPointer should not accept GL_FIXED.");
	glColorPointer(4, GL_FIXED, 0, verts);
	expect_error(GL_INVALID_ENUM, "glColorPointer should not accept GL_FIXED.");
	glTexCoordPointer(4, GL_FIXED, 0, verts);
	expect_error(GL_INVALID_ENUM, "glTexCoordPointer should not accept GL_FIXED.");

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FIXED, GL_FALSE, 0, verts);
	expect_error(GL_NO_ERROR, "glVertexAttribPointer should accept GL_FIXED.");

	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			verts[(i*4+j)*4+0] = ((opos[i*2+0] + vpos[j*2+0]) << 16) |
					     (unsigned)(ocol[i*4+0] * 65535);
			verts[(i*4+j)*4+1] = ((opos[i*2+1] + vpos[j*2+1]) << 16) |
					     (unsigned)(ocol[i*4+1] * 65535);
			verts[(i*4+j)*4+2] = (unsigned)(ocol[i*4+2] * 65535);
			verts[(i*4+j)*4+3] = (1 << 16) |
					     (unsigned)(ocol[i*4+3] * 65535);
		}
	}
}

enum piglit_result
piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	int i;

	glClear(GL_COLOR_BUFFER_BIT);

	glDrawArrays(GL_QUADS, 0, 16);

	for (i = 0; i < 4; i++) {
		pass = piglit_probe_pixel_rgba(opos[i*2]+25, opos[i*2+1]+25, &ocol[i*4]) && pass;
	}

 	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
