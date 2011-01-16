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
 *
 * Authors:
 *    Eric Anholt <eric@anholt.net>
 *
 */

/** @file arb_es2_compatibility-releasecompiler.c
 *
 * Tests that compiling a shader works again after doing
 * glReleaseShaderCompiler().
 */

#include "piglit-util.h"

int piglit_width = 100, piglit_height = 100;
int piglit_window_mode = GLUT_RGB | GLUT_ALPHA | GLUT_DOUBLE;

enum piglit_result
piglit_display(void)
{
#ifdef GL_ARB_ES2_compatibility
	GLboolean pass = GL_TRUE;
	float green[] = {0.0, 1.0, 0.0, 0.0};
	float blue[] = {0.0, 0.0, 1.0, 0.0};
	GLuint vs, fs, prog;

	static int color_location;

	/* Set up projection matrix so we can just draw using window
	 * coordinates.
	 */
	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	vs = piglit_compile_shader(GL_VERTEX_SHADER,
				   "shaders/glsl-mvp.vert");
	fs = piglit_compile_shader(GL_FRAGMENT_SHADER,
				   "shaders/glsl-uniform-color.frag");
	prog = piglit_link_simple_program(vs, fs);
	glUseProgram(prog);
	color_location = glGetUniformLocation(prog, "color");
	glUniform4fv(color_location, 1, green);
	piglit_draw_rect(0, 0, piglit_width / 2, piglit_height);
	glDeleteProgram(prog);

	glReleaseShaderCompiler();

	vs = piglit_compile_shader(GL_VERTEX_SHADER,
				   "shaders/glsl-mvp.vert");
	fs = piglit_compile_shader(GL_FRAGMENT_SHADER,
				   "shaders/glsl-uniform-color.frag");
	prog = piglit_link_simple_program(vs, fs);
	glUseProgram(prog);
	color_location = glGetUniformLocation(prog, "color");
	glUniform4fv(color_location, 1, blue);
	piglit_draw_rect(piglit_width / 2, 0,
			 piglit_width / 2 + 1, piglit_height);
	glDeleteProgram(prog);

	pass &= piglit_probe_pixel_rgba(piglit_width / 4, piglit_height / 2,
					green);
	pass &= piglit_probe_pixel_rgba(piglit_width * 3 / 4, piglit_height / 2,
					blue);

	assert(!glGetError());

	glutSwapBuffers();

	return pass ? PIGLIT_SUCCESS : PIGLIT_FAILURE;
#else
	return PIGLIT_SKIP;
#endif /* GL_ARB_ES2_compatibility */
}

void
piglit_init(int argc, char **argv)
{
#ifdef GL_ARB_ES2_compatibility
	if (!GLEW_VERSION_2_0) {
		printf("Requires OpenGL 2.0\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	if (!GLEW_ARB_ES2_compatibility) {
		printf("Requires ARB_ES2_compatibility\n");
		piglit_report_result(PIGLIT_SKIP);
	}
#endif
}
