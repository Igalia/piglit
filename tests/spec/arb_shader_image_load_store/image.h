/*
 * Copyright (C) 2014 Intel Corporation
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

/** @file image.h
 *
 * Common image format, target and shader stage metadata.
 */

#ifndef __PIGLIT_ARB_SHADER_IMAGE_LOAD_STORE_IMAGE_H__
#define __PIGLIT_ARB_SHADER_IMAGE_LOAD_STORE_IMAGE_H__

#include "piglit-util-gl.h"

/**
 * Image color value.
 */
struct image_datum {
        double x;
        double y;
        double z;
        double w;
};

/**
 * Size of an image.
 *
 * Note that most tests treat images as a 4-dimensional array of
 * pixels with no specific semantics attached to each dimension
 * (e.g. the x dimension will be the number of samples for multisample
 * images but the horizontal coordinate for 2D textures).  This is a
 * deliberate decision that greatly reduces the amount of duplication,
 * as in many cases you can just run the same test in a loop for all
 * image targets.
 *
 * Unused dimensions equal 1 by convention.
 */
struct image_extent {
        unsigned x;
        unsigned y;
        unsigned z;
        unsigned w;
};

#define get_idx(v, i)                          \
        ((i) == 0 ? (v).x :                    \
         (i) == 1 ? (v).y :                    \
         (i) == 2 ? (v).z :                    \
         (i) == 3 ? (v).w : 0)

#define set_idx(v, i, a)                       \
        (*((i) == 0 ? &(v).x :                 \
           (i) == 1 ? &(v).y :                 \
           (i) == 2 ? &(v).z :                 \
           (i) == 3 ? &(v).w : NULL)) = (a)

#define product(v) ((v).x * (v).y * (v).z * (v).w)

/**
 * Get a two-dimensional image_extent with the same number of elements
 * as the argument, where each dimension is reasonably close to the
 * square root of the total number of elements, e.g. for use as grid
 * invocation size.
 */
struct image_extent
image_optimal_extent(struct image_extent ext);

struct image_format_info {
        /** Format name as specified by GLSL. */
        const char *name;

        /** Format enum. */
        GLenum format;

        /** Pixel transfer format (e.g. as specified for glGetTexImage()). */
        GLenum pixel_format;

        /** Pixel transfer type (e.g. as specified for glGetTexImage()). */
        GLenum pixel_type;

        /** Number of storage bits for each component. */
        unsigned bits[4];
};

/**
 * Image formats supported by image load and store built-ins.
 */
extern const struct image_format_info image_formats_load_store[];

/**
 * Image formats supported by image atomic built-ins.
 */
extern const struct image_format_info image_formats_atomic[];

/**
 * Get information for the specified image format.
 */
const struct image_format_info *
get_image_format(GLenum format);

/**
 * Get the logical base format as seen by the shader (either GL_RGBA
 * or GL_RGBA_INTEGER).
 */
GLenum
image_base_format(const struct image_format_info *format);

/**
 * Get the logical component type as seen by the shader.
 */
GLenum
image_base_type(const struct image_format_info *format);

/**
 * Get the logical internal format as seen by the shader.
 */
GLenum
image_base_internal_format(const struct image_format_info *format);

/**
 * Get the GLSL component data type for an image format.
 */
const char *
image_scalar_type_name(const struct image_format_info *format);

/**
 * Get the GLSL vector data type for an image format.
 */
const char *
image_vector_type_name(const struct image_format_info *format);

/**
 * Get the GLSL image type prefix for an image format ("i", "u" or
 * "").
 */
const char *
image_type_name(const struct image_format_info *format);

/**
 * Get a compatible unsigned integer format of the same size.
 */
GLenum
image_compat_format(const struct image_format_info *format);

/**
 * Get the number of color components representable in an image format.
 */
unsigned
image_num_components(const struct image_format_info *format);

/**
 * Get an arbitrary per-component test scale used to make sure that we
 * exercise a significant portion of the representable range without
 * overflowing it.
 */
struct image_datum
image_format_scale(const struct image_format_info *format);

/**
 * Get the per-component error tolerance for an image format.
 */
struct image_datum
image_format_epsilon(const struct image_format_info *format);

/**
 * Convert \a x to the base data type of the specified image format.
 */
uint32_t
encode(const struct image_format_info *format, double x);

