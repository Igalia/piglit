/*
 * Copyright 2018 Intel Corporation
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

#include "piglit-util.h"
#include "piglit-util-egl.h"

int
main(int argc, char *argv[])
{
	EGLDisplay dpy;
	EGLBoolean (*peglQueryDmaBufFormatsEXT)(EGLDisplay dpy,
						EGLint max_formats,
						EGLint *formats,
						EGLint *num_formats);
	EGLBoolean (*peglQueryDmaBufModifiersEXT)(EGLDisplay dpy,
						  EGLint format,
						  EGLint max_modifiers,
						  EGLuint64KHR *modifiers,
						  EGLBoolean *external_only,
						  EGLint *num_modifiers);
	EGLint f, n_formats, *formats, rand_format, n_modifiers;
	EGLint egl_major, egl_minor;
	EGLBoolean ret;
	bool in_list;

	/* Strip common piglit args. */
	piglit_strip_arg(&argc, argv, "-fbo");
	piglit_strip_arg(&argc, argv, "-auto");

	dpy = piglit_egl_get_default_display(EGL_NONE);
	if (!dpy) {
		piglit_loge("failed to get EGLDisplay\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	ret = eglInitialize(dpy, &egl_major, &egl_minor);
	if (!ret) {
		EGLint egl_error = eglGetError();
		piglit_loge("failed to get EGLConfig: %s(0x%x)",
			    piglit_get_egl_error_name(egl_error), egl_error);
		piglit_report_result(PIGLIT_FAIL);
	}

	if (egl_major < 1 || (egl_major == 1 && egl_minor < 2)) {
		piglit_logi("EGL 1.2 required");
		piglit_report_result(PIGLIT_SKIP);
	}

	piglit_require_egl_extension(dpy, "EGL_EXT_image_dma_buf_import_modifiers");

	peglQueryDmaBufFormatsEXT =
		(void *)eglGetProcAddress("eglQueryDmaBufFormatsEXT");
	peglQueryDmaBufModifiersEXT =
		(void *)eglGetProcAddress("eglQueryDmaBufModifiersEXT");

	if (!peglQueryDmaBufFormatsEXT || !peglQueryDmaBufModifiersEXT) {
		piglit_loge("No display query entrypoint\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	ret = peglQueryDmaBufFormatsEXT(dpy, 0, NULL, &n_formats);
	if (!ret)
		piglit_report_result(PIGLIT_FAIL);
	piglit_logd("Found %i format(s):", n_formats);

	formats = calloc(n_formats, sizeof(*formats));
	peglQueryDmaBufFormatsEXT(dpy, n_formats, formats, &n_formats);

	for (f = 0; f < n_formats; f++) {
		EGLBoolean *external;
		EGLuint64KHR *modifiers;
		EGLint m;

		piglit_logd("Format 0x%x (%c%c%c%c):",
			    formats[f],
			    formats[f] & 0xff,
			    (formats[f] >> 8) & 0xff,
			    (formats[f] >> 16) & 0xff,
			    (formats[f] >> 24) & 0xff);

		ret = peglQueryDmaBufModifiersEXT(dpy, formats[f], 0, NULL,
						  NULL, &n_modifiers);
		if (!ret)
			piglit_report_result(PIGLIT_FAIL);

		piglit_logd("\t%i modifiers:", n_modifiers);

		modifiers = calloc(n_modifiers, sizeof(*modifiers));
		external = calloc(n_modifiers, sizeof(*external));
		ret = peglQueryDmaBufModifiersEXT(dpy, formats[f],
						  n_modifiers, modifiers,
						  external, &n_modifiers);
		if (!ret)
			piglit_report_result(PIGLIT_FAIL);

		for (m = 0; m < n_modifiers; m++) {
			piglit_logd("\t0x%016lx external=%i", modifiers[m], external[m]);
		}

		free(modifiers);
		free(external);
	}

	/* Try to query an invalid format. */
	do {
		in_list = false;
		rand_format = rand();

		for (f = 0; f < n_formats; f++) {
			if (formats[f] == rand_format) {
				in_list = true;
				break;
			}
		}
	} while (in_list);

	piglit_logd("Trying to query random format 0x%x", rand_format);
	ret = peglQueryDmaBufModifiersEXT(dpy, rand_format,
					  0, NULL, NULL, &n_modifiers);
	if (ret)
		piglit_report_result(PIGLIT_FAIL);
	if (eglGetError() != EGL_BAD_PARAMETER)
		piglit_report_result(PIGLIT_FAIL);

	free(formats);

	piglit_report_result(PIGLIT_PASS);
}
