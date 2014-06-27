/*
 * Copyright 2014 Intel Corporation
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
 * This tests copySubImageData on 3D targets.  The maximum testable
 * textures size is 32x32x32 due to the way the textures are
 * displayed/verified.  One texture is filled with a red background and a
 * green solid in the foreground.  Then the green solid is copied to a blue
 * texture.  The results are then verified.  This can test all possible
 * combinations of texture targets.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 13;

	config.window_width = 34 * 8;
	config.window_height = 34 * 8;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static const float green[3] = {0.0, 1.0, 0.0};
static const float red[3] = {1.0, 0.0, 0.0};
static const float blue[3] = {0.0, 0.0, 1.0};

GLuint prog_1d_array, prog_1D_array;

struct image {
	GLuint texture;
	GLenum target;
	int width, height, depth;
};

struct volume {
	int x, y, z;
	int w, h, d;
};

static void
image_init(struct image *image, GLenum target, int width, int height, int depth)
{
	switch (target) {
	case GL_TEXTURE_CUBE_MAP:
		assert(depth == 6);
	case GL_TEXTURE_CUBE_MAP_ARRAY_ARB:
		assert(width == height);
		assert(depth % 6 == 0);
		break;
	case GL_TEXTURE_1D:
		assert(height == 1);
	case GL_TEXTURE_2D:
	case GL_TEXTURE_RECTANGLE:
		assert(depth == 1);
		break;
	case GL_TEXTURE_1D_ARRAY:
		assert(height == 1);
		break;
	}

	/* Cube maps are always 6 deep */
	if (target == GL_TEXTURE_CUBE_MAP)
		assert(depth == 6);

	/* Cube map arrays always must be a multiple of 6 */
	if (target == GL_TEXTURE_CUBE_MAP_ARRAY_ARB)
		assert(depth % 6 == 0);

	image->target = target;
	image->width = width;
	image->height = height;
	image->depth = depth;

	glGenTextures(1, &image->texture);
	glBindTexture(image->target, image->texture);
	glTexParameteri(image->target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(image->target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

static void
image_fill(struct image *img, const float *bg_color, const float *fg_color,
	   const struct volume *vol)
{
	float *img_data;
	int i, j, k;
	const float *color;

	assert(vol->x >= 0);
	assert(vol->y >= 0);
	assert(vol->z >= 0);
	assert(vol->w >= 0);
	assert(vol->h >= 0);
	assert(vol->d >= 0);
	assert(vol->x + vol->w <= img->width);
	assert(vol->y + vol->h <= img->height);
	assert(vol->z + vol->d <= img->depth);

	img_data = malloc(img->width * img->height * img->depth *
			  3 * sizeof(float));

	/* Construct the image */
	for (k = 0; k < img->depth; ++k) {
		for (j = 0; j < img->height; ++j) {
			for (i = 0; i < img->width; ++i) {
				if (i >= vol->x && i < vol->x + vol->w &&
				    j >= vol->y && j < vol->y + vol->h &&
				    k >= vol->z && k < vol->z + vol->d) {
					color = fg_color;
				} else {
					color = bg_color;
				}

				memcpy(img_data + ((k * img->height + j) * img->width + i) * 3,
				       color, 3 * sizeof(float));
			}
		}
	}

	glBindTexture(img->target, img->texture);

	switch (img->target) {
	case GL_TEXTURE_1D:
		glTexImage1D(img->target, 0,
			     GL_RGB, img->width, 0,
			     GL_RGB, GL_FLOAT, img_data);
		break;

	case GL_TEXTURE_CUBE_MAP:
		for (k = 0; k < 6; ++k) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + k, 0,
				     GL_RGB, img->width, img->height, 0,
				     GL_RGB, GL_FLOAT,
				     img_data + k * img->height * img->width * 3);
		}
		break;

	case GL_TEXTURE_2D:
	case GL_TEXTURE_RECTANGLE:
		glTexImage2D(img->target, 0,
			     GL_RGB, img->width, img->height, 0,
			     GL_RGB, GL_FLOAT, img_data);
		break;

	case GL_TEXTURE_1D_ARRAY:
		glTexImage2D(img->target, 0,
			     GL_RGB, img->width, img->depth, 0,
			     GL_RGB, GL_FLOAT, img_data);
		break;

	case GL_TEXTURE_3D:
	case GL_TEXTURE_2D_ARRAY:
	case GL_TEXTURE_CUBE_MAP_ARRAY_ARB:
		glTexImage3D(img->target, 0,
			     GL_RGB, img->width, img->height, img->depth, 0,
			     GL_RGB, GL_FLOAT, img_data);
		break;
	default:
		assert(!"Invalid target");
	}
}

