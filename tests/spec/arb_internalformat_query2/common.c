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

#include "common.h"
#include <inttypes.h>  /* for PRIu64 macro */

/* Generic callback type, doing a cast of params to void*, to avoid
 * having two paths (32 and 64) for each check */
typedef void (APIENTRY *GetInternalformat)(GLenum target, GLenum internalformat,
                                  GLenum pname, GLsizei bufsize,
                                  void *params);

/* This struct is intended to abstract the fact that there are two
 * really similar methods, and two really similar params (just change
 * the type). All the castings and decision about which method should
 * be used would be done here, just to keep the code of the test
 * cleaner.
 */
struct _test_data {
        /* int instead of a bool to make easier iterate on the
         * possible values. */
        int testing64;
        int params_size;
        void *params;
        GetInternalformat callback;
};

/* Updates the callback and params based on current values of
 * testing64 and params_size */
static void
sync_test_data(test_data *data)
{
        if (data->params != NULL)
                free(data->params);

        if (data->testing64) {
                data->callback = (GetInternalformat) glGetInternalformati64v;
                data->params = malloc(sizeof(GLint64) * data->params_size);
        } else {
                data->callback = (GetInternalformat) glGetInternalformativ;
                data->params = malloc(sizeof(GLint) * data->params_size);
        }
}

test_data*
test_data_new(int testing64,
              int params_size)
{
        test_data *result;

        result = (test_data*) malloc(sizeof(test_data));
        result->testing64 = testing64;
        result->params_size = params_size;
        result->params = NULL;

        sync_test_data(result);

        return result;
}

/*
 * Frees @data, and sets its value to NULL.
 */
void
test_data_clear(test_data **data)
{
        test_data *_data = *data;

        if (_data == NULL)
                return;

        free(_data->params);
        _data->params = NULL;

        free(_data);
        *data = NULL;
}

void
test_data_execute(test_data *data,
                  const GLenum target,
                  const GLenum internalformat,
                  const GLenum pname)
{
        data->callback(target, internalformat, pname,
                       data->params_size, data->params);
}

/* Usually we want to call GetInternalformati*v with the size of the
 * buffer, but there are some cases where we want to specify a
 * different size */
void
test_data_execute_with_size(test_data *data,
                            const GLenum target,
                            const GLenum internalformat,
                            const GLenum pname,
                            int size)
{
        data->callback(target, internalformat, pname,
                       size, data->params);
}

void
test_data_set_testing64(test_data *data,
                        const int testing64)
{
        if (data->testing64 == testing64)
                return;

        data->testing64 = testing64;
        sync_test_data(data);
}

void
test_data_set_params_size(test_data *data,
                          const int params_size)
{
        if (data->params_size == params_size)
                return;

        data->params_size = params_size;
        sync_test_data(data);
}

GLint64
test_data_value_at_index(test_data *data,
                         const int index)
{
        if (index > data->params_size || index < 0) {
                fprintf(stderr, "ERROR: invalid index while retrieving"
                        " data from auxiliar test data\n");
                return -1;
        }

        return data->testing64 ?
                ((GLint64*)data->params)[index] :
                ((GLint*)data->params)[index];
}

/*
 * Returns if @target/@internalformat is supported using
 * INTERNALFORMAT_SUPPORTED for @target and @internalformat.
 *
 * @data is only used to known if we are testing the 32-bit or the
 * 64-bit query, so the content of @data will not be modified due this
 * call.
 */
bool
test_data_check_supported(const test_data *data,
                          const GLenum target,
                          const GLenum internalformat)
{
        bool result;
        test_data *local_data = test_data_new(data->testing64, 1);

        test_data_execute(local_data, target, internalformat,
                          GL_INTERNALFORMAT_SUPPORTED);

        if (!piglit_check_gl_error(GL_NO_ERROR))
                result = false;
        else
                result = test_data_value_at_index(local_data, 0) == GL_TRUE;

        test_data_clear(&local_data);

        return result;
}

/* Returns if @value is one of the values among @set */
bool
value_on_set(const GLint *set,
       const unsigned set_size,
       GLint value)
{
        unsigned i;

        for (i = 0; i < set_size; i++) {
                if (set[i] == value)
                        return true;
        }

        return false;
}

