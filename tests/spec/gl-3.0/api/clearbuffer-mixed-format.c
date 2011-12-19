/* Copyright © 2011 Intel Corporation
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

/*
 * \file clearbuffer-mixed-format.c
 * Verify clearing mixed format color buffers with glClearBuffer functions
 *
 * This test works by generating several mixed format color render buffer
 * objects and attempting to clear those buffers by calling glClearBufferfv,
 * glClearBufferiv or glClearBufferuiv
 * Verify:
 *	- glClearBufferfv should clear the float color buffer to a desired
 *	  float value
 *
 *	- glClearBufferuiv should clear the unsigned int color buffer to a
 *	  desired unsigned integer value
 *
 *	- glClearBufferiv should clear the integer color buffer to a desired
 *	  integer value
 *
 *	- No error should be generated for using glClearBufferuiv or
 *	  glClearBufferiv on a float color buffer or using glClearBufferfv
 *	  on a integer color buffers
 *
 * \author Anuj Phogat
 */

#include "piglit-util.h"
#include "clearbuffer-common.h"

#define COUNT ARRAY_SIZE(test_vectors)

	static const float fcolor[4][4] = {
		{ 0.5,  0.3,  0.7,  0.0 },
		{ 0.8,  0.0,  0.2,  1.0 },
		{ 1.2, -2.9,  0.2,  5.8 },
		{ 0.5,  2.5, -5.2,  1.0 } };

	static const unsigned int  uicolor[3][4] = {
		{  10,   90, 100, 150 },
		{ 100,  190, 200,  15 },
		{  15,   25,  20,  15 } };

	static const int  icolor[3][4] = {
		{ -10,  -90, 100,  15 },
		{ 100,  190, 200, -15 },
		{ -50,  -50, -50,  50 } };

	static const struct {
		GLenum rb_format;
		GLenum type;
		const GLvoid *clear_color;
	} test_vectors[] = {
		/* GL_RGBA8, GL_RGBA16 clamps the color vaues to [0, 1] */
		{ GL_RGBA8,	  GL_FLOAT,	    (const GLvoid *)fcolor[0] },
		{ GL_RGBA16,	  GL_FLOAT,	    (const GLvoid *)fcolor[1] },

		/* GL_RGBA16F, GL_RGBA32F doesn't clamp color values to [0, 1] */
		{ GL_RGBA16F,	  GL_FLOAT,	    (const GLvoid *)fcolor[2] },
		{ GL_RGBA32F,	  GL_FLOAT,	    (const GLvoid *)fcolor[3] },

		/* Integer formats */
		{ GL_RGBA8UI,	  GL_UNSIGNED_INT,  (const GLvoid *)uicolor[0] },
		{ GL_RGBA32UI,	  GL_UNSIGNED_INT,  (const GLvoid *)uicolor[2] },

		{ GL_RGBA16I,	  GL_INT,	    (const GLvoid *)icolor[1] },
		{ GL_RGBA32I,	  GL_INT,	    (const GLvoid *)icolor[2] },
	};


GLuint
generate_fbo(void)
{
	GLuint fb;
	GLuint rb[COUNT];
	GLuint i;
	GLenum status;

	/* Generate a frame buffer object */
	glGenFramebuffers(1, &fb);
	glBindFramebuffer(GL_FRAMEBUFFER, fb);
	/* Generate renderbuffers */
	glGenRenderbuffers(COUNT, rb);

	for(i = 0; i < COUNT; i++) {
		glBindRenderbuffer(GL_RENDERBUFFER, rb[i]);
		/* Buffer storage is allocated based on render buffer format */
		glRenderbufferStorage(GL_RENDERBUFFER,
				      test_vectors[i].rb_format,
				      piglit_width,
				      piglit_height);
		/* Attach the render buffer to a color attachment */
		glFramebufferRenderbuffer(GL_FRAMEBUFFER,
					  GL_COLOR_ATTACHMENT0 + i,
					  GL_RENDERBUFFER,
					  rb[i]);

		piglit_check_gl_error(GL_NO_ERROR, PIGLIT_FAIL);
	}

	status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		fprintf(stderr,
			"Framebuffer with color"
			"attachment was not complete: 0x%04x\n",
			status);
		piglit_report_result(PIGLIT_FAIL);
	}

	if (status == GL_FRAMEBUFFER_UNSUPPORTED) {
		glDeleteRenderbuffers(COUNT, rb);
		glDeleteFramebuffers(1, &fb);
		return 0;
	}
	/* All the color render buffers are cleared to default RGBA
	 * (0.0, 0.0, 0.0, 1.0) color
	 */
	for (i = 0; i < COUNT; i++) {
		glDrawBuffer(GL_COLOR_ATTACHMENT0 + i);
		glClear(GL_COLOR_BUFFER_BIT);
	}
	piglit_check_gl_error(GL_NO_ERROR, PIGLIT_FAIL);
	return fb;
}

