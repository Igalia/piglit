/*
 * Copyright © 2015 Intel Corporation
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
 * \file
 *
 * Test transcoding NV12 to RGB by importing the Y plane as DRM_FORMAT_R8 and the UV
 * plane as DRM_FORMAT_GR88.
 *
 * The shader implements a simple but fake NV12->RGB conversion equation,
 * because the test's goal is not to test NV12->RGB conversion. Its goal is to
 * test that EGL correctly imports and OpenGL correctly textures from the R8
 * and GR88 DRM formats.
 */

#include "image_common.h"

#define WINDOW_WIDTH 4
#define WINDOW_HEIGHT 4

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_es_version = 30;
	config.window_width = WINDOW_WIDTH;
	config.window_height = WINDOW_HEIGHT;

PIGLIT_GL_TEST_CONFIG_END

/* The kernel header drm_fourcc.h defines the DRM formats below. We duplicate
 * some of the definitions so that building Piglit won't require bleeding-edge
 * kernel headers.
 */
#ifndef DRM_FORMAT_R8
#define DRM_FORMAT_R8 fourcc_code('R', '8', ' ', ' ')
#endif

#ifndef DRM_FORMAT_GR88
#define DRM_FORMAT_GR88 fourcc_code('G', 'R', '8', '8')
#endif

/* Fake data for a 4x4 pixel image in NV12 format. */
static const uint8_t y_data[] = {
	0x00, 0x11, 0x22, 0x33,
	0x44, 0x55, 0x66, 0x77,
	0x88, 0x99, 0xaa, 0xbb,
	0xcc, 0xdd, 0xee, 0xff,
};

static const uint8_t u_data[] = {
	0xc0, 0xc4,
	0xc8, 0xcc,
};

static const uint8_t v_data[] = {
	0xd0, 0xd4,
	0xd8, 0xdc,
};

static GLuint
create_dma_buf_texture(uint32_t width, uint32_t height,
		       uint32_t cpp, uint32_t stride,
		       uint32_t drm_fourcc, const void *pixels)
{
	EGLDisplay dpy = eglGetCurrentDisplay();

	enum piglit_result result = PIGLIT_PASS;

	struct piglit_dma_buf *dma_buf;
	int dma_buf_fd;
	uint32_t dma_buf_offset;
	uint32_t dma_buf_stride;
	EGLImageKHR image;
	EGLint image_attrs[13];
	GLuint tex;
	int i;

	result = piglit_create_dma_buf(width, height, cpp, pixels, stride,
				       &dma_buf, &dma_buf_fd, &dma_buf_stride,
				       &dma_buf_offset);

	if (result != PIGLIT_PASS) {
		piglit_loge("failed to create dma_buf");
		piglit_report_result(result);
	}

	i = 0;
	image_attrs[i++] = EGL_LINUX_DRM_FOURCC_EXT;
	image_attrs[i++] = drm_fourcc;
	image_attrs[i++] = EGL_WIDTH;
	image_attrs[i++] = width;
	image_attrs[i++] = EGL_HEIGHT;
	image_attrs[i++] = height;
	image_attrs[i++] = EGL_DMA_BUF_PLANE0_FD_EXT;
	image_attrs[i++] = dma_buf_fd;
	image_attrs[i++] = EGL_DMA_BUF_PLANE0_PITCH_EXT;
	image_attrs[i++] = dma_buf_stride;
	image_attrs[i++] = EGL_DMA_BUF_PLANE0_OFFSET_EXT;
	image_attrs[i++] = dma_buf_offset;
	image_attrs[i++] = EGL_NONE;


	image = eglCreateImageKHR(dpy, EGL_NO_CONTEXT, EGL_LINUX_DMA_BUF_EXT,
				  (EGLClientBuffer) NULL, image_attrs);
	if (image == EGL_NO_IMAGE_KHR) {
		piglit_loge("failed to create EGLImage from dma_buf");
		piglit_report_result(PIGLIT_FAIL);
	}

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, (GLeglImageOES) image);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	return tex;
}

void
piglit_init(int argc, char **argv)
{
	EGLDisplay dpy = eglGetCurrentDisplay();

	piglit_require_egl_extension(dpy, "EGL_EXT_image_dma_buf_import");
	piglit_require_egl_extension(dpy, "EGL_KHR_image_base");
	piglit_require_extension("GL_OES_EGL_image");
}

