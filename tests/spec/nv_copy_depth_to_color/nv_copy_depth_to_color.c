/*
 * Copyright © 2016 Intel Corporation
 * Copyright © 2020 Advanced Micro Devices, Inc.
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

/**
 * @file nv_copy_depth_to_color.c
 *
 * This file is inspired by the following file: -
 * tests/general/copy-pixels.c
 *
 * Test to verify glCopyPixels with GL_DEPTH_STENCIL_TO_RGBA_NV
 * and GL_DEPTH_STENCIL_TO_BGRA_NV
 */

#include "piglit-util-gl.h"

#define IMAGE_WIDTH 60
#define IMAGE_HEIGHT 60

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_STENCIL | PIGLIT_GL_VISUAL_DEPTH;

PIGLIT_GL_TEST_CONFIG_END

#define DEFAULT_DEPTH_STENCIL_VAL 0x22334455;
static GLint rgba;
static GLuint depth_stencil_val = DEFAULT_DEPTH_STENCIL_VAL;

bool
test_depthstencil_to_color_copypix(int x, int y)
{
	int i;
	bool pass = true;
	uint32_t *depth_stencil_buf = malloc(IMAGE_WIDTH * IMAGE_HEIGHT * sizeof(uint32_t));

	GLuint big_depth_stencil_val = htobe32(depth_stencil_val);
	GLubyte *data_ptr = (GLubyte*)&big_depth_stencil_val;
	GLubyte red = *data_ptr++;
	GLubyte green = *data_ptr++;
	GLubyte blue = *data_ptr++;
	GLubyte stencil = *data_ptr;

	GLubyte *expected = malloc(IMAGE_WIDTH * IMAGE_HEIGHT * 4 * sizeof(GLubyte));
	GLubyte *ptr_to_expected = (GLubyte*)expected;

	if(rgba) {
		for(i=0; i<IMAGE_WIDTH*IMAGE_HEIGHT; i++){
			*ptr_to_expected++ = red;
			*ptr_to_expected++ = green;
			*ptr_to_expected++ = blue;
			*ptr_to_expected++ = stencil;
		}
	} else {
		for(i=0; i<IMAGE_WIDTH*IMAGE_HEIGHT; i++){
			*ptr_to_expected++ = blue;
			*ptr_to_expected++ = green;
			*ptr_to_expected++ = red;
			*ptr_to_expected++ = stencil;
		}
	}

	/* Initialize depth stencil data */
	for (i = 0; i < IMAGE_WIDTH * IMAGE_HEIGHT; i++)
		depth_stencil_buf[i] = depth_stencil_val;

	glRasterPos2i(0, 0);

	glDrawPixels(IMAGE_WIDTH, IMAGE_HEIGHT,
		     GL_DEPTH_STENCIL_EXT, GL_UNSIGNED_INT_24_8_EXT, depth_stencil_buf);

	glRasterPos2i(x, y);

	if(rgba)
		glCopyPixels(0, 0, IMAGE_WIDTH, IMAGE_HEIGHT, GL_DEPTH_STENCIL_TO_RGBA_NV);
	else
		glCopyPixels(0, 0, IMAGE_WIDTH, IMAGE_HEIGHT, GL_DEPTH_STENCIL_TO_BGRA_NV);

	GLubyte *pixels_read = malloc(IMAGE_WIDTH * IMAGE_HEIGHT * 4 * sizeof(GLubyte));
	glReadPixels(x, y, IMAGE_WIDTH, IMAGE_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, pixels_read);

	pass = piglit_compare_images_ubyte(x, y, IMAGE_WIDTH, IMAGE_HEIGHT,
					expected, pixels_read);

	free(depth_stencil_buf);
	free(expected);
	free(pixels_read);
	return pass;
}

enum piglit_result
piglit_display(void)
{
	bool pass = true;

	glClear(GL_COLOR_BUFFER_BIT);
	glClear(GL_DEPTH_BUFFER_BIT);
	glClear(GL_STENCIL_BUFFER_BIT);

	pass = test_depthstencil_to_color_copypix(IMAGE_WIDTH/4,IMAGE_HEIGHT/4)
	&& pass;
	pass = test_depthstencil_to_color_copypix(IMAGE_WIDTH/2,IMAGE_HEIGHT/2)
	&& pass;
	pass = test_depthstencil_to_color_copypix(IMAGE_WIDTH,IMAGE_HEIGHT)
	&& pass;

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
print_usage_and_exit(char *prog_name)
{
	printf("Usage: %s rgba(1|0) <depthstencilval>\n", prog_name);
	printf("Example: %s 1 0x12345678\n", prog_name);
	piglit_report_result(PIGLIT_FAIL);
}

void
piglit_init(int argc, char **argv)
{
	char *endptr = NULL;

	piglit_require_extension("GL_NV_packed_depth_stencil");
	piglit_require_extension("GL_NV_copy_depth_to_color");

	if (argc == 1) {
		rgba = 1;
	} else if (argc == 2) {
		rgba = strtol(argv[1], &endptr, 0);
		if (endptr != argv[1] + strlen(argv[1]))
			print_usage_and_exit(argv[0]);
	} else if (argc == 3) {
		rgba = strtol(argv[1], &endptr, 0);
		if (endptr != argv[1] + strlen(argv[1]))
			print_usage_and_exit(argv[0]);
		depth_stencil_val = strtol(argv[2], &endptr, 0);
		if (endptr != argv[2] + strlen(argv[2]))
			print_usage_and_exit(argv[0]);
	} else {
		print_usage_and_exit(argv[0]);
	}

	glClearColor(0.25, 0.25, 0.25, 1.0);
	glClearDepth(0.0);
	glClearStencil(0.0);

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);
}
