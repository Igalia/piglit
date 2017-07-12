/*
 * Copyright © 2017 VMware, Inc.
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

/*
 * Test rendering into one window with multiple contexts.
 *
 * Authors:
 *    Brian Paul
 */

#include <assert.h>
#include <stdio.h>
#include "piglit-util-gl.h"
#include "piglit-wgl-util.h"



#define MAX_CONTEXTS 8

static HGLRC ctx[MAX_CONTEXTS];
static int num_contexts = MAX_CONTEXTS;
static HWND win;

static const float colors[MAX_CONTEXTS][4] = {
	{1, 0, 0, 1},
	{0, 1, 0, 1},
	{0, 0, 1, 1},
	{0, 1, 1, 1},
	{1, 0, 1, 1},
	{1, 1, 0, 1},
	{1, 1, 1, 1},
	{.5, .5, .5, 1},
};


static int rect_size = 40;


static int
rect_pos(int i)
{
	return i * rect_size / 2;
}


enum piglit_result
draw(void)
{
	int i;
	bool pass = true;

	/* draw a series of colored quads, one per context, at increasing
	 * Z distance.
	 */
	for (i = 0; i < num_contexts; i++) {
		if (!wglMakeCurrent(GetDC(win), ctx[i])) {
			fprintf(stderr, "wglMakeCurrent failed\n");
			return PIGLIT_FAIL;
		}

		if (i == 0) {
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		}

		glEnable(GL_DEPTH_TEST);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0, piglit_width, 0, piglit_height, 0, 1);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glPushMatrix();
		float p = rect_pos(i);
		float z = -i / 10.0;
		glTranslatef(p, p, z);

		glColor4fv(colors[i]);
		piglit_draw_rect(0, 0, rect_size, rect_size);

		glPopMatrix();
	}

	/* probe rendering */
	wglMakeCurrent(GetDC(win), ctx[0]);
	for (i = 0; i < num_contexts; i++) {
		int x = rect_pos(i) + rect_size * 3 / 4;
		int p = piglit_probe_pixel_rgb(x, x, colors[i]);

		if (!p) {
			printf("Failed probe for rect/context %d\n", i);
			pass = false;
		}
	}

	SwapBuffers(GetDC(win));

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


int
main(int argc, char **argv)
{
	int i;

	piglit_width = 500;
	piglit_height = 500;

	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-auto") == 0) {
			piglit_automatic = 1;
			break;
		}
	}

	win = piglit_get_wgl_window();
	assert(win);

	for (i = 0; i < num_contexts; i++) {
		ctx[i] = piglit_get_wgl_context(win);
		assert(ctx[i]);
	}

	if (!wglMakeCurrent(GetDC(win), ctx[0])) {
		fprintf(stderr, "wglMakeCurrent failed\n");
		return 0;
	}

	piglit_dispatch_default_init(PIGLIT_DISPATCH_GL);

	piglit_wgl_event_loop(draw);

	return 0;
}
