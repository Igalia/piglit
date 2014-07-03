/*
 * Copyright (c) The Piglit project 2007
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

#if defined(_WIN32)
#include <windows.h>
#endif

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#include "piglit-util-gl-common.h"

/**
 * Create a depth texture.  The depth texture will be a gradient which varies
 * from 0.0 at the left side to 1.0 at the right side.  For a 2D array texture,
 * all the texture layers will have the same gradient.
 *
 * \param target  either GL_TEXTURE_1D, GL_TEXTURE_2D, GL_TEXTURE_1D_ARRAY, 
 *                GL_TEXTURE_2D_ARRAY or GL_TEXTURE_RECTANGLE.
 * \param internalformat  either GL_DEPTH_STENCIL, GL_DEPTH_COMPONENT,
 *                        GL_DEPTH24_STENCIL8_EXT or GL_DEPTH32F_STENCIL8.
 * \param w, h, d  level 0 image width, height and depth
 * \param mip  if true, create a full mipmap.  Else, create single-level texture.
 * \return the new texture object id
 */
GLuint
piglit_depth_texture(GLenum target, GLenum internalformat, int w, int h, int d, GLboolean mip)
{
	void *data;
	float *f = NULL, *f2 = NULL;
	unsigned int  *i = NULL;
	int size, x, y, level, layer;
	GLuint tex;
	GLenum type, format;

	glGenTextures(1, &tex);
	glBindTexture(target, tex);
	glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	if (mip) {
		glTexParameteri(target, GL_TEXTURE_MAG_FILTER,
				GL_LINEAR);
		glTexParameteri(target, GL_TEXTURE_MIN_FILTER,
				GL_LINEAR_MIPMAP_NEAREST);
	} else {
		glTexParameteri(target, GL_TEXTURE_MAG_FILTER,
				GL_NEAREST);
		glTexParameteri(target, GL_TEXTURE_MIN_FILTER,
				GL_NEAREST);
	}
	data = malloc(w * h * 4 * sizeof(GLfloat));

	if (internalformat == GL_DEPTH_STENCIL_EXT ||
	    internalformat == GL_DEPTH24_STENCIL8_EXT) {
		format = GL_DEPTH_STENCIL_EXT;
		type = GL_UNSIGNED_INT_24_8_EXT;
		i = data;
	} else if (internalformat == GL_DEPTH32F_STENCIL8) {
		format = GL_DEPTH_STENCIL;
		type = GL_FLOAT_32_UNSIGNED_INT_24_8_REV;
		f2 = data;
	} else {
		format = GL_DEPTH_COMPONENT;
		type = GL_FLOAT;
		f = data;
	}

	for (level = 0, size = w > h ? w : h; size > 0; level++, size >>= 1) {
		for (y = 0; y < h; y++) {
			for (x = 0; x < w; x++) {
				float val = (float)(x) / (w - 1);
				if (f)
					f[y * w + x] = val;
				else if (f2)
					f2[(y * w + x)*2] = val;
				else
					i[y * w + x] = 0xffffff00 * val;
			}
		}

		switch (target) {
		case GL_TEXTURE_1D:
			glTexImage1D(target, level,
				     internalformat,
				     w, 0,
				     format, type, data);
			break;

		case GL_TEXTURE_1D_ARRAY:
		case GL_TEXTURE_2D:
		case GL_TEXTURE_RECTANGLE:
			glTexImage2D(target, level,
				     internalformat,
				     w, h, 0,
				     format, type, data);
			break;

		case GL_TEXTURE_2D_ARRAY:
			glTexImage3D(target, level,
				     internalformat,
				     w, h, d, 0,
				     format, type, NULL);
			for (layer = 0; layer < d; layer++) {
				glTexSubImage3D(target, level,
						0, 0, layer, w, h, 1,
						format, type, data);
			}
			break;

		default:
			assert(0);
		}

		if (!mip)
			break;

		if (w > 1)
			w >>= 1;
		if (h > 1)
			h >>= 1;
	}
	free(data);
	return tex;
}

