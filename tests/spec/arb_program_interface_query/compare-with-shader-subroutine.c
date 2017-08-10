/*
 * Copyright © 2016 Intel Corporation
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
 * \file compare-with-shader-subroutine
 *
 * Tests that the values returned by equivalent queries from
 * ARB_shader_subroutine (glGetProgramStageiv) and
 * ARB_program_interface_query (glGetProgramInterfaveiv) return the
 * same value.
 *
 * From the GL_ARB_program_interface_query spec:
 *      "The command
 *
 *      void GetProgramInterfaceiv(uint program, enum programInterface,
 *                                 enum pname, int *params);
 *
 *      queries a property of the interface <programInterface> in program
 *      <program>, returning its value in <params>.  The property to return is
 *      specified by <pname>.
 *
 *      If <pname> is ACTIVE_RESOURCES, the value returned is the number of
 *      resources in the active resource list for <programInterface>. If the
 *      list of active resources for <programInterface> is empty, zero is
 *      returned."
 *
 *      "The supported values for <programInterface> are as follows:
 *
 *     * VERTEX_SUBROUTINE_UNIFORM, TESS_CONTROL_SUBROUTINE_UNIFORM,
 *       TESS_EVALUATION_SUBROUTINE_UNIFORM, GEOMETRY_SUBROUTINE_UNIFORM,
 *       FRAGMENT_SUBROUTINE_UNIFORM, and COMPUTE_SUBROUTINE_UNIFORM correspond
 *       to the set of active subroutine uniform variables used by the vertex,
 *       tessellation control, tessellation evaluation, geometry, fragment, and
 *       compute shader stages of <program>, respectively (section 2.14.8)."
 *
 * From the GL_ARB_shader_subroutine spec:
 *
 *     "The command
 *
 *      void GetProgramStageiv(uint program, enum shadertype,
 *                             enum pname, int *values);
 *
 *      returns properties of the program object <program> specific to the
 *      programmable stage corresponding to <shadertype> in <values>. The
 *      parameter value to return is specified by <pname>.  If <pname> is
 *      ACTIVE_SUBROUTINE_UNIFORMS, the number of active subroutine variables in
 *      the stage is returned."
 */

#include "piglit-util-gl.h"
#include "common.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

        config.supports_gl_core_version = 32;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

/* The shaders try to be as simple as possible for each stage, using
 * subroutines
 **/
static const char *vs_text = {
        "#version 150\n"
        "#extension GL_ARB_shader_subroutine : require\n"
        "\n"
        "in vec4 vs_input;\n"
        "out vec4 vs_output;\n"
        "\n"
        "subroutine vec4 vs_subroutine();\n"
        "subroutine uniform vs_subroutine vs[2];\n"
        "subroutine(vs_subroutine) vec4 vs1() {\n"
        "        return vec4(1, 0, 0, 0);\n"
        "}\n"
        "subroutine(vs_subroutine) vec4 vs2() {\n"
        "       return vec4(1, 0, 0, 0);\n"
        "}\n"
        "void main() {\n"
        "        gl_Position = vs_input;\n"
        "        vs_output = vs[0]() + vs[1]();\n"
        "}\n"
};

static const char *tcs_text = {
        "#version 150\n"
        "#extension GL_ARB_shader_subroutine : require\n"
        "#extension GL_ARB_tessellation_shader: require\n"
        "\n"
        "layout(vertices = 3) out;\n"
        "\n"
        "subroutine vec4 tcs_subroutine();\n"
        "subroutine uniform tcs_subroutine tcs[2];\n"
        "subroutine(tcs_subroutine) vec4 tcs1() {\n"
        "        return vec4(1, 0, 0, 0);\n"
        "}\n"
        "subroutine(tcs_subroutine) vec4 tcs2() {\n"
        "       return vec4(1, 0, 0, 0);\n"
        "}\n"
        "void main() {\n"
        "        gl_out[gl_InvocationID].gl_Position = tcs[0]() + tcs[1]();\n"
        "        gl_TessLevelInner[0] = 1.0;\n"
        "        gl_TessLevelInner[1] = 1.0;\n"
        "        gl_TessLevelOuter[0] = 1.0;\n"
        "        gl_TessLevelOuter[1] = 1.0;\n"
        "        gl_TessLevelOuter[2] = 1.0;\n"
        "}\n"
};

