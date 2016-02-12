// [config]
// expect_result: fail
// glsl_version: 1.50
// require_extensions: GL_ARB_gpu_shader_fp64
// [end config]
//
// GL_ARB_gpu_shader_fp64 spec states:
//
//     "No implicit conversions are
//      provided to convert from unsigned to signed integer types, from
//      floating-point to integer types, or from higher-precision to
//      lower-precision types.  There are no implicit array or structure
//      conversions."
//

#version 150
#extension GL_ARB_gpu_shader_fp64: require

uniform int i;
uniform ubo1 {
  dvec4 colors[4];
};

in vec4 vertex;
out vec4 v;

void main()
{
    gl_Position = vertex;
    dvec4 temp[4] = colors;
    temp[0] = dvec4(1.0, 0.0, 0.0, 0.0);
    v = temp[i];
}
