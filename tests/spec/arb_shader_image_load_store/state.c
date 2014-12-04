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

/** @file state.c
 *
 * Test image unit binding by creating a number of textures and
 * binding them as images with different parameters (including
 * incorrect arguments that are supposed to generate GL errors),
 * delete and unbind a few images and check using the state query API
 * that the implementation is keeping track of the image unit state
 * correctly.
 *
 * A second test checks that glUniform*() work as specified when used
 * to assign image units to shader image uniforms.
 */

#include "common.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

/** Image width. */
#define W 16

/** Image height. */
#define H 96

/** Total number of pixels in the image. */
#define N (W * H)

/** Maximum number of mipmap levels. */
#define M 11

config.supports_gl_core_version = 32;

config.window_width = 1;
config.window_height = 1;
config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

struct image_unit_action {
        enum {
                /** End of action list. */
                END = 0,

                /** Create a new texture object of type \a obj and
                 * bind it to the specified image unit. */
                BIND_NEW,

                /** Bind the same texture object that was previously
                 * bound to image unit \a obj to the specified image
                 * unit. */
                BIND_IDX,

                /** Bind texture object \a obj to the specified image
                 * unit. */
                BIND_OBJ,

                /** Delete the texture object that was previously
                 * bound to image unit \a obj. */
                DELETE_IDX
        } action;

        /** Image unit this action has an effect on. */
        unsigned idx;

        /** Object of this action. */
        unsigned obj;

        /** Texture mipmap level that should be bound. */
        int level;

        /** If true the whole texture level is bound rather than a
         * single layer. */
        bool layered;

        /** If \a layered is false, the index of the individual layer
         * to bind. */
        int layer;

        /** GL_READ_ONLY, GL_WRITE_ONLY or GL_READ_WRITE. */
        GLenum access;

        /** Image format used to interpret the texture data. */
        GLenum format;

        /** GL error code that should be expected after the completion
         * of this action. */
        GLenum expect_status;
};

static const struct image_unit_action actions[] = {
        { BIND_NEW, 0, GL_TEXTURE_2D,
          2, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16UI,
          GL_NO_ERROR },
        { BIND_NEW, 1, GL_TEXTURE_2D,
          1, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16F,
          GL_NO_ERROR },
        { BIND_NEW, 2, GL_TEXTURE_BUFFER,
          0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F,
          GL_NO_ERROR },
        { BIND_NEW, 3, GL_TEXTURE_2D,
          -1, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16_SNORM,
          GL_INVALID_VALUE },
        { BIND_NEW, 3, GL_TEXTURE_2D,
          0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGB565,
          GL_INVALID_VALUE },
        { BIND_NEW, 3, GL_TEXTURE_2D_ARRAY,
          0, GL_FALSE, -1, GL_WRITE_ONLY, GL_RGBA16_SNORM,
          GL_INVALID_VALUE },
        { BIND_OBJ, 3, 0xdeadcafe,
          0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8,
          GL_INVALID_VALUE },
        { BIND_NEW, 3, GL_TEXTURE_2D_ARRAY,
          0, GL_FALSE, 2, GL_WRITE_ONLY, GL_RGBA16,
          GL_NO_ERROR },
        { BIND_NEW, 4, GL_TEXTURE_2D_ARRAY,
          0, GL_TRUE, 0, GL_READ_ONLY, GL_RGBA16,
          GL_NO_ERROR },
        { BIND_OBJ, 2, 0,
          0, GL_FALSE, 0, GL_READ_ONLY, GL_R8,
          GL_NO_ERROR },
        { BIND_IDX, ~0, 1,
          0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16_SNORM,
          GL_INVALID_VALUE },
        { BIND_NEW, 5, GL_TEXTURE_2D,
          0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA16F,
          GL_NO_ERROR },
        { BIND_NEW, 6, GL_TEXTURE_3D,
          0, GL_FALSE, 3, GL_WRITE_ONLY, GL_RGBA16F,
          GL_NO_ERROR },
        { DELETE_IDX, 5, 5,
          0, GL_FALSE, 0, GL_READ_ONLY, GL_R8,
          GL_NO_ERROR },
        { END }
};

