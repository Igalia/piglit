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

/** @file image.c
 *
 * Common image format, target and shader stage metadata.
 */

#include "image.h"

struct image_extent
image_optimal_extent(struct image_extent ext)
{
        const unsigned n = product(ext);
        const unsigned w = 1 << MIN2(ffs(n) - 1, (int)log2(n) / 2);
        const struct image_extent opt = {
                w, n / w, 1, 1
        };

        return opt;
}

const struct image_format_info image_formats_load_store[] = {
        { "rgba32f", GL_RGBA32F, GL_RGBA, GL_FLOAT, { 32, 32, 32, 32 } },
        { "rgba16f", GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT, { 16, 16, 16, 16 } },
        { "rg32f", GL_RG32F, GL_RG, GL_FLOAT, { 32, 32, 0, 0 } },
        { "rg16f", GL_RG16F, GL_RG, GL_HALF_FLOAT, { 16, 16, 0, 0 } },
        { "r11f_g11f_b10f", GL_R11F_G11F_B10F, GL_RGB, GL_UNSIGNED_INT_10F_11F_11F_REV, { 11, 11, 10, 0 } },
        { "r32f", GL_R32F, GL_RED, GL_FLOAT, { 32, 0, 0, 0 } },
        { "r16f", GL_R16F, GL_RED, GL_HALF_FLOAT, { 16, 0, 0, 0 } },
        { "rgba32ui", GL_RGBA32UI, GL_RGBA_INTEGER, GL_UNSIGNED_INT, { 32, 32, 32, 32 } },
        { "rgba16ui", GL_RGBA16UI, GL_RGBA_INTEGER, GL_UNSIGNED_SHORT, { 16, 16, 16, 16 } },
        { "rgb10_a2ui", GL_RGB10_A2UI, GL_RGBA_INTEGER, GL_UNSIGNED_INT_2_10_10_10_REV, { 10, 10, 10, 2 } },
        { "rgba8ui", GL_RGBA8UI, GL_RGBA_INTEGER, GL_UNSIGNED_BYTE, { 8, 8, 8, 8 } },
        { "rg32ui", GL_RG32UI, GL_RG_INTEGER, GL_UNSIGNED_INT, { 32, 32, 0, 0 } },
        { "rg16ui", GL_RG16UI, GL_RG_INTEGER, GL_UNSIGNED_SHORT, { 16, 16, 0, 0 } },
        { "rg8ui", GL_RG8UI, GL_RG_INTEGER, GL_UNSIGNED_BYTE, { 8, 8, 0, 0 } },
        { "r32ui", GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT, { 32, 0, 0, 0 } },
        { "r16ui", GL_R16UI, GL_RED_INTEGER, GL_UNSIGNED_SHORT, { 16, 0, 0, 0 } },
        { "r8ui", GL_R8UI, GL_RED_INTEGER, GL_UNSIGNED_BYTE, { 8, 0, 0, 0 } },
        { "rgba32i", GL_RGBA32I, GL_RGBA_INTEGER, GL_INT, { 32, 32, 32, 32 } },
        { "rgba16i", GL_RGBA16I, GL_RGBA_INTEGER, GL_SHORT, { 16, 16, 16, 16 } },
        { "rgba8i", GL_RGBA8I, GL_RGBA_INTEGER, GL_BYTE, { 8, 8, 8, 8 } },
        { "rg32i", GL_RG32I, GL_RG_INTEGER, GL_INT, { 32, 32, 0, 0 } },
        { "rg16i", GL_RG16I, GL_RG_INTEGER, GL_SHORT, { 16, 16, 0, 0 } },
        { "rg8i", GL_RG8I, GL_RG_INTEGER, GL_BYTE, { 8, 8, 0, 0 } },
        { "r32i", GL_R32I, GL_RED_INTEGER, GL_INT, { 32, 0, 0, 0 } },
        { "r16i", GL_R16I, GL_RED_INTEGER, GL_SHORT, { 16, 0, 0, 0 } },
        { "r8i", GL_R8I, GL_RED_INTEGER, GL_BYTE, { 8, 0, 0, 0 } },
        { "rgba16", GL_RGBA16, GL_RGBA, GL_UNSIGNED_SHORT, { 16, 16, 16, 16 } },
        { "rgb10_a2", GL_RGB10_A2, GL_RGBA, GL_UNSIGNED_INT_2_10_10_10_REV, { 10, 10, 10, 2 } },
        { "rgba8", GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, { 8, 8, 8, 8 } },
        { "rg16", GL_RG16, GL_RG, GL_UNSIGNED_SHORT, { 16, 16, 0, 0 } },
        { "rg8", GL_RG8, GL_RG, GL_UNSIGNED_BYTE, { 8, 8, 0, 0 } },
        { "r16", GL_R16, GL_RED, GL_UNSIGNED_SHORT, { 16, 0, 0, 0 } },
        { "r8", GL_R8, GL_RED, GL_UNSIGNED_BYTE, { 8, 0, 0, 0 } },
        { "rgba16_snorm", GL_RGBA16_SNORM, GL_RGBA, GL_SHORT, { 16, 16, 16, 16 } },
        { "rgba8_snorm", GL_RGBA8_SNORM, GL_RGBA, GL_BYTE, { 8, 8, 8, 8 } },
        { "rg16_snorm", GL_RG16_SNORM, GL_RG, GL_SHORT, { 16, 16, 0, 0 } },
        { "rg8_snorm", GL_RG8_SNORM, GL_RG, GL_BYTE, { 8, 8, 0, 0 } },
        { "r16_snorm", GL_R16_SNORM, GL_RED, GL_SHORT, { 16, 0, 0, 0 } },
        { "r8_snorm", GL_R8_SNORM, GL_RED, GL_BYTE, { 8, 0, 0, 0 } },
        { 0 }
};