bool
test_data_check_possible_values(test_data *data,
                                const GLint *possible_values,
                                const unsigned num_possible_values)
{
        return value_on_set(possible_values, num_possible_values,
                            test_data_value_at_index(data, 0));
}

/*
 * Prints the info of a failing case for a given pname.
 *
 * Note that it tries to get the name of the value at @data as if it
 * were a enum, as it is useful on that case. But there are several
 * pnames that returns a value. A possible improvement would be that
 * for those just printing the value.
 */
void
print_failing_case(const GLenum target, const GLenum internalformat,
                   const GLenum pname, test_data *data)
{
        print_failing_case_full(target, internalformat, pname, -1, data);
}

/*
 * Prints the info of a failing case. If expected_value is smaller
 * that 0, it is not printed.
*/
void print_failing_case_full(const GLenum target, const GLenum internalformat,
                             const GLenum pname, GLint64 expected_value,
                             test_data *data)
{
        /* Knowing if it is supported is interesting in order to know
         * if the test is being too restrictive */
        bool supported = test_data_check_supported(data, target, internalformat);
        GLint64 current_value = test_data_value_at_index(data, 0);

        if (data->testing64) {
                fprintf(stderr,  "    64 bit failing case: ");
        } else {
                fprintf(stderr,  "    32 bit failing case: ");
        }

        fprintf(stderr, "pname = %s, "
                "target = %s, internalformat = %s, ",
                piglit_get_gl_enum_name(pname),
                piglit_get_gl_enum_name(target),
                piglit_get_gl_enum_name(internalformat));

        if (expected_value >= 0)
                fprintf(stderr, "expected value = (%" PRIi64 "), ",
                        expected_value);

        fprintf(stderr, "params[0] = (%" PRIi64 ",%s), "
                "supported=%i\n",
                current_value,
                piglit_get_gl_enum_name(current_value),
                supported);
}

/*
 * The most basic condition. From spec, a lot of pnames has a
 * condition like this:
 *
 * "Possible values returned are <set>. If the resource is not
 *  supported, or if the operation is not supported, NONE is
 *  returned."
 *
 * So this method, calls the callback defined at @data (that should be
 * GetInternalformativ or GetInternalformati64v) using @pname, for all
 * @num_targets at @targets, and all @num_internalformats at
 * @internalformats, and checks the following conditions:
 *
 * * If @pname is not supported (calling INTERNALFORMAT_SUPPORTED),
 *   checks that the value returned is always the same.
 * * If @pname is supported, checks that the returned value is among
 *   one of the values defined at @possible_values
 *
 * @possible_values,@num_possible_values is allowed to be NULL,0, for
 * the cases where the set of returned values is not specified in
 * detail by the spec (like INTERNALFORMAT_PREFERRED). On that case,
 * it is not tested the returned value, and just tested that if not
 * suppported, the returned value is the usupported value defined by
 * the spec.
 *
 */
bool
try_basic(const GLenum *targets, unsigned num_targets,
          const GLenum *internalformats, unsigned num_internalformats,
          const GLenum pname,
          const GLint *possible_values, unsigned num_possible_values,
          test_data *data)
{
        bool pass = true;
        unsigned i;
        unsigned j;

	for (i = 0; i < num_targets; i++) {
                for (j = 0; j < num_internalformats; j++) {
                        bool error_test;
                        bool value_test;
                        bool supported;

                        supported = check_query2_dependencies(pname, targets[i])
                                && test_data_check_supported(data, targets[i],
                                                             internalformats[j]);

                        test_data_execute(data, targets[i], internalformats[j], pname);

                        if (supported && num_possible_values == 0)
                                continue;

                        error_test =
                                piglit_check_gl_error(GL_NO_ERROR);

                        value_test = supported ?
                                test_data_check_possible_values(data, possible_values,
                                                                num_possible_values) :
                                test_data_is_unsupported_response(data, pname);

                        if (error_test && value_test)
                                continue;

                        print_failing_case(targets[i], internalformats[j],
                                           pname, data);

                        pass = false;
                }
        }

	return pass;
}

/* Returns a valid format for @internalformat, so it would be possible
 * to create a texture using glTexImageXD with that
 * format/internalformat combination */
