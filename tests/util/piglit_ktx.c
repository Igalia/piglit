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

#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "piglit_ktx.h"
#include "piglit-util-gl-common.h"

/* FIXME: Remove #defines when piglit-dispatch gains support for GLES. */
#define GL_TEXTURE_1D				0x0DE0
#define GL_TEXTURE_1D_ARRAY			0x8C18
#define GL_TEXTURE_2D				0x0DE1
#define GL_TEXTURE_2D_ARRAY			0x8C1A
#define GL_TEXTURE_3D				0x806F
#define GL_TEXTURE_CUBE_MAP			0x8513
#define GL_TEXTURE_CUBE_MAP_ARRAY		0x9009

#define GL_TEXTURE_BINDING_1D			0x8068
#define GL_TEXTURE_BINDING_1D_ARRAY		0x8C1C
#define GL_TEXTURE_BINDING_2D			0x8069
#define GL_TEXTURE_BINDING_2D_ARRAY		0x8C1D
#define GL_TEXTURE_BINDING_3D			0x806A
#define GL_TEXTURE_BINDING_CUBE_MAP		0x8514
#define GL_TEXTURE_BINDING_CUBE_MAP_ARRAY	0x900A

static const int piglit_ktx_header_length = 64;
static const char piglit_ktx_magic_number[12] =
	{ 0xab, 'K', 'T', 'X', ' ', '1', '1', 0xbb, '\r', '\n', 0x1a, '\n' };

static void
minify(uint32_t *n)
{
	assert(*n != 0);

	if (*n > 1)
		*n >>= 1;
}

/**
 * \brief KTX data.
 */
struct piglit_ktx {
	struct piglit_ktx_info info;

	/** \brief The raw KTX data. */
	void *data;

	/**
	 * \brief Array of images.
	 *
	 * Array length is piglit_ktx_info::num_images.
	 */
	struct piglit_ktx_image *images;
};

static void
piglit_ktx_error(const char *format, ...)
{
	va_list va;
	va_start(va, format);

	printf("error: piglit_ktx: ");
	vprintf(format, va);
	printf("\n");
	fflush(stdout);

	va_end(va);
}

void
piglit_ktx_destroy(struct piglit_ktx *self)
{
	if (self == NULL)
		return;

	if (self->images != NULL)
		free(self->images);

	if (self->data)
		free(self->data);

	free(self);
}

/**
 * \brief Calculate and set self->info->target.
 */
static bool
piglit_ktx_calc_target(struct piglit_ktx *self)
{
	struct piglit_ktx_info *info = &self->info;

	if (info->pixel_width == 0) {
		goto bad_target;
	} else if (info->pixel_height == 0) {
		if (info->pixel_depth != 0)
			goto bad_target;
		if (info->num_faces != 1)
			goto bad_target;

		if (info->array_length == 0) {
			info->target = GL_TEXTURE_1D;
		} else {
			info->target = GL_TEXTURE_1D_ARRAY;
		}
	} else if (info->pixel_depth == 0) {
		if (info->array_length == 0) {
			if (info->num_faces == 1)
				info->target = GL_TEXTURE_2D;
			else if (info->num_faces == 6)
				info->target = GL_TEXTURE_CUBE_MAP;
			else
				goto bad_target;
		} else {
			if (info->num_faces == 1)
				info->target = GL_TEXTURE_2D_ARRAY;
			else if (info->num_faces == 6)
				info->target = GL_TEXTURE_CUBE_MAP_ARRAY;
			else
				goto bad_target;
		}
	} else {
		if (info->array_length != 0)
			goto bad_target;
		if (info->num_faces != 0)
			goto bad_target;

		info->target = GL_TEXTURE_3D;
	}

	return true;

bad_target:
	piglit_ktx_error("%s", "invalid texture target: pixel_size, "
			 "array_size, and num_faces are incompatible");
	return false;
}

