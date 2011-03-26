/*
 * Copyright © 2011 Intel Corporation
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

/** @file glx-fbconfig-sanity.c
 * Verify that every FBconfig has a sane correspondence between its drawable
 * type and its visual ID.
 *
 * This reproduces X.org bugzilla #34265.
 */

#include "piglit-util.h"
#include "piglit-glx-util.h"

int piglit_width = 10;
int piglit_height = 10;

static PFNGLXGETFBCONFIGSPROC GetFBConfigs = NULL;
static PFNGLXGETFBCONFIGATTRIBPROC GetFBConfigAttrib = NULL;
static PFNGLXGETVISUALFROMFBCONFIGPROC GetVisualFromFBConfig = NULL;

int
main(int argc, char **argv)
{
	Display *dpy;
	int i;
	int result = PIGLIT_SUCCESS;
	GLXFBConfig *configs;
	int num_configs;

	dpy = XOpenDisplay(NULL);
	if (dpy == NULL) {
		fprintf(stderr, "couldn't open display\n");
		piglit_report_result(PIGLIT_FAILURE);
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

	/* Iterate over the list of fbconfigs.  Check that each fbconfig that
	 * has the GLX_WINDOW_BIT or GLX_PIXMAP_BIT set also has a non-zero
	 * X visual.  Also verify that glXGetVisualFromFBConfig returns NULL
	 * if and only if the X visual is 0.
	 */
	for (i = 0; i < num_configs; i++) {
		int draw_type;
		int visual_id;
		int config_id;
		XVisualInfo *vinfo;

		GetFBConfigAttrib(dpy, configs[i], GLX_FBCONFIG_ID,
				  &config_id);
		GetFBConfigAttrib(dpy, configs[i], GLX_DRAWABLE_TYPE,
				  &draw_type);
		GetFBConfigAttrib(dpy, configs[i], GLX_VISUAL_ID,
				  &visual_id);

		if ((draw_type & GLX_WINDOW_BIT) != 0
		    && visual_id == 0) {
			fprintf(stderr, "FBconfig 0x%x has GLX_WINDOW_BIT "
				"set, but the Visual ID is 0!\n",
				config_id);
			result = PIGLIT_FAILURE;
		}

		if ((draw_type & GLX_PIXMAP_BIT) != 0
		    && visual_id == 0) {
			fprintf(stderr, "FBconfig 0x%x has GLX_PIXMAP_BIT "
				"set, but the Visual ID is 0!\n",
				config_id);
			result = PIGLIT_FAILURE;
		}

		vinfo = GetVisualFromFBConfig(dpy, configs[i]);
		if ((vinfo == NULL) != (visual_id == 0)) {
			fprintf(stderr, "FBconfig 0x%x has vinfo = %p and "
				"visual ID = %d.  Both or neither must be "
				"NULL / zero.\n",
				config_id, vinfo, visual_id);
			result = PIGLIT_FAILURE;
		}

		if (vinfo != NULL && vinfo->visualid != visual_id) {
			fprintf(stderr, "FBconfig 0x%x has "
				"vinfo->visualid = %d and visual ID = %d.  "
				"These should match!\n",
				config_id, vinfo->visualid, visual_id);
			result = PIGLIT_FAILURE;
		}
	}

	piglit_report_result(result);
	return 0;
}
