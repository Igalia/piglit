/*
 * Copyright (C) 2016 VMware, Inc.
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
 * Test glBitmap, glCallList, glCallLists.
 * To exercise the texture atlas feature in Mesa.
 * Brian Paul
 * 3 Feb 2016
 */

#include "piglit-util-gl.h"


PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;
	config.window_width = 900;
	config.window_height = 300;
PIGLIT_GL_TEST_CONFIG_END


enum draw_mode
{
	PLAIN_BITMAP,
	CALL_LIST,
	CALL_LISTS,
	CALL_LISTS_IN_LIST,
};


struct bitmap_info
{
	unsigned width, height;
	float xorig, yorig;
	float xmove, ymove;
	GLubyte bits[100];
};

#define NUM_BITMAPS 25

static struct bitmap_info bitmaps[NUM_BITMAPS];
static GLuint ListBase;
static GLubyte *refImage;
static const float yellow[3] = {0.7, 1, 0.5};


static void
init_bitmaps(void)
{
	unsigned i, j;

	for (i = 0; i < NUM_BITMAPS; i++) {
		bitmaps[i].width = 4 + i * 2;
		bitmaps[i].height = 8 + 3 * (i & 1);
		bitmaps[i].xorig = i;
		bitmaps[i].yorig = 0.5 * i;
		bitmaps[i].xmove = bitmaps[i].width + i / 2;
		bitmaps[i].ymove = (i & 1) ? 1.5 * i : -1.5 * i;
		for (j = 0; j < sizeof(bitmaps[i].bits); j++)
			bitmaps[i].bits[j] = i + j;
	}

	/* create display lists */
	ListBase = glGenLists(NUM_BITMAPS);
	for (i = 0; i < NUM_BITMAPS; i++) {
		glNewList(ListBase + i, GL_COMPILE);
		glBitmap(bitmaps[i].width, bitmaps[i].height,
			 bitmaps[i].xorig, bitmaps[i].yorig,
			 bitmaps[i].xmove, bitmaps[i].ymove,
			 bitmaps[i].bits);
		glEndList();
	}
}


static void
draw_bitmaps(enum draw_mode mode, unsigned count)
{
	unsigned i;

	switch (mode) {
	case PLAIN_BITMAP:
		for (i = 0; i < NUM_BITMAPS; i++) {
			glBitmap(bitmaps[i].width, bitmaps[i].height,
				 bitmaps[i].xorig, bitmaps[i].yorig,
				 bitmaps[i].xmove, bitmaps[i].ymove,
				 bitmaps[i].bits);
		}
		break;
	case CALL_LIST:
		glListBase(0);
		for (i = 0; i < count; i++) {
			glCallList(ListBase + i);
		}
		break;
	case CALL_LISTS:
		{
			GLubyte ids[2 * NUM_BITMAPS];
			assert(count <= 2 * NUM_BITMAPS);
			for (i = 0; i < count; i++) {
				ids[i] = i;
			}
			glListBase(ListBase);
			glCallLists(count, GL_UNSIGNED_BYTE, ids);
		}
		break;
	case CALL_LISTS_IN_LIST:
		{
			GLubyte ids[2 * NUM_BITMAPS];
			GLuint l;
			assert(count <= 2 * NUM_BITMAPS);
			for (i = 0; i < count; i++) {
				ids[i] = i;
			}
			glListBase(ListBase);
			l = glGenLists(1);
			glNewList(l, GL_COMPILE);
			glCallLists(count, GL_UNSIGNED_BYTE, ids);
			glEndList();
			glCallList(l);
			glDeleteLists(l, 1);
		}
		break;
	}
}


static bool
test_mode(enum draw_mode mode, unsigned count, const char *fail_message)
{
	unsigned numBytes = piglit_width * piglit_height * 4 * sizeof(GLubyte);
	GLubyte *testImage = malloc(numBytes);
	bool pass = true;

	glClear(GL_COLOR_BUFFER_BIT);
	glColor3fv(yellow);
	glRasterPos2f(-1, 0);
	draw_bitmaps(mode, count);

	glReadPixels(0, 0, piglit_width, piglit_height,
		     GL_RGBA, GL_UNSIGNED_BYTE, testImage);

	if (memcmp(refImage, testImage, numBytes) != 0) {
		printf("%s failed\n", fail_message);
		pass = false;
	}

	free(testImage);

	return pass;
}



enum piglit_result
piglit_display(void)
{
	bool pass = true;
	unsigned numBytes = piglit_width * piglit_height * 4 * sizeof(GLubyte);

	refImage = malloc(numBytes);

	glViewport(0, 0, piglit_width, piglit_height);

	/*
	 * draw reference image with plain glBitmap calls
	 */
	glClear(GL_COLOR_BUFFER_BIT);
	glColor3fv(yellow);
	glRasterPos2f(-1, 0);
	draw_bitmaps(PLAIN_BITMAP, NUM_BITMAPS);
	glReadPixels(0, 0, piglit_width, piglit_height,
		     GL_RGBA, GL_UNSIGNED_BYTE, refImage);

	/*
	 * draw bitmaps as individual display lists
	 */
	if (!test_mode(CALL_LIST, NUM_BITMAPS, "glCallList(bitmap)")) {
		pass = false;
	}

	/*
	 * draw bitmaps with glCallLists
	 */
	if (!test_mode(CALL_LISTS, NUM_BITMAPS, "glCallLists(bitmaps)")) {
		pass = false;
	}

	/*
	 * draw bitmaps with glCallLists with extra large count to make
	 * sure nothing unexpected is drawn (and we don't crash).
	 */
	if (!test_mode(CALL_LISTS, NUM_BITMAPS + 5,
		       "glCallLists(bitmaps, count + 5)")) {
		pass = false;
	}

	/* draw with glCallLists inside another display list */
	if (!test_mode(CALL_LISTS_IN_LIST, NUM_BITMAPS,
		       "glCallLists(bitmaps) inside display list")) {
		pass = false;
	}

	piglit_present_results();

	free(refImage);

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


void
piglit_init(int argc, char **argv)
{
	init_bitmaps();
}
