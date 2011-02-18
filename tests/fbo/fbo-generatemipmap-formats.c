/*
 * Copyright © 2009-2010 Intel Corporation
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
 *
 * Authors:
 *    Eric Anholt <eric@anholt.net>
 *
 */

/** @file fbo-generatemipmap-formats.c
 *
 * Tests that glGenerateMipmapEXT works correctly on 2D textures of various
 * internalformats.
 */

#include "piglit-util.h"
#include "fbo-formats.h"

static int tex_width = 256;
static int tex_height = 256;
int piglit_width = 700;
int piglit_height = 300;
int piglit_window_mode = GLUT_DOUBLE | GLUT_RGB | GLUT_ALPHA;

static const struct test_desc *test_set;
static int test_index;
static int format_index;

static GLboolean
supported(const struct test_desc *test)
{
	unsigned i;

	for (i = 0; i < 3; i++) {
		if (test->ext[i]) {
			if (!glutExtensionSupported(test->ext[i])) {
				return GL_FALSE;
			}
		}
	}

	return GL_TRUE;
}

static void set_npot(GLboolean npot)
{
	if (npot) {
		tex_width = 293;
		tex_height = 277;
	} else {
		tex_width = 256;
		tex_height = 256;
	}
}

static void
key_func(unsigned char key, int x, int y)
{
	switch (key) {
	case 'n': /* next test set */
		do {
			test_index++;
			if (test_index >= ARRAY_SIZE(test_sets)) {
				test_index = 0;
			}
		} while (!supported(&test_sets[test_index]));
		format_index = 0;
		printf("Using test set: %s\n", test_sets[test_index].param);
		break;

	case 'N': /* previous test set */
		do {
			test_index--;
			if (test_index < 0) {
				test_index = ARRAY_SIZE(test_sets) - 1;
			}
		} while (!supported(&test_sets[test_index]));
		format_index = 0;
		printf("Using test set: %s\n", test_sets[test_index].param);
		break;

	case 'm': /* next format */
		format_index++;
		if (format_index >= test_sets[test_index].num_formats) {
			format_index = 0;
		}
		break;

	case 'M': /* previous format */
		format_index--;
		if (format_index < 0) {
			format_index = test_sets[test_index].num_formats - 1;
		}
		break;
	case 'd':
		set_npot(tex_width == 256 && GLEW_ARB_texture_non_power_of_two);
		break;
	}

	piglit_escape_exit_key(key, x, y);
}

static int
create_tex(GLenum internalformat, GLenum baseformat)
{
	GLuint tex;
	int i;
	GLenum type, format;

	if ((baseformat == GL_DEPTH_COMPONENT) || (baseformat == GL_DEPTH_STENCIL)) {
		tex = piglit_depth_texture(internalformat,
					   tex_width, tex_height, GL_FALSE);
		assert(glGetError() == 0);
		if (baseformat == GL_DEPTH_STENCIL) {
			format = GL_DEPTH_STENCIL;
			type = GL_UNSIGNED_INT_24_8;
		} else if (internalformat == GL_DEPTH32F_STENCIL8) {
			format = GL_DEPTH_STENCIL;
			type = GL_FLOAT_32_UNSIGNED_INT_24_8_REV;
		} else if (baseformat == GL_DEPTH_COMPONENT) {
			format = GL_DEPTH_COMPONENT;
			type = GL_FLOAT;
		} else {
			assert(0);
		}
	} else {
		tex = piglit_rgbw_texture(internalformat,
					  tex_width, tex_height, GL_FALSE,
					  GL_TRUE);
		format = GL_RGBA;
		type = GL_FLOAT;
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
			GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
			GL_NEAREST_MIPMAP_NEAREST);

	for (i = 1; (tex_width >> i) || (tex_height >> i); i++) {
		glTexImage2D(GL_TEXTURE_2D, i, internalformat,
			     (tex_width >> i) ? (tex_width >> i) : 1,
			     (tex_height >> i) ? (tex_height >> i) : 1,
			     0,
			     format, type, NULL);
	}
	assert(glGetError() == 0);

	glGenerateMipmapEXT(GL_TEXTURE_2D);

	return tex;
}

static void
draw_mipmap(int x, int y, int level)
{
	glViewport(0, 0, piglit_width, piglit_height);
	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	glEnable(GL_TEXTURE_2D);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	piglit_draw_rect_tex(x, y,
			     (tex_width >> level) ? (tex_width >> level) : 1,
			     (tex_height >> level) ? (tex_height >> level) : 1,
			     0, 0, 1, 1);
}

