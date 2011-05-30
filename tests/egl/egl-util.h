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
};

EGLSurface
egl_util_create_pixmap(struct egl_state *state,
		       int width, int height, const EGLint *attribs);

int egl_util_run(const struct egl_test *test, int argc, char *argv[]);

#endif
