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

#include <endian.h>
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
	case DRM_FORMAT_Y410:
	case DRM_FORMAT_Y412:
	case DRM_FORMAT_Y416:
		return true;
	default:
		return false;
	}
}

#ifndef __BIG_ENDIAN__
/* Assume __LITTLE_ENDIAN__ */
#  ifndef HAVE_HTOLE16
#    define htole16(x) (uint16_t)(x)
#    define HAVE_HTOLE16
#  endif
#  ifndef HAVE_HTOLE32
#    define htole32(x) (uint32_t)(x)
#    define HAVE_HTOLE32
#  endif
#  ifndef HAVE_HTOLE64
#    define htole64(x) (uint64_t)(x)
#    define HAVE_HTOLE64
#  endif
#endif

#define PACK_Y410(a, y, cr, cb)				\
	htole64(((uint32_t)(a  & 0x003) << 30) |	\
		((uint32_t)(cr & 0x3ff) << 20) |	\
		((uint32_t)(y  & 0x3ff) << 10) |	\
		((uint32_t)(cb & 0x3ff) <<  0))

#define PACK_Y416(a, y, cr, cb)				\
	htole64((((uint64_t)(uint16_t) a) << 48) |	\
		(((uint64_t)(uint16_t)cr) << 32) |	\
		(((uint64_t)(uint16_t) y) << 16) |	\
		(((uint64_t)(uint16_t)cb) <<  0))

enum piglit_result
piglit_display(void)
{
#ifdef HAVE_HTOLE16
	uint16_t p0xx[] = {
		/* Y */
		htole16(12850), htole16(17990), htole16(23130), htole16(28270),
		htole16(12850), htole16(17990), htole16(23130), htole16(28270),
		htole16(12850), htole16(17990), htole16(23130), htole16(28270),
		htole16(12850), htole16(17990), htole16(23130), htole16(28270),
		/* UV */
		htole16(30840), htole16(33410), htole16(35980), htole16(33410),
		htole16(30840), htole16(41120), htole16(35980), htole16(41120),
	};
#endif

	const uint32_t y410[] = {
		PACK_Y410(0, 200, 520, 480),
		PACK_Y410(1, 280, 520, 511),
		PACK_Y410(2, 360, 520, 535),
		PACK_Y410(3, 440, 520, 560),

		PACK_Y410(0, 200, 560, 480),
		PACK_Y410(1, 280, 560, 511),
		PACK_Y410(2, 360, 560, 535),
		PACK_Y410(3, 440, 560, 560),

		PACK_Y410(0, 200, 600, 480),
		PACK_Y410(1, 280, 600, 511),
		PACK_Y410(2, 360, 600, 535),
		PACK_Y410(3, 440, 600, 560),

		PACK_Y410(0, 200, 640, 480),
		PACK_Y410(1, 280, 640, 511),
		PACK_Y410(2, 360, 640, 535),
		PACK_Y410(3, 440, 640, 560),
	};

	uint64_t y41x[] = {
		PACK_Y416(0x0000, 0x3232, 0x8282, 0x7878),
		PACK_Y416(0x5555, 0x4646, 0x8282, 0x7F7F),
		PACK_Y416(0xAAAA, 0x5A5A, 0x8282, 0x8585),
		PACK_Y416(0xFFFF, 0x6E6E, 0x8282, 0x8C8C),

		PACK_Y416(0x0000, 0x3232, 0x8C8C, 0x7878),
		PACK_Y416(0x5555, 0x4646, 0x8C8C, 0x7F7F),
		PACK_Y416(0xAAAA, 0x5A5A, 0x8C8C, 0x8585),
		PACK_Y416(0xFFFF, 0x6E6E, 0x8C8C, 0x8C8C),

		PACK_Y416(0x0000, 0x3232, 0x9696, 0x7878),
		PACK_Y416(0x5555, 0x4646, 0x9696, 0x7F7F),
		PACK_Y416(0xAAAA, 0x5A5A, 0x9696, 0x8585),
		PACK_Y416(0xFFFF, 0x6E6E, 0x9696, 0x8C8C),

		PACK_Y416(0x0000, 0x3232, 0xA0A0, 0x7878),
		PACK_Y416(0x5555, 0x4646, 0xA0A0, 0x7F7F),
		PACK_Y416(0xAAAA, 0x5A5A, 0xA0A0, 0x8585),
		PACK_Y416(0xFFFF, 0x6E6E, 0xA0A0, 0x8C8C),
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
		130, 127,  70, 85,
		130, 133,  90, 170,
		130, 140, 110, 255,

		140, 120,  50, 0,
		140, 127,  70, 85,
		140, 133,  90, 170,
		140, 140, 110, 255,

		150, 120,  50, 0,
		150, 127,  70, 85,
		150, 133,  90, 170,
		150, 140, 110, 255,

		160, 120,  50, 0,
		160, 127,  70, 85,
		160, 133,  90, 170,
		160, 140, 110, 255,
	}, yuyv[] = {
		/* YUYV */
		50, 120,  70, 130,
		89, 140, 110, 130,
		50, 120,  70, 130,
		89, 140, 110, 130,
		50, 121,  70, 161,
		90, 140, 110, 160,
		50, 121,  70, 161,
		90, 140, 110, 160,
	}, uyvy[] = {
		/* UYVY */
		120, 50, 130, 70,
		140, 89, 130, 110,
		120, 50, 130, 70,
		140, 89, 130, 110,
		121, 50, 161, 70,
		140, 90, 160, 110,
		121, 50, 161, 70,
		140, 90, 160, 110,
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
#ifdef HAVE_HTOLE16
	case DRM_FORMAT_P010:
		for (uint32_t i = 0; i < ARRAY_SIZE(p0xx); i++)
			p0xx[i] &= htole16(1023 << 6);
		t = (unsigned char *) p0xx;
		break;
	case DRM_FORMAT_P012:
		for (uint32_t i = 0; i < ARRAY_SIZE(p0xx); i++)
			p0xx[i] &= htole16(4095 << 4);
		t = (unsigned char *) p0xx;
		break;
	case DRM_FORMAT_P016:
		t = (unsigned char *) p0xx;
		break;
#endif
#ifdef HAVE_HTOLE32
	case DRM_FORMAT_Y410:
		t = (unsigned char *) y410;
		break;
#endif
#ifdef HAVE_HTOLE64
	case DRM_FORMAT_Y412:
		for (unsigned i = 0; i < ARRAY_SIZE(y41x); i++)
			y41x[i] &= htole64(0xFFF0FFF0FFF0FFF0llu);

		t = (unsigned char *) y41x;
		break;
	case DRM_FORMAT_Y416:
		t = (unsigned char *) y41x;
		break;
#endif
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
	case DRM_FORMAT_YUYV:
		t = yuyv;
		break;
	case DRM_FORMAT_UYVY:
		t = uyvy;
		break;
	default:
		fprintf(stderr, "invalid fourcc: %.4s\n", (char *)&fourcc);
		return PIGLIT_SKIP;
	}

	/* Modify alpha values of the expected result. */
	if (format_has_alpha(fourcc)) {
		unsigned char *p = expected;
		for (uint32_t i = 0; i < 4 * 4; i++, p += 4)
			p[3] = (i % 4) * 255 / 3;
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
			usage(argv[0], "YUV");
			piglit_report_result(PIGLIT_SKIP);
		}
	}

	if (fourcc == -1) {
		fprintf(stderr, "format not specified\n");
		usage(argv[0], "YUV");
		piglit_report_result(PIGLIT_SKIP);
	}
}
