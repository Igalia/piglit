/*
 * Copyright © 2012 Intel Corporation
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

#include <assert.h>
#include <stdlib.h>
#include <unistd.h>

#include "piglit-util-gl.h"
#include "piglit_gbm_framework.h"

static void
piglit_gbm_console_display(void);

static void
enter_event_loop(struct piglit_winsys_framework *winsys_fw)
{
	const struct piglit_gl_test_config *test_config = winsys_fw->wfl_fw.gl_fw.test_config;

	enum piglit_result result = PIGLIT_PASS;

	if (test_config->display)
		result = test_config->display();

	if (piglit_automatic)
		piglit_report_result(result);

	piglit_gbm_console_display();

	/* gbm has no input, so we exit immediately, as if the user
	 * had pressed escape.
	 */
	exit(0);
}

static void
show_window(struct piglit_winsys_framework *winsys_fw)
{
	waffle_window_show(winsys_fw->wfl_fw.window);
}

static void
destroy(struct piglit_gl_framework *gl_fw)
{
	struct piglit_winsys_framework *winsys_fw= piglit_winsys_framework(gl_fw);

	if (winsys_fw == NULL)
		return;

	piglit_winsys_framework_teardown(winsys_fw);
	free(winsys_fw);
}

struct piglit_gl_framework*
piglit_gbm_framework_create(const struct piglit_gl_test_config *test_config)
{
	struct piglit_winsys_framework *winsys_fw = NULL;
	struct piglit_gl_framework *gl_fw = NULL;
	bool ok = true;

	winsys_fw = calloc(1, sizeof(*winsys_fw));
	gl_fw = &winsys_fw->wfl_fw.gl_fw;

	ok = piglit_winsys_framework_init(winsys_fw, test_config,
	                           WAFFLE_PLATFORM_GBM);
	if (!ok)
		goto fail;

	winsys_fw->show_window = show_window;
	winsys_fw->enter_event_loop = enter_event_loop;
	gl_fw->destroy = destroy;

	return gl_fw;

fail:
	destroy(gl_fw);
	return NULL;
}

#ifdef PIGLIT_HAS_LIBCACA
#include <caca.h>

static void
determine_canvas_size(int *width, int *height)
{
	int columns = 80, rows = 24;
	float test_aspect, console_aspect;
	const float font_aspect = 0.5;
	caca_display_t *display;

	display = caca_create_display(NULL);
	if (display) {
		caca_canvas_t *canvas = caca_get_canvas(display);
		columns = caca_get_canvas_width(canvas);
		rows = caca_get_canvas_height(canvas);
		caca_free_display(display);
	}

	/* Don't fill the entire window. */
	columns--;
	rows--;

	test_aspect = (float)piglit_width / (float)piglit_height;
	console_aspect = (float)(columns) / (float)(2 * (rows));
	if (console_aspect < test_aspect) {
		rows = (int) (font_aspect * piglit_height *
			      ((float)columns / piglit_width));
	} else {
		columns = (int) ((float)piglit_width *
				 ((float)rows / piglit_height / font_aspect));
	}

	if (columns >= (piglit_width / font_aspect) && rows >= piglit_height) {
		/* Yep, the console resolution is too high */
		*width = MAX2(1, piglit_width / font_aspect);
		*height = MAX2(1, piglit_height);
	} else {
		*width = MAX2(1, columns);
		*height = MAX2(1, rows);
	}
}

static void
piglit_gbm_console_display(void)
{
	caca_canvas_t *canvas;
	caca_dither_t *dither;
	void *export;
	uint32_t *pixels;
	size_t export_size;
	int width, height;

	determine_canvas_size(&width, &height);

	canvas = caca_create_canvas(width, height);
	if (!canvas) {
		printf("Failed to get canvas for gbm console display!\n");
		return;
	}

	caca_set_color_ansi(canvas, CACA_DEFAULT, CACA_TRANSPARENT);

	dither = caca_create_dither(32, piglit_width, piglit_height,
				    4 * piglit_width,
				    0x000000ff, 0x0000ff00,
				    0x00ff0000, 0xff000000);
	if (!dither) {
		caca_free_canvas(canvas);
		printf("Failed to get dither object for gbm console display!\n");
		return;
	}

	pixels = malloc(4 * piglit_width * piglit_height);

	while (!piglit_check_gl_error(GL_NO_ERROR)) {
		/* Clear any OpenGL errors */
	}
	glBindFramebuffer(GL_READ_FRAMEBUFFER, piglit_winsys_fbo);
	glReadPixels(0, 0, piglit_width, piglit_height,
		     GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*) pixels);
	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		caca_free_dither(dither);
		caca_free_canvas(canvas);
		printf("Error reading pixels for gbm console display!\n");
		return;
	}

	caca_dither_bitmap(canvas, 0, 0, width, height, dither, pixels);
	caca_flop(canvas);
	caca_free_dither(dither);
	free(pixels);

	export = caca_export_canvas_to_memory(canvas, "ansi", &export_size);
	caca_free_canvas(canvas);
	if (!export) {
		printf("Failed to export image for gbm console display!\n");
	} else {
		fwrite(export, export_size, 1, stdout);
		free(export);
	}
}

#else // PIGLIT_HAS_LIBCACA

static void
piglit_gbm_console_display(void)
{
}

#endif