/**
 * Get the last action that modified the state of image unit \a idx.
 */
static struct image_unit_action
get_last_unit_action(unsigned idx)
{
        /* The initial image unit state is equivalent to this
         * action. */
        const struct image_unit_action def_action = {
                BIND_OBJ, idx, 0, 0, GL_FALSE, 0,
                GL_READ_ONLY, GL_R8, GL_NO_ERROR
        };
        const struct image_unit_action *a, *la = &def_action;

        for (a = actions; a->action; ++a) {
                if (a->idx == idx)
                        la = a;
        }

        return *la;
}

/**
 * Execute the given action.
 */
static bool
exec_action(const struct image_unit_action a)
{
        if (a.action == BIND_NEW) {
                const GLenum format = (get_image_format(a.format) ?
                                       a.format : GL_RGBA32F);
                const struct image_info img = image_info(a.obj, format, W, H);
                const unsigned num_levels = image_num_levels(img);
                uint32_t pixels[4 * N * M] = { 0 };

                if (!upload_image_levels(img, num_levels, 0, a.idx, pixels))
                        return false;

                glBindImageTexture(a.idx, get_texture(a.idx),
                                   a.level, a.layered, a.layer,
                                   a.access, a.format);

        } else if (a.action == BIND_IDX) {
                const unsigned idx = MIN2(a.idx, max_image_units());

                glBindImageTexture(idx, get_texture(a.obj),
                                   a.level, a.layered, a.layer,
                                   a.access, a.format);

        } else if (a.action == BIND_OBJ) {
                glBindImageTexture(a.idx, a.obj,
                                   a.level, a.layered, a.layer,
                                   a.access, a.format);

        } else if (a.action == DELETE_IDX) {
                GLuint tex = get_texture(a.idx);

                glDeleteTextures(1, &tex);

        } else {
                abort();
        }

        return piglit_check_gl_error(a.expect_status);
}

static bool
check_integer(GLenum name, unsigned idx, int expect)
{
        int v = 0xdeadcafe;

        glGetIntegeri_v(name, idx, &v);
        if (v != expect) {
                fprintf(stderr, "Invalid value for integer %s index %d\n"
                        "   Expected: %d\n"
                        "   Observed: %d\n",
                        piglit_get_gl_enum_name(name), idx, expect, v);
                return false;
        }

        return true;
}

static bool
check_tex_parameter(GLenum target, GLuint obj, GLenum name, int expect)
{
        int v = 0xdeadcafe;

        glBindTexture(target, obj);
        glGetTexParameteriv(target, name, &v);
        if (v != expect) {
                fprintf(stderr, "Invalid value for tex parameter %s\n"
                        "   Expected: %d\n"
                        "   Observed: %d\n",
                        piglit_get_gl_enum_name(name), expect, v);
                return false;
        }

        return true;
}

/**
 * Check that the image unit state matches the result of the specified
 * action.
 */
static bool
check_action(const struct image_unit_action a)
{
        if ((a.action == BIND_NEW ||
             a.action == BIND_OBJ ||
             a.action == BIND_IDX) &&
            a.expect_status == GL_NO_ERROR) {
                const GLuint obj = (a.action == BIND_NEW ? get_texture(a.idx) :
                                    a.action == BIND_IDX ? get_texture(a.obj) :
                                    a.obj);

                if (a.action == BIND_NEW &&
                    !check_tex_parameter(a.obj, obj,
                                         GL_IMAGE_FORMAT_COMPATIBILITY_TYPE,
                                         GL_IMAGE_FORMAT_COMPATIBILITY_BY_SIZE))
                        return false;

                return check_integer(GL_IMAGE_BINDING_NAME, a.idx,
                                     obj) &&
                        check_integer(GL_IMAGE_BINDING_LEVEL, a.idx,
                                      a.level) &&
                        check_integer(GL_IMAGE_BINDING_LAYERED, a.idx,
                                      a.layered) &&
                        check_integer(GL_IMAGE_BINDING_LAYER, a.idx,
                                      a.layer) &&
                        check_integer(GL_IMAGE_BINDING_ACCESS, a.idx,
                                      a.access) &&
                        check_integer(GL_IMAGE_BINDING_FORMAT, a.idx,
                                      a.format);

        } else {
                return check_integer(GL_IMAGE_BINDING_NAME, a.idx, 0);
        }
}

