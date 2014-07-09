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
 * Verify that the list of fbconfigs conforms to GLX 1.4 section 3.3.3.
 *
 * This reproduces X.org bugzilla #34265, among much else.
 */

#include "piglit-util-gl.h"
#include "piglit-glx-util.h"

int piglit_width = 10;
int piglit_height = 10;

static PFNGLXGETFBCONFIGSPROC GetFBConfigs = NULL;
static PFNGLXGETFBCONFIGATTRIBPROC GetFBConfigAttrib = NULL;
static PFNGLXGETVISUALFROMFBCONFIGPROC GetVisualFromFBConfig = NULL;

static void
fbconfig_sanity_warn(int *result)
{
	if (*result != PIGLIT_FAIL)
		*result = PIGLIT_WARN;
}

int
main(int argc, char **argv)
{
	Display *dpy;
	int i;
	int result = PIGLIT_PASS;
	GLXFBConfig *configs;
	int num_configs;

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

	/* Iterate over the list of fbconfigs.  Check that each fbconfig that
	 * has the GLX_WINDOW_BIT or GLX_PIXMAP_BIT set also has a non-zero
	 * X visual.  Also verify that glXGetVisualFromFBConfig returns NULL
	 * if and only if the X visual is 0.
	 */
	for (i = 0; i < num_configs; i++) {
		int draw_type;
		int visual_id;
		int config_id;
		int sample_buffers;
		int render_type;
		int x_renderable;
		int caveat;
		int vtype;
		int transparency;
		XVisualInfo *vinfo;

		GetFBConfigAttrib(dpy, configs[i], GLX_FBCONFIG_ID,
				  &config_id);
		GetFBConfigAttrib(dpy, configs[i], GLX_DRAWABLE_TYPE,
				  &draw_type);
		GetFBConfigAttrib(dpy, configs[i], GLX_VISUAL_ID,
				  &visual_id);
		GetFBConfigAttrib(dpy, configs[i], GLX_SAMPLE_BUFFERS,
				  &sample_buffers);
		GetFBConfigAttrib(dpy, configs[i], GLX_RENDER_TYPE,
				  &render_type);
		GetFBConfigAttrib(dpy, configs[i], GLX_X_RENDERABLE,
				  &x_renderable);
		GetFBConfigAttrib(dpy, configs[i], GLX_CONFIG_CAVEAT,
				  &caveat);
		GetFBConfigAttrib(dpy, configs[i], GLX_TRANSPARENT_TYPE,
				  &transparency);
		GetFBConfigAttrib(dpy, configs[i], GLX_X_VISUAL_TYPE,
				  &vtype);

		if (!draw_type) {
			fprintf(stderr, "FBConfig 0x%x supports no "
				"drawables\n", config_id);
			fbconfig_sanity_warn(&result);
		}

		if ((draw_type & GLX_WINDOW_BIT) != 0
		    && visual_id == 0) {
			fprintf(stderr, "FBconfig 0x%x has GLX_WINDOW_BIT "
				"set, but the Visual ID is 0!\n",
				config_id);
			result = PIGLIT_FAIL;
		}

		GetFBConfigAttrib(dpy, configs[i], GLX_TRANSPARENT_TYPE,
				  &transparency);
		vinfo = GetVisualFromFBConfig(dpy, configs[i]);
		if ((vinfo == NULL) != (visual_id == 0)) {
			fprintf(stderr, "FBconfig 0x%x has vinfo = %p and "
				"visual ID = 0x%x.  Both or neither must be "
				"NULL / zero.\n",
				config_id, vinfo, visual_id);
			result = PIGLIT_FAIL;
		}

		if (vinfo != NULL && vinfo->visualid != visual_id) {
			fprintf(stderr, "FBconfig 0x%x has "
				"vinfo->visualid = 0x%x and visual ID = 0x%x. "
				"These should match!\n",
				config_id, (int) vinfo->visualid, visual_id);
			result = PIGLIT_FAIL;
		}

		if (vinfo) {
			int depth;
			GetFBConfigAttrib(dpy, configs[i], GLX_BUFFER_SIZE,
					  &depth);
			if (vinfo->class == StaticColor ||
			    vinfo->class == PseudoColor) {
				if (depth != vinfo->depth) {
					fprintf(stderr, "FBConfig 0x%x has "
						"depth %d but visual 0x%x has "
						"depth %d.  These should "
						"match!\n", config_id, depth,
						(int)vinfo->visualid,
						vinfo->depth);
					result = PIGLIT_FAIL;
				}
			}
			if (vinfo->class == TrueColor ||
			    vinfo->class == DirectColor) {
				if (depth < vinfo->depth) {
					fprintf(stderr, "FBConfig 0x%x has "
						"depth %d < visual 0x%x depth "
						"%d.  Should be >= visual "
						"depth!\n",
						config_id, depth,
						(int)vinfo->visualid,
						vinfo->depth);
					result = PIGLIT_FAIL;
				}
			}
			if (vtype == GLX_NONE) {
				fprintf(stderr, "FBConfig 0x%x supports "
					"windows but has no visual type\n",
					config_id);
				result = PIGLIT_FAIL;
			} else {
				int fail = 0;
				int ci_fail = 0;
				switch (vinfo->class) {
					case TrueColor:
						if (vtype != GLX_TRUE_COLOR)
							fail = 1;
						if (render_type &
						    GLX_COLOR_INDEX_BIT)
							ci_fail = 1;
						break;
					case DirectColor:
						if (vtype != GLX_DIRECT_COLOR)
							fail = 1;
						if (render_type &
						    GLX_COLOR_INDEX_BIT)
							ci_fail = 1;
						break;
					case PseudoColor:
						if (vtype != GLX_PSEUDO_COLOR)
							fail = 1;
						break;
					case StaticColor:
						if (vtype != GLX_STATIC_COLOR)
							fail = 1;
						break;
					case GrayScale:
						if (vtype != GLX_GRAY_SCALE)
							fail = 1;
						break;
					case StaticGray:
						if (vtype != GLX_STATIC_GRAY)
							fail = 1;
						break;
					default:
						fail = 2;
						break;
				}
				if (ci_fail) {
					fprintf(stderr, "FBConfig 0x%x is "
						"{True,Direct}Color but claims "
						"support for color-index\n",
						config_id);
					result = PIGLIT_FAIL;
				}
				if (fail == 2) {
					fprintf(stderr, "FBConfig 0x%x has "
						"visual with unknown class "
						"%d\n", config_id,
						vinfo->class);
					result = PIGLIT_FAIL;
				} else if (fail == 1) {
					fprintf(stderr, "FBConfig 0x%x claims "
						"visual class that does not "
						"match visual 0x%x\n",
						config_id,
						(int)vinfo->visualid);
					result = PIGLIT_FAIL;
				}
			}
		}

		if (sample_buffers == 0) {
			int samples;
			GetFBConfigAttrib(dpy, configs[i],
					  GLX_SAMPLES, &samples);
			if (samples != 0) {
				fprintf(stderr, "FBConfig 0x%x has "
					"0 sample buffers but %d "
					"samples, should be 0\n",
					config_id, samples);
				result = PIGLIT_FAIL;
			}
		} else if (sample_buffers == 1) {
			/* TODO check color/depth/stencil bits per sample */
		} else {
			fprintf(stderr, "FBConfig 0x%x has bizarre "
				"GLX_SAMPLE_BUFFERS of %d, should be "
				"0 or 1\n", config_id, sample_buffers);
			result = PIGLIT_FAIL;
		}

		if (render_type == 0) {
			fprintf(stderr, "FBConfig 0x%x can be bound to "
				"neither RGBA nor color-index contexts\n",
				config_id);
			result = PIGLIT_FAIL;
		} else if
		    (render_type & ~(GLX_RGBA_BIT | GLX_COLOR_INDEX_BIT)) {
			fprintf(stderr, "FBConfig 0x%x supports rendering "
				"to something other than RGBA or CI, "
				"piglit needs to be fixed\n", config_id);
			fbconfig_sanity_warn(&result);
		}

		if (x_renderable &&
		    !(draw_type & (GLX_WINDOW_BIT | GLX_PIXMAP_BIT))) {
			fprintf(stderr, "FBConfig 0x%x claims to be X "
				"renderable (0x%x), but does not support "
				"windows or pixmaps\n", config_id, draw_type);
			fbconfig_sanity_warn(&result);
		} else if (!x_renderable &&
			   (draw_type & (GLX_WINDOW_BIT | GLX_PIXMAP_BIT))) {
			fprintf(stderr, "FBConfig 0x%x claims to not be X "
				"renderable but claims to support windows "
				"and/or pixmaps\n", config_id);
			result = PIGLIT_FAIL;
		}

		switch (caveat) {
			case GLX_NONE:
			case GLX_SLOW_CONFIG:
			case GLX_NON_CONFORMANT_CONFIG:
				break;
			default:
				fprintf(stderr, "FBConfig 0x%x has unknown "
					"caveat 0x%x\n", config_id, caveat);
				result = PIGLIT_FAIL;
				break;
		}

		switch (transparency) {
			case GLX_NONE:
				break;
			case GLX_TRANSPARENT_RGB:
				if (vtype != GLX_TRUE_COLOR &&
				    vtype != GLX_DIRECT_COLOR) {
					fprintf(stderr, "FBConfig 0x%x is rgb "
						"transparent but not an rgb "
						"visual type\n", config_id);
					result = PIGLIT_FAIL;
				}
				break;
			case GLX_TRANSPARENT_INDEX:
				if (vtype == GLX_TRUE_COLOR ||
				    vtype == GLX_DIRECT_COLOR) {
					fprintf(stderr, "FBConfig 0x%x is ci "
						"transparent but not a ci "
						"visual type\n", config_id);
					result = PIGLIT_FAIL;
				}
				break;
			default:
				fprintf(stderr, "FBConfig 0x%x has unknown "
					"transparency type 0x%x\n", config_id,
					transparency);
				result = PIGLIT_FAIL;
				break;
		}
	}

	piglit_report_result(result);
	return 0;
}
