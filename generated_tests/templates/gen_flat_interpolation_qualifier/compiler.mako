## coding=utf-8
<%
    if ver == 'GL_ARB_gpu_shader_fp64':
        glsl_version_int = '150'
    else:
        glsl_version_int = ver

    glsl_version = '{}.{}'.format(glsl_version_int[0], glsl_version_int[1:3])
%>\
/* [config]
 * expect_result: ${'pass' if mode == 'flat' else 'fail'}
 * glsl_version: ${glsl_version}
% if ver == 'GL_ARB_gpu_shader_fp64':
 * require_extensions: ${ver}
% endif
 * [end config]
 *
<%block name="comments"/>\
% if ver == 'GL_ARB_gpu_shader_fp64':
 *
 * ${ver} spec states:
 *
 *    "Modifications to The OpenGL Shading Language Specification, Version 1.50
 *     (Revision 09)
 *    ...
 *        Modify Section 4.3.4, Inputs, p. 31
 *    ...
 *        (modify third paragraph, p. 32, allowing doubles as inputs and disallowing
 *        as non-flat fragment inputs) ... Fragment inputs can only be signed and
 *        unsigned integers and integer vectors, float, floating-point vectors,
 *        double, double-precision vectors, single- or double-precision matrices, or
 *        arrays or structures of these. Fragment shader inputs that are signed or
 *        unsigned integers, integer vectors, doubles, double-precision vectors, or
 *        double-precision matrices must be qualified with the interpolation
 *        qualifier flat."
% elif ver == '130':
 *
 * The OpenGL Shading Language 1.30 spec states:
 *
 *    "4.3.6 Outputs
 *    ...
 *        If a vertex output is a signed or unsigned integer or
 *        integer vector, then it must be qualified with the
 *        interpolation qualifier flat.
 *    ...
 *        The type and presence of the interpolation qualifiers and
 *        storage qualifiers and invariant qualifiers of variables
 *        with the same name declared in linked vertex and fragments
 *        shaders must match, otherwise the link command will fail."
 *
 * From this, it can be followed that if a fragment input is a signed
 * or unsigned integer or derived type, it must be qualified with the
 * interpolation qualifier flat.
% elif ver == '150':
 *
 * The OpenGL Shading Language 1.50 spec states:
 *
 *    "4.3.4 Inputs
 *    ...
 *        Fragment shader inputs that are signed or unsigned integers
 *        or integer vectors must be qualified with the interpolation
 *        qualifier flat."
% elif ver == '400':
 *
 * The OpenGL Shading Language 4.00 spec states:
 *
 *    "4.3.4 Inputs
 *    ...
 *        Fragment shader inputs that are signed or unsigned integers,
 *        integer vectors, or any double-precision floating-point type
 *        must be qualified with the interpolation qualifier flat."
% elif ver == '300 es':
 *
 * The OpenGL ES Shading Language 3.00 spec states:
 *
 *    "4.3.4 Input Variables
 *    ...
 *        Fragment shader inputs that are, or contain, signed or
 *        unsigned integers or integer vectors must be qualified with
 *        the interpolation qualifier flat."
% endif
 */

#version ${glsl_version_int}
% if ver == 'GL_ARB_gpu_shader_fp64':
#extension ${ver} : require
% endif
% if ver.endswith(' es'):

precision mediump float;
% endif

${next.body()}\
