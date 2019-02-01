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

#include "piglit-framework-gl/piglit_drm_dma_buf.h"

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

enum piglit_result
texture_for_egl_image(EGLImageKHR img, GLuint *out_tex)
{
	GLuint tex;
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

	*out_tex = tex;

	return PIGLIT_PASS;
}

void
sample_tex(GLuint tex, unsigned x, unsigned y, unsigned w, unsigned h)
{
	GLuint prog;

	prog = piglit_build_simple_program(vs_src, fs_src);
	
	glUseProgram(prog);

	glBindTexture(GL_TEXTURE_EXTERNAL_OES, tex);
	glUniform1i(glGetUniformLocation(prog, "sampler"), 0);

	glViewport(x, y, w, h);
	piglit_draw_rect_tex(-1, -1, 2, 2,
			     0, 0, 1, 1);

	glDeleteProgram(prog);
	glUseProgram(0);
}

enum piglit_result
egl_image_for_dma_buf_fd(struct piglit_dma_buf *buf, int fd, int fourcc, EGLImageKHR *out_img)
{
	EGLint error;
	EGLImageKHR img;
	EGLint attr_packed[] = {
		EGL_WIDTH, buf->w,
		EGL_HEIGHT, buf->h,
		EGL_LINUX_DRM_FOURCC_EXT, fourcc,
		EGL_DMA_BUF_PLANE0_FD_EXT, fd,
		EGL_DMA_BUF_PLANE0_OFFSET_EXT, buf->offset[0],
		EGL_DMA_BUF_PLANE0_PITCH_EXT, buf->stride[0],
		EGL_NONE
	};

	EGLint attr_nv12[] = {
		EGL_WIDTH, buf->w,
		EGL_HEIGHT, buf->h,
		EGL_LINUX_DRM_FOURCC_EXT, fourcc,
		EGL_DMA_BUF_PLANE0_FD_EXT, fd,
		EGL_DMA_BUF_PLANE0_OFFSET_EXT, buf->offset[0],
		EGL_DMA_BUF_PLANE0_PITCH_EXT, buf->stride[0],
		EGL_DMA_BUF_PLANE1_FD_EXT, fd,
		EGL_DMA_BUF_PLANE1_OFFSET_EXT, buf->offset[1],
		EGL_DMA_BUF_PLANE1_PITCH_EXT, buf->stride[1],
		EGL_NONE
	};

	EGLint attr_yuv420[] = {
		EGL_WIDTH, buf->w,
		EGL_HEIGHT, buf->h,
		EGL_LINUX_DRM_FOURCC_EXT, fourcc,
		EGL_DMA_BUF_PLANE0_FD_EXT, fd,
		EGL_DMA_BUF_PLANE0_OFFSET_EXT, buf->offset[0],
		EGL_DMA_BUF_PLANE0_PITCH_EXT, buf->stride[0],
		EGL_DMA_BUF_PLANE1_FD_EXT, fd,
		EGL_DMA_BUF_PLANE1_OFFSET_EXT, buf->offset[1],
		EGL_DMA_BUF_PLANE1_PITCH_EXT, buf->stride[1],
		EGL_DMA_BUF_PLANE2_FD_EXT, fd,
		EGL_DMA_BUF_PLANE2_OFFSET_EXT, buf->offset[2],
		EGL_DMA_BUF_PLANE2_PITCH_EXT, buf->stride[2],
		EGL_NONE
	};

	EGLint *attr;
	switch (fourcc) {
	case DRM_FORMAT_NV12:
	case DRM_FORMAT_P010:
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
sample_buffer(struct piglit_dma_buf *buf, int fourcc)
{
	enum piglit_result res;
	EGLImageKHR img;
	GLuint tex;
	int w = buf->w;
	int h = buf->h;

	res = egl_image_for_dma_buf_fd(buf, buf->fd, fourcc, &img);

	/* Release the creator side of the buffer. */
	piglit_destroy_dma_buf(buf);

	if (!img) {
		/* Close the descriptor also, EGL does not have ownership */
		close(buf->fd);
	}

	if (res != PIGLIT_PASS)
		return res;

	res = texture_for_egl_image(img, &tex);
	if (res != PIGLIT_PASS)
		goto destroy;

	sample_tex(tex, 0, 0, w, h);

destroy:
	glDeleteTextures(1, &tex);
	eglDestroyImageKHR(eglGetCurrentDisplay(), img);

	return res;
}

enum piglit_result
dma_buf_create_and_sample_32bpp(unsigned w, unsigned h,
				int fourcc, const unsigned char *src)
{
	struct piglit_dma_buf *buf;
	enum piglit_result res;

	res = piglit_create_dma_buf(w, h, fourcc, src, &buf);
	if (res != PIGLIT_PASS)
		return res;

	return sample_buffer(buf, fourcc);
}
