/*
 * Copyright © 2011 Intel Corporation
 * Copyright 2011 Red Hat, Inc.
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

/** @file glx-fbconfig-compliance.c
 * Verify that there exists at least one fbconfig conforming to the
 * minimum requirements specified by GLX 1.4 section 3.3.3
 */

#include "piglit-util-gl.h"
#include "piglit-glx-util.h"

static PFNGLXGETFBCONFIGSPROC GetFBConfigs = NULL;
static PFNGLXGETFBCONFIGATTRIBPROC GetFBConfigAttrib = NULL;
static PFNGLXGETVISUALFROMFBCONFIGPROC GetVisualFromFBConfig = NULL;
static PFNGLXCHOOSEFBCONFIGPROC ChooseFBConfig = NULL;

int piglit_width = 10;
int piglit_height = 10;

static int
config_is_sufficient(Display *dpy, GLXFBConfig config, int vdepth, int rgba)
{
	int draw_type;
	int render_type;
	int color_red, color_green, color_blue;
	int caveat;
	int stencil;
	int depth;
	int accum_red, accum_green, accum_blue;
	int color_buffer_size;
	int level;

	GetFBConfigAttrib(dpy, config, GLX_DRAWABLE_TYPE, &draw_type);
	GetFBConfigAttrib(dpy, config, GLX_RENDER_TYPE, &render_type);
	GetFBConfigAttrib(dpy, config, GLX_CONFIG_CAVEAT, &caveat);
	GetFBConfigAttrib(dpy, config, GLX_RED_SIZE, &color_red);
	GetFBConfigAttrib(dpy, config, GLX_GREEN_SIZE, &color_green);
	GetFBConfigAttrib(dpy, config, GLX_BLUE_SIZE, &color_blue);
	GetFBConfigAttrib(dpy, config, GLX_STENCIL_SIZE, &stencil);
	GetFBConfigAttrib(dpy, config, GLX_DEPTH_SIZE, &depth);
	GetFBConfigAttrib(dpy, config, GLX_ACCUM_RED_SIZE, &accum_red);
	GetFBConfigAttrib(dpy, config, GLX_ACCUM_GREEN_SIZE, &accum_green);
	GetFBConfigAttrib(dpy, config, GLX_ACCUM_BLUE_SIZE, &accum_blue);
	GetFBConfigAttrib(dpy, config, GLX_BUFFER_SIZE, &color_buffer_size);
	GetFBConfigAttrib(dpy, config, GLX_LEVEL, &level);
	
	/* must support window rendering */
	if ((draw_type & GLX_WINDOW_BIT) == 0) {
		return 0;
	}

	/* must support rgba, if rgba, or ci, if ci */
	if (rgba) {
		if ((render_type & GLX_RGBA_BIT) == 0) {
			return 0;
		}
	} else {
		if ((render_type & GLX_COLOR_INDEX_BIT) == 0) {
			return 0;
		}
	}

	/* must not be non-conformant */
	if (caveat == GLX_NON_CONFORMANT_CONFIG) {
		return 0;
	}

	/* must have at least one color buffer */
	if ((color_red + color_green + color_blue) < 1) {
		return 0;
	}

	/* must have at least 1 bit of stencil */
	if (stencil < 1) {
		return 0;
	}

	/* must have at least 12 bits of depth */
	if (depth < 12) {
		return 0;
	}

	/* must have an accumulation buffer */
	if ((accum_red + accum_green + accum_blue) < 1) {
		return 0;
	}

	/* color depth must match deepest visual depth */
	if (color_buffer_size != vdepth) {
		return 0;
	}
	
	/* must exist on fb level 0 */
	if (level != 0) {
		return 0;
	}

	return 1;
}

static int
get_max_visual_depth(Display *dpy, int rgba)
{
	XVisualInfo template, *vi = NULL;
	int i, nvis, class;
	int depth = 0;

	for (class = StaticColor; class <= DirectColor; class++) {
		if (class > PseudoColor && !rgba)
			break;

		template.class = class;
		vi = XGetVisualInfo(dpy, VisualClassMask, &template, &nvis);

		if (!vi)
			continue;

		for (i = 0; i < nvis; i++)
			if (vi[i].depth > depth)
				depth = vi[i].depth;

		XFree(vi);
		vi = NULL;
	}

	return depth;
}

int
main(int argc, char **argv)
{
	Display *dpy;
	int i;
	GLXFBConfig *configs;
	int num_configs;
	int visual_depth;
	int conformant = 0;

	dpy = XOpenDisplay(NULL);
	if (dpy == NULL) {
		fprintf(stderr, "couldn't open display\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	/* Test requires at least GLX version 1.3.  Otherwise there is no
	 * glXGetFBConfigs function.
	 */
	piglit_require_glx_version(dpy, 1, 3);
	piglit_require_glx_extension(dpy, "GLX_ARB_get_proc_address");

	GetFBConfigs = (PFNGLXGETFBCONFIGSPROC)
		glXGetProcAddressARB((GLubyte *) "glXGetFBConfigs");
	GetFBConfigAttrib = (PFNGLXGETFBCONFIGATTRIBPROC)
		glXGetProcAddressARB((GLubyte *) "glXGetFBConfigAttrib");
	GetVisualFromFBConfig = (PFNGLXGETVISUALFROMFBCONFIGPROC)
		glXGetProcAddressARB((GLubyte *) "glXGetVisualFromFBConfig");

	configs = GetFBConfigs(dpy, DefaultScreen(dpy), &num_configs);

	/* rgba support is mandatory */
	visual_depth = get_max_visual_depth(dpy, 1);
	if (!visual_depth)
		goto out;

	for (i = 0; i < num_configs; i++)
		conformant |= config_is_sufficient(dpy, configs[i],
						   visual_depth, 1);

	/* color index support is not mandatory */
	visual_depth = get_max_visual_depth(dpy, 0);
	if (visual_depth) {
		GLXFBConfig *ci_configs;
		int num_ci_configs;
		int ci_conformant = 0;
		const int ci_attribs[] = {
			GLX_RENDER_TYPE, GLX_COLOR_INDEX_BIT,
			GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
			None
		};

		/* even if you have CI visuals, you needn't have CI fbconfigs */
		ci_configs = ChooseFBConfig(dpy, DefaultScreen(dpy),
					    ci_attribs, &num_ci_configs);
		if (!ci_configs)
			goto out;

		/* but if you do have them, they must conform */
		for (i = 0; i < num_configs; i++)
			ci_conformant = config_is_sufficient(dpy, configs[i],
							     visual_depth, 0);

		conformant &= ci_conformant;
	}

out:
	piglit_report_result(conformant ? PIGLIT_PASS : PIGLIT_FAIL);
	return 0;
}