static const char *tes_text = {
        "#version 150\n"
        "#extension GL_ARB_shader_subroutine : require\n"
        "#extension GL_ARB_tessellation_shader: require\n"
        "\n"
        "layout(triangles, equal_spacing) in;\n"
        "\n"
        "subroutine vec4 tes_subroutine();\n"
        "subroutine uniform tes_subroutine tes[2];\n"
        "subroutine(tes_subroutine) vec4 tes1() {\n"
        "        return vec4(1, 0, 0, 0);\n"
        "}\n"
        "subroutine(tes_subroutine) vec4 tes2() {\n"
        "       return vec4(1, 0, 0, 0);\n"
        "}\n"
        "void main() {\n"
        "        gl_Position = tes[0]() + tes[1]();\n"
        "}\n"
};

static const char *gs_text = {
        "#version 150\n"
        "#extension GL_ARB_shader_subroutine : require\n"
        "layout(triangles) in;\n"
        "layout(triangle_strip, max_vertices = 4) out;\n"
        "\n"
        "subroutine vec4 gs_subroutine();\n"
        "subroutine uniform gs_subroutine gs[4];\n"
        "subroutine(gs_subroutine) vec4 gs1() {\n"
        "        return vec4(1, 0, 0, 0);\n"
        "}\n"
        "subroutine(gs_subroutine) vec4 gs2() {\n"
        "        return vec4(1, 0, 0, 0);\n"
        "}\n"
        "void main() {\n"
        "        gl_Position = gs[0]();\n"
        "        EmitVertex();\n"
        "        gl_Position = gs[1]();\n"
        "        EmitVertex();\n"
        "        gl_Position = gs[2]();\n"
        "        EmitVertex();\n"
        "        gl_Position = gs[3]();\n"
        "        EmitVertex();\n"
        "        EndPrimitive();\n"
        "}\n"
};

static const char *fs_text = {
        "#version 150\n"
        "#extension GL_ARB_shader_subroutine : require\n"
        "\n"
        "out vec4 fs_output;\n"
        "\n"
        "subroutine vec4 fs_subroutine();\n"
        "subroutine uniform fs_subroutine fs[3];\n"
        "subroutine(fs_subroutine) vec4 fs1() {\n"
        "        return vec4(1, 0, 0, 0);\n"
        "}\n"
        "subroutine(fs_subroutine) vec4 fs2() {\n"
        "        return vec4(1, 0, 0, 0);\n"
        "}\n"
        "void main() {\n"
        "        fs_output = fs[0]() + fs[1]() + fs[2]();\n"
        "}\n"
};

/* We need SSBO in order to be able to use a buffer. And we need a
 * buffer so the subroutine uniforms get active, in order to get a num
 * of active uniforms different to 0.
 */
static const char *cs_text = {
        "#version 150\n"
        "#extension GL_ARB_shader_subroutine : require\n"
        "#extension GL_ARB_compute_shader : require\n"
        "#extension GL_ARB_shader_storage_buffer_object : require\n"
        "\n"
        "layout(local_size_x = 1) in;\n"
        "\n"
        "buffer out_buffer {\n"
        "        vec4 data;\n"
        "} g_out;\n"
        "\n"
        "subroutine vec4 cs_subroutine();\n"
        "subroutine uniform cs_subroutine cs[4];\n"
        "subroutine(cs_subroutine) vec4 cs1() {\n"
        "        return vec4(1, 0, 0, 0);\n"
        "}\n"
        "subroutine(cs_subroutine) vec4 cs2() {\n"
        "        return vec4(1, 0, 0, 0);\n"
        "}\n"
        "void main() {\n"
        "        g_out.data = cs[0]() + cs[1]();\n"
        "}\n"
};

static const GLenum subtests[] = {
        GL_VERTEX_SUBROUTINE_UNIFORM,
        GL_TESS_CONTROL_SUBROUTINE_UNIFORM,
        GL_TESS_EVALUATION_SUBROUTINE_UNIFORM,
        GL_GEOMETRY_SUBROUTINE_UNIFORM,
        GL_FRAGMENT_SUBROUTINE_UNIFORM,
        GL_COMPUTE_SUBROUTINE_UNIFORM,
};

