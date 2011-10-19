
/**
 * \file egl-util.h
 * Common framework for EGL tests.
 *
 * \author Kristian Høgsberg <krh@bitplanet.net>
 */

#ifndef EGL_UTIL_H
#define EGL_UTIL_H

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <GL/gl.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

struct egl_state {
	Display *dpy;
	Window win;
	EGLDisplay egl_dpy;
	EGLConfig cfg;
	EGLContext ctx;
	EGLSurface surf;
	EGLint major, minor;
	int depth;
	int width;
	int height;
};

struct egl_test {
	const EGLint *config_attribs;
	const char **extensions;
	enum piglit_result (*draw)(struct egl_state *state);
	EGLint window_width;
	EGLint window_height;
};

static const EGLint egl_default_attribs[] = {
	EGL_SURFACE_TYPE, EGL_WINDOW_BIT | EGL_PIXMAP_BIT | EGL_PBUFFER_BIT,
	EGL_RED_SIZE, 1,
	EGL_GREEN_SIZE, 1,
	EGL_BLUE_SIZE, 1,
	EGL_DEPTH_SIZE, 1,
	EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
	EGL_NONE
};

static const EGLint egl_default_window_width = 300;
static const EGLint egl_default_window_height = 300;

/**
 * \brief Initialize test to default values.
 */
void
egl_init_test(struct egl_test *test);

EGLSurface
egl_util_create_pixmap(struct egl_state *state,
		       int width, int height, const EGLint *attribs);

int egl_util_run(const struct egl_test *test, int argc, char *argv[]);

#endif
