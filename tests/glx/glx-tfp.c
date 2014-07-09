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

/** @file glx-tfp.c
 * Tests the GLX_EXT_texture_from_pixmap extension, in particular the bug
 * reported in https://bugs.freedesktop.org/show_bug.cgi?id=19910 in which
 * the RGB/RGBA attribute of the drawable was misplaced, resulting in always
 * acting as if the pixmap had the alpha channel present.
 *
 * This test is based loosely on the testcase attached to that bug, by
 * Neil Roberts <neil@linux.intel.com>.
 */

#include "piglit-util-gl.h"
#include "piglit-glx-util.h"
#include "GL/glx.h"
#include "X11/extensions/Xrender.h"

GLfloat tex_data[4][4] = {
	{ 1.0, 0.0, 0.0, 1.0 },
	{ 1.0, 0.0, 0.0, 0.5 },
	{ 0.0, 1.0, 0.0, 1.0 },
	{ 0.0, 1.0, 0.0, 0.5 },
};

#define WIN_WIDTH	256
#define WIN_HEIGHT	128

static GLXPixmap rgb_pixmap, rgba_pixmap;
static Display *dpy;
static Window win;
int piglit_width = WIN_WIDTH;
int piglit_height = WIN_HEIGHT;

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

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	/* Set the texture combiner to give {r*a, g*a, b*a, a} so we can see
	 * the effect of the alpha channel in terms of color.
	 */
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);

	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_TEXTURE);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_TEXTURE);

	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_ALPHA);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_TEXTURE);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, GL_TEXTURE); /* ignored */

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

static enum piglit_result
draw(Display *dpy)
{
	GLboolean pass = GL_TRUE;
	int draw_w = piglit_width / 4;
	int draw_h = piglit_height / 2;
	int rgb_x = piglit_width / 8;
	int rgb_y = piglit_height / 4;
	int rgba_x = piglit_width * 5 / 8;
	int rgba_y = piglit_height / 4;

	/* Clear background to gray */
	glClearColor(0.5, 0.5, 0.5, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	draw_pixmap(rgb_pixmap, rgb_x, rgb_y, draw_w, draw_h);
	draw_pixmap(rgba_pixmap, rgba_x, rgba_y, draw_w, draw_h);

	pass &= check_results(GL_FALSE, rgb_x, rgb_y, draw_w, draw_h);
	pass &= check_results(GL_TRUE, rgba_x, rgba_y, draw_w, draw_h);

	glXSwapBuffers(dpy, win);

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

static void
set_pixel(Display *dpy, Picture picture, int x, int y, GLfloat *color)
{
	XRectangle rect = {x, y, 1, 1};
	XRenderColor render_color;

	render_color.red = 0xffff * color[0];
	render_color.green = 0xffff * color[1];
	render_color.blue = 0xffff * color[2];
	render_color.alpha = 0xffff * color[3];

	XRenderFillRectangles(dpy, PictOpSrc, picture, &render_color, &rect, 1);
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
	XRenderPictFormat *render_format;
	Picture picture;

	if (format == GL_RGBA) {
		fb_config_attribs = rgba_fb_config_attribs;
		pixmap_attribs = rgba_pixmap_attribs;
		render_format = XRenderFindStandardFormat(dpy,
							  PictStandardARGB32);
	} else {
		fb_config_attribs = rgb_fb_config_attribs;
		pixmap_attribs = rgb_pixmap_attribs;
		render_format = XRenderFindStandardFormat(dpy,
							  PictStandardRGB24);
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
			       2, 2, render_format->depth);
	picture = XRenderCreatePicture(dpy, pixmap, render_format, 0, NULL);

	glx_pixmap = glXCreatePixmap(dpy, fb_config, pixmap, pixmap_attribs);

	vis = glXGetVisualFromFBConfig(dpy, fb_config);

	set_pixel(dpy, picture, 0, 0, tex_data[0]);
	set_pixel(dpy, picture, 1, 0, tex_data[1]);
	set_pixel(dpy, picture, 0, 1, tex_data[2]);
	set_pixel(dpy, picture, 1, 1, tex_data[3]);

	XFree(fb_configs);
	XFree(vis);

	return glx_pixmap;
}

static void init(void)
{
	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	rgb_pixmap = create_pixmap(GL_RGB);
	rgba_pixmap = create_pixmap(GL_RGBA);
}

int main(int argc, char**argv)
{
	XVisualInfo *visinfo;
	GLXContext ctx;
	int i;

	for(i = 1; i < argc; ++i) {
		if (!strcmp(argv[i], "-auto"))
			piglit_automatic = 1;
		else
			fprintf(stderr, "Unknown option: %s\n", argv[i]);
	}

	dpy = XOpenDisplay(NULL);
	if (dpy == NULL) {
		fprintf(stderr, "couldn't open display\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	visinfo = piglit_get_glx_visual(dpy);
	ctx = piglit_get_glx_context(dpy, visinfo);
	win = piglit_get_glx_window(dpy, visinfo);
	XFree(visinfo);

	glXMakeCurrent(dpy, win, ctx);

	piglit_dispatch_default_init(PIGLIT_DISPATCH_GL);

	if (piglit_automatic)
		piglit_glx_set_no_input();

	XMapWindow(dpy, win);

	piglit_require_glx_extension(dpy, "GLX_EXT_texture_from_pixmap");
	if (!piglit_is_extension_supported("GL_ARB_texture_env_combine")) {
		fprintf(stderr, "Test requires GL_ARB_texture_env_combine\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	pglXBindTexImageEXT = (PFNGLXBINDTEXIMAGEEXTPROC)
		glXGetProcAddress((GLubyte *)"glXBindTexImageEXT");
	pglXReleaseTexImageEXT = (PFNGLXRELEASETEXIMAGEEXTPROC)
		glXGetProcAddress((GLubyte *)"glXReleaseTexImageEXT");
	if (pglXBindTexImageEXT == NULL || pglXReleaseTexImageEXT == NULL) {
		fprintf(stderr, "Couldn't get TFP functions\n");
		piglit_report_result(PIGLIT_FAIL);
		exit(1);
	}

	init();

	if (!piglit_automatic) {
		printf("Left rectangle (RGB) should be green on the top and\n"
		       "red on the bottom.  The right rectangle (RGBA) should\n"
		       "be the same, but darker on the right half.\n");
		printf("Press Escape to quit\n");
	}

	piglit_glx_event_loop(dpy, draw);

	return 0;
}
