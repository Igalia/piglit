## coding=utf-8
<%inherit file="base.mako"/>\
<%
    (glsl_version, glsl_version_int) = self.versioning()
%>\
/* [config]
 * expect_result: fail
 * glsl_version: ${glsl_version}
% if ver == 'GL_ARB_gpu_shader_fp64':
 * require_extensions: ${ver}
 * [end config]
 *
 * ${ver} spec states:
 *
 *     "No implicit conversions are
 *      provided to convert from unsigned to signed integer types, from
 *      floating-point to integer types, or from higher-precision to
 *      lower-precision types.  There are no implicit array or structure
 *      conversions."
% else:
 * [end config]
% endif
 */

${next.body()}\