static GLenum
format_for_internalformat(const GLenum internalformat)
{
        switch(internalformat) {
        case GL_DEPTH_COMPONENT:
        case GL_DEPTH_COMPONENT16:
        case GL_DEPTH_COMPONENT24:
        case GL_DEPTH_COMPONENT32:
        case GL_DEPTH_COMPONENT32F:
                return GL_DEPTH_COMPONENT;
        case GL_DEPTH_STENCIL:
        case GL_DEPTH24_STENCIL8:
        case GL_DEPTH32F_STENCIL8:
                return GL_DEPTH_STENCIL;
        case GL_RGB10_A2UI:
        case GL_R8I:
        case GL_R8UI:
        case GL_R16I:
        case GL_R16UI:
        case GL_R32I:
        case GL_R32UI:
        case GL_RG8I:
        case GL_RG16I:
        case GL_RG16UI:
        case GL_RG32I:
        case GL_RG32UI:
        case GL_RGB8I:
        case GL_RGB8UI:
        case GL_RGB16I:
        case GL_RGB16UI:
        case GL_RGB32I:
        case GL_RGB32UI:
        case GL_RGBA8I:
        case GL_RGBA8UI:
        case GL_RGBA16I:
        case GL_RGBA16UI:
        case GL_RGBA32I:
        case GL_RGBA32UI:
                return GL_RGBA_INTEGER;
        default:
                return GL_RGBA;
        }
}

static GLenum
type_for_internalformat(const GLenum internalformat)
{
        switch(internalformat) {
        case GL_DEPTH_STENCIL:
        case GL_DEPTH24_STENCIL8:
        case GL_DEPTH32F_STENCIL8:
                return GL_UNSIGNED_INT_24_8;
        default:
                return GL_UNSIGNED_BYTE;
        }
}

/*
 * Some GetInternalformati*v pnames returns the same that
 * GetTexParameter and GetTexLevelParameter. In order to use those, a
 * texture is needed to be bound. This method creates and bind one
 * texture based on @target and @internalformat. It returns the
 * texture name on @tex_out.  If target is GL_TEXTURE_BUFFER, a buffer
 * is also needed, and returned on @buffer_out. Caller is responsible
 * to free both if the call is successful.
 *
 * The type and format to be used to create the texture is any one
 * valid for the given @internalformat.
 *
 * For texture targets, we also use this function to check if the /resource/
 * (defined in the ARB_internalformat_query2 spec as an object of the
 * appropriate type that has been created with <internalformat> and <target>)
 * is supported by the implementation. If the texture creation fails, then the
 * resource is unsupported.
 *
 * Returns true if it was possible to create the texture. False
 * otherwise (unsupported /resource/).
 *
 */
bool
create_texture(const GLenum target,
               const GLenum internalformat,
               GLuint *tex_out,
               GLuint *buffer_out)
{
        GLuint tex = 0;
        GLuint buffer = 0;
        GLenum type = type_for_internalformat(internalformat);
        GLenum format = format_for_internalformat(internalformat);
        bool result = true;
        int height = 16;
        int width = 16;
        int depth = 16;
        unsigned i;

        glGenTextures(1, &tex);
        glBindTexture(target, tex);

        switch(target) {
        case GL_TEXTURE_1D:
                glTexImage1D(target, 0, internalformat, width, 0,
                             format, type, NULL);
                break;
        case GL_TEXTURE_1D_ARRAY:
        case GL_TEXTURE_2D:
        case GL_TEXTURE_RECTANGLE:
                glTexImage2D(target, 0, internalformat, width, height, 0,
                             format, type, NULL);
                break;
        case GL_TEXTURE_CUBE_MAP:
                for (i = 0; i < 6; i++) {
                        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,
                                     internalformat, width, height, 0, format, type,
                                     NULL);
                }
                break;

        case GL_TEXTURE_CUBE_MAP_ARRAY:
                /* cube map arrays also use TexImage3D buth depth
                 * needs to be a multiple of six */
                depth = 6;
                /* fall through */
        case GL_TEXTURE_2D_ARRAY:
        case GL_TEXTURE_3D:
                glTexImage3D(target, 0, internalformat, width, height, depth, 0,
                             format, type, NULL);
                break;
        case GL_TEXTURE_2D_MULTISAMPLE:
		glTexImage2DMultisample(target, 1, internalformat, width, height,
                                        GL_FALSE);
		break;
	case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
		glTexImage3DMultisample(target, 1, internalformat, width, height,
                                        depth, GL_FALSE);
		break;
        case GL_TEXTURE_BUFFER:
                glGenBuffers(1, &buffer);
                glBindBuffer(GL_TEXTURE_BUFFER, buffer);
                glTexBuffer(GL_TEXTURE_BUFFER, internalformat, buffer);
                break;
        default:
                result = false;
                fprintf(stderr, "\tError: %s is not a texture target\n",
                        piglit_get_gl_enum_name(target));
        }

        if (!piglit_check_gl_error(GL_NO_ERROR))
                result = false;

        if (!result) {
                glDeleteTextures(1, &tex);
                glDeleteBuffers(1, &buffer);
        } else {
                *tex_out = tex;
                *buffer_out = buffer;
        }
        return result;
}


