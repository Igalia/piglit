/*
 * Copyright © 2009 Intel Corporation
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
 *    Eric Anholt <eric@anholt.net>
 *
 */

#include "piglit-util-gl.h"
#include "piglit-glx-util.h"

int piglit_automatic;

#ifndef _WIN32
__attribute__((weak)) int piglit_width = 100;
__attribute__((weak)) int piglit_height = 100;
#endif

Display *
piglit_get_glx_display()
{
	Display *dpy;

	dpy = XOpenDisplay(NULL);
	if (!dpy) {
		fprintf(stderr, "couldn't open display\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	return dpy;
}

XVisualInfo *
piglit_get_glx_visual(Display *dpy)
{
	XVisualInfo *visinfo;
	int attrib[] = {
		GLX_RGBA,
		GLX_RED_SIZE, 1,
		GLX_GREEN_SIZE, 1,
		GLX_BLUE_SIZE, 1,
		GLX_DOUBLEBUFFER,
		None
	};
	int screen = DefaultScreen(dpy);

	visinfo = glXChooseVisual(dpy, screen, attrib);
	if (visinfo == NULL) {
		fprintf(stderr,
			"Couldn't get an RGBA, double-buffered visual\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	return visinfo;
}

GLXContext
piglit_get_glx_context(Display *dpy, XVisualInfo *visinfo)
{
	return piglit_get_glx_context_share(dpy, visinfo, NULL);
}

GLXContext
piglit_get_glx_context_share(Display *dpy, XVisualInfo *visinfo, GLXContext share)
{
	GLXContext ctx;

	ctx = glXCreateContext(dpy, visinfo, share, True);
	if (ctx == None) {
		fprintf(stderr, "glXCreateContext failed\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	return ctx;
}

static Window
_piglit_get_glx_window(Display *dpy, XVisualInfo *visinfo, bool map, bool fullscreen)
{
	XSetWindowAttributes window_attr;
	unsigned long mask;
	int screen = DefaultScreen(dpy);
	Window root_win = RootWindow(dpy, screen);
	Window win;

	window_attr.background_pixel = 0;
	window_attr.border_pixel = 0;
	window_attr.colormap = XCreateColormap(dpy, root_win,
					       visinfo->visual, AllocNone);
	window_attr.event_mask = StructureNotifyMask | ExposureMask |
		KeyPressMask;
	mask = CWBackPixel | CWBorderPixel | CWColormap | CWEventMask;

	if (fullscreen) {
		window_attr.override_redirect = True;
		mask |= CWOverrideRedirect;
		piglit_width = DisplayWidth(dpy, screen);
		piglit_height = DisplayHeight(dpy, screen);
	}

	win = XCreateWindow(dpy, root_win, 0, 0,
			    piglit_width, piglit_height,
			    0, visinfo->depth, InputOutput,
			    visinfo->visual, mask, &window_attr);

	if (piglit_automatic)
		piglit_glx_window_set_no_input(dpy, win);

	if (map)
		XMapWindow(dpy, win);

	return win;
}

Window
piglit_get_glx_window_unmapped(Display *dpy, XVisualInfo *visinfo)
{
	return _piglit_get_glx_window(dpy, visinfo, false, false);
}

Window
piglit_get_glx_window_fullscreen(Display *dpy, XVisualInfo *visinfo)
{
	return _piglit_get_glx_window(dpy, visinfo, true, true);
}

Window
piglit_get_glx_window(Display *dpy, XVisualInfo *visinfo)
{
	return _piglit_get_glx_window(dpy, visinfo, true, false);
}

bool
piglit_is_glx_extension_supported(Display *dpy, const char *name)
{
	int screen = DefaultScreen(dpy);
	const char *const glx_extension_list =
		glXQueryExtensionsString(dpy, screen);

	return piglit_is_extension_in_string(glx_extension_list, name);
}

void
piglit_require_glx_extension(Display *dpy, const char *name)
{
	if (!piglit_is_glx_extension_supported(dpy, name)) {
		fprintf(stderr, "Test requires %s\n", name);
		piglit_report_result(PIGLIT_SKIP);
	}
}


void
piglit_require_glx_version(Display *dpy, int major, int minor)
{
	int glxMajor;
	int glxMinor;

	if (! glXQueryVersion(dpy, & glxMajor, & glxMinor)) {
		fprintf(stderr, "Could not query GLX version!\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	if (glxMajor != major || glxMinor < minor) {
		fprintf(stderr, "Test requires GLX %d.%d.  Got %d.%d.\n",
			major, minor, glxMajor, glxMinor);
		piglit_report_result(PIGLIT_SKIP);
	}
}


void
piglit_glx_event_loop(Display *dpy, enum piglit_result (*draw)(Display *dpy))
{
	for (;;) {
		XEvent event;
		XNextEvent (dpy, &event);

		if (event.type == KeyPress) {
			int keysyms_per_keycode_return;
			KeySym *sym = XGetKeyboardMapping (dpy,
							   event.xkey.keycode,
							   1,
							   &keysyms_per_keycode_return);

			if (sym[0] == XK_Escape || sym[0] == XK_q || sym[0] == XK_Q)
				break;
			else
				draw(dpy);
		} else if (event.type == Expose) {
			enum piglit_result result = draw(dpy);

			if (piglit_automatic) {
				XCloseDisplay(dpy);
				piglit_report_result(result);
				break;
			}
		}
        }
}


static enum piglit_result
piglit_iterate_visuals_event_loop(Display *dpy,
				  enum piglit_result (*draw)(Display *dpy,
							     GLXFBConfig config),
				  GLXFBConfig config)
{
	for (;;) {
		XEvent event;
		XNextEvent (dpy, &event);

		if (event.type == Expose) {
			return draw(dpy, config);
		}
        }
}

void
piglit_glx_window_set_no_input(Display *dpy, GLXDrawable win)
{
	XWMHints *hints;
	hints = XAllocWMHints();
	hints->flags |= InputHint;
	hints->input = False;

	XSetWMHints(dpy, win, hints);

	XFree(hints);
}

void
piglit_glx_set_no_input(void)
{
	Display *d;
	GLXDrawable win;

	d = glXGetCurrentDisplay();
	win = glXGetCurrentDrawable();

	piglit_glx_window_set_no_input(d, win);
}

enum piglit_result
piglit_glx_iterate_pixmap_fbconfigs(enum piglit_result (*draw)(Display *dpy,
						      GLXFBConfig config))
{
	int screen;
	GLXFBConfig *configs;
	int n_configs;
	int i;
	bool any_fail = false;
	bool any_pass = false;
	Window root_win;

	Display *dpy = XOpenDisplay(NULL);
	if (!dpy) {
		fprintf(stderr, "couldn't open display\n");
		piglit_report_result(PIGLIT_FAIL);
	}
	screen = DefaultScreen(dpy);
	root_win = RootWindow(dpy, screen);

	configs = glXGetFBConfigs(dpy, screen, &n_configs);
	if (!configs) {
		fprintf(stderr, "No GLX FB configs\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	for (i = 0; i < n_configs; i++) {
		GLXFBConfig config = configs[i];
		enum piglit_result result;
		GLXContext ctx;
		Pixmap pix;
		GLXPixmap glx_pix;
		int draw_types;
		int depth;

		glXGetFBConfigAttrib(dpy, config, GLX_DRAWABLE_TYPE,
				     &draw_types);

		if (!(draw_types & GLX_PIXMAP_BIT))
			continue;

		glXGetFBConfigAttrib(dpy, config, GLX_BUFFER_SIZE,
				     &depth);
		ctx = glXCreateNewContext(dpy, config, GLX_RGBA_TYPE,
					  NULL, true);
		pix = XCreatePixmap(dpy, root_win,
				    piglit_width, piglit_height, depth);
		glx_pix = glXCreatePixmap(dpy, config, pix, NULL);
		glXMakeCurrent(dpy, glx_pix, ctx);

		result = draw(dpy, config);

		if (result == PIGLIT_FAIL)
			any_fail = true;
		else if (result == PIGLIT_PASS)
			any_pass = true;

		XFreePixmap(dpy, pix);
		glXDestroyContext(dpy, ctx);
	}

	if (any_fail)
		return PIGLIT_FAIL;
	else if (any_pass)
		return PIGLIT_PASS;
	else
		return PIGLIT_SKIP;
}

enum piglit_result
piglit_glx_iterate_visuals(enum piglit_result (*draw)(Display *dpy,
						      GLXFBConfig config))
{
	int screen;
	GLXFBConfig *configs;
	int n_configs;
	int i;
	bool any_fail = false;
	bool any_pass = false;

	Display *dpy = XOpenDisplay(NULL);
	if (!dpy) {
		fprintf(stderr, "couldn't open display\n");
		piglit_report_result(PIGLIT_FAIL);
	}
	screen = DefaultScreen(dpy);

	configs = glXGetFBConfigs(dpy, screen, &n_configs);
	if (!configs) {
		fprintf(stderr, "No GLX FB configs\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	for (i = 0; i < n_configs; i++) {
		enum piglit_result result;
		XVisualInfo *visinfo;
		GLXContext ctx;
		GLXDrawable d;

		visinfo = glXGetVisualFromFBConfig(dpy, configs[i]);
		if (!visinfo)
			continue;

		ctx = piglit_get_glx_context(dpy, visinfo);
		d = piglit_get_glx_window(dpy, visinfo);
		glXMakeCurrent(dpy, d, ctx);
		XFree(visinfo);

		result = piglit_iterate_visuals_event_loop(dpy, draw,
							   configs[i]);
		if (result == PIGLIT_FAIL)
			any_fail = true;
		else if (result == PIGLIT_PASS)
			any_pass = true;

		XDestroyWindow(dpy, d);
		glXDestroyContext(dpy, ctx);
	}

	if (any_fail)
		return PIGLIT_FAIL;
	else if (any_pass)
		return PIGLIT_PASS;
	else
		return PIGLIT_SKIP;
}

GLXFBConfig
piglit_glx_get_fbconfig_for_visinfo(Display *dpy, XVisualInfo *visinfo)
{
	int i, nconfigs;
	GLXFBConfig ret = None, *configs;

	configs = glXGetFBConfigs(dpy, visinfo->screen, &nconfigs);
	if (!configs)
		return None;

	for (i = 0; i < nconfigs; i++) {
		int v;

		if (glXGetFBConfigAttrib(dpy, configs[i], GLX_VISUAL_ID, &v))
			continue;

		if (v == visinfo->visualid) {
			ret = configs[i];
			break;
		}
	}

	XFree(configs);
	return ret;
}

/*
 * If you use this in an X error handler - and you will - pre-call it as:
 *  piglit_glx_get_error(dpy, NULL);
 * outside the error handler to cache errbase.  Otherwise this will
 * generate protocol, and you'll deadlock.
 *
 * Returns -1 if the error is not a GLX error, otherwise returns the
 * GLX error code.
 */
int
piglit_glx_get_error(Display *dpy, XErrorEvent *err)
{
	static int errbase, evbase;
	
	if (!errbase)
		glXQueryExtension(dpy, &errbase, &evbase);

	if (!err)
		return -1;

	if (err->error_code < errbase ||
	    err->error_code > errbase + GLXBadProfileARB)
		return -1;

	return err->error_code - errbase;
}

/**
 * Convert a GLX error code to a printable string
 *
 * \sa piglit_glx_get_error
 */
const char *
piglit_glx_error_string(int err)
{
	static const char *const error_table[] = {
		"GLXBadContext",
		"GLXBadContextState",
		"GLXBadDrawable",
		"GLXBadPixmap",
		"GLXBadContextTag",
		"GLXBadCurrentWindow",
		"GLXBadRenderRequest",
		"GLXBadLargeRequest",
		"GLXUnsupportedPrivateRequest",
		"GLXBadFBConfig",
		"GLXBadPbuffer",
		"GLXBadCurrentDrawable",
		"GLXBadWindow",
		"GLXBadProfileARB"
	};

	if (err < 0) {
		return "non-GLX error";
	} else if (err >= ARRAY_SIZE(error_table)) {
		return "unknown GLX error";
	} else {
		return error_table[err];
	}
}

/**
 * Get the procedure adddresses for a group of function names
 *
 * \note
 * If any call to \c glXGetProcAddress fails, this function will call
 * \c piglit_report_result with \c PIGLIT_FAIL.
 */
void
piglit_glx_get_all_proc_addresses(const struct piglit_glx_proc_reference *procedures,
				  unsigned num)
{
	unsigned i;

	for (i = 0; i < num; i++) {
		*procedures[i].procedure =
			glXGetProcAddress((const GLubyte *) procedures[i].name);
		if (*procedures[i].procedure == NULL) {
			fprintf(stderr,
				"Failed to get function pointer for %s.\n",
				procedures[i].name);
			piglit_report_result(PIGLIT_FAIL);
		}
	}
}

/* Creates a GLX context for rendering into an FBO */
void
piglit_framework_fbo_init_glx()
{

}
