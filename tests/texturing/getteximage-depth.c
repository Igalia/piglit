/*
 * Copyright (c) 2015 Red Hat
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
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NON-INFRINGEMENT.  IN NO EVENT SHALL VMWARE AND/OR THEIR SUPPLIERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/*
 * @file getteximage-depth
 *
 * Test glGetTexImage for depth/stencil format/target combinations in a roundtrip.
 * i.e. don't draw the textures, just create and readback.
 * this was due to a bug in mesa's handling of 1D array depth textures.
 */

#include "piglit-util-gl.h"
#include "../fbo/fbo-formats.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	/* UNREACHABLE */
	return PIGLIT_FAIL;
}

#define IMAGE_WIDTH 32
#define IMAGE_HEIGHT 32

static const GLenum target_list[] = { GL_TEXTURE_1D,
				      GL_TEXTURE_2D,
				      GL_TEXTURE_3D,
				      GL_TEXTURE_RECTANGLE,
				      GL_TEXTURE_CUBE_MAP,
				      GL_TEXTURE_1D_ARRAY,
				      GL_TEXTURE_2D_ARRAY,
				      GL_TEXTURE_CUBE_MAP_ARRAY };

static int get_test_height(GLenum target)
{
	switch (target) {
	case GL_TEXTURE_1D:
	case GL_TEXTURE_1D_ARRAY:
		return 1;
	default:
		return IMAGE_HEIGHT;
	}
}

static int get_test_depth(GLenum target)
{
	switch (target) {
	case GL_TEXTURE_3D:
		return 16;
	case GL_TEXTURE_1D_ARRAY:
	case GL_TEXTURE_2D_ARRAY:
		return 7;
	case GL_TEXTURE_CUBE_MAP_ARRAY:
		return 12;
	case GL_TEXTURE_CUBE_MAP:
		return 6;
	default:
		return 1;
	}
}

static float get_depth_value(int w, int x)
{
	if (w == 1)
		return 1.0;
	else
		return (float)(x) / (w - 1);
}

static int get_stencil_value(int w, int x)
{
	if (w == 1)
		return 255;
	else
		return (x * 255) / (w - 1);
}

static GLuint
create_depth_texture(const struct format_desc *format, GLenum target,
		     int w, int h, int d, bool mip)
{
	void *data;
	float *f = NULL;
	unsigned int  *i = NULL;
	int size, x, y, level, layer;
	GLuint tex;
	GLuint extra = 0;
	GLenum datatype, dataformat;
	int mul = 1;

	glGenTextures(1, &tex);
	glBindTexture(target, tex);

	if (format->internalformat == GL_DEPTH32F_STENCIL8) {
		mul = 2;
	}
	extra = d * sizeof(GLfloat) * mul;

	data = calloc(1, w * h * sizeof(GLfloat) * mul + extra);

	if (format->internalformat == GL_DEPTH_STENCIL_EXT ||
	    format->internalformat == GL_DEPTH24_STENCIL8_EXT) {
		dataformat = GL_DEPTH_STENCIL_EXT;
		datatype = GL_UNSIGNED_INT_24_8_EXT;
		i = data;
	} else if (format->internalformat == GL_DEPTH32F_STENCIL8) {
		dataformat = GL_DEPTH_STENCIL;
		datatype = GL_FLOAT_32_UNSIGNED_INT_24_8_REV;
		f = data;
	} else {
		dataformat = GL_DEPTH_COMPONENT;
		datatype = GL_FLOAT;
		f = data;
	}

	for (level = 0, size = w > h ? w : h; size > 0; level++, size >>= 1) {
		for (y = 0; y < h; y++) {
			for (x = 0; x < w; x++) {
				float val = get_depth_value(w, x);

				if (f)
					f[(y * w + x)*mul] = val;
				else
					i[y * w + x] = 0xffffff00 * val;
			}
		}

		for (x = 0; x < d; x++) {
			float val = get_depth_value(w, x % w);

			if (f)
				f[(h * w + x)*mul] = val;
			else
				i[h * w + x] = 0xffffff00 * val;
		}

		switch (target) {
		case GL_TEXTURE_1D:
			glTexImage1D(target, level,
				     format->internalformat,
				     w, 0,
				     dataformat, datatype, data);
			break;
		case GL_TEXTURE_2D:
		case GL_TEXTURE_RECTANGLE:
			glTexImage2D(target, level,
				     format->internalformat,
				     w, h, 0,
				     dataformat, datatype, data);
			break;
		case GL_TEXTURE_CUBE_MAP:
			assert(d == 6);
			for (layer = 0; layer < d; layer++) {
				char *ptr = data;
				ptr += layer * 4 * mul;
				glTexImage2D(cube_face_targets[layer],
					     level, format->internalformat,
					     w, h, 0,
					     dataformat, datatype, ptr);
			}
			break;
		case GL_TEXTURE_1D_ARRAY:
			glTexImage2D(target, level,
				     format->internalformat,
				     w, d, 0,
				     dataformat, datatype, NULL);
			for (layer = 0; layer < d; layer++) {
				char *ptr = data;
				ptr += layer * (4 * mul);
				glTexSubImage2D(target, level,
						0, layer, w, 1,
						dataformat, datatype, ptr);
			}
			break;
		case GL_TEXTURE_2D_ARRAY:
		case GL_TEXTURE_CUBE_MAP_ARRAY:
			glTexImage3D(target, level,
				     format->internalformat,
				     w, h, d, 0,
				     dataformat, datatype, NULL);
			for (layer = 0; layer < d; layer++) {
				char *ptr = data;
				ptr += layer * (4 * mul);
				glTexSubImage3D(target, level,
						0, 0, layer, w, h, 1,
						dataformat, datatype, ptr);
			}
			break;

		default:
			assert(0);
		}

		if (!mip)
			break;

		if (w > 1)
			w >>= 1;
		if (target != GL_TEXTURE_1D &&
		    target != GL_TEXTURE_1D_ARRAY &&
		    h > 1)
			h >>= 1;
	}
	free(data);
	return tex;
}

