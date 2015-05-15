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
	"attribute vec4 position;\n"
	"varying vec2 texcoords;\n"
	"\n"
	"void main()\n"
	"{\n"
	"texcoords = 0.5 * (position.xy + vec2(1.0, 1.0));\n"
	"gl_Position = position;\n"
	"}\n";

static void
set_vertices(GLuint prog)
{
	static const GLfloat v[] = { -1.0f,  1.0f, -1.0f, -1.0f,
				      1.0f, -1.0f,  1.0f,  1.0f };

	GLint i = glGetAttribLocation(prog, "position");
	glVertexAttribPointer(i, 2, GL_FLOAT, GL_FALSE, 0, v);
	glEnableVertexAttribArray(i);
}

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
	set_vertices(prog);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glViewport(0, 0, w, h);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	glDeleteProgram(prog);
	glUseProgram(0);

	glDeleteTextures(1, &tex);
	eglDestroyImageKHR(eglGetCurrentDisplay(), img);

	return PIGLIT_PASS;
}

static enum piglit_result
sample_buffer(void *buf, int fd, int fourcc, unsigned w, unsigned h,
	unsigned stride, unsigned offset)
{
	EGLint error;
	EGLImageKHR img;
	EGLint attr[] = {
		EGL_WIDTH, w,
		EGL_HEIGHT, h,
		EGL_LINUX_DRM_FOURCC_EXT, fourcc,
		EGL_DMA_BUF_PLANE0_FD_EXT, fd,
		EGL_DMA_BUF_PLANE0_OFFSET_EXT, offset,
		EGL_DMA_BUF_PLANE0_PITCH_EXT, stride,
		EGL_NONE
	};

	img = eglCreateImageKHR(eglGetCurrentDisplay(), EGL_NO_CONTEXT,
			EGL_LINUX_DMA_BUF_EXT, (EGLClientBuffer)0, attr);

	/* Release the creator side of the buffer. */
	piglit_destroy_dma_buf(buf);

	error = eglGetError();

	/* EGL may not support the format, this is not an error. */
	if (!img && error == EGL_BAD_MATCH)
		return PIGLIT_SKIP;

	if (error != EGL_SUCCESS) {
		printf("eglCreateImageKHR() failed: %s 0x%x\n",
			piglit_get_egl_error_name(error), error);

		/* Close the descriptor also, EGL does not have ownership */
		close(fd);

		return PIGLIT_FAIL;
	}

	if (!img) {
		fprintf(stderr, "image creation succeeded but returned NULL\n");
		return PIGLIT_FAIL;
	}

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

	res = piglit_create_dma_buf(w, h, cpp, src, w * cpp, &buf, &fd, &stride,
				&offset);
	if (res != PIGLIT_PASS)
		return res;

	return sample_buffer(buf, fd, fourcc, w, h, stride, offset);
}
