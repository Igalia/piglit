/*
 * Copyright © 2015 Intel Corporation
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
 * \file max dimensions.c
 *
 * Handle checks for several pnames involving max dimensions:
 *
 * MAX_WIDTH: From spec: If the resource is unsupported, zero is
 * returned."
 *
 * MAX_HEIGHT: From spec "If the resource does not have at least two
 *   dimensions, or if the resource is unsupported, zero is returned."
 *
 * MAX_DEPTH: From spec "If the resource does not have at least three
 *   dimensions, or if the resource is unsupported, zero is returned."
 *
 * MAX_LAYERS: From spec "For 1D array targets, the value returned is
 *   the same as the MAX_HEIGHT. For 2D and cube array targets, the
 *   value returned is the same as the MAX_DEPTH."
 *
 * MAX_COMBINED_DIMENSIONS: From spec "The maximum combined dimensions
 *   for the resource is returned in <params>. The combined dimensions
 *   is the product of the individual dimensions of the
 *   resource. <skip> If the resource is unsupported, zero is
 *   returned."
 *
 * Additionally it also checks that the returned values are the same
 * that the ones you receive calling GetIntegerv with equivalent
 * pnames like GL_MAX_TEXTURE_SIZE, GL_MAX_3D_TEXTURE_SIZE, etc.
 *
 * All those are internal format-independent, meanwhile
 * GetInternalformat allows to specify the internal format. So in
 * theory there is the possibility of being different for some
 * internal format. But in practice, this is not happening on any
 * driver at this moment. Query2 spec mentions this case:
 *
 *   "7) There some <pnames> which it makes no sense to be qualified
 *    by a per-format/target scope, how should we handle them?
 *    e.g. MAX_WIDTH and MAX_HEIGHT might be the same for all formats.
 *    e.g. properties like AUTO_GENERATE_MIPMAP and
 *    MANUAL_GENERATE_MIPMAP might depend only on the GL version.
 *
 *    <skip>
 *
 *    A) Just use this entry point as is, if there are no per-format
 *    or target differences, it is perfectly acceptable to have the
 *    implementation return the same information for all valid
 *    parameters. This does allow implementations to report caveats
 *    that may exist for some formats but not others, even though all
 *    formats/targets may be supported."
 *
 * So at this point, taking into account the current implementation,
 * it makes sense to check against those values.
 *
 * For MAX_COMBINED_DIMENSIONS it compares the returned value against
 * a combination of MAX_WIDH, MAX_HEIGHT, MAX_DEPTH, MAX_LAYERS and
 * SAMPLES if supported. Compares against 0 if unsupported.
 */

#include "common.h"
#include <inttypes.h>  /* for PRIu64 macro */
#include <limits.h> /* For INT_MAX */

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_RGB;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

/* Returns the equivalent GetInteger pname for a Getinternalformat
 * pname/target combination. Values 0 due number of dimensions should
 * be already filtered out */
static GLenum
equivalentPname(GLenum target,
                GLenum pname)
{
	switch (target) {
	case GL_TEXTURE_1D:
	case GL_TEXTURE_2D:
        case GL_TEXTURE_2D_MULTISAMPLE:
		return GL_MAX_TEXTURE_SIZE;
	case GL_TEXTURE_3D:
		return GL_MAX_3D_TEXTURE_SIZE;
	case GL_TEXTURE_CUBE_MAP_ARB:
		return GL_MAX_CUBE_MAP_TEXTURE_SIZE_ARB;
	case GL_TEXTURE_RECTANGLE:
		return GL_MAX_RECTANGLE_TEXTURE_SIZE;
	case GL_RENDERBUFFER_EXT:
		return GL_MAX_RENDERBUFFER_SIZE_EXT;
        case GL_TEXTURE_1D_ARRAY:
                if (pname == GL_MAX_HEIGHT)
                        return GL_MAX_ARRAY_TEXTURE_LAYERS;
                else
                        return GL_MAX_TEXTURE_SIZE;
        case GL_TEXTURE_2D_ARRAY:
        case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
                if (pname == GL_MAX_DEPTH)
                        return GL_MAX_ARRAY_TEXTURE_LAYERS;
                else
                        return GL_MAX_TEXTURE_SIZE;
        case GL_TEXTURE_CUBE_MAP_ARRAY:
                if (pname == GL_MAX_DEPTH)
                        return GL_MAX_ARRAY_TEXTURE_LAYERS;
                else
                        return GL_MAX_CUBE_MAP_TEXTURE_SIZE_ARB;
        case GL_TEXTURE_BUFFER:
                return GL_MAX_TEXTURE_BUFFER_SIZE;
	default:
		fprintf(stderr, "Invalid texture target %s\n",
                        piglit_get_gl_enum_name(target));
		return 0;
	}
}