static GLuint
create_stencil_texture(const struct format_desc *format, GLenum target,
		       int w, int h, int d, bool mip)
{
	void *data;
	unsigned char *u;
	int size, x, y, level, layer;
	GLuint tex;
	GLenum datatype, dataformat;

	glGenTextures(1, &tex);
	glBindTexture(target, tex);

	data = malloc(w * h * sizeof(GLubyte) + d * sizeof(GLubyte));

	dataformat = GL_STENCIL_INDEX;
	datatype = GL_UNSIGNED_BYTE;
	u = data;
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	for (level = 0, size = w > h ? w : h; size > 0; level++, size >>= 1) {
		for (y = 0; y < h; y++) {
			for (x = 0; x < w; x++) {
				GLuint val = get_stencil_value(w, x);

				u[y * w + x] = val;
			}
		}

		for (x = 0; x < d; x++) {
			GLuint val = get_stencil_value(w, x % w);
			u[h * w + x] = val;
		}

		switch (target) {
		case GL_TEXTURE_1D:
			glTexImage1D(target, level,
				     format->internalformat,
				     w, 0,
				     dataformat, datatype, data);
			break;

		case GL_TEXTURE_2D:
		case GL_TEXTURE_RECTANGLE:
			glTexImage2D(target, level,
				     format->internalformat,
				     w, h, 0,
				     dataformat, datatype, data);
			break;
		case GL_TEXTURE_CUBE_MAP:
			assert(d == 6);
			for (layer = 0; layer < d; layer++) {
				char *ptr = data;
				ptr += layer;
				glTexImage2D(cube_face_targets[layer],
					     level, format->internalformat,
					     w, h, 0,
					     dataformat, datatype, ptr);
			}
			break;
		case GL_TEXTURE_1D_ARRAY:
			glTexImage2D(target, level,
				     format->internalformat,
				     w, d, 0,
				     dataformat, datatype, NULL);
			for (layer = 0; layer < d; layer++) {
				char *ptr = data;
				ptr += layer;
				glTexSubImage2D(target, level,
						0, layer, w, 1,
						dataformat, datatype, ptr);
			}
			break;
		case GL_TEXTURE_CUBE_MAP_ARRAY:
		case GL_TEXTURE_2D_ARRAY:
			glTexImage3D(target, level,
				     format->internalformat,
				     w, h, d, 0,
				     dataformat, datatype, NULL);
			for (layer = 0; layer < d; layer++) {
				char *ptr = data;
				ptr += layer;
				glTexSubImage3D(target, level,
						0, 0, layer, w, h, 1,
						dataformat, datatype, ptr);
			}
			break;

		default:
			assert(0);
		}

		if (!mip)
			break;

		if (w > 1)
			w >>= 1;
		if (target != GL_TEXTURE_1D &&
		    target != GL_TEXTURE_1D_ARRAY &&
		    h > 1)
			h >>= 1;
	}
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	free(data);
	return tex;
}

