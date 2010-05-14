#ifndef EGL_UTIL_H
#define EGL_UTIL_H

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <GL/gl.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

struct egl_test {
	const char **extensions;
	enum piglit_result (*draw)(EGLDisplay egl_dpy, EGLSurface surf);
};

int egl_run(const struct egl_test *test, int argc, char *argv[]);

#endif
