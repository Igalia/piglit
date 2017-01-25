## coding=utf-8
<%inherit file="base.mako"/>\
<%
    (glsl_version, glsl_version_int) = self.versioning()
%>\
/* [config]
 * expect_result: fail
 * glsl_version: ${glsl_version}
% if extensions:
 * require_extensions:\
% for extension in extensions:
 ${extension}\
% endfor

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
