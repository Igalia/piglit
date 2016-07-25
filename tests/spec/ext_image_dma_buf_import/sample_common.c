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

#include <unistd.h>

#include "image_common.h"
#include "sample_common.h"

static const char fs_src[] =
	"#extension GL_OES_EGL_image_external : require\n"
	"precision mediump float;\n"
	"uniform samplerExternalOES sampler;\n"
	"varying vec2 texcoords;\n"
	"\n"
	"void main()\n"
	"{\n"
	"gl_FragColor = texture2D(sampler, texcoords);\n"
	"}\n";
static const char vs_src[] =
	"attribute vec4 piglit_vertex;\n"
	"attribute vec4 piglit_texcoords;\n"
	"varying vec2 texcoords;\n"
	"\n"
	"void main()\n"
	"{\n"
	"	texcoords = piglit_texcoords.xy;\n"
	"	gl_Position = piglit_vertex;\n"
	"}\n";

static enum piglit_result
sample_and_destroy_img(unsigned w, unsigned h, EGLImageKHR img)
{
	GLuint prog, tex;
	GLenum error;

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_EXTERNAL_OES, tex);

	/* Set the image as level zero */
	glEGLImageTargetTexture2DOES(GL_TEXTURE_EXTERNAL_OES,
			(GLeglImageOES)img);
	error = glGetError();

	/**
	 * EGL may not support binding of external textures, this is not an
	 * error.
	 */
	if (error == GL_INVALID_OPERATION)
		return PIGLIT_SKIP;

	if (error != GL_NO_ERROR) {
		printf("glEGLImageTargetTexture2DOES() failed: %s 0x%x\n",
			piglit_get_gl_error_name(error), error);
		return PIGLIT_FAIL;
	}

	glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER,
			GL_NEAREST);
	glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER,
			GL_NEAREST);

	prog = piglit_build_simple_program(vs_src, fs_src);
	
	glUseProgram(prog);

	glUniform1i(glGetUniformLocation(prog, "sampler"), 0);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glViewport(0, 0, w, h);
	piglit_draw_rect_tex(-1, -1, 2, 2,
			     0, 0, 1, 1);

	glDeleteProgram(prog);
	glUseProgram(0);

	glDeleteTextures(1, &tex);
	eglDestroyImageKHR(eglGetCurrentDisplay(), img);

	return PIGLIT_PASS;
}

enum piglit_result
egl_image_for_dma_buf_fd(int fd, int fourcc, int w, int h,
			 unsigned stride, unsigned offset, EGLImageKHR *out_img)
{
	EGLint error;
	EGLImageKHR img;
	EGLint attr_packed[] = {
		EGL_WIDTH, w,
		EGL_HEIGHT, h,
		EGL_LINUX_DRM_FOURCC_EXT, fourcc,
		EGL_DMA_BUF_PLANE0_FD_EXT, fd,
		EGL_DMA_BUF_PLANE0_OFFSET_EXT, offset,
		EGL_DMA_BUF_PLANE0_PITCH_EXT, stride,
		EGL_NONE
	};

	EGLint attr_nv12[] = {
		EGL_WIDTH, w,
		EGL_HEIGHT, h,
		EGL_LINUX_DRM_FOURCC_EXT, fourcc,
		EGL_DMA_BUF_PLANE0_FD_EXT, fd,
		EGL_DMA_BUF_PLANE0_OFFSET_EXT, offset,
		EGL_DMA_BUF_PLANE0_PITCH_EXT, stride,
		EGL_DMA_BUF_PLANE1_FD_EXT, fd,
		EGL_DMA_BUF_PLANE1_OFFSET_EXT, offset + h * stride,
		EGL_DMA_BUF_PLANE1_PITCH_EXT, stride,
		EGL_NONE
	};

	EGLint attr_yuv420[] = {
		EGL_WIDTH, w,
		EGL_HEIGHT, h,
		EGL_LINUX_DRM_FOURCC_EXT, fourcc,
		EGL_DMA_BUF_PLANE0_FD_EXT, fd,
		EGL_DMA_BUF_PLANE0_OFFSET_EXT, offset,
		EGL_DMA_BUF_PLANE0_PITCH_EXT, stride,
		EGL_DMA_BUF_PLANE1_FD_EXT, fd,
		EGL_DMA_BUF_PLANE1_OFFSET_EXT, offset + h * stride,
		EGL_DMA_BUF_PLANE1_PITCH_EXT, stride,
		EGL_DMA_BUF_PLANE2_FD_EXT, fd,
		EGL_DMA_BUF_PLANE2_OFFSET_EXT, offset + h * stride + w / 2,
		EGL_DMA_BUF_PLANE2_PITCH_EXT, stride,
		EGL_NONE
	};

	EGLint *attr;
	switch (fourcc) {
	case DRM_FORMAT_NV12:
		attr = attr_nv12;
		break;
	case DRM_FORMAT_YUV420:
	case DRM_FORMAT_YVU420:
		attr = attr_yuv420;
		break;
	default:
		attr = attr_packed;
		break;
	}

	img = eglCreateImageKHR(eglGetCurrentDisplay(), EGL_NO_CONTEXT,
				 EGL_LINUX_DMA_BUF_EXT, (EGLClientBuffer)0,
				 attr);
	*out_img = img;

	error = eglGetError();

	/* EGL may not support the format, this is not an error. */
	if (!img && error == EGL_BAD_MATCH)
		return PIGLIT_SKIP;

	if (error != EGL_SUCCESS) {
		printf("eglCreateImageKHR() failed: %s 0x%x\n",
			piglit_get_egl_error_name(error), error);

		return PIGLIT_FAIL;
	}

	if (!img) {
		fprintf(stderr, "image creation succeeded but returned NULL\n");
		return PIGLIT_FAIL;
	}

	*out_img = img;
	return PIGLIT_PASS;
}

static enum piglit_result
sample_buffer(void *buf, int fd, int fourcc, unsigned w, unsigned h,
	      unsigned stride, unsigned offset)
{
	enum piglit_result res;
	EGLImageKHR img;

	res = egl_image_for_dma_buf_fd(fd, fourcc, w, h, stride, offset, &img);

	/* Release the creator side of the buffer. */
	piglit_destroy_dma_buf(buf);

	if (!img) {
		/* Close the descriptor also, EGL does not have ownership */
		close(fd);
	}

	if (res != PIGLIT_PASS)
		return res;

	return sample_and_destroy_img(w, h, img);
}

enum piglit_result
dma_buf_create_and_sample_32bpp(unsigned w, unsigned h, unsigned cpp,
				int fourcc, const unsigned char *src)
{
	struct piglit_dma_buf *buf;
	unsigned stride, offset;
	int fd;
	enum piglit_result res;

	unsigned buffer_height;

	switch (fourcc) {
	case DRM_FORMAT_NV12:
	case DRM_FORMAT_YUV420:
	case DRM_FORMAT_YVU420:
		buffer_height = h * 3 / 2;
		break;
	default:
		buffer_height = h;
		break;
	}

	res = piglit_create_dma_buf(w, buffer_height,
				    cpp, src, w * cpp, &buf, &fd, &stride,
				    &offset);
	if (res != PIGLIT_PASS)
		return res;

	return sample_buffer(buf, fd, fourcc, w, h, stride, offset);
}