static GLenum
translate_pname(const GLenum pname)
{
        switch (pname) {
        case GL_INTERNALFORMAT_RED_TYPE:
        case GL_INTERNALFORMAT_GREEN_TYPE:
        case GL_INTERNALFORMAT_BLUE_TYPE:
        case GL_INTERNALFORMAT_ALPHA_TYPE:
               return pname - GL_INTERNALFORMAT_RED_TYPE + GL_TEXTURE_RED_TYPE;
         case GL_INTERNALFORMAT_DEPTH_TYPE:
        /* case GL_INTERNALFORMAT_STENCIL_TYPE, */
                return GL_TEXTURE_DEPTH_TYPE;
        case GL_INTERNALFORMAT_RED_SIZE:
        case GL_INTERNALFORMAT_GREEN_SIZE:
        case GL_INTERNALFORMAT_BLUE_SIZE:
        case GL_INTERNALFORMAT_ALPHA_SIZE:
                return pname - GL_INTERNALFORMAT_RED_SIZE + GL_TEXTURE_RED_SIZE;
        case GL_INTERNALFORMAT_DEPTH_SIZE:
                return GL_TEXTURE_DEPTH_SIZE;
        case GL_INTERNALFORMAT_STENCIL_SIZE:
                return GL_TEXTURE_STENCIL_SIZE;
        case GL_INTERNALFORMAT_SHARED_SIZE:
                return GL_TEXTURE_SHARED_SIZE;
        default:
                assert(!"incorrect pname");
                return 0;
        }
}

/*
 * Builds a a texture using @target and @internalformat, and compares
 * the result of calling GetTexLevelParameter using @pname with the
 * result included at @data.params.
 *
 * At this point it is assumed that @target/@internalformat is a valid
 * combination to create a texture unless it is not supported by the
 * implementation. If the call to create_texture with those parameters
 * fails, it is assumed that the resource is unsupported, so the check
 * only compares against zero (the unsupported value).
 *
 * Returns true if the value is the same, false otherwise
 */
bool
test_data_check_against_get_tex_level_parameter(test_data *data,
                                                const GLenum target,
                                                const GLenum pname,
                                                const GLenum internalformat)
{
        GLint param;
        bool result = true;
        GLuint tex;
        GLuint buffer;
        GLenum real_target = target;
        GLenum pname_equiv = translate_pname(pname);

        result = create_texture(target, internalformat, &tex, &buffer);
        if (!result)
                return test_data_is_unsupported_response(data, pname);

        /* For cube maps GetTexLevelParameter receives one of the face
         * targets, or proxy */
        if (target == GL_TEXTURE_CUBE_MAP) {
                real_target = GL_TEXTURE_CUBE_MAP_POSITIVE_X;
        }
        glGetTexLevelParameteriv(real_target, 0, pname_equiv, &param);
        if (!piglit_check_gl_error(GL_NO_ERROR)) {
                result = false;
                fprintf(stderr, "\tError calling glGetTexLevelParameter\n");
                goto cleanup;
        }

        result = test_data_value_at_index(data, 0) == param;

        if (!result) {
                fprintf(stderr, "\tError comparing glGetInternalformat "
                        "and glGetTexLevelParameter, params value=%" PRIi64 ", "
                        "expected value=%i\n",
                        test_data_value_at_index(data, 0), param);
        }

cleanup:
        glDeleteTextures(1, &tex);
        glDeleteBuffers(1, &buffer);

        return result;
}

