/*
 * Copyright (c) 2010 Marek Olšák <maraeo@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.  IN NO EVENT SHALL
 * VA LINUX SYSTEM, IBM AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/**
 * @file
 * Tests 3D textures.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 12;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END


static GLint MaxSize;



/**
 * Compute size (in megabytes) of a texture of the given dimensions and
 * internal format.
 */
static unsigned
tex_size(GLenum internalFormat, int width, int height, int depth)
{
	uint64_t sz;

	sz = (uint64_t) width * (uint64_t) height * (uint64_t) depth;

	switch (internalFormat) {
	case GL_INTENSITY8:
		sz *= 1;
		break;
	case GL_RGBA8:
		sz *= 4;
		break;
	case GL_RGBA32F:
		sz *= 16;
		break;
	default:
		assert(!"Unexpected internalFormat");
	}

	return (unsigned) (sz / (uint64_t) (1024 * 1024));
}


/**
 * Allocate a 1-level 3D texture.
 */
static void
alloc_tex3d(GLenum target, GLenum internalFormat,
	    GLsizei width, GLsizei height, GLsizei depth)
{
	if (target == GL_TEXTURE_3D) {
		glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}

	if (piglit_is_extension_supported("GL_ARB_texture_storage")) {
		glTexStorage3D(target, 1, internalFormat,
			       width, height, depth);
	}
	else {
		glTexImage3D(target, 0, internalFormat,
			     width, height, depth, 0,
			     GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	}
}


/*
 * Use proxy texture to find largest possible 3D texture size.
 */
static void
find_max_tex3d_size(GLenum internalFormat,
		    GLint initSize, GLint *width, GLint *height, GLint *depth)
{
	GLint dim = 0, w, h, d, pw, ph, pd;

	piglit_check_gl_error(GL_NO_ERROR);

	w = h = d = initSize;

	while (w >= 1 && h >= 1 && d >= 1) {
		/* try proxy image */
		const int level = 0;

		alloc_tex3d(GL_PROXY_TEXTURE_3D, internalFormat, w, h, d);

		glGetTexLevelParameteriv(GL_PROXY_TEXTURE_3D, level,
					 GL_TEXTURE_WIDTH, &pw);
		glGetTexLevelParameteriv(GL_PROXY_TEXTURE_3D, level,
					 GL_TEXTURE_HEIGHT, &ph);
		glGetTexLevelParameteriv(GL_PROXY_TEXTURE_3D, level,
					 GL_TEXTURE_DEPTH, &pd);

		if (!piglit_check_gl_error(GL_NO_ERROR)) {
			printf("Unexpected error during texture proxy test.\n");
			piglit_report_result(PIGLIT_FAIL);
		}

		if (pw == w && ph == h && pd == d) {
			/* this size should be supported, but test it to
			 * be sure.
			 */
			GLuint tex;
			GLenum err;

			/* Create a texture object for the non-proxy texture below */
			glGenTextures(1, &tex);
			glBindTexture(GL_TEXTURE_3D, tex);
			alloc_tex3d(GL_TEXTURE_3D, internalFormat, w, h, d);

			err = glGetError();

			glDeleteTextures(1, &tex);

			if (err == GL_NO_ERROR) {
				/* success! */
				*width = w;
				*height = h;
				*depth = d;
				return;
			}
			else {
				printf("Note: proxy texture of size "
				       "%d x %d x %d worked, but actual "
				       "glTexImage3D call failed!\n",
				       w, h, d);
			}
		}

		/* halve one of the dimensions and try again */
		if (dim == 0)
			w /= 2;
		else if (dim == 1)
			h /= 2;
		else
			d /= 2;

		dim = (dim + 1) % 3;
	}
}


/**
 * Create a 3D texture of the given format and size, draw a textured quad
 * with that texture, and check results.
 */
static bool
test_render(GLenum internalFormat, int width, int height, int depth)
{
	static const float c1[4] = {0.25, 0.25, 0.25, 1.0};
	static const float c2[4] = {0.75, 0.75, 0.75, 1.0};
	bool pass = true;
	char *data;
	int i, j;
	GLuint tex;
	unsigned mbytes = tex_size(internalFormat, width, height, depth);

	printf("Testing %d x %d x %d %s (%u MB) texture\n",
	       width, height, depth,
	       piglit_get_gl_enum_name(internalFormat), mbytes);
	fflush(stdout);

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_3D, tex);
	alloc_tex3d(GL_TEXTURE_3D, internalFormat, width, height, depth);

	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		printf("Creating texture failed in test_render().\n");
		pass = false;
		goto end;
	}

	/* Set its pixels, slice by slice. */
	data = malloc(width * height * 4);
	for (j = 0; j < height; j++) {
		for (i = 0; i < width; i++) {
			int a = (j * width + i) * 4;
			data[a+0] =
			data[a+1] =
			data[a+2] =
			data[a+3] = (i * 255) / (width - 1);
		}
	}

	if (piglit_is_extension_supported("GL_ARB_copy_image")) {
		/* load 0th slice */
		glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0,
				width, height, 1,
				GL_RGBA, GL_UNSIGNED_BYTE, data);

		/* copy 0th slice to other slices (should be faster) */
		for (i = 1; i < depth; i++) {
			glCopyImageSubData(tex, GL_TEXTURE_3D, 0, 0, 0, 0,
					   tex, GL_TEXTURE_3D, 0, 0, 0, i,
					   width, height, 1);
		}
	}
	else {
		/* load each slice with glTexSubImage3D */
		for (i = 0; i < depth; i++) {
			glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, i,
					width, height, 1,
					GL_RGBA, GL_UNSIGNED_BYTE, data);
		}
	}
	free(data);

	glClear(GL_COLOR_BUFFER_BIT);

	/* Now try basic rendering. */
	glEnable(GL_TEXTURE_3D);
	glBegin(GL_QUADS);
	glTexCoord3f(0, 0, 0.5);
	glVertex2f(0, 0);
	glTexCoord3f(0, 1, 0.5);
	glVertex2f(0, piglit_height);
	glTexCoord3f(1, 1, 0.5);
	glVertex2f(piglit_width, piglit_height);
	glTexCoord3f(1, 0, 0.5);
	glVertex2f(piglit_width, 0);
	glEnd();

	pass = piglit_probe_pixel_rgb(piglit_width * 1 / 4,
				      piglit_height * 1 / 4, c1) && pass;
	pass = piglit_probe_pixel_rgb(piglit_width * 3 / 4,
				      piglit_height * 1 / 4, c2) && pass;
	pass = piglit_probe_pixel_rgb(piglit_width * 1 / 4,
				      piglit_height * 3 / 4, c1) && pass;
	pass = piglit_probe_pixel_rgb(piglit_width * 3 / 4,
				      piglit_height * 3 / 4, c2) && pass;
	piglit_present_results();

	if (!pass) {
		printf("rendering failed with %d x %d x %d %s texture\n",
		       width, height, depth,
		       piglit_get_gl_enum_name(internalFormat));
	}