void piglit_init(int argc, char **argv)
{
	GLuint fb;
	bool pass = true;
	int i, j;

	piglit_require_gl_version(30);
	fb = generate_fbo();
	if (fb == 0) {
		if (!piglit_automatic) {
			printf("Skipping framebuffer with color"
			       " attachments\n");
		}
	}
	if (!piglit_automatic)
		printf("Created framebuffer with color"
		       " attachments\n");

        for (i = 0; i < COUNT; i++) {

		/* Set the draw buffer and read buffer */
		glDrawBuffer(GL_COLOR_ATTACHMENT0 + i);
		glReadBuffer(GL_COLOR_ATTACHMENT0 + i);

		/* Clear the color buffer to a unique color */
		switch (test_vectors[i].type) {

		/* Float buffer types */
		case GL_FLOAT:
			glClearBufferfv(GL_COLOR,
					i,
					(GLfloat *)test_vectors[i].clear_color);
			break;

		/* Signed/unsigned integer buffer types */
		case GL_INT:
			glClearBufferiv(GL_COLOR,
					i,
					(GLint *)test_vectors[i].clear_color);
			break;

		case GL_UNSIGNED_INT:
			glClearBufferuiv(GL_COLOR,
					 i,
					 (GLuint *)test_vectors[i].clear_color);

			break;
		}
		/* Test the pixel values of color buffer against
		 * expected color values
		 */
		pass = pass &&
		       probe_rect_color(0,
					0,
					piglit_width,
					piglit_height,
					test_vectors[i].type,
					test_vectors[i].clear_color);

		/* Verify that glClearBuffer[uif]v functions only modify the
		 * color data of current draw buffer. Other color buffers stay
		 * uneffected
		 */
		for (j = 0; j < i; j++) {
			/* Set the read buffer */
			glReadBuffer(GL_COLOR_ATTACHMENT0 + j);
			/* Test the pixel values of color buffer against
			 * expected color values
			 */
			pass = pass &&
			       probe_rect_color(0,
						0,
						piglit_width,
						piglit_height,
						test_vectors[j].type,
						test_vectors[j].clear_color);
		}
	}

	/* No GL error should be generated for clearing integer buffers using
	 * glClearBufferfv or clearing float buffers with glClearBufferiv/
	 * glClearBufferuiv. But the result of ClearBuffer is undefined.
	 * Reference:  OpenGL 3.0 specification	section 4.2.3 "Clearing the
	 * Buffers"
	 */
	for (i = 0; i < COUNT; i++) {
		glDrawBuffer(GL_COLOR_ATTACHMENT0 + i);

		glClearBufferuiv(GL_COLOR,
				 i,
				 uicolor[0]);
		glClearBufferiv(GL_COLOR,
				i,
				icolor[0]);
		glClearBufferfv(GL_COLOR,
				i,
				fcolor[0]);

		piglit_check_gl_error(GL_NO_ERROR, PIGLIT_FAIL);
	}
	/* Delete framebuffer object */
	glDeleteFramebuffers(1, &fb);
	piglit_check_gl_error(GL_NO_ERROR, PIGLIT_FAIL);
	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