/*
 * Returns if @pname query2 dependencies are fulfilled. So for
 * example, FRAMEBUFFER_RENDERABLE needs
 * ARB/EXT_framebuffer_object. With that pname, if
 * ARB/EXT_framebuffer_object is not present, it returns false.
 *
 * It is assumed that @pname is a valid query2 pname, as would
 * simplify the implementation of this method.
 */
static bool
check_query2_pname_dependencies(const GLenum pname)
{
        switch(pname) {
        case GL_FRAMEBUFFER_RENDERABLE:
        case GL_FRAMEBUFFER_BLEND:
                if (!piglit_is_extension_supported("GL_ARB_framebuffer_object") &&
                    !piglit_is_extension_supported("GL_EXT_framebuffer_object"))
                        return false;
                break;

        case GL_FRAMEBUFFER_RENDERABLE_LAYERED:
                if (!piglit_is_extension_supported("GL_ARB_framebuffer_object") &&
                    !piglit_is_extension_supported("GL_EXT_framebuffer_object"))
                        return false;
                if (!piglit_is_extension_supported("GL_EXT_texture_array"))
                        return false;
                break;

        case GL_MANUAL_GENERATE_MIPMAP:
                if (!piglit_is_extension_supported("GL_ARB_framebuffer_object") &&
                    !piglit_is_extension_supported("GL_EXT_framebuffer_object"))
                        return false;
                break;

        case GL_MAX_LAYERS:
                if (!piglit_is_extension_supported("GL_EXT_texture_array"))
                        return false;
                break;

        case GL_SRGB_READ:
                if (!piglit_is_extension_supported("GL_EXT_texture_sRGB"))
                        return false;
                break;

        case GL_SRGB_WRITE:
                if (!piglit_is_extension_supported("GL_ARB_framebuffer_sRGB"))
                        return false;
                break;

        case GL_SRGB_DECODE_ARB:
                /* Note that if the extension is not supported, it
                 * should return INVALID_ENUM, not unsupported */
                if (!piglit_is_extension_supported("GL_ARB_texture_sRGB_decode") &&
                    !piglit_is_extension_supported("GL_EXT_texture_sRGB_decode"))
                        return false;
                break;

        case GL_TESS_CONTROL_TEXTURE:
        case GL_TESS_EVALUATION_TEXTURE:
                if (!piglit_is_extension_supported("GL_ARB_tessellation_shader"))
                        return false;
                break;

        case GL_GEOMETRY_TEXTURE:
                if (!piglit_is_extension_supported("GL_ARB_geometry_shader4") &&
                    !(piglit_get_gl_version() >= 32))
                        return false;
                break;

        case GL_COMPUTE_TEXTURE:
                if (!piglit_is_extension_supported("GL_ARB_compute_shader"))
                        return false;
                break;

        case GL_TEXTURE_GATHER:
                if (!piglit_is_extension_supported("GL_ARB_texture_gather"))
                        return false;
                break;

        case GL_SHADER_IMAGE_LOAD:
        case GL_SHADER_IMAGE_STORE:
        case GL_SHADER_IMAGE_ATOMIC:
        case GL_IMAGE_TEXEL_SIZE:
        case GL_IMAGE_COMPATIBILITY_CLASS:
        case GL_IMAGE_PIXEL_FORMAT:
        case GL_IMAGE_PIXEL_TYPE:
        case GL_IMAGE_FORMAT_COMPATIBILITY_TYPE:
                if (!piglit_is_extension_supported("GL_ARB_shader_image_load_store"))
                        return false;
                break;

        case GL_CLEAR_BUFFER:
                if (!piglit_is_extension_supported("GL_ARB_clear_buffer_object"))
                        return false;
                break;

        case GL_TEXTURE_VIEW:
        case GL_VIEW_COMPATIBILITY_CLASS:
                if (!piglit_is_extension_supported("GL_ARB_texture_view"))
                        return false;
                break;

                        case GL_SAMPLES:
                break;

        default:
                return true;
        }

        return true;
}

/*
 * Returns if @target query2 dependencies are fulfilled. So for
 * example, TEXTURE_1D_ARRAY needs
 * EXT_texture_array. With that pname, if
 * ARB/EXT_framebuffer_object is not present, it returns false.
 *
 * It is assumed that @target is a valid query2 target, as would
 * simplify the implementation of this method.
 */
