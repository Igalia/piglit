/*
 * Copyright © 2011 Intel Corporation
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

/**
 * @file glx-query-drawable.c
 *
 * Test the behavior of glXQueryDrawable(). See GLX 1.4 spec, Section 3.3.6
 * Querying Attributes.
 *
 * For usage information, see usage_error();
 */

#include "piglit-util-gl-common.h"
#include "piglit-glx-util.h"

int piglit_width = 137;
int piglit_height = 119;

void
usage_error()
{
   const char *message =
	"usage:\n"
	"    glx-query-drawable [-auto] --bad-drawable\n"
	"        Call glXQueryDrawable(drawable=0) and expect that error\n"
	"        GLXBadDrawable is generated.\n"
	"\n"
	"    glx-query-drawable [-auto] --attr=GLX_WIDTH\n"
	"    glx-query-drawable [-auto] --attr=GLX_HEIGHT\n"
	"        Call glXQueryDrawable() with the given attribute.\n";
	printf("%s", message);
	piglit_report_result(PIGLIT_FAIL);
}

/***************************************************************************/

/**
 * @name X Error Handlers
 * @{
 */

bool found_error_glxbaddrawable = false;

static int
expect_no_error(Display *display, XErrorEvent *error)
{
	static char buf[256];
	XGetErrorText(display, error->error_code, buf, 256);
	fprintf(stderr, "error: unexpected X error: %s\n", buf);
	piglit_report_result(PIGLIT_FAIL);
	return 0;
}

static int
expect_glxbaddrawable(Display *display, XErrorEvent *error)
{
	if (piglit_glx_get_error(display, error) == GLXBadDrawable) {
		found_error_glxbaddrawable = true;
	} else {
		static char buf[256];
		XGetErrorText(display, error->error_code, buf, 256);
		fprintf(stderr, "error: unexpected X error: %s\n", buf);
		piglit_report_result(PIGLIT_FAIL);
	}
	return 0;
}

/***************************************************************************/

/**
 * @}
 * @name Test Functions
 * @{
 */

static void query_width(Display *display, GLXDrawable draw) {
	unsigned int width = 0;

	XSetErrorHandler(expect_no_error);
	glXQueryDrawable(display, draw, GLX_WIDTH, &width);

	/* Sync before checking width in order to catch X errors. */
	XSync(display, 0);

	if (width != piglit_width) {
		fprintf(stderr,
		        "error: width=%d but glXQueryDrawable returned %d\n",
		        piglit_width, width);
		piglit_report_result(PIGLIT_FAIL);
	}

	piglit_report_result(PIGLIT_PASS);
}

static void query_height(Display *display, GLXDrawable draw) {
	unsigned int height = 0;

	XSetErrorHandler(expect_no_error);
	glXQueryDrawable(display, draw, GLX_HEIGHT, &height);

	/* Sync before checking height in order to catch X errors. */
	XSync(display, 0);

	if (height != piglit_height) {
		fprintf(stderr,
		        "error: height=%d but glXQueryDrawable returned %d\n",
		        piglit_height, height);
		piglit_report_result(PIGLIT_FAIL);
	}

	piglit_report_result(PIGLIT_PASS);
}


static void query_bad_drawable(Display *display, GLXDrawable draw) {
	unsigned int width;
	(void) draw;

	XSetErrorHandler(expect_glxbaddrawable);
	glXQueryDrawable(display, 0, GLX_WIDTH, &width);
	XSync(display, 0);

	if (!found_error_glxbaddrawable) {
		fprintf(stderr,
		        "error: glXQueryDrawable(draw=0) did not generate "
		        "GLXBadDrawable\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	piglit_report_result(PIGLIT_PASS);
}

/** @} */

/***************************************************************************/

static void
parse_args(int argc, char **argv,
           void (**test_func)(Display*, GLXDrawable))
{
	int i;

	/* Count of parsed args, excluding -auto. */
	int num_parsed_args = 0;

	for (i = 1; i < argc; ++i) {
		const char *arg = argv[i];
		if (!strncmp(arg, "-auto", 5)) {
			piglit_automatic = 1;
		} else if (!strncmp(arg, "--bad-drawable", 14)) {
			++num_parsed_args;
			*test_func = query_bad_drawable;
		} else if (!strncmp(arg, "--attr=GLX_WIDTH", 16)) {
			++num_parsed_args;
			*test_func = query_width;
		} else if (!strncmp(arg, "--attr=GLX_HEIGHT", 17)) {
			++num_parsed_args;
			*test_func = query_height;
		} else {
		   /* Unrecognized argument. */
		   usage_error();
		}
	}

	if (num_parsed_args != 1) {
	   usage_error();
	}
}

int main(int argc, char **argv) {
	Display *display;
	XVisualInfo *visual;
	Window window;
	GLXContext ctx;
	void (*test_func)(Display*, GLXDrawable);

	parse_args(argc, argv, &test_func);

	display = piglit_get_glx_display();
	visual = piglit_get_glx_visual(display);
	window = piglit_get_glx_window(display, visual);
	ctx = piglit_get_glx_context(display, visual);
	glXMakeCurrent(display, window, ctx);

	/* Must initialize static variables of this function. */
	piglit_glx_get_error(display, NULL);

	test_func(display, window);

	return 0;
}