static bool
has_layers(GLenum target)
{
        switch(target) {

        case GL_TEXTURE_1D_ARRAY:
        case GL_TEXTURE_2D_ARRAY:
        case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
        case GL_TEXTURE_CUBE_MAP_ARRAY:
                return true;
	default:
                return false;
        }
}

static bool
check_params_against_get_integer(test_data *data,
                                 GLenum pname)
{
        GLint size;
        GLint size_at_params;

        glGetIntegerv(pname, &size);
        size_at_params = test_data_value_at_index(data, 0);

        if (size != size_at_params) {
                fprintf(stderr, "GetInternalformat returns %i while GetInteger returns %i\n",
                        size_at_params, size);
        }

        return size == size_at_params;
}

/*
 * Returns the number of dimensions of @target
 */
static int
num_dimensions(const GLenum target)
{
        switch(target) {
        case GL_TEXTURE_1D:
        case GL_TEXTURE_BUFFER:
                return 1;

        /* Although technically 1D_ARRAY holds a 1D dimension texture,
         * it uses the height as the number of layers, and it is
         * created using TexImage2D, additionally from MAX_LAYERS
         * description at spec:
         *
         * "For 1D array targets, the value returned is the same as the
         * MAX_HEIGHT."
         *
         * So here are considered as having 3 dimensions
         */
        case GL_TEXTURE_1D_ARRAY:
        case GL_TEXTURE_2D:
        case GL_TEXTURE_CUBE_MAP:
        case GL_TEXTURE_RECTANGLE:
        case GL_TEXTURE_2D_MULTISAMPLE:
        case GL_RENDERBUFFER:
                return 2;

        /* Although technically, CUBE_MAP_ARRAY and 2D_ARRAY holds 2D
         * dimension textures, they use the depth as the number of
         * layers, and it is created using TexImage3D, additionally
         * from MAX_LAYERS description at spec:
         *
         * "For 2D and cube array targets, the value returned is the
         * same as the MAX_DEPTH."
         *
         * So here are considered as having 3 dimensions
         */
        case GL_TEXTURE_CUBE_MAP_ARRAY:
        case GL_TEXTURE_2D_ARRAY:
        case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
        case GL_TEXTURE_3D:
                return 3;
        default:
                return 0;
        }
}

static bool
is_supported_renderbuffer(const GLenum target, const GLenum internalformat)
{
        GLuint rb;
        bool result = true;

        glGenRenderbuffers(1, &rb);
        glBindRenderbuffer(GL_RENDERBUFFER, rb);
        glRenderbufferStorage(GL_RENDERBUFFER, internalformat, 16, 16);

        if (!piglit_check_gl_error(GL_NO_ERROR))
                result = false;

        glDeleteRenderbuffers(1, &rb);
        return result;
}

static bool
is_resource_supported(const test_data *data,
                      const GLenum pname,
                      const GLenum target,
                      const GLenum internalformat)
{
        GLuint tex;
        GLuint buffer;

        if (!check_query2_dependencies(pname, target))
                return false;

        if (!test_data_check_supported(data, target, internalformat))
                return false;

        if (target == GL_RENDERBUFFER)
                return is_supported_renderbuffer(target, internalformat);

        if (!create_texture(target, internalformat, &tex, &buffer))
                return false;

        glDeleteTextures(1, &tex);
        glDeleteBuffers(1, &buffer);

        return true;
}