static bool
check_query2_target_dependencies(const GLenum target)
{
        switch(target) {
        case GL_TEXTURE_1D_ARRAY:
        case GL_TEXTURE_2D_ARRAY:
                if (!piglit_is_extension_supported("GL_EXT_texture_array"))
                        return false;
                break;

        case GL_TEXTURE_CUBE_MAP_ARRAY:
                if (!piglit_is_extension_supported("GL_ARB_texture_cube_map_array"))
                        return false;
                break;

        case GL_TEXTURE_2D_MULTISAMPLE:
        case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
                if (!piglit_is_extension_supported("GL_ARB_texture_multisample"))
                        return false;
                break;

        case GL_TEXTURE_RECTANGLE:
                if (!piglit_is_extension_supported("GL_ARB_texture_rectangle"))
                        return false;
                break;

        case GL_RENDERBUFFER:
                if (!piglit_is_extension_supported("GL_ARB_framebuffer_object") &&
                    !piglit_is_extension_supported("GL_EXT_framebuffer_object"))
                        return false;
                break;

        case GL_TEXTURE_BUFFER:
                if (!piglit_is_extension_supported("GL_ARB_texture_buffer_object") &&
                     piglit_get_gl_version() < 31)
                        return false;
                break;

        default:
                return true;
        }

        return true;
}

/* Returns is @pname and @target dependencies are fullfilled */
bool
check_query2_dependencies(const GLenum pname,
                          const GLenum target)
{
        return check_query2_target_dependencies(target) &&
                check_query2_pname_dependencies(pname);
}

/*
 * Gets the unsupported response for any given @pname.
 */
static GLint
get_unsupported_response(GLenum pname)
{
        switch(pname) {
        case GL_SAMPLES:
                /* This one is special as if unsupported, the params
                 * parameter at GetInternalformativ should not be
                 * modified. We return 0 for this method, but should
                 * be took into account by the caller */
                return 0;
                break;

        case GL_MAX_COMBINED_DIMENSIONS:
        case GL_NUM_SAMPLE_COUNTS:
        case GL_INTERNALFORMAT_RED_SIZE:
        case GL_INTERNALFORMAT_GREEN_SIZE:
        case GL_INTERNALFORMAT_BLUE_SIZE:
        case GL_INTERNALFORMAT_ALPHA_SIZE:
        case GL_INTERNALFORMAT_DEPTH_SIZE:
        case GL_INTERNALFORMAT_STENCIL_SIZE:
        case GL_INTERNALFORMAT_SHARED_SIZE:
        case GL_MAX_WIDTH:
        case GL_MAX_HEIGHT:
        case GL_MAX_DEPTH:
        case GL_MAX_LAYERS:
        case GL_IMAGE_TEXEL_SIZE:
        case GL_TEXTURE_COMPRESSED_BLOCK_WIDTH:
        case GL_TEXTURE_COMPRESSED_BLOCK_HEIGHT:
        case GL_TEXTURE_COMPRESSED_BLOCK_SIZE:
                return 0;
                break;

        case GL_INTERNALFORMAT_PREFERRED:
        case GL_INTERNALFORMAT_RED_TYPE:
        case GL_INTERNALFORMAT_GREEN_TYPE:
        case GL_INTERNALFORMAT_BLUE_TYPE:
        case GL_INTERNALFORMAT_ALPHA_TYPE:
        case GL_INTERNALFORMAT_DEPTH_TYPE:
        case GL_INTERNALFORMAT_STENCIL_TYPE:
        case GL_FRAMEBUFFER_RENDERABLE:
        case GL_FRAMEBUFFER_RENDERABLE_LAYERED:
        case GL_FRAMEBUFFER_BLEND:
        case GL_READ_PIXELS:
        case GL_READ_PIXELS_FORMAT:
        case GL_READ_PIXELS_TYPE:
        case GL_TEXTURE_IMAGE_FORMAT:
        case GL_TEXTURE_IMAGE_TYPE:
        case GL_GET_TEXTURE_IMAGE_FORMAT:
        case GL_GET_TEXTURE_IMAGE_TYPE:
        case GL_MANUAL_GENERATE_MIPMAP:
        case GL_AUTO_GENERATE_MIPMAP:
        case GL_COLOR_ENCODING:
        case GL_SRGB_READ:
        case GL_SRGB_WRITE:
        case GL_SRGB_DECODE_ARB:
        case GL_FILTER:
        case GL_VERTEX_TEXTURE:
        case GL_TESS_CONTROL_TEXTURE:
        case GL_TESS_EVALUATION_TEXTURE:
        case GL_GEOMETRY_TEXTURE:
        case GL_FRAGMENT_TEXTURE:
        case GL_COMPUTE_TEXTURE:
        case GL_TEXTURE_SHADOW:
        case GL_TEXTURE_GATHER:
        case GL_TEXTURE_GATHER_SHADOW:
        case GL_SHADER_IMAGE_LOAD:
        case GL_SHADER_IMAGE_STORE:
        case GL_SHADER_IMAGE_ATOMIC:
        case GL_IMAGE_COMPATIBILITY_CLASS:
        case GL_IMAGE_PIXEL_FORMAT:
        case GL_IMAGE_PIXEL_TYPE:
        case GL_IMAGE_FORMAT_COMPATIBILITY_TYPE:
        case GL_SIMULTANEOUS_TEXTURE_AND_DEPTH_TEST:
        case GL_SIMULTANEOUS_TEXTURE_AND_STENCIL_TEST:
        case GL_SIMULTANEOUS_TEXTURE_AND_DEPTH_WRITE:
        case GL_SIMULTANEOUS_TEXTURE_AND_STENCIL_WRITE:
        case GL_CLEAR_BUFFER:
        case GL_TEXTURE_VIEW:
        case GL_VIEW_COMPATIBILITY_CLASS:
                return GL_NONE;
                break;

        case GL_INTERNALFORMAT_SUPPORTED:
        case GL_COLOR_COMPONENTS:
        case GL_DEPTH_COMPONENTS:
        case GL_STENCIL_COMPONENTS:
        case GL_COLOR_RENDERABLE:
        case GL_DEPTH_RENDERABLE:
        case GL_STENCIL_RENDERABLE:
        case GL_MIPMAP:
        case GL_TEXTURE_COMPRESSED:
                return GL_FALSE;
                break;

        default:
                fprintf(stderr, "Error: %i(%s) is not a valid GetInternalformativ pname",
                        pname, piglit_get_gl_enum_name(pname));
        }

        return 0;
}