static bool
piglit_ktx_parse_header(struct piglit_ktx *self)
{
	struct piglit_ktx_info *info = &self->info;
	bool ok = true;

	/*
	 * Used to iterate through KTX header with a step of 32 bits. The KTX
	 * spec declares all header fields as u32 values.
	 */
	const uint32_t *u32 = self->data;

	if (info->size < piglit_ktx_header_length) {
		piglit_ktx_error("data size must be at least length of KTX "
				 "header, %d bytes", piglit_ktx_header_length);
		return false;
	}

	if (memcmp(u32, piglit_ktx_magic_number,
		   sizeof(piglit_ktx_magic_number)) != 0) {
		piglit_ktx_error("%s", "KTX header does not begin with KTX "
				 "magic number");
		return false;
	}

	switch (u32[3]) {
	case 0x04030201:
		/* Little endian is supported. */
		break;
	case 0x01020304:
		piglit_ktx_error("%s", "KTX header declares big endian data, "
				 "but Piglit supports only little endian");
		return false;
	default:
		piglit_ktx_error("KTX header has bad value (0x%x) for "
				 "endianness flag", u32[3]);
		return false;
	}

	info->gl_type = u32[4];
	info->gl_type_size = u32[5];
	info->gl_format = u32[6];
	info->gl_internal_format = u32[7];
	info->gl_base_internal_format = u32[8];
	info->pixel_width = u32[9];
	info->pixel_height = u32[10];
	info->pixel_depth = u32[11];
	info->array_length = u32[12];
	info->num_faces = u32[13];
	info->num_miplevels = u32[14];

	if (info->num_miplevels == 0) {
		piglit_ktx_error("%s", "KTX header requests automatic "
		                 "mipmap generation, which Piglit does not "
		                 "support");
		return false;
	}

	if (u32[15] != 0) {
		piglit_ktx_error("%s", "KTX header declares presence of "
				 "arbitrary key/value data, which Piglit "
				 "does not support");
		return false;
	}

	ok = piglit_ktx_calc_target(self);
	if (!ok)
		return false;

	if (info->target == GL_TEXTURE_CUBE_MAP)
		info->num_images = 6 * info->num_miplevels;
	else
		info->num_images = info->num_miplevels;

	return true;
}

static void
piglit_ktx_calc_base_image_size(struct piglit_ktx *self,
				uint32_t *width,
				uint32_t *height,
				uint32_t *depth)
{
	struct piglit_ktx_info *info = &self->info;

	switch (info->target) {
		case GL_TEXTURE_1D:
			*width = info->pixel_width;
			*height = 0;
			*depth = 0;
			break;
		case GL_TEXTURE_1D_ARRAY:
			*width = info->pixel_width;
			*height = info->array_length;
			*depth = 0;
			break;
		case GL_TEXTURE_2D:
			*width = info->pixel_width;
			*height = info->pixel_height;
			*depth = 0;
			break;
		case GL_TEXTURE_2D_ARRAY:
			*width = info->pixel_width;
			*height = info->pixel_height;
			*depth = info->array_length;
			break;
		case GL_TEXTURE_CUBE_MAP:
			*width = info->pixel_width;
			*height = info->pixel_height;
			*depth = 0;
			break;
		case GL_TEXTURE_CUBE_MAP_ARRAY:
			*width = info->pixel_width;
			*height = info->pixel_height;
			*depth = 6 * info->array_length;
			break;
		case GL_TEXTURE_3D:
			*width = info->pixel_width;
			*height = info->pixel_height;
			*depth = info->pixel_depth;
			break;
		default:
			assert(0);
			break;
	}
}

