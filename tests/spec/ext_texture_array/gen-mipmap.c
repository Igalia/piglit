/*
 * Copyright (c) 2013 VMware, Inc.
 *
 * Permission is hereby, free of charge, to any person obtaining a
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
 * Test glGenerateMipmaps with a texture array.
 * In particular, test with texture compression to expose a Mesa bug.
 * See https://bugs.freedesktop.org/show_bug.cgi?id=66850
 *
 * Brian Paul
 * 24 July 2013
 */


#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;
PIGLIT_GL_TEST_CONFIG_END


/* texture size */
#define WIDTH 128
#define HEIGHT 64
#define DEPTH 3
#define BPP 4  /* GL_RGBA/ubyte */


static bool
run_test(GLenum internalFormat)
{
	unsigned char *buf, *buf2;
	int level;
	GLuint tex;
	bool pass = true;

	buf = malloc(WIDTH * HEIGHT * DEPTH * BPP);
	buf2 = malloc(WIDTH * HEIGHT * DEPTH * BPP);

	/* Create 2D array texture */
	memset(buf, 255, WIDTH * HEIGHT * DEPTH * BPP); /* white */
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D_ARRAY, tex);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0,
		     internalFormat,
		     WIDTH, HEIGHT, DEPTH, 0,
		     GL_RGBA, GL_UNSIGNED_BYTE,
		     buf);

	glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

	/* now read back texture images and test */
	for (level = 0; (WIDTH >> level) > 0; level++) {
		int x, y, z, level_size, row_len;

		memset(buf2, 0, WIDTH * HEIGHT * DEPTH * BPP);

		glGetTexImage(GL_TEXTURE_2D_ARRAY, level,
			      GL_RGBA, GL_UNSIGNED_BYTE, buf2);

		/* test center texel */
		x = (WIDTH >> level) / 2;
		y = (HEIGHT >> level) / 2;

		level_size = (WIDTH >> level) * (HEIGHT >> level) * BPP;
		row_len = (WIDTH >> level) * BPP;

		for (z = 0; z < DEPTH; z++) {
			int pos = z * level_size + y * row_len + x * BPP;

			if (buf2[pos + 0] != 255 ||
			    buf2[pos + 1] != 255 ||
			    buf2[pos + 2] != 255 ||
			    buf2[pos + 3] != 255) {
				printf("Probe at level %d, x %d, y %d, z %d = "
				       " (%u, %u, %u, %u),"
				       " expected (255,255,255,255)\n",
				       level, x, y, z,
				       buf2[pos + 0],
				       buf2[pos + 1],
				       buf2[pos + 2],
				       buf2[pos + 3]);
				printf("Internal tex format %s\n",
				       piglit_get_gl_enum_name(internalFormat));
				pass = false;
				break;
			}
		}
	}

	free(buf);
	free(buf2);

	return pass;
}


enum piglit_result
piglit_display(void)
{
	bool pass = true;

	pass = run_test(GL_RGBA) && pass;
	if (piglit_is_extension_supported("GL_ARB_texture_compression")) {
		pass = run_test(GL_COMPRESSED_RGBA) && pass;
		pass = run_test(GL_COMPRESSED_RGB) && pass;
	}

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_EXT_texture_array");
}