/**
 * Require transform feedback.
 *
 * Transform feedback may either be provided by GL 3.0 or
 * EXT_transform_feedback.
 */
void
piglit_require_transform_feedback(void)
{
	if (!(piglit_get_gl_version() >= 30 ||
	      piglit_is_extension_supported("GL_EXT_transform_feedback"))) {
		printf("Transform feedback not supported.\n");
		piglit_report_result(PIGLIT_SKIP);
	}
}

/**
 * Convert the image into a format that can be easily understood by
 * visual inspection, and display it on the screen.
 *
 * Luminance and intensity values are converted to a grayscale value.
 * Alpha values are visualized by blending the image with a grayscale
 * checkerboard.
 *
 * Pass image_count = 0 to disable drawing multiple images to window
 * system framebuffer.
 */
void
piglit_visualize_image(float *img, GLenum base_internal_format,
		       int image_width, int image_height,
		       int image_count, bool rhs)
{
	int x, y;
	float checker;
	unsigned components = piglit_num_components(base_internal_format);
	float *visualization =
		(float *) malloc(sizeof(float)*3*image_width*image_height);
	for (y = 0; y < image_height; ++y) {
		for ( x = 0; x < image_width; ++x) {
			float r = 0, g = 0, b = 0, a = 1;
			float *pixel =
				&img[(y * image_width + x) * components];
			switch (base_internal_format) {
			case GL_ALPHA:
				a = pixel[0];
				break;
			case GL_RGBA:
				a = pixel[3];
				/* Fall through */
			case GL_RGB:
				b = pixel[2];
				/* Fall through */
			case GL_RG:
				g = pixel[1];
				/* Fall through */
			case GL_RED:
				r = pixel[0];
				break;
			case GL_LUMINANCE_ALPHA:
				a = pixel[1];
				/* Fall through */
			case GL_INTENSITY:
			case GL_LUMINANCE:
				r = pixel[0];
				g = pixel[0];
				b = pixel[0];
				break;
			}
			checker = ((x ^ y) & 0x10) ? 0.75 : 0.25;
			r = r * a + checker * (1 - a);
			g = g * a + checker * (1 - a);
			b = b * a + checker * (1 - a);
			visualization[(y * image_width + x) * 3] = r;
			visualization[(y * image_width + x) * 3 + 1] = g;
			visualization[(y * image_width + x) * 3 + 2] = b;
		}
	}
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
	glUseProgram(0);

	/* To simultaneously display multiple images on window system
	 * framebuffer.
	 */
	if(image_count) {
		/* Use glWindowPos to directly update x, y coordinates of
		 * current raster position without getting transformed by
		 * modelview projection matrix and viewport-to-window
		 * transform.
		 */
		glWindowPos2f(rhs ? image_width : 0,
			      (image_count - 1) * image_height);
	}
	else {
		glRasterPos2f(rhs ? 0 : -1, -1);
	}
	glDrawPixels(image_width, image_height, GL_RGB, GL_FLOAT,
		     visualization);
	free(visualization);
}

/**
 * Convert from sRGB color space to linear color space, using the
 * formula from the GL 3.0 spec, section 4.1.8 (sRGB Texture Color
 * Conversion).
 */
float
piglit_srgb_to_linear(float x)
{
        if (x <= 0.0405)
                return x / 12.92;
        else
                return pow((x + 0.055) / 1.055, 2.4);
}

/* Convert from linear color space to sRGB color space. */
float
piglit_linear_to_srgb(float x)
{
   if (x < 0.0f)
      return 0.0f;
   else if (x < 0.0031308f)
      return 12.92f * x;
   else if (x < 1.0f)
      return 1.055f * powf(x, 0.41666f) - 0.055f;
   else
      return 1.0f;
}