static GLboolean
test_mipmap_drawing(int x, int y, int level, GLuint internalformat)
{
	GLboolean pass = GL_TRUE;
	int w = (tex_width >> level) ? (tex_width >> level) : 1;
	int h = (tex_height >> level) ? (tex_height >> level) : 1;
	int x1 = x, y1 = y, x2 = x + w/2, y2 = y + h/2;
	float r[] = {1, 0, 0, 0};
	float g[] = {0, 1, 0, 0.25};
	float b[] = {0, 0, 1, 0.5};
	float wh[] = {1, 1, 1, 1};
	GLint r_size, g_size, b_size, l_size, a_size, d_size, i_size;
	GLint compressed;

	glGetTexLevelParameteriv(GL_TEXTURE_2D, level,
				 GL_TEXTURE_COMPRESSED, &compressed);
	if (compressed && (w < h ? w : h) < 8)
		return GL_TRUE;

	if (GLEW_ARB_depth_texture) {
		glGetTexLevelParameteriv(GL_TEXTURE_2D, level,
					 GL_TEXTURE_DEPTH_SIZE, &d_size);
	} else {
		d_size = 0;
	}
	glGetTexLevelParameteriv(GL_TEXTURE_2D, level,
				 GL_TEXTURE_LUMINANCE_SIZE, &l_size);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, level,
				 GL_TEXTURE_ALPHA_SIZE, &a_size);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, level,
				 GL_TEXTURE_INTENSITY_SIZE, &i_size);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, level,
				 GL_TEXTURE_RED_SIZE, &r_size);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, level,
				 GL_TEXTURE_GREEN_SIZE, &g_size);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, level,
				 GL_TEXTURE_BLUE_SIZE, &b_size);

	if (d_size) {
		for (x1 = x; x1 < x + w; x1++) {
			float val = (x1 - x + 0.5) / w;
			float color[3] = {val, val, val};
			pass = pass && piglit_probe_rect_rgb(x1, y, 1, h,
							     color);
		}
		return pass;
	}

	if (i_size || l_size) {
		r[0] = 1.0;
		r[1] = 1.0;
		r[2] = 1.0;

		g[0] = 0.0;
		g[1] = 0.0;
		g[2] = 0.0;

		b[0] = 0.0;
		b[1] = 0.0;
		b[2] = 0.0;

		if (i_size) {
			r[3] = 1.0;
			g[3] = 0.0;
			b[3] = 0.0;
		} else if (l_size && !a_size) {
			r[3] = 1.0;
			g[3] = 1.0;
			b[3] = 1.0;
			wh[3] = 1.0;
		}
	} else if (a_size && !r_size && !l_size) {
		r[0] = 1.0;
		r[1] = 1.0;
		r[2] = 1.0;
		g[0] = 1.0;
		g[1] = 1.0;
		g[2] = 1.0;
		b[0] = 1.0;
		b[1] = 1.0;
		b[2] = 1.0;
	} else {
		if (!r_size) {
			r[0] = 0.0;
			wh[0] = 0.0;
		}

		if (!g_size) {
			g[1] = 0.0;
			wh[1] = 0.0;
		}

		if (!b_size) {
			b[2] = 0.0;
			wh[2] = 0.0;
		}
		if (!a_size) {
			r[3] = 1.0;
			g[3] = 1.0;
			b[3] = 1.0;
			wh[3] = 1.0;
		}
	}

	/* Clamp the bits for the framebuffer, except we aren't checking
	 * the actual framebuffer bits.
	 */
	if (l_size > 8)
		l_size = 8;
	if (i_size > 8)
		i_size = 8;
	if (r_size > 8)
		r_size = 8;
	if (g_size > 8)
		g_size = 8;
	if (b_size > 8)
		b_size = 8;
	if (a_size > 8)
		a_size = 8;

	if (d_size) {
		piglit_set_tolerance_for_bits(8, 8, 8, 8);
	} else if (i_size) {
		piglit_set_tolerance_for_bits(i_size, i_size, i_size, i_size);
	} else if (l_size) {
		piglit_set_tolerance_for_bits(l_size, l_size, l_size, a_size);
	} else {
		piglit_set_tolerance_for_bits(r_size, g_size, b_size, a_size);
	}

	if (internalformat == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT ||
	    internalformat == GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT) {
		/* If alpha in DXT1 is < 0.5, the whole pixel should be black. */
		r[0] = r[1] = r[2] = r[3] = 0;
		g[0] = g[1] = g[2] = g[3] = 0;
		/* If alpha in DXT1 is >= 0.5, it should be white. */
		b[3] = 1;
	}

	if (tex_width == 256) {
		pass = pass && piglit_probe_rect_rgba(x1, y1, w/2, h/2, r);
		pass = pass && piglit_probe_rect_rgba(x2, y1, w/2, h/2, g);
		pass = pass && piglit_probe_rect_rgba(x1, y2, w/2, h/2, b);
		pass = pass && piglit_probe_rect_rgba(x2, y2, w/2, h/2, wh);
	} else if (w > 1 && h > 1) {
		if (compressed) {
			/* DXT1 RGBA blurs the pixels in the NPOT case. */
			if (w <= 7 || h <= 7) {
				return pass;
			}
			pass = pass && piglit_probe_rect_rgba(x1,   y1,   w/2-4, h/2-4, r);
			pass = pass && piglit_probe_rect_rgba(x2+4, y1,   w/2-4, h/2-4, g);
			pass = pass && piglit_probe_rect_rgba(x1,   y2+4, w/2-4, h/2-4, b);
			pass = pass && piglit_probe_rect_rgba(x2+4, y2+4, w/2-4, h/2-4, wh);
		} else {
			/* There may be inaccuracies with NPOT sampling in the middle of the texture. */
			pass = pass && piglit_probe_rect_rgba(x1,   y1,   w/2-1, h/2-1, r);
			pass = pass && piglit_probe_rect_rgba(x2+1, y1,   w/2-1, h/2-1, g);
			pass = pass && piglit_probe_rect_rgba(x1,   y2+1, w/2-1, h/2-1, b);
			pass = pass && piglit_probe_rect_rgba(x2+1, y2+1, w/2-1, h/2-1, wh);
		}
	}

	return pass;
}