static bool
verify_depth_data(const struct format_desc *format, GLenum target,
		  int w, int h, int d, bool mip)
{
	int x, y, layer, level;
	GLfloat *f;
	unsigned *i = NULL;
	int size;
	GLenum datatype = format->base_internal_format;
	GLenum dataformat;
	int layer_size;
	int mul = 1;
	void *data, *ptr;
	if (format->internalformat == GL_DEPTH32F_STENCIL8) {
		dataformat = GL_FLOAT_32_UNSIGNED_INT_24_8_REV;
		mul = 2;
	}
	else if (format->base_internal_format == GL_DEPTH_STENCIL)
		dataformat = GL_UNSIGNED_INT_24_8_EXT;
	else
		dataformat = GL_FLOAT;

	ptr = calloc(mul * 4, w * h * d);
	if (!ptr)
		return false;

	for (level = 0, size = w > h ? w : h; size > 0; level++, size >>= 1) {
		data = ptr;
		layer_size = w * h * 4 * mul;

		if (target == GL_TEXTURE_CUBE_MAP) {
			int idx;
			for (idx = 0; idx < 6; idx++)
				glGetTexImage(cube_face_targets[idx], level, datatype, dataformat,
					      (char *)data + layer_size * idx);
		} else
			glGetTexImage(target, level, datatype, dataformat,
				      data);

		for (layer = 0; layer < d; layer++) {
			f = data;
			if (format->internalformat == GL_DEPTH_STENCIL_EXT ||
			    format->internalformat == GL_DEPTH24_STENCIL8_EXT)
				i = data;

			for (y = 0; y < h; y++) {
				for (x = 0; x < w; x++) {
					int offset = (x + layer) % w;
					float exp_val = get_depth_value(w, offset);
					float got_val;

					if (i)
						got_val = (float)(i[y * w + x] >> 8) / (float)0xffffff;
					else
						got_val = f[(y * w + x) * mul];
					if (fabs(got_val - exp_val) >= 1e-4) {
						fprintf(stderr, "mismatch at %d %d %d %d %g vs %g: %d %dx%d\n", level, x, y, layer, got_val, exp_val, layer_size, w, h);
						if (0){
							uint32_t *myptr = data;
							int myx;
							for (myx = 0; myx < w * h; myx++){
								fprintf(stderr, "%08x ", myptr[myx]);
							}
							fprintf(stderr, "\n");
						}
						free(ptr);
						return false;
					}
				}
			}
			data = (char *)data + layer_size;
		}

		if (!mip)
			break;
		if (w > 1)
			w >>= 1;
		if (target != GL_TEXTURE_1D &&
		    target != GL_TEXTURE_1D_ARRAY &&
		    h > 1)
			h >>= 1;
	}
	free(ptr);
	return true;
}

static bool
verify_stencil_data(const struct format_desc *format,
		    GLenum target, int w, int h, int d, bool mip)
{
	int x, y, layer, level, size;
	GLubyte *ub;
	GLenum datatype = format->base_internal_format;
	GLenum dataformat = GL_UNSIGNED_BYTE;
	void *data, *ptr;
	int layer_size;

	ptr = calloc(1, w * h * d);
	if (!ptr)
		return false;

	for (level = 0, size = w > h ? w : h; size > 0; level++, size >>= 1) {
		data = ptr;
		layer_size = w * h;
		glPixelStorei(GL_PACK_ALIGNMENT, 1);
		if (target == GL_TEXTURE_CUBE_MAP) {
			int idx;
			for (idx = 0; idx < 6; idx++)
				glGetTexImage(cube_face_targets[idx], level, datatype, dataformat,
					      (char *)data + layer_size * idx);
		} else
			glGetTexImage(target, level, datatype, dataformat,
				      data);
		glPixelStorei(GL_PACK_ALIGNMENT, 4);
		for (layer = 0; layer < d; layer++) {
			ub = data;

			for (y = 0; y < h; y++) {
				for (x = 0; x < w; x++) {
					int offset = (x + layer) % w;
					GLubyte got_val;
					GLubyte exp_val = get_stencil_value(w, offset);

					got_val = ub[y * w + x];
					if (exp_val != got_val) {
						fprintf(stderr, "mismatch at %d %d %d %d %d vs %d: %d %dx%d\n", level, x, y, layer, got_val, exp_val, layer_size, w, h);
						free(ptr);
						return false;
					}
				}
			}
			data = (char *)data + layer_size;
		}

		if (!mip)
			break;
		if (w > 1)
			w >>= 1;
		if (target != GL_TEXTURE_1D &&
		    target != GL_TEXTURE_1D_ARRAY &&
		    h > 1)
			h >>= 1;
	}
	free(ptr);
	return true;
}