const struct image_format_info image_formats_atomic[] = {
        { "r32ui", GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT, { 32, 0, 0, 0 } },
        { "r32i", GL_R32I, GL_RED_INTEGER, GL_INT, { 32, 0, 0, 0 } },
        { 0 }
};

const struct image_format_info *
get_image_format(GLenum f)
{
        const struct image_format_info *format;

        for (format = image_formats_load_store; format->format; ++format) {
                if (format->format == f)
                        return format;
        }

        return NULL;
}

/**
 * Information specific to an image base data type as seen by the
 * shader.
 */
struct image_type_info {
        /** Logical base format as seen by the shader. */
        GLenum base_format;

        /** Logical component type as seen by the shader. */
        GLenum base_type;

        /** Logical internal format as seen by the shader. */
        GLenum base_internal_format;

        /** Matching GLSL component data type. */
        const char *scalar_type_name;

        /** Matching GLSL vector data type. */
        const char *vector_type_name;

        /** GLSL image type prefix ("i", "u" or ""). */
        const char *image_type_name;
};

static const struct image_type_info *
get_image_type(const struct image_format_info *format)
{
        switch (format->pixel_format) {
        case GL_RGBA:
        case GL_RGB:
        case GL_RG:
        case GL_RED: {
                static const struct image_type_info type = {
                        GL_RGBA, GL_FLOAT, GL_RGBA32F,
                        "float", "vec4", "image"
                };
                return &type;
        }
        case GL_RGBA_INTEGER:
        case GL_RG_INTEGER:
        case GL_RED_INTEGER:
                switch (format->pixel_type) {
                case GL_INT:
                case GL_SHORT:
                case GL_BYTE: {
                        static const struct image_type_info type = {
                                GL_RGBA_INTEGER, GL_INT, GL_RGBA32I,
                                "int", "ivec4", "iimage"
                        };
                        return &type;
                }
                case GL_UNSIGNED_INT:
                case GL_UNSIGNED_SHORT:
                case GL_UNSIGNED_INT_2_10_10_10_REV:
                case GL_UNSIGNED_BYTE: {
                        static const struct image_type_info type = {
                                GL_RGBA_INTEGER, GL_UNSIGNED_INT, GL_RGBA32UI,
                                "uint", "uvec4", "uimage"
                        };
                        return &type;
                }
                default:
                        abort();
                }
        default:
                abort();
        }
}

GLenum
image_base_format(const struct image_format_info *format)
{
        return get_image_type(format)->base_format;
}

GLenum
image_base_type(const struct image_format_info *format)
{
        return get_image_type(format)->base_type;
}