/**
 * Bind a number of texture objects to different image units and check
 * that the image unit state was updated correctly.
 */
static bool
run_test_binding(void)
{
        const struct image_unit_action *action;
        bool ret = true;
        int i;

        for (action = actions; action->action; ++action)
                ret &= exec_action(*action);

        for (i = 0; i < max_image_units(); ++i)
                ret &= check_action(get_last_unit_action(i));

        return ret;
}

static bool
check_uniform_int(GLuint prog, int loc, int expect)
{
        int v = 0xdeadcafe;

        glGetUniformiv(prog, loc, &v);
        if (v != expect) {
                fprintf(stderr, "Invalid value for uniform %d\n"
                        "   Expected: %d\n"
                        "   Observed: %d\n",
                        loc, expect, v);
                return false;
        }

        return piglit_check_gl_error(GL_NO_ERROR);
}

#define CHECK_INVAL_1(prefix, suffix, args, ret) do {                      \
                prefix##suffix args;                                       \
                ret &= piglit_check_gl_error(GL_INVALID_OPERATION);        \
        } while (0)

#define CHECK_INVAL_2(prefix, suffix0, suffix1, args, ret) do {            \
                CHECK_INVAL_1(prefix, suffix0, args, ret);                 \
                CHECK_INVAL_1(prefix, suffix1, args, ret);                 \
        } while (0)

#define CHECK_INVAL_3(prefix, suffix0, suffix1, suffix2, args, ret) do {   \
                CHECK_INVAL_2(prefix, suffix0, suffix1, args, ret);        \
                CHECK_INVAL_1(prefix, suffix2, args, ret);                 \
        } while (0)

/**
 * Test binding image uniforms to image units for a simple shader
 * program.
 */
