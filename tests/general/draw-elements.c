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

/* The test for some tricky bits of the OpenGL vertex submission.
 * The emphasis is taken on non-dword-aligned offsets and various
 * elements formats.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 320;
	config.window_height = 60;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

GLboolean user = GL_FALSE;

void piglit_init(int argc, char **argv)
{
	unsigned i;

	for (i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "user")) {
			user = GL_TRUE;
			puts("Testing user arrays.");
		}
	}

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	piglit_require_gl_version(15);

	glShadeModel(GL_FLAT);
	glClearColor(0.2, 0.2, 0.2, 1.0);
}

static void test_ubyte_indices(float x1, float y1, float x2, float y2, int index)
{
	float v[] = {
		x1, y1,
		x1, y2,
		x2, y1,

		x1, y1,
		x1, y2,
		x2, y1,

		x1, y1,
		x1, y2,
		x2, y1,

		x1, y1,
		x1, y2,
		x2, y1
	};
	unsigned char indx[] = {
		/*aligned:*/ 0, 1, 2, /*unused:*/ 2,
		2, 2, 3, 3,
		3, /* unaligned:*/ 3, 4, 5,
		/*unused:*/ 5, 5, 5, 6,
		6, 6, /*unaligned:*/ 6, 7,
		8, /*unused:*/ 8, 8, 8,
		9, 9, 9, /*unaligned:*/ 9,
		10, 11, /*unused:*/ 11, 11,
		11, 11, 11, 11
	};
	GLuint buf;

	glVertexPointer(2, GL_FLOAT, 0, v);

	if (!user) {
		glGenBuffers(1, &buf);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buf);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indx), indx, GL_STATIC_DRAW);
		glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_BYTE, (void*)(intptr_t)(index*9));
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glDeleteBuffers(1, &buf);
	} else {
		glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_BYTE, indx + index*9);
	}
}

static void test_ushort_indices(float x1, float y1, float x2, float y2, int index)
{
	float v[] = {
		x1, y1,
		x1, y2,
		x2, y1,

		x1, y1,
		x1, y2,
		x2, y1
	};
	unsigned short indx[] = {
		/*aligned:*/ 0, 1,
		2, /*unused:*/ 2,
		2, 2,
		3, 3,
		3, /* unaligned:*/ 3,
		4, 5,
		/*unused:*/ 5, 5,
		5, 5
	};
	GLuint buf;

	glVertexPointer(2, GL_FLOAT, 0, v);

	if (!user) {
		glGenBuffers(1, &buf);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buf);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indx), indx, GL_STATIC_DRAW);
		glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, (void*)(intptr_t)(index*18));
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glDeleteBuffers(1, &buf);
	} else {
		glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, (char*)indx + index*18);
	}
}

static void test_large_index_count(float x1, float y1, float x2, float y2, int index)
{
	float v[] = {
		x1, y1,
		x1, y2,
		x2, y1
	};
	unsigned tris = 100000;
	unsigned *indx = (unsigned*)malloc(sizeof(unsigned) * 3 * tris);
	unsigned i;

	/* A large index count for DrawElements */
	for (i = 0; i < tris*3; i += 3) {
		indx[i+0] = 0;
		indx[i+1] = 1;
		indx[i+2] = 2;
	}

	glVertexPointer(2, GL_FLOAT, 0, v);
	glDrawElements(GL_TRIANGLES, tris*3, GL_UNSIGNED_INT, indx);

	free(indx);
}

static void test_large_indexbuf_offset(float x1, float y1, float x2, float y2, int index)
{
	float v[] = {
		x1, y1,
		x1, y2,
		x2, y1
	};
	GLuint buf;
	unsigned *map;
	unsigned num = 1000000;

	glGenBuffers(1, &buf);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buf);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, num*4, 0, GL_STATIC_DRAW);
	map = (unsigned*)glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
	memset(map, 0, num*4);
	map[num-2] = 1;
	map[num-1] = 2;
	glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);

	glVertexPointer(2, GL_FLOAT, 0, v);
	glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (void*)(intptr_t)((num-3)*4));

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &buf);
}

enum {
	USER,
	VBO,
	BOTH
};
struct test {
	void (*test)(float x1, float y1, float x2, float y2, int index);
	int index;
	float expected_color[3];
	int flag;
	const char *name;
};

struct test tests[] = {
	{test_ubyte_indices, 0, {1, 1, 1}, BOTH, "Ubyte indices - offset: 0"},
	{test_ubyte_indices, 1, {1, 1, 1}, BOTH, "Ubyte indices - offset: 1"},
	{test_ubyte_indices, 2, {1, 1, 1}, BOTH, "Ubyte indices - offset: 2"},
	{test_ubyte_indices, 3, {1, 1, 1}, BOTH, "Ubyte indices - offset: 3"},

	{test_ushort_indices, 0, {1, 1, 1}, BOTH, "Ushort indices - offset: 0"},
	{test_ushort_indices, 1, {1, 1, 1}, BOTH, "Ushort indices - offset: 2"},

	{test_large_index_count, 1, {1, 1, 1}, USER, "Large index count"},
	{test_large_indexbuf_offset, 0, {1, 1, 1}, VBO, "Large index offset"},

	{0}
};

enum piglit_result
		piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	unsigned i;
	float x = 0, y = 0;

	glClear(GL_COLOR_BUFFER_BIT);
	glEnableClientState(GL_VERTEX_ARRAY);

	for (i = 0; tests[i].test; i++) {
		if (user && tests[i].flag == VBO)
			continue;
		if (!user && tests[i].flag == USER)
			continue;

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		printf("%s\n", tests[i].name);
		tests[i].test(x, y, x+20, y+20, tests[i].index);
		assert(glGetError() == 0);
		pass = piglit_probe_pixel_rgb(x+5, y+5, tests[i].expected_color) && pass;

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
