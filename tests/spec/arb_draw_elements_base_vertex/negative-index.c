/*
 * Copyright © 2010 Marek Olšák <maraeo@gmail.com>
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
 *    Marek Olšák <maraeo@gmail.com>
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 320;
	config.window_height = 80;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

GLboolean user_va = GL_FALSE;

void piglit_init(int argc, char **argv)
{
	unsigned i;

	for (i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "user_varrays")) {
			user_va = GL_TRUE;
			puts("Testing user vertex arrays.");
		}
	}

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	piglit_require_gl_version(15);

	piglit_require_extension("GL_ARB_draw_elements_base_vertex");

	glShadeModel(GL_FLAT);
	glClearColor(0.2, 0.2, 0.2, 1.0);
}

static GLuint vboVertexPointer(GLint size, GLenum type, GLsizei stride,
                               const GLvoid *buf, GLsizei bufSize, intptr_t bufOffset)
{
	GLuint id;
	if (user_va) {
		glVertexPointer(size, type, stride, (char*)buf + bufOffset);
		return 0;
	}
	glGenBuffers(1, &id);
	glBindBuffer(GL_ARRAY_BUFFER, id);
	glBufferData(GL_ARRAY_BUFFER, bufSize, buf, GL_STATIC_DRAW);
	glVertexPointer(size, type, stride, (void*)bufOffset);
	return id;
}

static GLuint vboElementPointer(const GLvoid *buf, GLsizei bufSize)
{
	GLuint id;
	if (user_va) {
		return 0;
	}
	glGenBuffers(1, &id);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, bufSize, buf, GL_STATIC_DRAW);
	return id;
}

static void test_negative_index_offset(float x1, float y1, float x2, float y2, int index)
{
	float v2[] = {
		x1, y1,
		x1, y2,
		x2, y1
	};
	int indices[] = {
		index, index+1, index+2
	};
	GLuint vbo, ib;

	vbo = vboVertexPointer(2, GL_FLOAT, 0, v2, sizeof(v2), 0);
	ib = vboElementPointer(indices, sizeof(indices));
	glDrawElementsBaseVertex(GL_TRIANGLES, 3, GL_UNSIGNED_INT, user_va ? indices : NULL, -index);
	if (vbo)
		glDeleteBuffers(1, &vbo);
	if (ib)
		glDeleteBuffers(1, &ib);
}

enum piglit_result piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	unsigned i;
	float x = 0, y = 0;
	float expected[] = {1,1,1};

	glClear(GL_COLOR_BUFFER_BIT);
	glEnableClientState(GL_VERTEX_ARRAY);

	for (i = 0; i < 63; i++) {
		int index = (int)pow(i, 5.2)+1;
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		printf("BaseVertex = -%i\n", index);
		test_negative_index_offset(x, y, x+20, y+20, index);
		if (!piglit_check_gl_error(GL_NO_ERROR))
		        piglit_report_result(PIGLIT_FAIL);
		pass = piglit_probe_pixel_rgb(x+5, y+5, expected) && pass;

		x += 20;
		if (x > 300) {
			x = 0;
			y += 20;
		}
	}

	glFinish();
	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
