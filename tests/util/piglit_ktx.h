/*
 * Copyright 2012 Intel Corporation
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
 * \file
 *
 * \brief Utilities for the KTX file format.
 *
 * The KTX (Khronos texture) file format specifies a simple format for
 * storing texture miptrees. The file format allows texture data for any
 * GL texture format and any GL texture target.
 *
 * \see http://www.khronos.org/opengles/sdk/tools/KTX/file_format_spec/
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>

#include <piglit/gl_wrap.h>

#ifdef __cplusplus
extern "C" {
#endif

struct piglit_ktx;

struct piglit_ktx_info {
	/** \brief Size in bytes of the raw KTX data. */
	size_t size;

	/**
	 * \brief GL texture target.
	 *
	 * This is the `target` agument passed to glTexImage() It is
	 * completely determined by pixel_size, array_length, and num_faces.
	 * Valid values are
	 *   - GL_TEXTURE_1D
	 *   - GL_TEXTURE_1D_ARRAY
	 *   - GL_TEXTURE_2D
	 *   - GL_TEXTURE_2D_ARRAY
	 *   - GL_TEXTURE_3D
	 *   - GL_TEXTURE_CUBE_MAP
	 *   - GL_TEXTURE_CUBE_MAP_ARRAY
	 */
	GLenum target;

	/**
	 * For compressed textures, gl_type is 0. For non-compressed textures,
	 * gl_type is the 'type' argument passed to glTexImage(). For example,
	 * GL_FLOAT.
	 */
	uint32_t gl_type;

	/**
	 * For compressed textures, gl_type_size is 1. For non-compressed
	 * textures, gl_type_size is the size in bytes of gl_type. For example,
	 * if gl_type is GL_FLOAT, gl_type_size is 4.
	 */
	uint32_t gl_type_size;

	/**
	 * For compressed textures, gl_format is 0. For non-compressed textures,
	 * gl_format is the 'format' argument passed to glTexImage(). For
	 * example, GL_RGBA.
	 */
	uint32_t gl_format;

	/**
	 * For compressed and non-compressed textures, gl_internal_format is the
	 * 'internal_format' argument passed to glTexImage(). For
	 * non-compressed textures, this is always a sized format. For example,
	 * GL_RGBA32F.
	 */
	uint32_t gl_internal_format;

	/**
	 * For compressed textures, gl_base_internal_format is the same as
	 * gl_internal_format. For non-compressed textures,
	 * gl_base_internal_format is the same as gl_internal_format.
	 *
	 * (I (chadv) dont' understand what purpose this field serves, But
	 * the KTX spec requires it in the header).
	 */
	uint32_t gl_base_internal_format;

	/**
	 * \name Size of texture, in pixels.
	 * \{
	 *
	 * For 1D textures, height and depth are 0. For 2D and cube textures,
	 * depth is 0.  For block compressed textures, the sizes are not
	 * rounded to block size.
	 *
	 * Note: The sizes here are those in the KTX header, which differ from
	 * the sizes passed to glTexImage().
	 */
	uint32_t pixel_width;
	uint32_t pixel_height;
	uint32_t pixel_depth;
	/** \} */

	/**
	 * If the texture is not an array texture, then array_lengthis 0.
	 */
	uint32_t array_length;

	/**
	 * For cubemaps and cubemap arrays, num_faces is 6. For all other
	 * textures, it is 1.
	 */
	uint32_t num_faces;

	/**
	 * For non-mipmapped textures, num_miplevels is 1.
	 */
	uint32_t num_miplevels;

	/**
	 * For non-array cubemaps, the number of images is 6 * num_miplevels.
	 * For all other textures, the number of images and miplevels is the
	 * same.
	 */
	uint32_t num_images;
};

struct piglit_ktx_image {
	/**
	 * \brief The raw image data.
	 *
	 * This points to the image located in piglit_ktx_info::data. It may
	 * be passed as the 'data' argument to glTexImage().
	 */
	const void *data;

	/**
	 * \brief Size of image, in bytes.
	 *
	 * This is 'imageSize' argument passed to glTexImage(). It does not
	 * include any padding which may be present in ktx_info::bytes.
	 */
	size_t size;

	/**
	 * In range [0, num_miplevels).
	 */
	uint32_t miplevel;

	/**
	 * For non-array cubemap textures, `face` is in range [0, 6). For all
	 * other textures, it is 0.
	 */
	uint32_t face;

	/**
	 * \name Size of image, in pixels.
	 * \{
	 *
	 * These are the sizes passed to glTexImage().
	 * Note: These sizes differ from those in the KTX header.
	 */
	uint32_t pixel_width;
	uint32_t pixel_height;
	uint32_t pixel_depth;
	/** \} */
};

void
piglit_ktx_destroy(struct piglit_ktx *self);

/**
 * \brief Read KTX data from a file.
 *
 * The file is read until EOF.
 *
 * Return null on error, including I/O error and invalid data.
 */
struct piglit_ktx*
piglit_ktx_read_file(const char *filename);

/**
 * \brief Read KTX data from a byte array.
 *
 * Read at most \a size bytes.
 *
 * The given \a size is not used to calculate the expected length of the KTX
 * data; that is completely determined by the content of the KTX header.
 * Instead, the given \a size is a safeguard against reading out-of-bounds
 * memory when incorrect data is present into the header.
 *
 * Return null on error, including invalid data and insufficient \a size.
 */
struct piglit_ktx*
piglit_ktx_read_bytes(const void *bytes, size_t size);

/**
 * \brief Write KTX data to a file.
 *
 * The number of bytes written is `piglit_ktx_get_info()->size`.
 */
bool
piglit_ktx_write_file(struct piglit_ktx *self, const char *filename);

/**
 * \brief Write KTX data to a byte array.
 *
 * The number of bytes written is `piglit_ktx_get_info()->size`.
 */
bool
piglit_ktx_write_bytes(struct piglit_ktx *self, void *bytes);

const struct piglit_ktx_info*
piglit_ktx_get_info(struct piglit_ktx *self);

/**
 * \brief Get a texture image from a KTX file.
 *
 * The given \a miplevel must be in the range `[0,
 * piglit_ktx_info::num_miplevels)`.  For cubemap non-array textures, \a
 * cube_face must be in the range [0, 5].  For all other textures, \a
 * cube_face must be 0. If the above is not satisfied, an error is produced.
 *
 * Note: For cubemap array textures, \a cube_face must be 0 because
 * piglit_ktx_image::data is the data that would be passed to glTexImage3D(),
 * which is not separated into individual faces.
 */
const struct piglit_ktx_image*
piglit_ktx_get_image(struct piglit_ktx *self,
		     int miplevel,
		     int cube_face);

/**
 * \brief Load texture into the GL with glTexImage().
 *
 * If \a *tex_name is non-zero, then that texture is bound to
 * `piglit_ktx_info::target` and the texture images are loaded into it with
 * glTexImage().  If \a *tex_name is 0, then a new texture is first created.
 * The new texture name is returned \a tex_name.
 *
 * Return false on failure. If failure is due to a GL error and \a gl_error is
 * not null, then the value of glGetError() is returned in \a gl_error.
 */
bool
piglit_ktx_load_texture(struct piglit_ktx *self,
			GLuint *tex_name,
			GLenum *gl_error);

#ifdef __cplusplus
}
#endif
