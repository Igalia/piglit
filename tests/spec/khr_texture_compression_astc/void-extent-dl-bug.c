/*
 * Copyright © 2018 Intel Corporation
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

#include "piglit-util-gl.h"
#include "common.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 11;
	config.supports_gl_es_version = 20;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_KHR_texture_compression_astc_ldr");
}

enum piglit_result
piglit_display(void)
{
	/* Mesa commit 710b1d2e665ed654fb8d52b146fa22469e1dc3a7 introduced a
	 * bug with void-extent blocks that have channel values between 0 and
	 * 4. Test this case.
	 */
	const short void_extent_block_upload[8] = {0x0DFC,   // ve header
		                                   0x0000,   // don't care
		                                   0x0000,   // don't care
		                                   0x0000,   // don't care
		                                   0x0001,   // r channel
		                                   0x0002,   // g channel
		                                   0x0003,   // b channel
					           0x0004,}; // a channel
	short void_extent_block_download[8] = {0,};

	bool pass = true;
	for (int i = 0; i < ARRAY_SIZE(formats); i++) {
		GLuint tex;
		glGenTextures(1, &tex);
		glBindTexture(GL_TEXTURE_2D, tex);

		glCompressedTexImage2D(GL_TEXTURE_2D, 0, formats[i].fmt,
				       formats[i].bw, formats[i].bh, 0,
				       formats[i].bb, void_extent_block_upload);
		glGetCompressedTexImage(GL_TEXTURE_2D, 0,
					void_extent_block_download);
		if (memcmp(void_extent_block_upload,
			   void_extent_block_download, 16) != 0) {
			pass = false;
			printf("Failed case %d\n", i);
		}

		glDeleteTextures(1, &tex);
	}

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