static bool
run_test_uniform(void)
{
        const struct grid_info grid =
                grid_info(GL_FRAGMENT_SHADER, GL_RGBA32F, W, H);
        GLuint prog = generate_program(
                grid, GL_FRAGMENT_SHADER,
                concat(image_hunk(image_info_for_grid(grid), ""),
                       hunk("uniform IMAGE_T imgs[2];\n"
                            "\n"
                            "GRID_T op(ivec2 idx, GRID_T x) {\n"
                            "        imageStore(imgs[0], IMAGE_ADDR(idx), x);\n"
                            "        imageStore(imgs[1], IMAGE_ADDR(idx), x);\n"
                            "        return x;\n"
                            "}\n"), NULL));
        const int loc = glGetUniformLocation(prog, "imgs");
        bool ret = prog && check_uniform_int(prog, loc, 0) &&
                check_uniform_int(prog, loc + 1, 0);
        int v[2];

        glUseProgram(prog);

        /*
         * Image uniforms are bound to image units using
         * glUniform1i{v}.
         */
        glUniform1i(loc, 3);
        ret &= check_uniform_int(prog, loc, 3) &&
                check_uniform_int(prog, loc + 1, 0);

        glUniform1i(loc + 1, 3);
        ret &= check_uniform_int(prog, loc, 3) &&
                check_uniform_int(prog, loc + 1, 3);

        v[0] = 4;
        v[1] = 5;
        glUniform1iv(loc, 2, v);
        ret &= check_uniform_int(prog, loc, 4) &&
                check_uniform_int(prog, loc + 1, 5);

        /*
         * GL_INVALID_VALUE is generated if the value specified is
         * greater than or equal to the value of GL_MAX_IMAGE_UNITS.
         */
        glUniform1i(loc, max_image_units());
        ret &= piglit_check_gl_error(GL_INVALID_VALUE);

        v[0] = 3;
        v[1] = max_image_units() + 1;
        glUniform1iv(loc, 2, v);
        ret &= piglit_check_gl_error(GL_INVALID_VALUE);

        /*
         * GL_INVALID_VALUE is generated if the value specified is
         * less than zero.
         */
        glUniform1i(loc, -1);
        ret &= piglit_check_gl_error(GL_INVALID_VALUE);

        v[0] = 3;
        v[1] = -4;
        glUniform1iv(loc, 2, v);
        ret &= piglit_check_gl_error(GL_INVALID_VALUE);

        /*
         * GL_INVALID_OPERATION is generated by Uniform* functions
         * other than Uniform1i{v}.
         */
        CHECK_INVAL_2(glUniform, 1f, 1ui, (loc, 0), ret);
        CHECK_INVAL_3(glUniform, 2i, 2f, 2ui, (loc, 0, 0), ret);
        CHECK_INVAL_3(glUniform, 3i, 3f, 3ui, (loc, 0, 0, 0), ret);
        CHECK_INVAL_3(glUniform, 4i, 4f, 4ui, (loc, 0, 0, 0, 0), ret);

        CHECK_INVAL_2(glUniform, 1fv, 1uiv, (loc, 1, (void *)v), ret);
        CHECK_INVAL_3(glUniform, 2iv, 2fv, 2uiv, (loc, 1, (void *)v), ret);
        CHECK_INVAL_3(glUniform, 3iv, 3fv, 3uiv, (loc, 1, (void *)v), ret);
        CHECK_INVAL_3(glUniform, 4iv, 4fv, 4uiv, (loc, 1, (void *)v), ret);

        CHECK_INVAL_3(glUniformMatrix, 2fv, 3fv, 4fv,
                (loc, 1, GL_FALSE, (float *)v), ret);
        CHECK_INVAL_3(glUniformMatrix, 2x3fv, 3x2fv, 2x4fv,
                (loc, 1, GL_FALSE, (float *)v), ret);
        CHECK_INVAL_3(glUniformMatrix, 4x2fv, 3x4fv, 4x3fv,
                (loc, 1, GL_FALSE, (float *)v), ret);

        if (piglit_is_extension_supported("GL_ARB_gpu_shader_fp64")) {
                CHECK_INVAL_1(glUniform, 1d, (loc, 0), ret);
                CHECK_INVAL_1(glUniform, 2d, (loc, 0, 0), ret);
                CHECK_INVAL_1(glUniform, 3d, (loc, 0, 0, 0), ret);
                CHECK_INVAL_1(glUniform, 4d, (loc, 0, 0, 0, 0), ret);

                CHECK_INVAL_2(glUniform, 1dv, 2dv, (loc, 1, (double *)v), ret);
                CHECK_INVAL_2(glUniform, 3dv, 4dv, (loc, 1, (double *)v), ret);

                CHECK_INVAL_3(glUniformMatrix, 2dv, 3dv, 4dv,
                        (loc, 1, GL_FALSE, (double *)v), ret);
                CHECK_INVAL_3(glUniformMatrix, 2x3dv, 3x2dv, 2x4dv,
                        (loc, 1, GL_FALSE, (double *)v), ret);
                CHECK_INVAL_3(glUniformMatrix, 4x2dv, 3x4dv, 4x3dv,
                        (loc, 1, GL_FALSE, (double *)v), ret);
        }

        glDeleteProgram(prog);
        return ret;
}

void
piglit_init(int argc, char **argv)
{
        enum piglit_result status = PIGLIT_PASS;

        piglit_require_extension("GL_ARB_shader_image_load_store");

        subtest(&status, true, run_test_binding(),
                "binding state test");

        subtest(&status, true, run_test_uniform(),
                "uniform state test");

        piglit_report_result(status);
}

enum piglit_result
piglit_display(void)
{
        return PIGLIT_FAIL;
}
