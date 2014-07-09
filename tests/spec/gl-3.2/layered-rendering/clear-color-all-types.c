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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/** \file
 *
 * Test that layered color clear works properly with all of the
 * following texture types:
 *
 * - GL_TEXTURE_3D
 * - GL_TEXTURE_2D_ARRAY
 * - GL_TEXTURE_2D_MULTISAMPLE_ARRAY
 * - GL_TEXTURE_1D_ARRAY
 * - GL_TEXTURE_CUBE_MAP
 * - GL_TEXTURE_CUBE_MAP_ARRAY (requires GL_ARB_texture_cube_map_array)
 *
 * The test can be run in two modes:
 *
 * - single_level, which tests layered clears on a texture with just a
 *   single miplevel.
 *
 * - mipmapped, which tests layered clears on a mipmapped texture.
 *
 * The test operates as follows:
 *
 * - A texture is created with the requested type and the appropriate
 *   number of miplevels for the test.
 *
 * - Every level and layer of the texture is individually cleared to
 *   red.
 *
 * - Every level and layer of the texture is checked to verify that it
 *   has been properly cleared to red.
 *
 * - The texture is cleared in layered fashion, with each level being
 *   bound to a layered framebuffer and then cleared all at once.
 *   Each level is cleared to a different color.
 *
 * - Every level and layer of the texture is checked to verify that it
 *   has been cleared to the expected color.
 */

#include "piglit-util-gl.h"
#include "piglit-util.h"

#define TEX_LEVELS 6
#define TEX_SIZE (1 << (TEX_LEVELS - 1))
#define TEX_DEPTH 4

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 32;
	config.supports_gl_core_version = 32;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END


static GLenum texture_type;
static int num_layers;
static int layer_height;
static GLuint probe_fbo = 0;
static bool mipmapped;
static int num_miplevels;
static GLfloat level_colors[TEX_LEVELS][4] = {
	{ 0,   1,   0,   1 },
	{ 0,   1,   0.5, 1 },
	{ 0,   1,   1,   1 },
	{ 0,   0.5, 1,   1 },
	{ 0,   0,   1,   1 },
	{ 0.5, 0,   1,   1 }
};



static void
print_usage_and_exit(const char *prog_name)
{
	printf("Usage: %s <texture_type> <test_type>\n"
	       "  Where <texture_type> is one of:\n"
	       "    3d\n"
	       "    2d_array\n"
	       "    2d_multisample_array\n"
	       "    1d_array\n"
	       "    cube_map\n"
	       "    cube_map_array\n"
	       "  And <test_type> is one of:\n"
	       "    single_level\n"
	       "    mipmapped\n",
	       prog_name);
	piglit_report_result(PIGLIT_FAIL);
}


GLenum cube_map_faces[6] = {
	GL_TEXTURE_CUBE_MAP_POSITIVE_X,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
};


static int
level_width(int level)
{
	return MAX2(TEX_SIZE >> level, 1);
}


static int
level_height(int level)
{
	return MAX2(layer_height >> level, 1);
}


static int
level_layers(int level)
{
	if (texture_type == GL_TEXTURE_3D)
		return MAX2(num_layers >> level, 1);
	else
		return num_layers;
}


