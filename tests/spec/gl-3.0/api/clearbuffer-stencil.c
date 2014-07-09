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

/**
 * \file clearbuffer-stencil.c
 * Verify clearing stencil buffers with glClearBufferiv
 *
 * This test works by generating several framebuffer objects and attempting to
 * clear the depth buffer of those FBOs by calling \c glClearBufferfv.
 *
 *     - An FBO with only a color attachment.  This should not generate an
 *       error, but the color data should not be modified.
 *
 *     - An FBO with only a stencil attachment.
 *
 *     - An FBO with a stencil attachment and a color attachment.  The color
 *       data should not be modified.
 *
 *     - An FBO with a depth attachment and a stencil attachment.  The depth
 *       data should not be modified.
 *
 *     - An FBO with a packed depth/stencil attachment.  The depth data
 *       should not be modified.
 *
 * In each case, \c glClearBufferfv is called twice.  Each call uses a
 * different clear value.  This ensures that the test doesn't erroneously pass
 * because the depth buffer was already filled with the clear color.
 *
 * \author Ian Romanick
 */
#include "piglit-util-gl.h"
#include "clearbuffer-common.h"

void piglit_init(int argc, char **argv)
{
	static const struct {
		bool color;
		bool stencil;
		bool depth;
		bool packed;
	} test_vectors[] = {
		{ true,  false, false, false },
		{ false, true,  false, false },
		{ true,  true,  false, false },
		{ false, true,  true,  false },
		{ true,  true,  true,  false },
		{ false, true,  true,  true },
		{ true,  true,  true,  true },
	};

	static const int first[4]  = { 0x01, 0x00, 0x00, 0x00 };
	static const int second[4] = { 0xfe, 0x88, 0x88, 0x88 };

	unsigned i;
	bool pass = true;

	piglit_require_gl_version(30);

	for (i = 0; i < ARRAY_SIZE(test_vectors); i++) {
		GLenum err;
		GLuint fb = generate_simple_fbo(test_vectors[i].color,
						test_vectors[i].stencil,
						test_vectors[i].depth,
						test_vectors[i].packed);

		if (fb == 0) {
			if (!piglit_automatic) {
				printf("Skipping framebuffer %s color, "
				       "%s depth, and "
				       "%s stencil (%s).\n",
				       test_vectors[i].color
				       ? "with" : "without",
				       test_vectors[i].depth
				       ? "with" : "without",
				       test_vectors[i].stencil
				       ? "with" : "without",
				       test_vectors[i].packed
				       ? "packed" : "separate");
			}

			continue;
		}

		if (!piglit_automatic) {
			printf("Trying framebuffer %s color, "
			       "%s depth and "
			       "%s stencil (%s)...\n",
			       test_vectors[i].color ? "with" : "without",
			       test_vectors[i].depth ? "with" : "without",
			       test_vectors[i].stencil ? "with" : "without",
			       test_vectors[i].packed ? "packed" : "separate");
		}

		/* The GL spec says nothing about generating an error for
		 * clearing a buffer that does not exist.  Certainly glClear
		 * does not.
		 */
		glClearBufferiv(GL_STENCIL, 0, first);
		err = glGetError();
		if (err != GL_NO_ERROR) {
			fprintf(stderr,
				"First call to glClearBufferiv erroneously "
				"generated a GL error (%s, 0x%04x)\n",
				piglit_get_gl_error_name(err), err);
			pass = false;
		}

		pass = simple_probe(test_vectors[i].color,
				    default_color,
				    test_vectors[i].stencil,
				    first[0],
				    test_vectors[i].depth,
				    default_depth)
			&& pass;

		glClearBufferiv(GL_STENCIL, 0, second);
		err = glGetError();
		if (err != GL_NO_ERROR) {
			fprintf(stderr,
				"Second call to glClearBufferiv erroneously "
				"generated a GL error (%s, 0x%04x)\n",
				piglit_get_gl_error_name(err), err);
			pass = false;
		}

		pass = simple_probe(test_vectors[i].color,
				    default_color,
				    test_vectors[i].stencil,
				    second[0],
				    test_vectors[i].depth,
				    default_depth)
			&& pass;

		glDeleteFramebuffers(1, &fb);
		pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
	}

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
