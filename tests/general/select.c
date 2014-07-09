/*
 * Copyright © 2011
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
 *	 Pierre-Eric Pelloux-Prayer <pelloux@gmail.com>
 *
 * Description:
 *  This test represents a simple usage of GL_SELECT rendering mode.
 *  It draws several squares to screen, with various GL_..._TEST active,
 *  and then verifies the number of hits and the content of the select buffer.
 *  Based on this documentation: http://glprogramming.com/red/chapter13.html
 *
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DEPTH | PIGLIT_GL_VISUAL_STENCIL;

PIGLIT_GL_TEST_CONFIG_END

GLuint ReferenceHitEntries[3][64];
#define NAME_STACK_DEPTH	0
#define MIN_Z			1
#define MAX_Z			2
#define NAME_STACK_0		3

/**
 * Draw 4 objects and handle name stack
 */
static void
draw_objects()
{
	float zscale = (float)(~0u);
	glInitNames();

	/* no draw call issued for name '0' */
	glPushName(0);

	/* OBJECT 1 */
	glPushName(1);
	/* draw object */
	glColor3f(1.0, 0.0, 0.0);
	piglit_draw_rect_z(0.8, 10, 30, 50, 50);
	/* fill reference buffer */
	ReferenceHitEntries[0][NAME_STACK_DEPTH] = 2;
	ReferenceHitEntries[0][MIN_Z] = (GLuint)roundf(zscale * ((1 - 0.8) * 0.5));
	ReferenceHitEntries[0][MAX_Z] = ReferenceHitEntries[0][MIN_Z];
	ReferenceHitEntries[0][NAME_STACK_0] = 0;
	ReferenceHitEntries[0][NAME_STACK_0 + 1] = 1;

	/* OBJECT 2 */
	/* 2 draw calls for name '2' */
	glPushName(2);
	glColor3f(0.0, 1.0, 0.0);
	piglit_draw_rect_z(0.5, 40, 5, 25, 30);
	piglit_draw_rect_z(0.4, 10, 75, 25, 10);
	/* fill reference buffer */
	ReferenceHitEntries[1][NAME_STACK_DEPTH] = 3;
	ReferenceHitEntries[1][MIN_Z] = (GLuint)roundf(zscale * ((1 - 0.5)*0.5));
	ReferenceHitEntries[1][MAX_Z] = (GLuint)roundf(zscale * ((1 - 0.4)*0.5));
	ReferenceHitEntries[1][NAME_STACK_0] = 0;
	ReferenceHitEntries[1][NAME_STACK_0 + 1] = 1;
	ReferenceHitEntries[1][NAME_STACK_0 + 2] = 2;

	/* OBJECT 3 */
	glPopName();
	glPushName(3);
	/* drawn offscreen */
	piglit_draw_rect_z(0.3, 250, 45, 280, 20);

	/* OBJECT 4 */
	/* glLoadName instead of glPushName */
	glLoadName(4);
	glColor3f(0.0, 0.0, 1.0);
	piglit_draw_rect_z(0.2, 50, 45, 80, 20);
	/* fill reference buffer */
	ReferenceHitEntries[2][NAME_STACK_DEPTH] = 3;
	ReferenceHitEntries[2][MIN_Z] = (GLuint)roundf(zscale * ((1 - 0.2)*0.5));
	ReferenceHitEntries[2][MAX_Z] = ReferenceHitEntries[2][MIN_Z];
	ReferenceHitEntries[2][NAME_STACK_0] = 0;
	ReferenceHitEntries[2][NAME_STACK_0 + 1] = 1;
	ReferenceHitEntries[2][NAME_STACK_0 + 2] = 4;
}

/**
 * Helper function to compare 2 hit records
 */