static bool
piglit_ktx_parse_images(struct piglit_ktx *self)
{
	struct piglit_ktx_info *info = &self->info;

	/* Current image being parsed. */
	struct piglit_ktx_image *image;

	/*
	 * Current byte being parsed. Used to traverse data with a step of
	 * either 8 or 32 bits.
	 */
	union piglit_ktx_position {
		uint8_t *u8;
		uint32_t *u32;
	} p;

	/* Size of image, as passed to glTexImage(). */
	uint32_t pixel_width;
	uint32_t pixel_height;
	uint32_t pixel_depth;

	/* Loop counters */
	int miplevel;
	int face;

	piglit_ktx_calc_base_image_size(self,
					&pixel_width,
					&pixel_height,
					&pixel_depth);

	self->images = calloc(info->num_images, sizeof(*self->images));

	/* Skip header. */
	p.u8 = self->data;
	p.u8 += piglit_ktx_header_length;

	/* Begin parsing first image. */
	image = &self->images[0];

#define CUR_SIZE (p.u8 - (uint8_t *) self->data)

	for (miplevel = 0; miplevel < info->num_miplevels; ++miplevel) {
		uint32_t image_size;

		if (info->size < CUR_SIZE + 1) {
			/*
			 * Reading the image size below would access
			 * out-of-bounds memory.
			 */
			piglit_ktx_error("size of data stream must be at "
					 "least %u", CUR_SIZE + 1);
			return false;
		}

		image_size = *p.u32;
		++p.u32;

		for (face = 0; face < 6; ++face) {
			assert(image - self->images < info->num_images);

			image->data = p.u8;
			image->size = image_size;
			image->miplevel = miplevel;
			image->face = face;
			image->pixel_width = pixel_width;
			image->pixel_height = pixel_height;
			image->pixel_depth = pixel_depth;

			p.u8 += image_size;
			++image;

			/* Padding */
			while (CUR_SIZE % 4 != 0)
				++p.u8;

			if (info->target != GL_TEXTURE_CUBE_MAP)
				break;
		}

		switch (info->target) {
			case GL_TEXTURE_3D:
				minify(&pixel_width);
				minify(&pixel_height);
				minify(&pixel_depth);
				break;
			case GL_TEXTURE_2D:
			case GL_TEXTURE_2D_ARRAY:
			case GL_TEXTURE_CUBE_MAP:
			case GL_TEXTURE_CUBE_MAP_ARRAY:
				minify(&pixel_width);
				minify(&pixel_height);
				break;
			case GL_TEXTURE_1D:
			case GL_TEXTURE_1D_ARRAY:
				minify(&pixel_width);
				break;
			default:
				assert(0);
				break;
		}
	}

	if (info->size < CUR_SIZE) {
		/*
		 * The last image's data lies, at least partially, in
		 * out-of-bounds memory.
		 */
		piglit_ktx_error("size of data stream must be at least %zd",
				 CUR_SIZE);
		return false;
	}


	/*
	 * Up until now, info->size was an upper bound on the data size. Now
	 * the actual data size is known.
	 */
	info->size = CUR_SIZE;

	return true;

#undef CUR_SIZE
}

static bool
piglit_ktx_parse_data(struct piglit_ktx *self)
{
	bool ok = true;

	ok = ok && piglit_ktx_parse_header(self);
	ok = ok && piglit_ktx_parse_images(self);

	return ok;
}

struct piglit_ktx*
piglit_ktx_read_file(const char *filename)
{
	struct piglit_ktx *self;

	FILE *file = NULL;
	size_t size_read = 0;

	bool ok = true;
	int error = 0;

	self = calloc(1, sizeof(*self));
	if (self == NULL)
		goto out_of_memory;

	file = fopen(filename, "rb");
	if (file == NULL)
		goto bad_open;

	error = fseek(file, 0, SEEK_END);
	if (error)
		goto bad_read;
	self->info.size = ftell(file);
	error = fseek(file, 0, SEEK_SET);
	if (error)
		goto bad_read;

	self->data = malloc(self->info.size);
	if (self->data == NULL)
		goto out_of_memory;

	/*
	 * On Unix, one should mmap() here rather than copy. But this code
	 * needs to support Windows too.
	 */
	size_read = fread(self->data, 1, self->info.size, file);
	if (size_read < self->info.size)
		goto bad_read;

	ok = piglit_ktx_parse_data(self);
	goto end;

out_of_memory:
	ok = false;
	piglit_ktx_error("%s", "out of memory");
	goto end;

bad_open:
	ok = false;
	piglit_ktx_error("failed to open file: %s", filename);
	goto end;

bad_read:
	ok = false;
	piglit_ktx_error("errors in reading file: %s", filename);
	goto end;

end:
	if (file != NULL)
		fclose(file);

	if (!ok) {
		piglit_ktx_destroy(self);
		self = NULL;
	}

	return self;
}

struct piglit_ktx*
ktx_file_read_bytes(const void *bytes , size_t size)
{
	struct piglit_ktx *self;
	bool ok = true;

	self = calloc(1, sizeof(*self));
	if (self == NULL) {
		piglit_ktx_error("%s", "out of memory");
		return NULL;
	}

	self->info.size = size;
	memcpy(self->data, bytes, size);

	ok = piglit_ktx_parse_data(self);
	if (!ok) {
		piglit_ktx_destroy(self);
		return NULL;
	}

	return self;
}

