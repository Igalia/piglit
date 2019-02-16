/*
 * Copyright © 2016 Intel Corporation
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

#include "sample_common.h"
#include "image_common.h"

/**
 * @file sample_rgb.c
 *
 * Create EGL images out of various YUV formatted dma buffers, set
 * them as external textures, set texture filters for avoiding need
 * for other mipmap-levels and sample the textures using a shader
 * program.
 */

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_es_version = 20;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

static int fourcc = -1;

static bool
format_has_alpha(int fourcc)
{
	switch (fourcc) {
	case DRM_FORMAT_AYUV:
		return true;
	default:
		return false;
	}
}

enum piglit_result
piglit_display(void)
{
	static const uint16_t p010[] = {
		/* Y */
		201,  281,  361, 441,
		201,  281,  361, 441,
		201,  281,  361, 441,
		201,  281,  361, 441,
		/* UV */
		481, 522, 562, 522,
		481, 642, 562, 642,
	};

	static const uint16_t p012[] = {
		/* Y */
		803,  1124,  1445, 1766,
		803,  1124,  1445, 1766,
		803,  1124,  1445, 1766,
		803,  1124,  1445, 1766,
		/* UV */
		1927, 2088, 2248, 2088,
		1927, 2569, 2248, 2569,
	};

	static const uint16_t p016[] = {
		/* Y */
		12850, 17990, 23130, 28270,
		12850, 17990, 23130, 28270,
		12850, 17990, 23130, 28270,
		12850, 17990, 23130, 28270,
		/* UV */
		30840, 33410, 35980, 33410,
		30840, 41120, 35980, 41120,
	};

	static const unsigned char nv12[] = {
		/* Y */
		 50,  70,  90, 110,
		 50,  70,  90, 110,
		 50,  70,  90, 110,
		 50,  70,  90, 110,
		/* UV */
		120, 130, 140, 130,
		120, 160, 140, 160,
	}, yuv420[] = {
		/* Y */
		 50,  70,  90, 110,
		 50,  70,  90, 110,
		 50,  70,  90, 110,
		 50,  70,  90, 110,
		/* U */
		120, 140,
		120, 140,
		/* V */
		130, 130,
		160, 160,
	}, yvu420[] = {
		/* Y */
		 50,  70,  90, 110,
		 50,  70,  90, 110,
		 50,  70,  90, 110,
		 50,  70,  90, 110,
		/* V */
		130, 130,
		160, 160,
		/* U */
		120, 140,
		120, 140,
	}, ayuv[] = {
		/* Increasing alpha ramp. */
		130, 120,  50, 0,
		130, 127,  70, 64,
		130, 133,  90, 128,
		130, 140, 110, 192,

		140, 120,  50, 0,
		140, 127,  70, 64,
		140, 133,  90, 128,
		140, 140, 110, 192,

		150, 120,  50, 0,
		150, 127,  70, 64,
		150, 133,  90, 128,
		150, 140, 110, 192,

		160, 120,  50, 0,
		160, 127,  70, 64,
		160, 133,  90, 128,
		160, 140, 110, 192,
	};

	static unsigned char expected[4 * 4 * 4] = {
		 44,  41,  25, 255,
		 67,  64,  48, 255,
		 90,  79, 111, 255,
		114, 103, 135, 255,

		 44,  41,  25, 255,
		 67,  64,  48, 255,
		 90,  79, 111, 255,
		114, 103, 135, 255,

		 92,  16,  25, 255,
		115,  39,  48, 255,
		138,  55, 111, 255,
		161,  78, 135, 255,

		 92,  16,  25, 255,
		115,  39,  48, 255,
		138,  55, 111, 255,
		161,  78, 135, 255,
	};

	const unsigned char *t;

	enum piglit_result res;
	switch (fourcc) {
	case DRM_FORMAT_P010:
		t = (unsigned char *) p010;
		break;
	case DRM_FORMAT_P012:
		t = (unsigned char *) p012;
		break;
	case DRM_FORMAT_P016:
		t = (unsigned char *) p016;
		break;
	case DRM_FORMAT_NV12:
		t = nv12;
		break;
	case DRM_FORMAT_YUV420:
		t = yuv420;
		break;
	case DRM_FORMAT_YVU420:
		t = yvu420;
		break;
	case DRM_FORMAT_AYUV:
	case DRM_FORMAT_XYUV8888:
		t = ayuv;
		break;
	default:
		return PIGLIT_SKIP;
	}

	/* Modify alpha values of the expected result. */
	if (format_has_alpha(fourcc)) {
		unsigned char *p = expected;
		for (uint32_t i = 0; i < 4 * 4; i++, p += 4)
			p[3] = i * (256 / 4);
	}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	res = dma_buf_create_and_sample_32bpp(4, 4, fourcc, t);
	if (res != PIGLIT_PASS)
		return res;

	/* Lower tolerance in case we're running against a 565 render
	 * target (gbm).
	 */
	piglit_set_tolerance_for_bits(5, 6, 5, 8);

	if (!piglit_probe_image_ubyte(0, 0, 4, 4, GL_RGBA, expected))
		res = PIGLIT_FAIL;

	piglit_present_results();

	return res;
}

static int
parse_format(const char *s)
{
	if (strlen(s) != 4)
		return -1;

	return (int)fourcc_code(s[0], s[1], s[2], s[3]);
}

void
piglit_init(int argc, char **argv)
{
	unsigned i;
	EGLDisplay egl_dpy = eglGetCurrentDisplay();

	piglit_require_egl_extension(egl_dpy, "EGL_EXT_image_dma_buf_import");
	piglit_require_extension("GL_OES_EGL_image_external");

	for (i = 1; i < argc; i++) {
                static const char fmt[] = "-fmt=";

                if (strncmp(argv[i], fmt, sizeof(fmt) - 1)) {
			fprintf(stderr, "unknown argument %s\n", argv[i]);
                        continue;
		}

		fourcc = parse_format(argv[i] + sizeof(fmt) - 1);
		if (fourcc == -1) {
			fprintf(stderr, "invalid format: %s\n", argv[i]);
			piglit_report_result(PIGLIT_SKIP);
		}
        }

	if (fourcc == -1) {
		fprintf(stderr, "format not specified\n");
		piglit_report_result(PIGLIT_SKIP);
	}
}