static bool
try(const GLenum *targets, unsigned num_targets,
    const GLenum *internalformats, unsigned num_internalformats,
    const GLenum pname,
    const unsigned min_dimensions,
    test_data *data)
{
        bool pass = true;
        unsigned i;
        unsigned j;

	for (i = 0; i < num_targets; i++) {
		for (j = 0; j < num_internalformats; j++) {
                        bool error_test;
                        bool value_test = true;
                        bool supported;

                        supported = is_resource_supported(data, pname,
                                                          targets[i], internalformats[j]);

                        test_data_execute(data, targets[i], internalformats[j],
                                          pname);

                        error_test =
                                piglit_check_gl_error(GL_NO_ERROR);

                        /* From the spec:
                         *
                         * MAX_WIDTH
                         * "If the resource is unsupported, zero is
                         *  returned."
                         *
                         * MAX_HEIGHT
                         * "If the resource does not have at least two
                         *  dimensions, or if the resource is
                         *  unsupported, zero is returned."
                         *
                         * MAX_DEPTH:
                         * "If the resource does not have at least
                         * three dimensions, or if the resource is
                         * unsupported, zero is returned."
                         *
                         * For all those cases, we test that is zero.
                         */

                        if (!supported || num_dimensions(targets[i]) < min_dimensions) {
                                value_test = test_data_is_unsupported_response(data, pname);
                        } else {
                                /*
                                 * If suppported and enough dimensions, we compare against the values
                                 * returned by GetInteger
                                 */
                                value_test = check_params_against_get_integer(data,
                                                                              equivalentPname(targets[i], pname));
                        }

                        if (error_test && value_test)
                                continue;

                        print_failing_case(targets[i], internalformats[j], pname, data);

                        pass = false;
                }
        }

	return pass;
}

static bool
check_max_dimension(const GLenum pname,
                    const unsigned min_dimensions)
{
        bool pass = true;
        test_data *data = test_data_new(0, 1);
        int testing64;

        for (testing64 = 0; testing64 <= 1; testing64++) {
                test_data_set_testing64(data, testing64);

                pass = try(valid_targets, ARRAY_SIZE(valid_targets),
                           valid_internalformats, num_valid_internalformats,
                           pname, min_dimensions, data)
                        && pass;
        }

        piglit_report_subtest_result(pass ? PIGLIT_PASS : PIGLIT_FAIL,
                                     "%s", piglit_get_gl_enum_name(pname));
        test_data_clear(&data);
        return pass;
}

static bool
is_array(const GLenum target)
{
        switch(target) {
        case GL_TEXTURE_1D_ARRAY:
        case GL_TEXTURE_CUBE_MAP_ARRAY:
        case GL_TEXTURE_2D_ARRAY:
        case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
                return true;
        default:
                return false;
        }
}

static bool
is_1d_array(const GLenum target)
{
        switch(target) {
        case GL_TEXTURE_1D_ARRAY:
                return true;
        default:
                return false;
        }
}

/*
 * From the spec:
 *
 * "MAX_LAYERS: The maximum supported number of layers for the
 *  resource is returned in <params>. For 1D array targets, the value
 *  returned is the same as the MAX_HEIGHT. For 2D and cube array
 *  targets, the value returned is the same as the MAX_DEPTH. If the
 *  resource does not support layers, or if the resource is
 *  unsupported, zero is returned."
 *
 * This function is a check to ensure that the value is the same that
 * the other pnames. So this function calls GetInternalformat with the
 * pname MAX_HEIGHT or MAX_DEPTH (depends on @target), and compare it
 * against the value stored at @data, that is a test_data that have
 * just called GetInternalformat with MAX_LAYERS.
 */