static GLboolean
test_format(const struct format_desc *format, GLenum baseformat)
{
	GLuint tex;
	int x;
	int level;
	GLboolean pass = GL_TRUE;

	printf("Testing %s%s\n", format->name, tex_width == 256 ? "" : " (NPOT)");
	tex = create_tex(format->internalformat, baseformat);

	x = 1;
	for (level = 0; (tex_width >> level) || (tex_height >> level); level++) {
		draw_mipmap(x, 1, level);
		x += (tex_width >> level) + 1;
	}

	x = 1;
	for (level = 0; (tex_width >> level) || (tex_height >> level); level++) {
		pass = pass && test_mipmap_drawing(x, 1, level,
						   format->internalformat);
		x += (tex_width >> level) + 1;
	}

	glDeleteTextures(1, &tex);

	return pass;
}

enum piglit_result
piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	int i;

	glClearColor(0.5, 0.5, 0.5, 0.5);
	glClear(GL_COLOR_BUFFER_BIT);

	if (piglit_automatic) {
		for (i = 0; i < test_set->num_formats; i++) {
			pass = test_format(&test_set->format[i],
					   test_set->base) && pass;
		}
		if (GLEW_ARB_texture_non_power_of_two) {
			set_npot(GL_TRUE);
			for (i = 0; i < test_set->num_formats; i++) {
				pass = test_format(&test_set->format[i],
						   test_set->base) && pass;
			}
			set_npot(GL_FALSE);
		}
	} else {
		pass = test_format(&test_sets[test_index].format[format_index],
				   test_sets[test_index].base);
	}

	glutSwapBuffers();

	return pass ? PIGLIT_SUCCESS : PIGLIT_FAILURE;
}


void piglit_init(int argc, char **argv)
{
	int i, j, k;

	glutKeyboardFunc(key_func);

	piglit_require_extension("GL_EXT_framebuffer_object");

	test_set = &test_sets[0];

	for (i = 1; i < argc; i++) {
		for (j = 1; j < ARRAY_SIZE(test_sets); j++) {
			if (!strcmp(argv[i], test_sets[j].param)) {
				for (k = 0; k < 3; k++) {
					if (test_sets[j].ext[k]) {
						piglit_require_extension(test_sets[j].ext[k]);
					}
				}

				test_set = &test_sets[j];
				test_index = j;
				break;
			}
		}
		if (j == ARRAY_SIZE(test_sets)) {
			fprintf(stderr, "Unknown argument: %s\n", argv[i]);
			exit(1);
		}
	}

	if (!piglit_automatic) {
		printf("    -n   Next test set.\n"
		       "    -N   Previous test set.\n"
		       "    -m   Next format in the set.\n"
		       "    -M   Previous format in the set.\n"
		       "    -d   Switch between POT and NPOT\n");
	}

	printf("Using test set: %s\n", test_set->param);
}