GLenum
image_base_internal_format(const struct image_format_info *format)
{
        return get_image_type(format)->base_internal_format;
}

const char *
image_scalar_type_name(const struct image_format_info *format)
{
        return get_image_type(format)->scalar_type_name;
}

const char *
image_vector_type_name(const struct image_format_info *format)
{
        return get_image_type(format)->vector_type_name;
}

const char *
image_type_name(const struct image_format_info *format)
{
        return get_image_type(format)->image_type_name;
}

GLenum
image_compat_format(const struct image_format_info *format)
{
        const unsigned bits = (format->bits[0] + format->bits[1] +
                               format->bits[2] + format->bits[3]);

        switch (bits) {
        case 128:
                return GL_RGBA32UI;

        case 64:
                return GL_RG32UI;

        case 32:
                return GL_R32UI;

        case 16:
                return GL_R16UI;

        case 8:
                return GL_R8UI;

        default:
                abort();
        }
}

unsigned
image_num_components(const struct image_format_info *format)
{
        return (!!format->bits[0] + !!format->bits[1] +
                !!format->bits[2] + !!format->bits[3]);
}

struct image_datum
image_format_scale(const struct image_format_info *format)
{
        struct image_datum v = { 0.0 };
        int i;

        for (i = 0; i < 4 && format->bits[i]; ++i) {
                switch (image_base_type(format)) {
                case GL_FLOAT:
                        set_idx(v, i, 1.0);
                        break;

                case GL_INT:
                        set_idx(v, i, 1u << (format->bits[i] - 2));
                        break;

                case GL_UNSIGNED_INT:
                        set_idx(v, i, 1u << (format->bits[i] - 1));
                        break;

                default:
                        abort();
                }
        }

        return v;
}

static unsigned
image_channel_fraction_bits(const struct image_format_info *format, unsigned i)
{
        if (image_base_type(format) == GL_FLOAT && format->bits[i]) {
                switch (format->pixel_type) {
                case GL_FLOAT:
                        return 23;

                case GL_HALF_FLOAT:
                        return 10;

                case GL_UNSIGNED_INT_10F_11F_11F_REV:
                        return format->bits[i] - 5;

                case GL_SHORT:
                case GL_BYTE:
                        return format->bits[i] - 1;

                case GL_UNSIGNED_SHORT:
                case GL_UNSIGNED_INT_2_10_10_10_REV:
                case GL_UNSIGNED_BYTE:
                        return format->bits[i];

                default:
                        abort();
                }
        } else {
                return 0;
        }
}

struct image_datum
image_format_epsilon(const struct image_format_info *format)
{
        struct image_datum v = { 0.0 };
        int i;

        for (i = 0; i < 4; ++i) {
                unsigned p = image_channel_fraction_bits(format, i);
                set_idx(v, i, (p ? MAX2(1.0 / ((1 << p) - 1), 1e-5) : 0));
        }

        return v;
}

uint32_t
encode(const struct image_format_info *format, double x)
{
        switch (image_base_type(format)) {
        case GL_UNSIGNED_INT:
                return x;

        case GL_INT:
                return (int32_t)x;

        case GL_FLOAT: {
                float y = x;
                return *(uint32_t *)&y;
        }
        default:
                abort();
        }
}

double
decode(const struct image_format_info *format, uint32_t x)
{
        switch (image_base_type(format)) {
        case GL_UNSIGNED_INT:
                return x;

        case GL_INT:
                return (int32_t)x;

        case GL_FLOAT:
                return *(float *)&x;

        default:
                abort();
        }
}