static bool
check_params_against_dimension(test_data *data,
                               const GLenum target,
                               const GLenum internalformat)
{
        test_data *local_data;
        GLenum dimension_pname;
        bool result = true;

        if (!is_array(target))
                return true;

        if (is_1d_array(target)) {
                dimension_pname = GL_MAX_HEIGHT;
        } else {
                dimension_pname = GL_MAX_DEPTH;
        }

        local_data = test_data_clone(data);

        test_data_execute(local_data, target, internalformat,
                          dimension_pname);

        result = test_data_equal_at_index(data, local_data, 0);

        if (!result)
                fprintf(stderr, "%s value different to %s value (%" PRIi64 " != %" PRIi64 ")\n",
                        piglit_get_gl_enum_name(GL_MAX_LAYERS),
                        piglit_get_gl_enum_name(dimension_pname),
                        test_data_value_at_index(data, 0),
                        test_data_value_at_index(local_data, 0));

        test_data_clear(&local_data);

        return result;
}
/*
 * From the spec:
 *
 * "MAX_LAYERS: The maximum supported number of layers for the
 *  resource is returned in <params>. For 1D array targets, the value
 *  returned is the same as the MAX_HEIGHT. For 2D and cube array
 *  targets, the value returned is the same as the MAX_DEPTH. If the
 *  resource does not support layers, or if the resource is
 *  unsupported, zero is returned."
 */
static bool
try_max_layers(const GLenum *targets, unsigned num_targets,
               const GLenum *internalformats, unsigned num_internalformats,
               test_data *data)
{
        bool pass = true;
        unsigned i;
        unsigned j;

	for (i = 0; i < num_targets; i++) {
		for (j = 0; j < num_internalformats; j++) {
                        bool error_test;
                        bool value_test = true;
                        bool supported;

                        supported = is_resource_supported(data, GL_MAX_LAYERS,
                                                          targets[i], internalformats[j]);

                        test_data_execute(data, targets[i], internalformats[j],
                                          GL_MAX_LAYERS);

                        error_test =
                                piglit_check_gl_error(GL_NO_ERROR);

                        if (!supported || !has_layers(targets[i])) {
                                value_test = test_data_is_unsupported_response(data, GL_MAX_LAYERS);
                        } else {
                                /* We check that MAX_LAYERS is the
                                 * equal to the equivalent
                                 * MAX_HEIGHT/WIDTH */
                                value_test =
                                        check_params_against_dimension(data,
                                                                       targets[i],
                                                                       internalformats[i]);
                                /* We check that is the returned value by GetInteger */
                                value_test = value_test &&
                                        check_params_against_get_integer(data,
                                                                         GL_MAX_ARRAY_TEXTURE_LAYERS);
                        }

                        if (error_test && value_test)
                                continue;

                        print_failing_case(targets[i], internalformats[j],
                                           GL_MAX_LAYERS, data);

                        pass = false;
                }
        }

	return pass;
}

static bool
check_max_layers()
{
        bool pass = true;
        test_data *data = test_data_new(0, 1);
        int testing64;

        for (testing64 = 0; testing64 <= 1; testing64++) {
                test_data_set_testing64(data, testing64);

                pass = try_max_layers(valid_targets, ARRAY_SIZE(valid_targets),
                                      valid_internalformats, num_valid_internalformats,
                                      data)
                        && pass;
        }

        piglit_report_subtest_result(pass ? PIGLIT_PASS : PIGLIT_FAIL,
                                     "%s", piglit_get_gl_enum_name(GL_MAX_LAYERS));
        test_data_clear(&data);
        return pass;
}

static bool
is_cubemap(GLenum target)
{
        switch(target) {
        case GL_TEXTURE_CUBE_MAP:
        case GL_TEXTURE_CUBE_MAP_ARRAY:
                return true;
        default:
                return false;
        }
}

static bool
is_multisample(GLenum target)
{
        switch(target) {
        case GL_TEXTURE_2D_MULTISAMPLE:
        case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
                return true;
        default:
                return false;
        }
}

/* It computes the max dimension for @target and @internalformat, and
 * compares it against the content of @data */
