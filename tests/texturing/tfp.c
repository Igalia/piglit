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

/** @file tfp.c
 * Tests the GLX_EXT_texture_from_pixmap extension, in particular the bug
 * reported in https://bugs.freedesktop.org/show_bug.cgi?id=19910 in which
 * the RGB/RGBA attribute of the drawable was misplaced, resulting in always
 * acting as if the pixmap had the alpha channel present.
 *
 * This test is based loosely on the testcase attached to that bug, by
 * Neil Roberts <neil@linux.intel.com>.
 */

#include "GL/glx.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "piglit-util.h"

GLfloat tex_data[4][4] = {
	{ 1.0, 0.0, 0.0, 1.0 },
	{ 1.0, 0.0, 0.0, 0.5 },
	{ 0.0, 1.0, 0.0, 1.0 },
	{ 0.0, 1.0, 0.0, 0.5 },
};

#define WIN_WIDTH	256
#define WIN_HEIGHT	128

static GLboolean Automatic = GL_FALSE;
static GLXPixmap rgb_pixmap, rgba_pixmap;
static Display *dpy;
static Window win;
static int win_width = WIN_WIDTH;
static int win_height = WIN_HEIGHT;

static PFNGLXBINDTEXIMAGEEXTPROC pglXBindTexImageEXT;
static PFNGLXRELEASETEXIMAGEEXTPROC pglXReleaseTexImageEXT;

static GLboolean
check_pixel(GLboolean has_alpha, GLfloat *tex_color, int x, int y)
{
	GLfloat color[3];

	if (has_alpha) {
		color[0] = tex_color[0] * tex_color[3];
		color[1] = tex_color[1] * tex_color[3];
		color[2] = tex_color[2] * tex_color[3];
	} else {
		color[0] = tex_color[0];
		color[1] = tex_color[1];
		color[2] = tex_color[2];
	}

	return piglit_probe_pixel_rgb(x, y, color);
}

static GLboolean
check_results(GLboolean has_alpha, int x, int y, int w, int h)
{
	GLboolean pass = GL_TRUE;

	pass &= check_pixel(has_alpha, tex_data[0],
			    x + w * 1 / 4, y + h * 1 / 4);
	pass &= check_pixel(has_alpha, tex_data[1],
			    x + w * 3 / 4, y + h * 1 / 4);
	pass &= check_pixel(has_alpha, tex_data[2],
			    x + w * 1 / 4, y + h * 3 / 4);
	pass &= check_pixel(has_alpha, tex_data[3],
			    x + w * 3 / 4, y + h * 3 / 4);

	return pass;
}

static void
draw_pixmap(GLXPixmap pixmap, int x, int y, int w, int h)
{
	GLuint texname;
	GLfloat tex_coords[] = {
		0.0f, 0.0f,
		1.0f, 0.0f,
		1.0f, 1.0f,
		0.0f, 1.0f,
	};
	GLfloat vertex_coords[4][2];

	vertex_coords[0][0] = x;
	vertex_coords[0][1] = y;
	vertex_coords[1][0] = x + w;
	vertex_coords[1][1] = y;
	vertex_coords[2][0] = x + w;
	vertex_coords[2][1] = y + h;
	vertex_coords[3][0] = x;
	vertex_coords[3][1] = y + h;

	/* Create the texture. */
	glGenTextures(1, &texname);
	glBindTexture(GL_TEXTURE_2D, texname);
	glEnable(GL_TEXTURE_2D);

	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	/* Set the texture combiner to give {r*a, g*a, b*a, a} so we can see
	 * the effect of the alpha channel in terms of color.
	 */
	glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
	glTexEnvi (GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
	glTexEnvi (GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);

	glTexEnvi (GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
	glTexEnvi (GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_TEXTURE);
	glTexEnvi (GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_TEXTURE);

	glTexEnvi (GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_ALPHA);
	glTexEnvi (GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_TEXTURE);
	glTexEnvi (GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, GL_TEXTURE); /* ignored */

	pglXBindTexImageEXT(dpy, pixmap, GLX_FRONT_LEFT_EXT, NULL);

	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, tex_coords);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2, GL_FLOAT, 0, vertex_coords);

	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	pglXReleaseTexImageEXT(dpy, pixmap, GLX_FRONT_LEFT_EXT);
	glDeleteTextures(1, &texname);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisable(GL_TEXTURE_2D);
}

