/*
 * Copyright (c) The Piglit project 2008
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.  IN NO EVENT SHALL
 * VA LINUX SYSTEM, IBM AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/**
 * @file
 * Test basic line stippling functionality.
 */

#include "piglit-util-gl-common.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.window_width = 128;
	config.window_height = 128;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static void probe_pixel(int x, int y, const float* expected)
{
	if (!piglit_probe_pixel_rgb(x, y, expected)) {
		if (piglit_automatic)
			piglit_report_result(PIGLIT_FAIL);
	}
}

struct vertex {
	GLuint x;
	GLuint y;
};

struct stipple_line {
	const char *name;
	GLint factor;
	GLuint pattern;
	GLfloat color[3];
	GLuint primitive;
	GLuint nvertices;
	struct vertex *vertices;
};

static const int basex = 10;
static const int basey = 10;

static float background[3] = { 0, 0, 0 };

/**
 * @note Only horizontal and vertical lines supported right now.
 */
static GLuint probe_line(const struct stipple_line *line, const struct vertex *v1, const struct vertex *v2, GLuint fragment)
{
	int x = v1->x;
	int y = v1->y;
	int length, dx = 0, dy = 0;

	if (v2->x != v1->x) {
		if (v2->x > v1->x)
			dx = 1;
		else
			dx = -1;
		length = (v2->x - v1->x)*dx;
	} else {
		if (v2->y > v1->y)
			dy = 1;
		else
			dy = -1;
		length = (v2->y - v1->y)*dy;
	}

	while(length) {
		GLuint s = (fragment/line->factor) & 15;
		if (line->pattern & (1 << s))
			probe_pixel(basex+x, basey+y, line->color);
		else
			probe_pixel(basex+x, basey+y, background);

		length--;
		fragment++;
		x += dx;
		y += dy;
	}

	return fragment;
}

static void test_line(const struct stipple_line *line)
{
	GLuint i;

	glLineStipple(line->factor, line->pattern);
	glColor3f(line->color[0], line->color[1], line->color[2]);
	glBegin(line->primitive);
	for(i = 0; i < line->nvertices; ++i)
		glVertex2f(line->vertices[i].x + 0.5, line->vertices[i].y + 0.5);
	glEnd();

	glReadBuffer(GL_BACK);
	if (line->primitive == GL_LINES) {
		for(i = 0; i+1 < line->nvertices; i += 2)
			probe_line(line, &line->vertices[i], &line->vertices[i+1], 0);
	} else {
		GLuint fragment = 0;
		for(i = 0; i+1 < line->nvertices; ++i)
			fragment = probe_line(line, &line->vertices[i], &line->vertices[i+1], fragment);
		if (line->primitive == GL_LINE_LOOP)
			probe_line(line, &line->vertices[i], &line->vertices[0], fragment);
	}
}

static struct vertex BaselineVertices[] = { { 0, 0 }, { 24, 0 } };
static struct vertex RestartVertices[] = { { 0, 2 }, { 24, 2 }, { 0, 4 }, { 24, 4 } };
static struct vertex LinestripVertices[] = { { 0, 6 }, { 24, 6 }, { 24, 30 } };
static struct vertex LineloopVertices[] = { { 26, 0 }, { 46, 0 }, { 46, 20 }, { 26, 20 } };
static struct vertex Factor2Vertices[] = { { 0, 32 }, { 32, 32 }, { 32, 33 }, { 0, 33 } };
static struct vertex Factor3Vertices[] = { { 0, 35 }, { 63, 35 }, { 63, 36 }, { 0, 36 } };
static struct stipple_line Lines[] = {
	{ /* Baseline */
		"Baseline",
		1, 0xffff, { 1.0, 1.0, 1.0 },
		GL_LINES, 2,
		BaselineVertices
	},
	{ /* Restarting lines within a single Begin/End block */
		"Restarting lines within a single Begin/End block",
		1, 0x00ff, { 1.0, 0.0, 0.0 },
		GL_LINES, 4,
		RestartVertices
	},
	{ /* Line strip */
		"Line strip",
		1, 0x0f8f, { 1.0, 1.0, 0.0 },
		GL_LINE_STRIP, 3,
		LinestripVertices
	},
	{ /* Line loop */
		"Line loop",
		1, 0x8cef, { 0.0, 1.0, 0.0 },
		GL_LINE_LOOP, 4,
		LineloopVertices
	},
	{ /* Factor 2x */
		"Factor 2x",
		2, 0x838f, { 0.0, 0.0, 1.0 },
		GL_LINE_LOOP, 4,
		Factor2Vertices
	},
	{ /* Factor 3x */
		"Factor 3x",
		3, 0xf731, { 0.0, 1.0, 1.0 },
		GL_LINE_LOOP, 4,
		Factor3Vertices
	}
};

static void test(void)
{
	int i;

	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glEnable(GL_LINE_STIPPLE);

	glPushMatrix();
	glTranslatef(basex, basey, 0.0);

	for(i = 0; i < sizeof(Lines)/sizeof(Lines[0]); ++i) {
		puts(Lines[i].name);
		test_line(&Lines[i]);
	}
	glPopMatrix();

	piglit_present_results();
}


enum piglit_result
piglit_display(void)
{
	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);
	test();

	return PIGLIT_PASS;
}

void piglit_init(int argc, char **argv)
{
}