static void
image_bind_layer(struct image *img, GLenum target, int layer)
{
	switch(img->target) {
	case GL_TEXTURE_1D:
		glFramebufferTexture1D(target,
				       GL_COLOR_ATTACHMENT0_EXT,
				       img->target, img->texture, 0);
		break;
	case GL_TEXTURE_2D:
	case GL_TEXTURE_RECTANGLE:
		glFramebufferTexture2D(target,
				       GL_COLOR_ATTACHMENT0_EXT,
				       img->target, img->texture, 0);
		break;
	case GL_TEXTURE_CUBE_MAP:
		glFramebufferTexture2D(target,
				       GL_COLOR_ATTACHMENT0_EXT,
				       GL_TEXTURE_CUBE_MAP_POSITIVE_X + layer,
				       img->texture, 0);
		break;
	case GL_TEXTURE_3D:
	case GL_TEXTURE_1D_ARRAY:
	case GL_TEXTURE_2D_ARRAY:
	case GL_TEXTURE_CUBE_MAP_ARRAY_ARB:
		glFramebufferTextureLayer(target,
					  GL_COLOR_ATTACHMENT0_EXT,
					  img->texture, 0, layer);
		break;
	}
}

static bool
image_verify(struct image *img, const float *bg_color, const float *fg_color,
	     const struct volume *vol)
{
	GLuint fbo;
	int k;
	bool pass = true;

	glGenFramebuffersEXT(1, &fbo);
	glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, fbo);

	for (k = 0; k < img->depth; ++k) {
		image_bind_layer(img, GL_READ_FRAMEBUFFER_EXT, k);

		if (k < vol->z || k >= vol->z + vol->d) {
			pass &= piglit_probe_rect_rgb(0, 0,
						      img->width, img->height,
						      bg_color);
		} else {
			pass &= piglit_probe_rect_rgb(vol->x,
						      vol->y,
						      vol->w,
						      vol->h,
						      fg_color);
			pass &= piglit_probe_rect_rgb(0, 0,
						      img->width,
						      vol->y,
						      bg_color);
			pass &= piglit_probe_rect_rgb(0, vol->y + vol->h,
						      img->width,
						      img->height - vol->y - vol->h,
						      bg_color);
			pass &= piglit_probe_rect_rgb(0, 0,
						      vol->x,
						      img->height,
						      bg_color);
			pass &= piglit_probe_rect_rgb(vol->x + vol->w, 0,
						      img->width - vol->x - vol->w,
						      img->height,
						      bg_color);
		}
	}

	return pass;
}

static void
image_display(struct image *img, int parent_x, int parent_y)
{
	GLuint fbo;
	int k, off_x, off_y;

	glGenFramebuffersEXT(1, &fbo);
	glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, fbo);

	for (k = 0; k < img->depth; ++k) {
		off_x = parent_x + (k % 8) * (img->width + 2) + 1;
		off_y = parent_y + (k / 8) * (img->height + 2) + 1;

		image_bind_layer(img, GL_READ_FRAMEBUFFER_EXT, k);

		glBlitFramebufferEXT(0, 0, img->width, img->height,
				     off_x, off_y,
				     off_x + img->width, off_y + img->height,
				     GL_COLOR_BUFFER_BIT, GL_NEAREST);
	}

	glDeleteFramebuffersEXT(1, &fbo);
}

struct image src, dst;
struct volume src_vol, dst_vol;
int dst_x, dst_y, dst_z;