static GLenum
get_shadertype_from_program_interface(const GLenum programInterface)
{
        switch(programInterface) {
        case GL_VERTEX_SUBROUTINE_UNIFORM:
                return GL_VERTEX_SHADER;
        case GL_TESS_CONTROL_SUBROUTINE_UNIFORM:
                return GL_TESS_CONTROL_SHADER;
        case GL_TESS_EVALUATION_SUBROUTINE_UNIFORM:
                return GL_TESS_EVALUATION_SHADER;
        case GL_GEOMETRY_SUBROUTINE_UNIFORM:
                return GL_GEOMETRY_SHADER;
        case GL_FRAGMENT_SUBROUTINE_UNIFORM:
                return GL_FRAGMENT_SHADER;
        case GL_COMPUTE_SUBROUTINE_UNIFORM:
                return GL_COMPUTE_SHADER;
        default:
                assert(!"unexpected programInterface value");
                return GL_INVALID_ENUM;
        }
}

static GLuint
create_program(const GLenum programInterface)
{
        GLuint program = 0;

        switch(programInterface) {
        case GL_VERTEX_SUBROUTINE_UNIFORM:
        case GL_GEOMETRY_SUBROUTINE_UNIFORM:
        case GL_FRAGMENT_SUBROUTINE_UNIFORM:
                program = piglit_build_simple_program_unlinked_multiple_shaders(
                                              GL_VERTEX_SHADER, vs_text,
                                              GL_GEOMETRY_SHADER, gs_text,
                                              GL_FRAGMENT_SHADER, fs_text, 0);
                break;

        case GL_TESS_CONTROL_SUBROUTINE_UNIFORM:
        case GL_TESS_EVALUATION_SUBROUTINE_UNIFORM:
                program = piglit_build_simple_program_unlinked_multiple_shaders(
                                              GL_VERTEX_SHADER, vs_text,
                                              GL_TESS_CONTROL_SHADER, tcs_text,
                                              GL_TESS_EVALUATION_SHADER, tes_text, 0);
                break;

        case GL_COMPUTE_SUBROUTINE_UNIFORM:
                program = piglit_build_simple_program_unlinked_multiple_shaders(
                                              GL_COMPUTE_SHADER, cs_text, 0);
                break;
        default:
                assert(!"unexpected programInterface value");
        }

        piglit_check_gl_error(GL_NO_ERROR);

        return program;
}

static bool
skip_subtest(const GLenum programInterface)
{
        switch(programInterface) {
        case GL_COMPUTE_SUBROUTINE_UNIFORM:
                return !piglit_is_extension_supported("GL_ARB_compute_shader") ||
                        !piglit_is_extension_supported("GL_ARB_shader_storage_buffer_object");

        case GL_TESS_CONTROL_SUBROUTINE_UNIFORM:
        case GL_TESS_EVALUATION_SUBROUTINE_UNIFORM:
                return !piglit_is_extension_supported("GL_ARB_tessellation_shader");

        default:
                return false;
        }
}

static bool
run_subtest(const GLenum programInterface,
            const bool link)
{
        GLenum shader_type;
        GLint value;
        GLint value2;
        GLuint program = 0;
        const char *linked_string = link ? "linked" : "not linked";

        if (skip_subtest(programInterface))
                return true;

        program = create_program(programInterface);
        if (link) {
                glLinkProgram(program);
                if (!piglit_link_check_status(program))
                        piglit_report_result(PIGLIT_FAIL);
	}

        shader_type = get_shadertype_from_program_interface(programInterface);

        glGetProgramStageiv(program, shader_type, GL_ACTIVE_SUBROUTINE_UNIFORMS, &value);
        glGetProgramInterfaceiv(program, programInterface, GL_ACTIVE_RESOURCES, &value2);

        if (value == value2) {
                piglit_report_subtest_result(PIGLIT_PASS, "%s (%s)",
                                             piglit_get_gl_enum_name(programInterface),
                                             linked_string);

        } else {
                piglit_report_subtest_result(PIGLIT_FAIL, "%s (%s): GetProgramStage returns %i, "
                                             "GetProgramInterfaceiv returns %i",
                                             piglit_get_gl_enum_name(programInterface),
                                             linked_string, value, value2);
        }

        return value == value2;
}

void
piglit_init(int argc, char **argv)
{
        piglit_require_extension("GL_ARB_program_interface_query");
        piglit_require_extension("GL_ARB_shader_subroutine");
}

enum piglit_result
piglit_display(void)
{
        bool pass = true;
        int i;

        for (i = 0; i < ARRAY_SIZE(subtests); i++) {
                pass = run_subtest(subtests[i], false) && pass;
        }

        for (i = 0; i < ARRAY_SIZE(subtests); i++) {
                pass = run_subtest(subtests[i], true) && pass;
        }

        return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