static void
create_textures(GLuint *r8_tex, GLuint *gr88_tex, float **ref_rgba_image)
{
	const int width = WINDOW_WIDTH;
	const int height = WINDOW_HEIGHT;

	uint8_t *r8_pixels;
	uint8_t *gr88_pixels;

	int i, x, y;

	r8_pixels = malloc(width * height);
	gr88_pixels = malloc(2 * (width / 2) * (height / 2));
	*ref_rgba_image = malloc(4 * sizeof(float) * width * height);

	if (!r8_pixels || !gr88_pixels || !*ref_rgba_image)
		abort();

	for (i = 0; i < width * height ; ++i) {
		r8_pixels[i] = y_data[i];
	}

	for (i = 0; i < (width / 2) * (height / 2); ++i) {
		gr88_pixels[2*i + 0] = u_data[i];
		gr88_pixels[2*i + 1] = v_data[i];
	}

	for (y = 0; y < height; ++y) {
		for (x = 0; x < width; ++x) {
			float *rgba = &(*ref_rgba_image)[4 * (y * width + x)];

			/* This must match the fake NV12->RGB conversion in the
			 * fragment shader.
			 */
			rgba[0] = y_data[y * width + x] / 255.0f;
			rgba[1] = u_data[(y / 2) * (width / 2) + (x / 2)] / 255.0f;
			rgba[2] = v_data[(y / 2) * (width / 2) + (x / 2)] / 255.0f;
			rgba[3] = 1.0f;
		}
	}

	glActiveTexture(GL_TEXTURE0);
	*r8_tex = create_dma_buf_texture(width, height,
					 /*cpp*/ 1, /*stride*/ width,
					 DRM_FORMAT_R8, r8_pixels);

	glActiveTexture(GL_TEXTURE1);
	*gr88_tex = create_dma_buf_texture(width / 2, height / 2,
					   /*cpp*/ 2, /*stride*/ width,
					   DRM_FORMAT_GR88, gr88_pixels);
}

enum piglit_result
piglit_display(void)
{
	GLuint r8_tex, gr88_tex;
	float *ref_rgba_image;

	GLuint va; /* vertex array */
	GLuint vb; /* vertex buffer */
	GLuint prog;

	static const float vb_data[] = {
		-1, -1,
		 1, -1,
		 1,  1,
		-1,  1,
	};

	if (piglit_width != WINDOW_WIDTH ||
	    piglit_height != WINDOW_HEIGHT) {
		piglit_loge("window is not %dx%d",
			    WINDOW_WIDTH, WINDOW_HEIGHT);
		return PIGLIT_FAIL;
	}

	create_textures(&r8_tex, &gr88_tex, &ref_rgba_image);

	prog = piglit_build_simple_program(
		"#version 300 es\n"
		"\n"
		"in vec2 a_position;\n"
		"out vec2 v_texcoord;\n"
		"\n"
		"void main()\n"
		"{\n"
		"	gl_Position = vec4(a_position, 0, 1);\n"
		"\n"
		"	v_texcoord = a_position;\n"
		"	v_texcoord += vec2(1, 1);\n"
		"	v_texcoord /= vec2(2, 2);\n"
		"}\n",

		"#version 300 es\n"
		"\n"
		"precision highp float;\n"
		"uniform sampler2D u_r8_tex;\n"
		"uniform sampler2D u_gr88_tex;\n"
		"in vec2 v_texcoord;\n"
		"out vec4 f_color;\n"
		"\n"
		"void main()\n"
		"{\n"
		"	float y = texture(u_r8_tex, v_texcoord).r;\n"
		"	vec2 uv = texture(u_gr88_tex, v_texcoord).rg;\n"
		"\n"
		"	/* A very fake NV12->RGB conversion */\n"
		"	f_color = vec4(y, uv.r, uv.g, 1);\n"
		"}\n");

	glUseProgram(prog);

	glUniform1i(glGetUniformLocation(prog, "u_r8_tex"), 0);
	glUniform1i(glGetUniformLocation(prog, "u_gr88_tex"), 1);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	glGenBuffers(1, &vb);
	glBindBuffer(GL_ARRAY_BUFFER, vb);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vb_data), vb_data,
		     GL_STATIC_DRAW);

	glGenVertexArrays(1, &va);
	glBindVertexArray(va);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void *) 0);
	glEnableVertexAttribArray(0);

	glViewport(0, 0, piglit_width, piglit_height);
	glClearColor(0.2, 0.2, 0.2, 0.2);
	glClear(GL_COLOR_BUFFER_BIT);
	glDrawArrays(GL_TRIANGLE_FAN, /*first*/ 0, /*count*/ 4);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	/* Increase the tolerance because the conversion path
	 *     ubyte --(texturing)--> float --(glReadPixels)--> ubyte
	 * is lossy.
	 */
	piglit_tolerance[0] = 0.05;
	piglit_tolerance[1] = 0.05;
	piglit_tolerance[2] = 0.05;
	if (!piglit_probe_image_rgba(0, 0, piglit_width, piglit_height,
				     ref_rgba_image)) {
		return PIGLIT_FAIL;
	}

	return PIGLIT_PASS;
}