/*
 * Returns if the content of @data contains the unsupported value for
 * @pname. It is assumed that the pname would return just one value.
 */
bool
test_data_is_unsupported_response(test_data *data,
                                  GLenum pname)
{
        return test_data_value_at_index(data, 0) == get_unsupported_response(pname);
}

/*
 * Sets the value of @data params at @index to @value.
 */
void
test_data_set_value_at_index(test_data *data,
                             const int index,
                             const GLint64 value)
{
        if (index > data->params_size || index < 0) {
                fprintf(stderr, "ERROR: invalid index while setting"
                        " auxiliar test data\n");
                return;
        }

        if (data->testing64) {
                ((GLint64*)data->params)[index] = value;
        } else {
                ((GLint*)data->params)[index] = value;
        }
}

bool
test_data_equal_at_index(test_data *data,
                         test_data *data_copy,
                         unsigned index)
{
        if (data->testing64 != data_copy->testing64) {
                fprintf(stderr, "ERROR: trying to compare imcompatible"
                        " auxiliar test data structures\n");
                return false;
        }
        if (data->params_size != data_copy->params_size) {
                fprintf(stderr, "ERROR: trying to compare imcompatible"
                        " auxiliar test data structures\n");
                return false;
        }
        if (index > data->params_size) {
                fprintf(stderr, "ERROR: invalid index while setting"
                        " auxiliar test data\n");
                return false;
        }

        return (test_data_value_at_index(data, index) ==
                test_data_value_at_index(data_copy, index));
}

test_data*
test_data_clone(test_data *data)
{
        test_data *clone;

        clone = test_data_new(data->testing64, data->params_size);

        return clone;
}

int
test_data_get_testing64(test_data *data)
{
        return data->testing64;
}

int
test_data_get_params_size(test_data *data)
{
        return data->params_size;
}