/**
 * Convert \a x from the base data type of the specified image format.
 */
double
decode(const struct image_format_info *format, uint32_t x);

struct image_target_info {
        /** Target name and GLSL image type suffix. */
        const char *name;

        /** Target enum. */
        GLenum target;

        /** Vector type used as address argument for this target. */
        const char *addr_type_name;
};

/**
 * Get all image targets supported by the implementation.
 */
const struct image_target_info *
image_targets(void);

/**
 * Get information for the specified target.
 */
const struct image_target_info *
get_image_target(GLenum t);

/**
 * Get the maximum supported dimensions for the specified target.
 */
struct image_extent
image_target_limits(const struct image_target_info *target);

/**
 * Get the maximum supported number of samples for the specified
 * target.
 */
unsigned
image_target_samples(const struct image_target_info *target);

/**
 * Get reasonable dimensions for an image of type \a target intended
 * to be in one-to-one mapping to a two-dimensional grid of dimensions
 * \a w and \a h.
 */
struct image_extent
image_extent_for_target(const struct image_target_info *target,
                        unsigned w, unsigned h);

/**
 * Get the target type for a single layer of the specified image
 * target.
 */
GLenum
image_layer_target(const struct image_target_info *target);

/**
 * Get the number of dimensions of an image target that are minified
 * in higher mipmap levels.
 */
unsigned
image_target_mipmapping_dimensions(const struct image_target_info *target);

struct image_stage_info {
        /** Shader stage name. */
        const char *name;

        /** Target enum. */
        GLenum stage;

        /** Value used in bit sets for this shader stage. */
        GLbitfield bit;
};

/**
 * Get all shader stages in pipeline order regardless of whether they
 * support image access.
 */
const struct image_stage_info *
known_image_stages(void);

/**
 * Get all shader stages that support image access in pipeline order.
 */
const struct image_stage_info *
image_stages(void);

/**
 * Get information for the specified stage, or NULL if the specified
 * stage doesn't support images.
 */
const struct image_stage_info *
get_image_stage(GLenum s);

/**
 * Get the maximum number of supported image uniforms from the
 * specified stage.
 */
unsigned
image_stage_max_images(const struct image_stage_info *stage);

/**
 * Get the maximum sum of image uniforms from all shaders.
 */
unsigned
max_combined_images(void);

/**
 * Get the maximum number of independent image units.
 */
unsigned
max_image_units(void);

struct image_info {
        /** Texture target of this image object. */
        const struct image_target_info *target;

        /** Format of this image object. */
        const struct image_format_info *format;

        /** Dimensions of this image object. */
        struct image_extent size;

        /** Error tolerance for this image object. */
        struct image_datum epsilon;
};

/**
 * Construct an image_info object.
 */
static inline struct image_info
image_info(GLenum target, GLenum format, unsigned w, unsigned h)
{
        const struct image_target_info *t = get_image_target(target);
        const struct image_format_info *f = get_image_format(format);
        const struct image_info img = {
                t, f,
                image_extent_for_target(t, w, h),
                image_format_epsilon(f)
        };

        return img;
}

/**
 * Set the dimensions of an image.
 */
static inline struct image_info
set_image_size(struct image_info img,
               unsigned x, unsigned y, unsigned z, unsigned w)
{
        const struct image_extent size = { x, y, z, w };
        img.size = size;
        return img;
}

/**
 * Get the number of layers of an image.
 */
unsigned
image_num_layers(const struct image_info img);

/**
 * Get the maximum number of mipmap levels for an image.
 */
unsigned
image_num_levels(const struct image_info img);

/**
 * Get the dimensions of the specified mipmap level of an image.
 */
struct image_extent
image_level_size(const struct image_info img, unsigned l);

/**
 * Get the offset in texels of the specified mipmap level of an
 * image.
 */
static inline unsigned
image_level_offset(const struct image_info img, unsigned l)
{
        return (l == 0 ? 0 :
                image_level_offset(img, l - 1) +
                product(image_level_size(img, l - 1)));
}

/**
 * Construct an image_info object for mipmap level \a l of the
 * specified base image.
 */
static inline struct image_info
image_info_for_level(struct image_info img, unsigned l)
{
        const struct image_info level_img = {
                img.target, img.format,
                image_level_size(img, l),
                img.epsilon
        };

        return level_img;
}

#endif