bool
piglit_ktx_write_file(struct piglit_ktx *self, const char *filename)
{
	FILE *file = NULL;
	size_t size_written = 0;
	bool ok = true;

	file = fopen(filename, "wb");
	if (file == NULL)
		goto bad_open;

	size_written = fwrite(self->data, self->info.size, 1, file);
	if (size_written < self->info.size)
		goto bad_write;

	goto end;

bad_open:
	ok = false;
	piglit_ktx_error("failed to open file: %s", filename);
	goto end;

bad_write:
	ok = false;
	piglit_ktx_error("errors in writing file: %s", filename);
	goto end;

end:
	if (file != NULL)
		fclose(file);

	return ok;
}

bool
piglit_ktx_write_bytes(struct piglit_ktx *self, void *bytes)
{
	memcpy(bytes, self->data, self->info.size);
	return true;
}

const struct piglit_ktx_image*
piglit_ktx_get_image(struct piglit_ktx *self,
		     int miplevel,
		     int cube_face)
{
	const struct piglit_ktx_info *info = &self->info;

	if (miplevel < 0 || miplevel >= info->num_miplevels) {
		piglit_ktx_error("bad miplevel %d", miplevel);
		return NULL;
	}

	if (cube_face < 0 || cube_face >= 6) {
		piglit_ktx_error("bad cube_face %d", cube_face);
		return NULL;
	}

	if (cube_face != 0 && info->target != GL_TEXTURE_CUBE_MAP) {
		piglit_ktx_error("cube face %d was requested. cube face may "
				 "be requested only for non-array cubemaps",
				 cube_face);
		return NULL;
	}

	if (info->target == GL_TEXTURE_CUBE_MAP)
		return &self->images[6 * miplevel + cube_face];
	else
		return &self->images[miplevel];
}

static bool
piglit_ktx_load_cubeface(struct piglit_ktx *self,
                         int image,
                         GLenum *gl_error)
{
	const struct piglit_ktx_info *info = &self->info;
	const struct piglit_ktx_image *img = &self->images[image];

	GLenum face = GL_TEXTURE_CUBE_MAP_POSITIVE_X + (image % 6);
	int level = image / 6;

	if (info->gl_type == 0)
		glCompressedTexImage2D(face,
				       level,
				       info->gl_internal_format,
				       img->pixel_width,
				       img->pixel_height,
				       0 /*border*/,
				       img->size,
				       img->data);
	else
		glTexImage2D(face,
			     level,
			     info->gl_internal_format,
			     img->pixel_width,
			     img->pixel_height,
			     0 /*border*/,
			     info->gl_format,
			     info->gl_type,
			     img->data);

	*gl_error = glGetError();
	return *gl_error == 0;
}

static bool
piglit_ktx_load_noncubeface(struct piglit_ktx *self,
                            int image,
                            GLenum *gl_error)
{
	const struct piglit_ktx_info *info = &self->info;
	const struct piglit_ktx_image *img = &self->images[image];
	int level = image;

	switch (info->target) {
	case GL_TEXTURE_1D:
		if (piglit_is_gles())
			goto unsupported_on_gles;
		else if (info->gl_type == 0)
			glCompressedTexImage1D(info->target,
					       level,
					       info->gl_internal_format,
					       img->pixel_width,
					       0 /*border*/,
					       img->size,
					       img->data);
		else
			glTexImage1D(info->target,
				     level,
				     info->gl_internal_format,
				     img->pixel_width,
				     0 /*border*/,
				     info->gl_format,
				     info->gl_type,
				     img->data);
		break;
	case GL_TEXTURE_1D_ARRAY:
	case GL_TEXTURE_2D:
	case GL_TEXTURE_CUBE_MAP:
		if (info->gl_type == 0)
			glCompressedTexImage2D(info->target,
					       level,
					       info->gl_internal_format,
					       img->pixel_width,
					       img->pixel_height,
					       0 /*border*/,
					       img->size,
					       img->data);
		else
			glTexImage2D(info->target,
				     level,
				     info->gl_internal_format,
				     img->pixel_width,
				     img->pixel_height,
				     0 /*border*/,
				     info->gl_format,
				     info->gl_type,
				     img->data);
		break;
	case GL_TEXTURE_2D_ARRAY:
	case GL_TEXTURE_3D:
	case GL_TEXTURE_CUBE_MAP_ARRAY:
		if (piglit_is_gles())
			goto unsupported_on_gles;
		else if (info->gl_type == 0)
			glCompressedTexImage3D(info->target,
					       level,
					       info->gl_internal_format,
					       img->pixel_width,
					       img->pixel_height,
					       img->pixel_depth,
					       0 /*border*/,
					       img->size,
					       img->data);
		else
			glTexImage3D(info->target,
				     level,
				     info->gl_internal_format,
				     img->pixel_width,
				     img->pixel_height,
				     img->pixel_depth,
				     0 /*border*/,
				     info->gl_format,
				     info->gl_type,
				     img->data);
		break;
	default:
		*gl_error = 0;
		piglit_ktx_error("bad texture target 0x%x",
		                 info->target);
		return false;
	}

	*gl_error = glGetError();
	return *gl_error == 0;

unsupported_on_gles:
	*gl_error = 0;
	piglit_ktx_error("%s", "GLES supports only GL_TEXTURE_2D and "
			 "GL_TEXTURE_CUBE_MAP");
	return false;
}