static bool
check_against_combined_dimensions(test_data *data,
                                  GLenum target,
                                  GLenum internalformat,
                                  GLint64 *_combined_value)
{
        GLint current_value;
        GLint64 combined_value = 1;
        /* We don't include MAX_LAYERS, because his value is returned
           as HEIGHT (on 1d arrays) or DEPTH (on 2d arrays) */
        GLenum max_dimensions_pnames[] = {
                GL_MAX_WIDTH,
                GL_MAX_HEIGHT,
                GL_MAX_DEPTH,
                GL_SAMPLES
        };
        unsigned i;
        test_data *local_data = test_data_clone(data);

        for (i = 0; i < ARRAY_SIZE(max_dimensions_pnames); i++) {
                if (max_dimensions_pnames[i] == GL_SAMPLES &&
                    !is_multisample(target))
                        continue;

                test_data_execute(local_data, target, internalformat,
                                  max_dimensions_pnames[i]);

                current_value = test_data_value_at_index(local_data, 0);
                if (current_value != 0)
                        combined_value *= current_value;
        }

        test_data_clear(&local_data);

        /* From query2 spec: "For cube map targets this is the maximum
         * combined width, height and faces. "*/
        if (is_cubemap(target))
                combined_value *= 6;

        *_combined_value = combined_value;
        /* query2 spec doesn't specify what the 32-bit query would
         * return on cases where a 64-bit integer is needed. NVIDIA
         * drivers returns a 0, but nothing would prevent to just
         * return INT_MAX. We assume that in this case, any value
         * would be correct.
         */
        if (!test_data_get_testing64(data) && combined_value > INT_MAX)
          return true;

        return combined_value == test_data_value_at_index(data, 0);
}
/*
 * From the spec:
 *
 * "MAX_COMBINED_DIMENSIONS: The maximum combined dimensions for the
 *  resource is returned in <params>. The combined dimensions is the
 *  product of the individual dimensions of the resource. <skip> If
 *  the resource is unsupported, zero is returned"
 *
 * This method compares against zero if unsupported. If supported it
 * computes the value using MAX_WIDTH, MAX_HEIGHT, MAX_DEPTH,
 * MAX_LAYERS and SAMPLES, and compare it agains the returned value.
 */
static bool
try_max_combined_dimensions(const GLenum *targets, unsigned num_targets,
                            const GLenum *internalformats, unsigned num_internalformats,
                            test_data *data)
{
        bool pass = true;
        unsigned i;
        unsigned j;

	for (i = 0; i < num_targets; i++) {
		for (j = 0; j < num_internalformats; j++) {
                        bool error_test;
                        bool value_test = true;
                        bool supported;
                        GLint64 combined_value = -1;

                        supported = is_resource_supported(data, GL_MAX_COMBINED_DIMENSIONS,
                                                          targets[i],
                                                          internalformats[j]);

                        test_data_execute(data, targets[i], internalformats[j],
                                          GL_MAX_COMBINED_DIMENSIONS);

                        error_test =
                                piglit_check_gl_error(GL_NO_ERROR);

                        value_test = supported ?
                                check_against_combined_dimensions(data,
                                                                  targets[i],
                                                                  internalformats[j],
                                                                  &combined_value) :
                                test_data_is_unsupported_response(data,
                                                                  GL_MAX_COMBINED_DIMENSIONS);

                        if (error_test && value_test)
                                continue;

                        print_failing_case_full(targets[i], internalformats[j],
                                                GL_MAX_COMBINED_DIMENSIONS,
                                                combined_value, data);

                        pass = false;
                }
        }

	return pass;
}

static bool
check_max_combined_dimensions()
{
        bool pass = true;
        test_data *data = test_data_new(0, 1);
        int testing64;

        for (testing64 = 0; testing64 <= 1; testing64++) {
                test_data_set_testing64(data, testing64);

                pass = try_max_combined_dimensions(valid_targets, ARRAY_SIZE(valid_targets),
                                                   valid_internalformats, num_valid_internalformats,
                                                   data)
                        && pass;
        }

        piglit_report_subtest_result(pass ? PIGLIT_PASS : PIGLIT_FAIL,
                                     "%s", piglit_get_gl_enum_name(GL_MAX_COMBINED_DIMENSIONS));
        test_data_clear(&data);
        return pass;
}

void
piglit_init(int argc, char **argv)
{
        bool pass = true;

        piglit_require_extension("GL_ARB_internalformat_query2");
        initialize_valid_internalformats();

        pass = check_max_dimension(GL_MAX_WIDTH, 1) && pass;
        pass = check_max_dimension(GL_MAX_HEIGHT, 2) && pass;
        pass = check_max_dimension(GL_MAX_DEPTH, 3) && pass;
        pass = check_max_layers() && pass;
        pass = check_max_combined_dimensions() && pass;

        piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