end:
	glDeleteTextures(1, &tex);

	return pass;
}


static bool
test_3d_tex_format(GLenum internalFormat)
{
	GLint width, height, depth;
	bool pass = true;
	unsigned mbytes;

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	/* use proxy texture to find actual max texture size */
	width = height = depth = 0;
	find_max_tex3d_size(internalFormat, MaxSize,
			    &width, &height, &depth);

	mbytes = tex_size(internalFormat, width, height, depth);
	printf("Actual max 3D texture size for %s: %d x %d x %d (%u MB)\n",
	       piglit_get_gl_enum_name(internalFormat),
	       width, height, depth, mbytes);

	/* first, try some smaller res 3D texture rendering */
	pass = test_render(internalFormat, width, height, depth/4);
	pass = test_render(internalFormat, width, height, depth/2) && pass;

	/* test largest 3D texture size */
	pass = test_render(internalFormat, width, height, depth) && pass;

	return pass;
}


enum piglit_result
piglit_display(void)
{
	bool pass;

	glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &MaxSize);
	printf("GL_MAX_3D_TEXTURE_SIZE = %d\n", MaxSize);

	pass = test_3d_tex_format(GL_INTENSITY8);

	pass = test_3d_tex_format(GL_RGBA8) && pass;

	if (piglit_is_extension_supported("GL_ARB_texture_float")) {
		pass = test_3d_tex_format(GL_RGBA32F) && pass;
	}

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


void piglit_init(int argc, char **argv)
{
	glDisable(GL_DITHER);

	/* Set the tolerance a little looser since we're using GL_NEAREST
	 * texture sampling.  GL_NEAREST is fastest for software rendering.
	 * We probably wouldn't have to loosen the tolerance if we used
	 * GL_LINEAR filtering.
	 */
	piglit_set_tolerance_for_bits(7, 7, 7, 7);
	printf("Probe tolerance: %f, %f, %f, %f\n",
	       piglit_tolerance[0],
	       piglit_tolerance[1],
	       piglit_tolerance[2],
	       piglit_tolerance[3]);
}