static GLboolean
draw(void)
{
	GLboolean pass = GL_TRUE;
	int draw_w = win_width / 4;
	int draw_h = win_height / 2;
	int rgb_x = win_width / 8;
	int rgb_y = win_height / 4;
	int rgba_x = win_width * 5 / 8;
	int rgba_y = win_height / 4;

	/* Clear background to gray */
	glClearColor(0.5, 0.5, 0.5, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	draw_pixmap(rgb_pixmap, rgb_x, rgb_y, draw_w, draw_h);
	draw_pixmap(rgba_pixmap, rgba_x, rgba_y, draw_w, draw_h);

	glXSwapBuffers(dpy, win);

	pass &= check_results(GL_FALSE, rgb_x, rgb_y, draw_w, draw_h);
	pass &= check_results(GL_TRUE, rgba_x, rgba_y, draw_w, draw_h);

	return pass;
}

static void
set_pixel(Display *dpy, Pixmap pixmap, int x, int y, GLfloat *color,
	  unsigned long *masks)
{
	unsigned long pixel = 0;
	XGCValues gc_values;
	GC gc;

	pixel |= (unsigned long)(color[0] * masks[0]) & masks[0];
	pixel |= (unsigned long)(color[1] * masks[1]) & masks[1];
	pixel |= (unsigned long)(color[2] * masks[2]) & masks[2];
	pixel |= (unsigned long)(color[3] * masks[3]) & masks[3];

	gc_values.foreground = pixel;
	gc_values.background = pixel;
	gc = XCreateGC(dpy, pixmap, GCForeground | GCBackground, &gc_values);

	XFillRectangle(dpy, pixmap, gc, x, y, 1, 1);

	XFreeGC(dpy, gc);
}

/**
 * Creates a Pixmap and GLXPixmap with tex_data as the contents.
 */
static GLXPixmap
create_pixmap(GLenum format)
{
	static const int rgb_fb_config_attribs[] = {
		GLX_RENDER_TYPE, GLX_RGBA_BIT,
		GLX_RED_SIZE, 8,
		GLX_GREEN_SIZE, 8,
		GLX_BLUE_SIZE, 8,
		GLX_ALPHA_SIZE, 0,
		GLX_DRAWABLE_TYPE, GLX_PIXMAP_BIT,
		GLX_BIND_TO_TEXTURE_RGB_EXT, 1,
		None
	};
	static const int rgba_fb_config_attribs[] = {
		GLX_RENDER_TYPE, GLX_RGBA_BIT,
		GLX_RED_SIZE, 8,
		GLX_GREEN_SIZE, 8,
		GLX_BLUE_SIZE, 8,
		GLX_ALPHA_SIZE, 8,
		GLX_DRAWABLE_TYPE, GLX_PIXMAP_BIT,
		GLX_BIND_TO_TEXTURE_RGBA_EXT, 1,
		None
	};
	static const int rgb_pixmap_attribs[] =	{
		GLX_TEXTURE_TARGET_EXT, GLX_TEXTURE_2D_EXT,
		GLX_TEXTURE_FORMAT_EXT, GLX_TEXTURE_FORMAT_RGB_EXT,
		None
	};
	static const int rgba_pixmap_attribs[] =	{
		GLX_TEXTURE_TARGET_EXT, GLX_TEXTURE_2D_EXT,
		GLX_TEXTURE_FORMAT_EXT, GLX_TEXTURE_FORMAT_RGBA_EXT,
		None
	};
	static const int *fb_config_attribs, *pixmap_attribs;
	GLXFBConfig *fb_configs;
	GLXFBConfig fb_config;
	int n_fb_configs;
	Pixmap pixmap;
	GLXPixmap glx_pixmap;
	XVisualInfo *vis;
	unsigned long channel_masks[4];

	if (format == GL_RGBA) {
		fb_config_attribs = rgba_fb_config_attribs;
		pixmap_attribs = rgba_pixmap_attribs;
	} else {
		fb_config_attribs = rgb_fb_config_attribs;
		pixmap_attribs = rgb_pixmap_attribs;
	}

	fb_configs = glXChooseFBConfig(dpy, DefaultScreen(dpy),
				       fb_config_attribs,
				       &n_fb_configs);


	if (fb_configs == NULL || n_fb_configs < 1) {
		fprintf(stderr, "No %s TFP FB config found\n",
			format == GL_RGBA ? "RGBA" : "RGB");
		return None;
	}
	fb_config = fb_configs[n_fb_configs - 1];

	pixmap = XCreatePixmap(dpy, RootWindow(dpy, DefaultScreen(dpy)),
			       2, 2,
			       format == GL_RGBA ? 32 : 24);

	glx_pixmap = glXCreatePixmap(dpy, fb_config, pixmap, pixmap_attribs);

	vis = glXGetVisualFromFBConfig(dpy, fb_config);

	channel_masks[0] = vis->red_mask;
	channel_masks[1] = vis->green_mask;
	channel_masks[2] = vis->blue_mask;
	if (format == GL_RGBA) {
		channel_masks[3] = ~(vis->red_mask | vis->green_mask |
				     vis->blue_mask);
	} else {
		channel_masks[3] = 0;
	}

	set_pixel(dpy, pixmap, 0, 0, tex_data[0], channel_masks);
	set_pixel(dpy, pixmap, 1, 0, tex_data[1], channel_masks);
	set_pixel(dpy, pixmap, 0, 1, tex_data[2], channel_masks);
	set_pixel(dpy, pixmap, 1, 1, tex_data[3], channel_masks);

	XFree(fb_configs);
	XFree(vis);

	return glx_pixmap;
}

static void init(void)
{
	/* Set up projection matrix so we can just draw using window
	 * coordinates.
	 */
	glMatrixMode( GL_PROJECTION );
	glPushMatrix();
	glLoadIdentity();
	glOrtho( 0, WIN_WIDTH, 0, WIN_HEIGHT, -1, 1 );

	glMatrixMode( GL_MODELVIEW );
	glPushMatrix();
	glLoadIdentity();

	rgb_pixmap = create_pixmap(GL_RGB);
	rgba_pixmap = create_pixmap(GL_RGBA);
}


static void
event_loop(void)
{
	for (;;) {
		XEvent event;
		XNextEvent (dpy, &event);

		if (event.type == KeyPress) {
			KeySym sym = XKeycodeToKeysym (dpy,
						       event.xkey.keycode, 0);

			if (sym == XK_Escape || sym == XK_q || sym == XK_Q)
				break;
			else
				draw();
		} else if (event.type == Expose) {
			GLboolean pass = draw();

			if (Automatic) {
				if (pass) {
					piglit_report_result(PIGLIT_SUCCESS);
					exit(0);
				} else {
					piglit_report_result(PIGLIT_FAILURE);
					exit(1);
				}
			}
		}
        }
}

int main(int argc, char**argv)
{
	unsigned long mask;
	XVisualInfo *visinfo;
	int attrib[] = {
		GLX_RGBA,
		GLX_RED_SIZE, 1,
		GLX_GREEN_SIZE, 1,
		GLX_BLUE_SIZE, 1,
		GLX_DOUBLEBUFFER,
		None
	};
	XSetWindowAttributes window_attr;
	GLXContext ctx;
	int i;
	const char *glx_extension_list;
	const GLubyte *extension_list;
	int screen;
	Window root_win;

	for(i = 1; i < argc; ++i) {
		if (!strcmp(argv[i], "-auto"))
			Automatic = 1;
		else
			fprintf(stderr, "Unknown option: %s\n", argv[i]);
	}

	dpy = XOpenDisplay(NULL);
	if (dpy == NULL) {
		fprintf(stderr, "couldn't open display\n");
		piglit_report_result(PIGLIT_FAILURE);
	}
	screen = DefaultScreen(dpy);
	root_win = RootWindow(dpy, screen);

	visinfo = glXChooseVisual(dpy, screen, attrib);
	if (visinfo == NULL) {
		fprintf(stderr,
			"Couldn't get an RGBA, double-buffered visual\n");
		piglit_report_result(PIGLIT_FAILURE);
		exit(1);
	}

	ctx = glXCreateContext(dpy, visinfo, NULL, True);
	if (ctx == None) {
		fprintf(stderr, "glXCreateContext failed\n");
		piglit_report_result(PIGLIT_FAILURE);
	}

	window_attr.background_pixel = 0;
	window_attr.border_pixel = 0;
	window_attr.colormap = XCreateColormap(dpy, root_win,
					       visinfo->visual, AllocNone);
	window_attr.event_mask = StructureNotifyMask | ExposureMask |
		KeyPressMask;
	mask = CWBackPixel | CWBorderPixel | CWColormap | CWEventMask;
	win = XCreateWindow(dpy, root_win, 0, 0,
			    WIN_WIDTH, WIN_HEIGHT,
			    0, visinfo->depth, InputOutput,
			    visinfo->visual, mask, &window_attr);
	XFree(visinfo);

	XMapWindow(dpy, win);

	glXMakeCurrent(dpy, win, ctx);

	/* This is a bogus way of checking for the extension.
	 * Needs more GLEW.
	 */
	glx_extension_list = glXQueryExtensionsString(dpy, screen);
	if (strstr(glx_extension_list, "GLX_EXT_texture_from_pixmap") == NULL) {
		fprintf(stderr, "Test requires GLX_EXT_texture_from_pixmap\n");
		piglit_report_result(PIGLIT_SKIP);
		exit(1);
	}
	extension_list = glGetString(GL_EXTENSIONS);
	if (strstr((const char *)extension_list,
		   "GL_ARB_texture_env_combine") == NULL) {
		fprintf(stderr, "Test requires GL_ARB_texture_env_combine\n");
		piglit_report_result(PIGLIT_SKIP);
		exit(1);
	}

	pglXBindTexImageEXT = (PFNGLXBINDTEXIMAGEEXTPROC)
		glXGetProcAddress((GLubyte *)"glXBindTexImageEXT");
	pglXReleaseTexImageEXT = (PFNGLXRELEASETEXIMAGEEXTPROC)
		glXGetProcAddress((GLubyte *)"glXReleaseTexImageEXT");
	if (pglXBindTexImageEXT == NULL || pglXReleaseTexImageEXT == NULL) {
		fprintf(stderr, "Couldn't get TFP functions\n");
		piglit_report_result(PIGLIT_FAILURE);
		exit(1);
	}

	init();

	if (!Automatic) {
		printf("Left rectangle (RGB) should be green on the top and\n"
		       "red on the bottom.  The right rectangle (RGBA) should\n"
		       "be the same, but darker on the right half.\n");
		printf("Press Escape to quit\n");
	}

	event_loop();

	return 0;
}
