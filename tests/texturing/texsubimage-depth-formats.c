/*
 * Copyright © 2014 Intel Corporation
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/** \file texsubimage-depth-formats.c
 *
 * Test glTexSubimage2D() with different depth formats and X, Y offsets.
 **/

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static GLuint tex[4];
struct size {
	int width, height;
};

/* For ease of testing, use even dimensions. */
static const struct size tex_size[] = {
	{   6, 	12 },
	{   8, 	14 },
	{  12, 	22 },
	{  16,	32 },
	{ 130, 	64 }
};

struct format_info {
	GLenum internal_format, format, type;
	int size;
};
static const struct format_info formats[] = {
	{ GL_DEPTH_COMPONENT16,  GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT,                 sizeof(short) },
	{ GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT,                          sizeof(float) },
	{ GL_DEPTH24_STENCIL8,   GL_DEPTH_STENCIL,   GL_UNSIGNED_INT_24_8,              sizeof(int) },
	{ GL_DEPTH32F_STENCIL8,  GL_DEPTH_STENCIL,   GL_FLOAT_32_UNSIGNED_INT_24_8_REV, sizeof(int) + sizeof(float) }
};

static void
load_texture(int formats_idx, int tex_size_idx)
{
	unsigned i;
	unsigned w_by_2 = tex_size[tex_size_idx].width / 2;
	unsigned h_by_2 =  tex_size[tex_size_idx].height / 2;
	unsigned n_pixels = w_by_2 * h_by_2;
	unsigned buffer_size = n_pixels * formats[formats_idx].size;
	void* texDepthData[4];

	for (i = 0; i < 4; i++)
		texDepthData[i] = malloc(buffer_size);

	for (i = 0; i < n_pixels; i++) {
		switch (formats[formats_idx].type) {
		case GL_UNSIGNED_SHORT:
			((unsigned short *)texDepthData[0])[i] = 0x4000;
			((unsigned short *)texDepthData[1])[i] = 0x7F00;
			((unsigned short *)texDepthData[2])[i] = 0xC000;
			((unsigned short *)texDepthData[3])[i] = 0xFF00;
			break;
		case GL_UNSIGNED_INT_24_8:
			((unsigned *)texDepthData[0])[i] = 0x400000BB;
			((unsigned *)texDepthData[1])[i] = 0x7F0000BB;
			((unsigned *)texDepthData[2])[i] = 0xC00000BB;
			((unsigned *)texDepthData[3])[i] = 0xFF0000BB;
			break;
		case GL_FLOAT:
			((float *)texDepthData[0])[i] = 0.25;
			((float *)texDepthData[1])[i] = 0.50;
			((float *)texDepthData[2])[i] = 0.75;
			((float *)texDepthData[3])[i] = 1.00;
			break;
		case GL_FLOAT_32_UNSIGNED_INT_24_8_REV:
			((float *)texDepthData[0])[2 * i] = 0.25;
			((float *)texDepthData[1])[2 * i] = 0.50;
			((float *)texDepthData[2])[2 * i] = 0.75;
			((float *)texDepthData[3])[2 * i] = 1.00;

			((unsigned *)texDepthData[0])[2 * i + 1] = 0xBB;
			((unsigned *)texDepthData[1])[2 * i + 1] = 0xBB;
			((unsigned *)texDepthData[2])[2 * i + 1] = 0xBB;
			((unsigned *)texDepthData[3])[2 * i + 1] = 0xBB;
			break;
		}
	}

	glBindTexture(GL_TEXTURE_2D, tex[formats_idx]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0,
		     formats[formats_idx].internal_format,
		     tex_size[tex_size_idx].width,
		     tex_size[tex_size_idx].height, 0,
		     formats[formats_idx].format,
		     formats[formats_idx].type, NULL);

	/* Use glTexSubImage2D() to initialize each texture quadrant with
	 * different depth data.
	 */
	if (formats[formats_idx].type == GL_UNSIGNED_SHORT)
		glPixelStorei(GL_UNPACK_ALIGNMENT, 2);
	else
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w_by_2, h_by_2,
			formats[formats_idx].format, formats[formats_idx].type,
			texDepthData[0]);
	glTexSubImage2D(GL_TEXTURE_2D, 0, w_by_2, 0, w_by_2, h_by_2,
			formats[formats_idx].format, formats[formats_idx].type,
			texDepthData[1]);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, h_by_2, w_by_2, h_by_2,
			formats[formats_idx].format, formats[formats_idx].type,
			texDepthData[2]);
	glTexSubImage2D(GL_TEXTURE_2D, 0, w_by_2, h_by_2, w_by_2, h_by_2,
			formats[formats_idx].format, formats[formats_idx].type,
			texDepthData[3]);

	for (i = 0; i < 4; i++)
		free(texDepthData[i]);
}

void
piglit_init(int argc, char **argv)
{
	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);
	glClearColor(0.0, 0.0, 0.0, 1.0);
}

enum piglit_result
piglit_display(void)
{
	int i, j;
	bool result, pass = true;
	const int w = piglit_width / 2, h = piglit_height / 2;
	const float expected[4][4] = {{ 0.25, 0.25, 0.25, 1.00 },
				      { 0.50, 0.50, 0.50, 1.00 },
				      { 0.75, 0.75, 0.75, 1.00 },
				      { 1.00, 1.00, 1.00, 1.00 }};
	glClear(GL_COLOR_BUFFER_BIT);
	glEnable(GL_TEXTURE_2D);
	glGenTextures(ARRAY_SIZE(formats), tex);

	for (i = 0; i < ARRAY_SIZE(formats); i++) {
		for (j = 0; j < ARRAY_SIZE(tex_size); j++) {
			result = true;
			load_texture(i, j);

			glBindTexture(GL_TEXTURE_2D, tex[i]);
			piglit_draw_rect_tex(0, 0, piglit_width, piglit_height,
					     0, 0, 1, 1);
			result = result && piglit_check_gl_error(GL_NO_ERROR);

			/* Probe four quadrants of the rectangle */
			result = piglit_probe_rect_rgba(0, 0, w, h, expected[0])
				 && result;
			result = piglit_probe_rect_rgba(w, 0, w, h, expected[1])
				 && result;
			result = piglit_probe_rect_rgba(0, h, w, h, expected[2])
				 && result;
			result = piglit_probe_rect_rgba(w, h, w, h, expected[3])
				 && result;
			pass = pass && result;

			printf("internal_format = %s, width = %d, height = %d,"
			       " result = %s\n",
			       piglit_get_gl_enum_name(formats[i].internal_format),
			       tex_size[j].width, tex_size[j].height,
			       result ? "pass" : "fail");
		}
	}
	piglit_present_results();
	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
