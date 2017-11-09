/*
 * Copyright 2017 Red Hat, Inc.
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
#include "piglit-util-gl.h"
#include "piglit-glx-util.h"
#include "common.h"

int main(int argc, char **argv)
{
	int ctxattribs[] = {
		GLX_SCREEN, 0,
		None
	};
	int cfgattribs[] = {
		GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
		None
	};
	GLXContext ctx;
	int i, n_configs, screen;
	GLXFBConfig *configs;

	GLX_ARB_create_context_setup();
	piglit_require_glx_extension(dpy, "GLX_EXT_no_config_context");

	ctxattribs[1] = screen = DefaultScreen(dpy);
	ctx = glXCreateContextAttribsARB(dpy, NULL, NULL, True, ctxattribs);

	if (!ctx) {
		fprintf(stderr, "Failed to create a no-config context\n");
		piglit_report_result(PIGLIT_FAIL);
		return 0;
	}

	configs = glXChooseFBConfig(dpy, screen, cfgattribs, &n_configs);
	if (!configs) {
		fprintf(stderr, "No window fbconfigs\n");
		piglit_report_result(PIGLIT_FAIL);
		return 0;
	}

	for (i = 0; i < n_configs; i++) {
		XVisualInfo *visinfo;
		Window window;

		visinfo = glXGetVisualFromFBConfig(dpy, configs[i]);
		window = piglit_get_glx_window(dpy, visinfo);

		if (!glXMakeCurrent(dpy, window, ctx)) {
			fprintf(stderr, "Failed to make current\n");
			piglit_report_result(PIGLIT_FAIL);
			return 0;
		}
	}

	GLX_ARB_create_context_teardown();

	piglit_report_result(PIGLIT_PASS);
	return 0;
}
