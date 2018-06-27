/*
 * Copyright © 2013 Intel Corporation
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


/** @file readpixels.c
 *
 * Simple touch test of glReadPixels() using GL_PACK_INVERT_MESA, to a
 * PBO or user memory, with format conversions or (hopefully) not.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

/* Size of the RGBW rect on the screen: at 6 pixels, the unorm failure
 * result fits in an 80-column terminal.
 */
#define W 6
#define H 6

static bool
check_unorm(const uint8_t *data, const char *name)
{
	static const uint8_t colors[4][4] = {
		{0xff, 0x00, 0x00, 0x00},
		{0xff, 0xff, 0xff, 0xff},
		{0x00, 0x00, 0xff, 0x00},
		{0x00, 0xff, 0x00, 0x00}
	};
	int x, y;

	for (y = 0; y < H; y++) {
		for (x = 0; x < W; x++) {
			int i = (y >= H / 2) * 2 + (x >= W / 2);
			const uint8_t *expected = &colors[i][0];
			const uint8_t *observed = &data[(y * W + x) * 4];

			if (expected[0] != observed[0] ||
			    expected[1] != observed[1] ||
			    expected[2] != observed[2] ||
			    expected[3] != observed[3]) {
				fprintf(stderr,
					"%s pixel value read at (%d, %d)\n", name, x, y);
				fprintf(stderr,
					"  Expected: b = 0x%02x  g = 0x%02x  r = 0x%02x  a = 0x%02x\n",
						expected[0], expected[1], expected[2], expected[3]);
				fprintf(stderr,
					"  Observed: b = 0x%02x  g = 0x%02x  r = 0x%02x  a = 0x%02x\n",
						observed[0], observed[1], observed[2], observed[3]);

				for (y = 0; y < H; y++) {
					for (x = 0; x < W; x++) {
						observed = &data[(y * W + x) * 4];
						fprintf(stderr,
								"b = 0x%02x  g = 0x%02x  r = 0x%02x  a = 0x%02x\n",
								observed[0], observed[1], observed[2], observed[3]);
					}
				}

				piglit_report_subtest_result(PIGLIT_FAIL,
							     "%s", name);
				return false;
			}
		}
	}

	piglit_report_subtest_result(PIGLIT_PASS, "%s", name);
	return true;
}

static bool
check_float(const float *data, const char *name)
{
	static const float colors[4][4] = {
		{0, 0, 1, 0},
		{1, 1, 1, 1},
		{1, 0, 0, 0},
		{0, 1, 0, 0},
	};
	int x, y;

	for (y = 0; y < H; y++) {
		for (x = 0; x < W; x++) {
			int i = (y >= H / 2) * 2 + (x >= W / 2);
			const float *expected = colors[i];
			const float *result = &data[(y * W + x) * 4];

			if (memcmp(result, expected, sizeof(float[4])) != 0) {
				fprintf(stderr,
					"%s pixel value read at (%d, %d):\n"
					"    was      %f, %f, %f, %f\n"
					"    expected %f, %f, %f, %f\n",
					name, x, y,
					result[0],
					result[1],
					result[2],
					result[3],
					expected[0],
					expected[1],
					expected[2],
					expected[3]);
				fprintf(stderr, "\n");

				piglit_report_subtest_result(PIGLIT_FAIL,
							     "%s", name);
				return false;
			}
		}
	}

	piglit_report_subtest_result(PIGLIT_PASS, "%s", name);
	return true;
}

enum piglit_result
piglit_display(void)
{
	bool pass = true;
	GLuint pbo;
	uint8_t bgra_unorm[W * H * 4];
	float rgba_float[W * H * 4];
	void *map;

	glGenBuffers(1, &pbo);
	glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo);
	glBufferData(GL_PIXEL_PACK_BUFFER, W * H * sizeof(float[4]),
		     NULL, GL_STREAM_READ_ARB);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);

	glClearColor(0.5, 0.5, 0.5, 0.5);
	glClear(GL_COLOR_BUFFER_BIT);

	piglit_ortho_projection(piglit_width, piglit_height, false);

	glColor4f(1, 0, 0, 0);
	piglit_draw_rect(5, 5,
			 W / 2, H / 2);
	glColor4f(0, 1, 0, 0);
	piglit_draw_rect(5 + W / 2, 5,
			 W / 2, H / 2);
	glColor4f(0, 0, 1, 0);
	piglit_draw_rect(5, 5 + H / 2,
			 W / 2, H / 2);
	glColor4f(1, 1, 1, 1);
	piglit_draw_rect(5 + W / 2, 5 + H / 2,
			 W / 2, H / 2);

	glPixelStorei(GL_PACK_INVERT_MESA, 1);

	glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
	glReadPixels(5, 5, W, H, GL_BGRA, GL_UNSIGNED_BYTE, bgra_unorm);
	if (!check_unorm(bgra_unorm, "non-PBO unorm BGRA"))
		pass = false;

	glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo);
	glReadPixels(5, 5, W, H, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
	map = glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
	if (!check_unorm(map, "PBO unorm BGRA"))
		pass = false;
	glUnmapBuffer(GL_PIXEL_PACK_BUFFER);

	glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
	glReadPixels(5, 5, W, H, GL_RGBA, GL_FLOAT, rgba_float);
	if (!check_float(rgba_float, "non-PBO float RGBA"))
		pass = false;

	glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo);
	glReadPixels(5, 5, W, H, GL_RGBA, GL_FLOAT, NULL);
	map = glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
	if (!check_float(map, "PBO float RGBA"))
		pass = false;
	glUnmapBuffer(GL_PIXEL_PACK_BUFFER);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_ARB_pixel_buffer_object");
	piglit_require_extension("GL_MESA_pack_invert");
	piglit_require_extension("GL_EXT_bgra");

	const char * names[] = {
		"non-PBO unorm BGRA",
		"PBO unorm BGRA",
		"non-PBO float RGBA",
		"PBO float RGBA",
		NULL,
	};
	piglit_register_subtests(names);
}