static void
init_texture(void)
{
	int i, level;

	for (level = 0; level < num_miplevels; level++) {
		switch (texture_type) {
		case GL_TEXTURE_3D:
		case GL_TEXTURE_2D_ARRAY:
		case GL_TEXTURE_CUBE_MAP_ARRAY:
			glTexImage3D(texture_type, level, GL_RGBA,
				     level_width(level), level_height(level),
				     level_layers(level), 0 /* border */,
				     GL_RGBA, GL_FLOAT, NULL);
			break;
		case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
			glTexImage3DMultisample(texture_type, 4, GL_RGBA,
						level_width(level),
						level_height(level), TEX_DEPTH,
						GL_FALSE);
			break;
		case GL_TEXTURE_CUBE_MAP:
			for (i = 0; i < 6; i++) {
				glTexImage2D(cube_map_faces[i], level, GL_RGBA,
					     level_width(level),
					     level_height(level),
					     0 /* border */, GL_RGBA, GL_FLOAT,
					     NULL);
			}
			break;
		case GL_TEXTURE_1D_ARRAY:
			for (level = 0; level < num_miplevels; level++) {
				glTexImage2D(texture_type, level, GL_RGBA,
					     level_width(level),
					     level_layers(level),
					     0 /* border */, GL_RGBA, GL_FLOAT,
					     NULL);
			}
			break;
		default:
			printf("Don't know how to create texture type %s\n",
			       piglit_get_gl_enum_name(texture_type));
			piglit_report_result(PIGLIT_FAIL);
		}
	}
}


static void
bind_layer(GLenum target, GLuint texture, int level, int layer)
{
	switch (texture_type) {
	case GL_TEXTURE_3D:
	case GL_TEXTURE_2D_ARRAY:
	case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
	case GL_TEXTURE_1D_ARRAY:
	case GL_TEXTURE_CUBE_MAP_ARRAY:
		glFramebufferTextureLayer(target, GL_COLOR_ATTACHMENT0,
					  texture, level, layer);
		break;
	case GL_TEXTURE_CUBE_MAP:
		glFramebufferTexture2D(target, GL_COLOR_ATTACHMENT0,
				       cube_map_faces[layer], texture, level);
		break;
	default:
		printf("Don't know how to bind texture type %s\n",
		       piglit_get_gl_enum_name(texture_type));
		piglit_report_result(PIGLIT_FAIL);
	}
}


static void
check_completeness(const char *when, GLenum target)
{
	GLenum fbstatus = glCheckFramebufferStatus(target);
	if (fbstatus != GL_FRAMEBUFFER_COMPLETE) {
		printf("Framebuffer incomplete when %s: %s\n", when,
		       piglit_get_gl_enum_name(fbstatus));
		piglit_report_result(PIGLIT_FAIL);
	}
}


static bool
check_layers(GLuint fbo, GLuint tex, bool expect_red)
{
	const GLfloat red[] = { 1, 0, 0, 1 };
	bool pass = true;
	int level, layer;

	for (level = 0; level < num_miplevels; level++) {
		const GLfloat *expected_color;
		if (expect_red)
			expected_color = red;
		else
			expected_color = level_colors[level];

		for (layer = 0; layer < level_layers(level); layer++) {
			glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
			bind_layer(GL_READ_FRAMEBUFFER, tex, level, layer);
			check_completeness("reading layers",
					   GL_READ_FRAMEBUFFER);
			printf("Probing level %d, layer %d\n", level, layer);

			if (probe_fbo != 0) {
				/* We can't probe a multisampled texture
				 * directly, so first blit it to a temporary
				 * framebuffer.
				 */
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER,
						  probe_fbo);
				glBlitFramebuffer(0, 0, level_width(level),
						  level_height(level),
						  0, 0, level_width(level),
						  level_height(level),
						  GL_COLOR_BUFFER_BIT,
						  GL_NEAREST);
				glBindFramebuffer(GL_READ_FRAMEBUFFER,
						  probe_fbo);
			}

			pass = piglit_probe_rect_rgba(0, 0, level_width(level),
						      level_height(level),
						      expected_color) && pass;
		}
	}
	return pass;
}


