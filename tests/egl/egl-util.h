
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
#include <EGL/egl.h>
#include <EGL/eglext.h>

#ifndef EGL_KHR_gl_colorspace
#define EGL_KHR_gl_colorspace 1
#define EGL_GL_COLORSPACE_KHR             0x309D
#define EGL_GL_COLORSPACE_SRGB_KHR        0x3089
#define EGL_GL_COLORSPACE_LINEAR_KHR      0x308A
#endif /* EGL_KHR_gl_colorspace */

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
	const EGLint *surface_attribs;
	const char **extensions;
	enum piglit_result (*draw)(struct egl_state *state);
	EGLint window_width;
	EGLint window_height;
	bool stop_on_failure;
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

EGLNativePixmapType
egl_util_create_native_pixmap(struct egl_state *state, int width, int height);

EGLSurface
egl_util_create_pixmap(struct egl_state *state,
		       int width, int height, const EGLint *attribs);

enum piglit_result egl_util_run(const struct egl_test *test, int argc, char *argv[]);

int
egl_probe_front_pixel_rgb(struct egl_state *state,
			  int x, int y, const float *expected);

#endif