const struct image_target_info *
image_targets(void)
{
        const struct image_target_info known[] = {
                { "1D", GL_TEXTURE_1D, "int" },
                { "2D", GL_TEXTURE_2D, "ivec2" },
                { "3D", GL_TEXTURE_3D, "ivec3" },
                { "2DRect", GL_TEXTURE_RECTANGLE, "ivec2" },
                { "Cube", GL_TEXTURE_CUBE_MAP, "ivec3" },
                { "Buffer", GL_TEXTURE_BUFFER, "int" },
                { "1DArray", GL_TEXTURE_1D_ARRAY, "ivec2" },
                { "2DArray", GL_TEXTURE_2D_ARRAY, "ivec3" },
                { "CubeArray", GL_TEXTURE_CUBE_MAP_ARRAY, "ivec3" },
                { "2DMS", GL_TEXTURE_2D_MULTISAMPLE, "ivec2" },
                { "2DMSArray", GL_TEXTURE_2D_MULTISAMPLE_ARRAY, "ivec3" },
                { 0 }
        };
        static struct image_target_info supported[ARRAY_SIZE(known)];

        if (!supported[0].name) {
                int max_samples = 0, i, n = 0;

                glGetIntegerv(GL_MAX_IMAGE_SAMPLES, &max_samples);

                for (i = 0; i < ARRAY_SIZE(known); ++i) {
                        if ((known[i].target != GL_TEXTURE_2D_MULTISAMPLE &&
                             known[i].target != GL_TEXTURE_2D_MULTISAMPLE_ARRAY) ||
                            max_samples > 1) {
                                supported[n++] = known[i];
                        }
                }
        }

        return supported;
}

const struct image_target_info *
get_image_target(GLenum t)
{
        const struct image_target_info *target;

        for (target = image_targets(); target->target; ++target) {
                if (target->target == t)
                        return target;
        }

        return NULL;
}

struct image_extent
image_target_limits(const struct image_target_info *target)
{
        struct image_extent ext = { 1, 1, 1, 1 };

        switch (target->target) {
        case GL_TEXTURE_1D:
                glGetIntegerv(GL_MAX_TEXTURE_SIZE, (int *)&ext.x);
                break;

        case GL_TEXTURE_2D:
                glGetIntegerv(GL_MAX_TEXTURE_SIZE, (int *)&ext.x);
                glGetIntegerv(GL_MAX_TEXTURE_SIZE, (int *)&ext.y);
                break;

        case GL_TEXTURE_3D:
                glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, (int *)&ext.x);
                glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, (int *)&ext.y);
                glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, (int *)&ext.z);
                break;

        case GL_TEXTURE_RECTANGLE:
                glGetIntegerv(GL_MAX_RECTANGLE_TEXTURE_SIZE, (int *)&ext.x);
                glGetIntegerv(GL_MAX_RECTANGLE_TEXTURE_SIZE, (int *)&ext.y);
                break;

        case GL_TEXTURE_CUBE_MAP:
                glGetIntegerv(GL_MAX_CUBE_MAP_TEXTURE_SIZE, (int *)&ext.x);
                glGetIntegerv(GL_MAX_CUBE_MAP_TEXTURE_SIZE, (int *)&ext.y);
                ext.z = 6;
                break;

        case GL_TEXTURE_BUFFER:
                glGetIntegerv(GL_MAX_TEXTURE_BUFFER_SIZE, (int *)&ext.x);
                break;

        case GL_TEXTURE_1D_ARRAY:
                glGetIntegerv(GL_MAX_TEXTURE_SIZE, (int *)&ext.x);
                glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, (int *)&ext.y);
                break;

        case GL_TEXTURE_2D_ARRAY:
                glGetIntegerv(GL_MAX_TEXTURE_SIZE, (int *)&ext.x);
                glGetIntegerv(GL_MAX_TEXTURE_SIZE, (int *)&ext.y);
                glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, (int *)&ext.z);
                break;

        case GL_TEXTURE_CUBE_MAP_ARRAY:
                glGetIntegerv(GL_MAX_CUBE_MAP_TEXTURE_SIZE, (int *)&ext.x);
                glGetIntegerv(GL_MAX_CUBE_MAP_TEXTURE_SIZE, (int *)&ext.y);
                glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, (int *)&ext.z);
                break;

        case GL_TEXTURE_2D_MULTISAMPLE:
                glGetIntegerv(GL_MAX_IMAGE_SAMPLES, (int *)&ext.x);
                glGetIntegerv(GL_MAX_TEXTURE_SIZE, (int *)&ext.y);
                glGetIntegerv(GL_MAX_TEXTURE_SIZE, (int *)&ext.z);
                break;

        case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
                glGetIntegerv(GL_MAX_IMAGE_SAMPLES, (int *)&ext.x);
                glGetIntegerv(GL_MAX_TEXTURE_SIZE, (int *)&ext.y);
                glGetIntegerv(GL_MAX_TEXTURE_SIZE, (int *)&ext.z);
                glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, (int *)&ext.w);
                break;

        default:
                abort();
        }

        return ext;
}

