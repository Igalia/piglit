/* [config]
 * expect_result: fail
 * glsl_version: ${glsl_version}
% if glsl_version == '1.50':
 * require_extensions: GL_ARB_gpu_shader_fp64
 * [end config]
 *
 * GL_ARB_gpu_shader_fp64 spec states:
 *
 *     "Fragment outputs can only be float, single-precision
 *      floating-point vectors, signed or unsigned integers or
 *      integer vectors, or arrays of these."
% else:
 * [end config]
% endif
 */

#version ${glsl_version_int}
% if glsl_version == '1.50':
#extension GL_ARB_gpu_shader_fp64 : require
% endif

out ${type_name} color;

void main()
{
	color = ${type_name}(0.0);
}