static bool
piglit_ktx_load_image(struct piglit_ktx *self,
                      int image,
                      GLenum *gl_error)
{
	if (self->info.target == GL_TEXTURE_CUBE_MAP)
		return piglit_ktx_load_cubeface(self, image, gl_error);
	else
		return piglit_ktx_load_noncubeface(self, image, gl_error);
}

static GLuint
target_to_texture_binding(GLuint target)
{
	switch (target) {
		case GL_TEXTURE_1D:
			return GL_TEXTURE_BINDING_1D;
		case GL_TEXTURE_1D_ARRAY:
			return GL_TEXTURE_BINDING_1D_ARRAY;
		case GL_TEXTURE_2D:
			return GL_TEXTURE_BINDING_2D;
		case GL_TEXTURE_2D_ARRAY:
			return GL_TEXTURE_BINDING_2D_ARRAY;
		case GL_TEXTURE_CUBE_MAP:
			return GL_TEXTURE_BINDING_2D;
		case GL_TEXTURE_CUBE_MAP_ARRAY:
			return GL_TEXTURE_BINDING_CUBE_MAP_ARRAY;
			break;
		case GL_TEXTURE_3D:
			return GL_TEXTURE_BINDING_3D;
		default:
			assert(0);
			break;
	}

	return 0;
}

bool
piglit_ktx_load_texture(struct piglit_ktx *self,
			GLuint *tex_name,
			GLenum *gl_error)
{
	const struct piglit_ktx_info *info = &self->info;

	/*
	 * Local store for glGetError(). A local store is needed because
	 * gl_error may be null.
	 */
	GLenum my_gl_error = GL_NO_ERROR;

	/*
	 * The texture object bound to the texture target before this function call.
	 * Before returning, we rebind the texture.
	 */
	GLint old_bound_tex;

	/*
	 * The GL_UNPACK_ALIGNMENT before this function call.
	 * For KTX, the alignment must be set to 4. Before returning, we restore the unpack alignment
	 *
	 */
	GLint old_unpack_alignment;

	bool made_texture = false;

	bool ok = true;
	int i;

	assert(tex_name != NULL);

	glGetIntegerv(target_to_texture_binding(info->target),
	              &old_bound_tex);
	glGetIntegerv(GL_UNPACK_ALIGNMENT, &old_unpack_alignment);

	/* Reset GL error state. */
	while (glGetError())
		;;

	if (*tex_name == 0) {
		glGenTextures(1, tex_name);
		made_texture = true;

		my_gl_error = glGetError();
		if (my_gl_error)
			goto fail;
	}

	glBindTexture(info->target, *tex_name);
	my_gl_error = glGetError();
	if (my_gl_error)
		goto fail;

	for (i = 0; i < info->num_images; ++i) {
		ok = piglit_ktx_load_image(self, i, &my_gl_error);
		if (!ok)
			goto fail;
	}

	goto cleanup;

fail:
	ok = false;

	if (gl_error != NULL)
		*gl_error = my_gl_error;

	if (made_texture && *tex_name != 0) {
		glDeleteTextures(1, tex_name);
		*tex_name = 0;
	}

cleanup:
	/* Reset GL error state. */
	while (glGetError())
		;;

	glBindTexture(info->target, old_bound_tex);
	glPixelStorei(GL_UNPACK_ALIGNMENT, old_unpack_alignment);
	return ok;
}

const struct piglit_ktx_info*
piglit_ktx_get_info(struct piglit_ktx *self)
{
	return &self->info;
}