static bool
test_depth_format(GLenum target, const struct format_desc *format)
{
	GLuint tex;
	int height, num_layers;
	bool ret;

	/* 3D depth textures don't occur */
	if (target == GL_TEXTURE_3D)
		return true;

	height = get_test_height(target);
	num_layers = get_test_depth(target);

	tex = create_depth_texture(format, target, IMAGE_WIDTH, height,
				   num_layers,
				   target == GL_TEXTURE_RECTANGLE ? false : true);

	ret = verify_depth_data(format, target, IMAGE_WIDTH, height, num_layers,
				target == GL_TEXTURE_RECTANGLE ? false : true);

	piglit_report_subtest_result(ret ? PIGLIT_PASS : PIGLIT_FAIL,
				     "%s-%s",
				     piglit_get_gl_enum_name(target),
				     piglit_get_gl_enum_name(format->internalformat));
	glDeleteTextures(1, &tex);
	return ret;
}

static bool
test_stencil_format(GLenum target, const struct format_desc *format)
{
	GLuint tex;
	int height, num_layers;
	bool ret;
	/* 3D depth textures don't occur */
	if (target == GL_TEXTURE_3D)
		return true;

	height = get_test_height(target);
	num_layers = get_test_depth(target);

	tex = create_stencil_texture(format, target, IMAGE_WIDTH, height,
				     num_layers,
				     target == GL_TEXTURE_RECTANGLE ? false : true);

	ret = verify_stencil_data(format, target, IMAGE_WIDTH, height, num_layers,
				  target == GL_TEXTURE_RECTANGLE ? false : true);
	piglit_report_subtest_result(ret ? PIGLIT_PASS : PIGLIT_FAIL,
				     "%s-%s",
				     piglit_get_gl_enum_name(target),
				     piglit_get_gl_enum_name(format->internalformat));
	glDeleteTextures(1, &tex);
	return ret;
}

static bool
test_target_format(GLenum target, const struct format_desc *format)
{
	if (format->base_internal_format == GL_DEPTH_COMPONENT ||
	    format->base_internal_format == GL_DEPTH_STENCIL) {
		return test_depth_format(target, format);
	} else if (format->base_internal_format == GL_STENCIL_INDEX) {
		return test_stencil_format(target, format);
	}
	return true;
}

static bool
test_target(GLenum target)
{
	int fmt_idx;
	int set_idx;
	int ext_idx;
	bool do_test_set;
	bool result = true, ret;

	for (set_idx = 0; set_idx < ARRAY_SIZE(test_sets); set_idx++) {
		do_test_set = true;
		for (ext_idx = 0; ext_idx < 3; ext_idx++) {
			if (test_sets[set_idx].ext[ext_idx])
				if (!piglit_is_extension_supported(test_sets[set_idx].ext[ext_idx])) {
					do_test_set = false;
					break;
				}
		}
		if (!do_test_set)
			continue;

		for (fmt_idx = 0; fmt_idx < test_sets[set_idx].num_formats; fmt_idx++) {
			ret = test_target_format(target, &test_sets[set_idx].format[fmt_idx]);
			if (ret == false)
				result = false;
		}
	}
	return result;
}

void
piglit_init(int argc, char **argv)
{
	int i;
	enum piglit_result status = PIGLIT_PASS;
	bool ret;

	piglit_require_extension("GL_ARB_depth_texture");

	for (i = 0; i < ARRAY_SIZE(target_list); i++) {

		switch (target_list[i]) {
		case GL_TEXTURE_1D:
		case GL_TEXTURE_2D:
		case GL_TEXTURE_3D:
		default:
			break;
		case GL_TEXTURE_RECTANGLE:
			if (!piglit_is_extension_supported("GL_ARB_texture_rectangle"))
				continue;
			break;
		case GL_TEXTURE_CUBE_MAP:
			if (!piglit_is_extension_supported("GL_ARB_texture_cube_map") ||
			    piglit_get_gl_version() < 30)
				continue;
			break;
		case GL_TEXTURE_1D_ARRAY:
		case GL_TEXTURE_2D_ARRAY:
			if (!piglit_is_extension_supported("GL_EXT_texture_array"))
				continue;
			break;
		case GL_TEXTURE_CUBE_MAP_ARRAY:
			if (!piglit_is_extension_supported("GL_ARB_texture_cube_map_array"))
				continue;
			break;
		}

		ret = test_target(target_list[i]);
		if (ret == false)
			status = PIGLIT_FAIL;
	}

	piglit_report_result(status);
	(void)fbo_formats_display;
}