void
piglit_init(int argc, char **argv)
{
	int level, layer;
	bool pass = true;
	GLuint tex, fbo;

	layer_height = TEX_SIZE;
	num_layers = TEX_DEPTH;

	if (argc != 3)
		print_usage_and_exit(argv[0]);
	if (strcmp(argv[1], "3d") == 0) {
		texture_type = GL_TEXTURE_3D;
	} else if (strcmp(argv[1], "2d_array") == 0) {
		texture_type = GL_TEXTURE_2D_ARRAY;
	} else if (strcmp(argv[1], "2d_multisample_array") == 0) {
		texture_type = GL_TEXTURE_2D_MULTISAMPLE_ARRAY;
	} else if (strcmp(argv[1], "1d_array") == 0) {
		texture_type = GL_TEXTURE_1D_ARRAY;
		layer_height = 1;
	} else if (strcmp(argv[1], "cube_map") == 0) {
		texture_type = GL_TEXTURE_CUBE_MAP;
		num_layers = 6;
	} else if (strcmp(argv[1], "cube_map_array") == 0) {
		piglit_require_extension("GL_ARB_texture_cube_map_array");
		texture_type = GL_TEXTURE_CUBE_MAP_ARRAY;
		num_layers = 6 * TEX_DEPTH;
	} else {
		print_usage_and_exit(argv[0]);
	}
	if (strcmp(argv[2], "single_level") == 0) {
		mipmapped = false;
		num_miplevels = 1;
	} else if (strcmp(argv[2], "mipmapped") == 0) {
		if (texture_type == GL_TEXTURE_2D_MULTISAMPLE_ARRAY) {
			printf("2d_multisample_array is incompatible with "
			       "mipmapped\n");
			piglit_report_result(PIGLIT_FAIL);
		}
		mipmapped = true;
		num_miplevels = TEX_LEVELS;
	} else {
		print_usage_and_exit(argv[0]);
	}

	glGenTextures(1, &tex);
	glBindTexture(texture_type, tex);
	if (texture_type != GL_TEXTURE_2D_MULTISAMPLE_ARRAY) {
		if (mipmapped) {
			glTexParameteri(texture_type, GL_TEXTURE_MIN_FILTER,
					GL_LINEAR_MIPMAP_NEAREST);
		} else {
			glTexParameteri(texture_type, GL_TEXTURE_MIN_FILTER,
					GL_LINEAR);
		}
		glTexParameteri(texture_type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	init_texture();
	glGenFramebuffers(1, &fbo);

	if (texture_type == GL_TEXTURE_2D_MULTISAMPLE_ARRAY) {
		/* We'll need a framebuffer we can blit to (in order
		 * to resolve the multisamples) before probing.
		 */
		GLuint rb;
		glGenRenderbuffers(1, &rb);
		glBindRenderbuffer(GL_RENDERBUFFER, rb);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA, TEX_SIZE,
				      layer_height);
		glGenFramebuffers(1, &probe_fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, probe_fbo);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
					  GL_RENDERBUFFER, rb);
	}

	/* Bind each layer of the texture individually and clear it to red. */
	printf("Clearing each layer individually\n");
	glViewport(0, 0, TEX_SIZE, layer_height);
	glClearColor(1, 0, 0, 1);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
	for (level = 0; level < num_miplevels; level++) {
		for (layer = 0; layer < level_layers(level); layer++) {
			bind_layer(GL_DRAW_FRAMEBUFFER, tex, level, layer);
			check_completeness("clearing individual layers",
					   GL_DRAW_FRAMEBUFFER);
			glClear(GL_COLOR_BUFFER_BIT);
		}
	}

	/* Check that each level/layer is cleared to red. */
	pass = check_layers(fbo, tex, true) && pass;

	/* Bind the entire texture in layered fashion and clear each
	 * miplevel to level_colors[level].
	 */
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
	for (level = 0; level < num_miplevels; level++) {
		printf("Clearing all layers in miplevel %d at once\n", level);
		glClearColor(level_colors[level][0], level_colors[level][1],
			     level_colors[level][2], level_colors[level][3]);
		glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
				     tex, level);
		check_completeness("clearing whole texture",
				   GL_DRAW_FRAMEBUFFER);
		glClear(GL_COLOR_BUFFER_BIT);
	}

	/* Check that each level/layer is cleared to the proper
	 * color.
	 */
	pass = check_layers(fbo, tex, false) && pass;

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}


enum piglit_result
piglit_display(void)
{
	/* Should never be reached */
	return PIGLIT_FAIL;
}