static void
quit_with_usage()
{
	printf("usage: arb_copy_image-targets srcTarget srcTexWidth srcTexHeight srcTexDepth\n"
	       "              dstTarget dstTexWidth dstTexHeight dstTexDepth\n"
	       "              srcVolX srcVolY srcVolZ dstVolX dstVolY dstVolZ\n"
	       "              volWidth volHeight volDepth\n");
	exit(1);
}

struct texture_target {
	GLenum val;
	const char *str;
};

#define TARGET(x) { x, #x }
struct texture_target targets[] = {
	TARGET(GL_TEXTURE_1D),
	TARGET(GL_TEXTURE_1D_ARRAY),
	TARGET(GL_TEXTURE_2D),
/*	TARGET(GL_TEXTURE_2D_MULTISAMPLE), */
	TARGET(GL_TEXTURE_RECTANGLE),
	TARGET(GL_TEXTURE_2D_ARRAY),
/*	TARGET(GL_TEXTURE_2D_MULTISAMPLE_ARRAY), */
	TARGET(GL_TEXTURE_CUBE_MAP),
	TARGET(GL_TEXTURE_CUBE_MAP_ARRAY),
	TARGET(GL_TEXTURE_3D),
};
#undef TARGET

static GLenum
parse_target(const char *target_str)
{
	int i;
	GLenum target = GL_NONE;

	for (i = 0; i < sizeof(targets) / sizeof(*targets); ++i) {
		if (strcmp(target_str, targets[i].str) == 0) {
			target = targets[i].val;
			break;
		}
	}

	if (target == GL_NONE)
		quit_with_usage();

	switch (target) {
	case GL_TEXTURE_CUBE_MAP:
		piglit_require_extension("GL_ARB_texture_cube_map");
		break;
	case GL_TEXTURE_1D_ARRAY:
	case GL_TEXTURE_2D_ARRAY:
		piglit_require_extension("GL_EXT_texture_array");
		break;
	case GL_TEXTURE_CUBE_MAP_ARRAY:
		piglit_require_extension("GL_ARB_texture_cube_map_array");
		break;
	}

	return target;
}

void
piglit_init(int argc, char **argv)
{
	if (argc < 18)
		quit_with_usage();

	piglit_require_extension("GL_ARB_copy_image");
	piglit_require_extension("GL_EXT_framebuffer_object");

	image_init(&src, parse_target(argv[1]),
		   atoi(argv[2]), atoi(argv[3]), atoi(argv[4]));

	image_init(&dst, parse_target(argv[5]),
		   atoi(argv[6]), atoi(argv[7]), atoi(argv[8]));

	src_vol.x = atoi(argv[9]);
	src_vol.y = atoi(argv[10]);
	src_vol.z = atoi(argv[11]);

	dst_vol.x = atoi(argv[12]);
	dst_vol.y = atoi(argv[13]);
	dst_vol.z = atoi(argv[14]);

	dst_vol.w = src_vol.w = atoi(argv[15]);
	dst_vol.h = src_vol.h = atoi(argv[16]);
	dst_vol.d = src_vol.d = atoi(argv[17]);
}

enum piglit_result
piglit_display(void)
{
	bool pass = true;

	image_fill(&src, red, green, &src_vol);
	pass &= image_verify(&src, red, green, &src_vol);

	if (!pass) goto out;

	image_fill(&dst, blue, blue, &dst_vol);
	pass &= image_verify(&dst, blue, blue, &dst_vol);

	glCopyImageSubData(src.texture, src.target, 0,
			   src_vol.x, src_vol.y, src_vol.z,
			   dst.texture, dst.target, 0,
			   dst_vol.x, dst_vol.y, dst_vol.z,
			   src_vol.w, src_vol.h, src_vol.d);
	pass &= piglit_check_gl_error(GL_NO_ERROR);

	pass &= image_verify(&dst, blue, green, &dst_vol);

out:
	if (!piglit_automatic) {
		glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT,
				     piglit_winsys_fbo);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		image_display(&dst, 0, 0);
		image_display(&src, 0, 34 * 4);

		piglit_present_results();
	}

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