static bool
compare_hit_record(GLuint* hit1, GLuint* hit2)
{
	int i;
	float zscale = (float)(~0u);
	float diffz;

	if (hit1[NAME_STACK_DEPTH] != hit2[NAME_STACK_DEPTH]) {
		printf("\t%s : Incorrect name stack depth : %u %u\n",
			__FUNCTION__,
			hit1[NAME_STACK_DEPTH],
			hit2[NAME_STACK_DEPTH]);
		return false;
	}

	diffz = abs(hit1[MIN_Z] - hit2[MIN_Z])/zscale;
	if (diffz > 0.1) {
		printf("\t%s : Incorrect Minz : %u %u (%f %f) %f\n",
			__FUNCTION__,
			hit1[MIN_Z],
			hit2[MIN_Z],
			hit1[MIN_Z] / zscale,
			hit2[MIN_Z] / zscale,
			diffz);
		return false;
	}

	diffz = abs(hit1[MAX_Z] - hit2[MAX_Z])/zscale;
	if (diffz > 0.1) {
		printf("\t%s : Incorrect Maxz : %u %u (%f %f) %f\n",
			__FUNCTION__,
			hit1[MAX_Z],
			hit2[MAX_Z],
			hit1[MAX_Z] / zscale,
			hit2[MAX_Z] / zscale,
			diffz);
		return false;
	}

	for (i=0; i<hit1[NAME_STACK_DEPTH]; i++) {
		if (hit1[NAME_STACK_0 + i] != hit2[NAME_STACK_0 + i])
			return false;
	}
	return true;
}

/**
 * Helper function to check select buffer
 */
static bool
validate_select_buffer(GLuint* buffer)
{
	int i,j;
	GLint hits = glRenderMode(GL_RENDER);
	bool object_hit_found[3] = {false, false, false};
	GLuint* ptr;

	if (hits != 3) {
		printf("\t%s : unexpected hit count:%d\n", __FUNCTION__, hits);
		return false;
	}

	ptr = buffer;
	/* ordering in select buffer isn't necessarly the same as drawing order,
	 *  so we need to look for each hit entry
	 */
	for (i=0; i<hits; i++) {
		for (j=0; j<3; j++) {
			if (!object_hit_found[j]) {
				if (compare_hit_record(ptr, ReferenceHitEntries[j])) {
					object_hit_found[j] = true;
					break;
				}
			}
		}
		if (j == 3)
			return false;

		/* advance pointer */
		ptr += 3 + ptr[NAME_STACK_DEPTH];
	}

	return true;
}

void
test_case_setup(bool depth, bool stencil, bool alpha, bool scissor) {
	if (depth)
		glEnable(GL_DEPTH_TEST);
	else
		glDisable(GL_DEPTH_TEST);

	if (stencil)
		glEnable(GL_STENCIL_TEST);
	else
		glDisable(GL_STENCIL_TEST);

	if (alpha)
		glEnable(GL_ALPHA_TEST);
	else
		glDisable(GL_ALPHA_TEST);

	if (scissor)
		glEnable(GL_SCISSOR_TEST);
	else
		glDisable(GL_SCISSOR_TEST);

	/* setup all test functions to never pass */
	glDepthFunc(GL_NEVER);
	glStencilFunc(GL_NEVER, 0, 0);
	glAlphaFunc(GL_NEVER, 0);
	glScissor(0, 0, 0, 0);
}

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

enum piglit_result
do_blit_test(void)
{
	GLuint buff[64] = {0};
	glSelectBuffer(64, buff);

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);
	glClearColor(0.5, 0.5, 0.5, 0.0);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glRenderMode(GL_SELECT);
	draw_objects();

	return validate_select_buffer(buff) ? PIGLIT_PASS : PIGLIT_FAIL;
}

void piglit_init(int argc, char**argv)
{
	enum piglit_result pass;

	if (argc < 2) {
		test_case_setup(false, false, false, false);
		pass = do_blit_test();
	} else {
		test_case_setup(
			strcmp(argv[1], "depth") == 0,
			strcmp(argv[1], "stencil") == 0,
			strcmp(argv[1], "alpha") == 0,
			strcmp(argv[1], "scissor") == 0);
		pass = do_blit_test();
	}
	piglit_report_result(pass);
}
