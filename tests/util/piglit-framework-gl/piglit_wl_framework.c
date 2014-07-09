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
#include "piglit_wl_framework.h"

static void
enter_event_loop(struct piglit_winsys_framework *winsys_fw)
{
	const struct piglit_gl_test_config *test_config = winsys_fw->wfl_fw.gl_fw.test_config;

	/* The Wayland window fails to appear on the first call to
	 * swapBuffers (which occured in display_cb above). This is
	 * likely due to swapBuffers being called before receiving an
	 * expose event. Until piglit has proper Wayland support,
	 * redraw as a workaround.
	 */
	if (test_config->display)
		test_config->display();

	/* FINISHME: Write event loop for Wayland.
	 *
	 * Until we have proper Wayland support, give the user enough time
	 * to view the window by sleeping.
	 */
	sleep(8);
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
piglit_wl_framework_create(const struct piglit_gl_test_config *test_config)
{
	struct piglit_winsys_framework *winsys_fw = NULL;
	struct piglit_gl_framework *gl_fw = NULL;
	bool ok = true;

	winsys_fw = calloc(1, sizeof(*winsys_fw));
	gl_fw = &winsys_fw->wfl_fw.gl_fw;

	ok = piglit_winsys_framework_init(winsys_fw, test_config,
	                           WAFFLE_PLATFORM_WAYLAND);
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
