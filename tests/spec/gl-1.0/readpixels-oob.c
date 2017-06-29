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
 *
 */

/** @file readpixels-oob.c
 *
 * Test that requesting an area larger than the readbuffer (with
 * glReadPixels) will only modify the valid area in the user's buffer.
 * This is equivalent to a user requesting to read into a sub-rectangle
 * of the larger rectangle contained in the provided buffer (via
 * PACK_ROW_LENGTH, PACK_SKIP_ROWS, PACK_SKIP_PIXELS).
 *
 * This behaviour ensures that Mesa does as little work as possible in
 * this imprecise usage of glReadpixels. It also reduces the bugs that
 * may occur with this special case by converting it to an analogous
 * common case.
 *
 */

#include "piglit-util-gl.h"

#define BIG_MULT 3

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

static bool
test_with_offsets(GLint x_offset, GLint y_offset)
{
	bool pass = true;
	int x, y, i;

	/* Allocate into an oversized buffer. We'll check that the contents
	 * are still 0 after the glReadPixels.
	 */
	const size_t num_chan = 4;
	const size_t big_buf_h = piglit_height * BIG_MULT;
	const size_t big_buf_w = piglit_width * BIG_MULT;
	const size_t tot_elements = big_buf_h * big_buf_w * num_chan;
	const size_t fb_w = piglit_width - MAX2(0, x_offset);
	const size_t fb_h = piglit_height - MAX2(0, y_offset);
	const unsigned dst_x = abs(MIN2(0, x_offset));
	const unsigned dst_y = abs(MIN2(0, y_offset));
	GLubyte * black_img = (GLubyte*)calloc(tot_elements, sizeof(GLubyte));
	GLfloat * black_imgf = (GLfloat*)malloc(tot_elements *sizeof(GLfloat));

	/* Clear background to purple */
	glClearColor(1.0, 0.0, 1.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);

	/* Perform over-sized glReadPixels. Read the readbuffer as
	 * GLubytes in order to hit most HW fast-paths.
	 */
	glReadPixels(x_offset, y_offset, big_buf_w, big_buf_h,
		     GL_RGBA, GL_UNSIGNED_BYTE, black_img);

	/* Convert values to float in order to use utility comparison function.
	 */
	for (i = 0; i < tot_elements; i++)
		black_imgf[i] = black_img[i] / 255.0;

	/* Confirm the result */
	for (y = 0; y < big_buf_h; y++) {
		for (x = 0; x < big_buf_w; x++) {
			static const GLfloat valid_pixel[4] = {1.0, 0, 1.0, 0};
			static const GLfloat invalid_pixel[4] = {0};
			const unsigned index = y * big_buf_w * num_chan +
						x * num_chan;
			const GLfloat * expected_pixel =
					((x >= dst_x  &&
					  x < fb_w + dst_x) &&
					 (y >= dst_y &&
					  y < fb_h + dst_y)) ?
						valid_pixel : invalid_pixel;

			pass = piglit_compare_pixels(x, y, expected_pixel,
						      black_imgf + index,
						      piglit_tolerance,
						      num_chan);
			if (!pass) {
				printf("Tested with offsets, x: %d\ty: %d\n",
				       x_offset, y_offset);
				goto end;
			}
		}
	}
	
end:
	free(black_img);
	free(black_imgf);

	return pass;
}

enum piglit_result
piglit_display(void)
{
	bool pass = test_with_offsets(0, 0);
	pass &= test_with_offsets(-piglit_width, 0);
	pass &= test_with_offsets(0, -piglit_height);
	pass &= test_with_offsets(-piglit_width, -piglit_height);
	pass &= test_with_offsets(piglit_width/2, piglit_height/2);
	piglit_present_results();
	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void piglit_init(int argc, char **argv)
{
}