unsigned
image_target_samples(const struct image_target_info *target)
{
        if (target->target == GL_TEXTURE_2D_MULTISAMPLE ||
            target->target == GL_TEXTURE_2D_MULTISAMPLE_ARRAY) {
                return image_target_limits(target).x;
        } else {
                return 1;
        }
}

struct image_extent
image_extent_for_target(const struct image_target_info *target,
                        unsigned w, unsigned h)
{
        switch (target->target) {
        case GL_TEXTURE_1D: {
                struct image_extent ext = { w * h, 1, 1, 1 };
                return ext;
        }
        case GL_TEXTURE_2D: {
                struct image_extent ext = { w, h, 1, 1 };
                return ext;
        }
        case GL_TEXTURE_3D: {
                struct image_extent ext = { w, w, h / w, 1 };
                return ext;
        }
        case GL_TEXTURE_RECTANGLE: {
                struct image_extent ext = { w, h, 1, 1 };
                return ext;
        }
        case GL_TEXTURE_CUBE_MAP: {
                struct image_extent ext = { w, w, h / w, 1 };
                assert(ext.z == 6);
                return ext;
        }
        case GL_TEXTURE_BUFFER: {
                struct image_extent ext = { w * h, 1, 1, 1 };
                return ext;
        }
        case GL_TEXTURE_1D_ARRAY: {
                struct image_extent ext = { w, h, 1, 1 };
                return ext;
        }
        case GL_TEXTURE_2D_ARRAY: {
                struct image_extent ext = { w, w, h / w, 1 };
                return ext;
        }
        case GL_TEXTURE_CUBE_MAP_ARRAY: {
                struct image_extent ext = { w, w, h / w, 1 };
                assert(ext.z % 6 == 0);
                return ext;
        }
        case GL_TEXTURE_2D_MULTISAMPLE: {
                struct image_extent ext = { 2, w / 2, h, 1 };
                return ext;
        }
        case GL_TEXTURE_2D_MULTISAMPLE_ARRAY: {
                struct image_extent ext = { 2, w / 2, w, h / w };
                return ext;
        }
        default:
                abort();
        }
}

GLenum
image_layer_target(const struct image_target_info *target)
{
        switch (target->target) {
        case GL_TEXTURE_1D:
        case GL_TEXTURE_1D_ARRAY:
                return GL_TEXTURE_1D;

        case GL_TEXTURE_2D:
        case GL_TEXTURE_3D:
        case GL_TEXTURE_CUBE_MAP:
        case GL_TEXTURE_2D_ARRAY:
        case GL_TEXTURE_CUBE_MAP_ARRAY:
                return GL_TEXTURE_2D;

        case GL_TEXTURE_RECTANGLE:
                return GL_TEXTURE_RECTANGLE;

        case GL_TEXTURE_BUFFER:
                return GL_TEXTURE_BUFFER;

        case GL_TEXTURE_2D_MULTISAMPLE:
        case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
                return GL_TEXTURE_2D_MULTISAMPLE;

        default:
                abort();
        }
}

unsigned
image_target_mipmapping_dimensions(const struct image_target_info *target)
{
        switch (target->target) {
        case GL_TEXTURE_RECTANGLE:
        case GL_TEXTURE_BUFFER:
        case GL_TEXTURE_2D_MULTISAMPLE:
        case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
                return 0;

        case GL_TEXTURE_1D:
        case GL_TEXTURE_1D_ARRAY:
                return 1;

        case GL_TEXTURE_2D:
        case GL_TEXTURE_CUBE_MAP:
        case GL_TEXTURE_2D_ARRAY:
        case GL_TEXTURE_CUBE_MAP_ARRAY:
                return 2;

        case GL_TEXTURE_3D:
                return 3;

        default:
                abort();
        }
}

const struct image_stage_info *
image_stages(void)
{
        const struct image_stage_info known[] = {
                { "Vertex", GL_VERTEX_SHADER, GL_VERTEX_SHADER_BIT },
                { "Tessellation control", GL_TESS_CONTROL_SHADER,
                  GL_TESS_CONTROL_SHADER_BIT },
                { "Tessellation evaluation", GL_TESS_EVALUATION_SHADER,
                  GL_TESS_EVALUATION_SHADER_BIT },
                { "Geometry", GL_GEOMETRY_SHADER, GL_GEOMETRY_SHADER_BIT },
                { "Fragment", GL_FRAGMENT_SHADER, GL_FRAGMENT_SHADER_BIT },
                { "Compute", GL_COMPUTE_SHADER, GL_COMPUTE_SHADER_BIT },
        };
        static struct image_stage_info supported[ARRAY_SIZE(known) + 1];

        if (!supported[0].name) {
                int i, n = 0;

                for (i = 0; i < ARRAY_SIZE(known); ++i) {
                        if (image_stage_max_images(&known[i]))
                                supported[n++] = known[i];
                }
        }

        return supported;
}

const struct image_stage_info *
get_image_stage(GLenum s)
{
        const struct image_stage_info *stage;

        for (stage = image_stages(); stage->stage; ++stage) {
                if (stage->stage == s)
                        return stage;
        }

        return NULL;
}

unsigned
image_stage_max_images(const struct image_stage_info *stage)
{
        int n = 0;

        switch (stage->stage) {
        case GL_FRAGMENT_SHADER:
                glGetIntegerv(GL_MAX_FRAGMENT_IMAGE_UNIFORMS, &n);
                return n;

        case GL_VERTEX_SHADER:
                glGetIntegerv(GL_MAX_VERTEX_IMAGE_UNIFORMS, &n);
                return n;

        case GL_GEOMETRY_SHADER:
                if (piglit_get_gl_version() >= 32)
                        glGetIntegerv(GL_MAX_GEOMETRY_IMAGE_UNIFORMS, &n);
                return n;

        case GL_TESS_CONTROL_SHADER:
                if (piglit_is_extension_supported("GL_ARB_tessellation_shader"))
                        glGetIntegerv(GL_MAX_TESS_CONTROL_IMAGE_UNIFORMS, &n);
                return n;

        case GL_TESS_EVALUATION_SHADER:
                if (piglit_is_extension_supported("GL_ARB_tessellation_shader"))
                        glGetIntegerv(GL_MAX_TESS_EVALUATION_IMAGE_UNIFORMS,
                                      &n);
                return n;

        case GL_COMPUTE_SHADER:
                if (piglit_is_extension_supported("GL_ARB_compute_shader"))
                        glGetIntegerv(GL_MAX_COMPUTE_IMAGE_UNIFORMS, &n);
                return n;

        default:
                return 0;
        }
}

unsigned
max_combined_images(void)
{
        int n;

        glGetIntegerv(GL_MAX_COMBINED_IMAGE_UNIFORMS, &n);
        return n;
}

unsigned
max_image_units(void)
{
        int n;

        glGetIntegerv(GL_MAX_IMAGE_UNITS, &n);
        return n;
}

unsigned
image_num_layers(const struct image_info img)
{
        switch (image_layer_target(img.target)) {
        case GL_TEXTURE_1D:
                return img.size.y;

        case GL_TEXTURE_2D:
                return img.size.z;

        case GL_TEXTURE_2D_MULTISAMPLE:
                return img.size.w;

        default:
                return 1;
        }
}

unsigned
image_num_levels(const struct image_info img)
{
        const unsigned d = image_target_mipmapping_dimensions(img.target);
        unsigned i, size = 1;

        for (i = 0; i < d; ++i)
                size = MAX2(size, get_idx(img.size, i));

        return (unsigned)log2(size) + 1;
}

struct image_extent
image_level_size(const struct image_info img, unsigned l)
{
        const unsigned d = image_target_mipmapping_dimensions(img.target);
        struct image_extent size;
        int i;

        for (i = 0; i < d; ++i)
                set_idx(size, i, MAX2(get_idx(img.size, i) >> l, 1));

        for (i = d; i < 4; ++i)
                set_idx(size, i, get_idx(img.size, i));

        return size;
}
