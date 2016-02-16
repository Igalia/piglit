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
#extension GL_ARB_gpu_shader_fp64 : require

uniform dmat3x2 dmat3x2_value;
uniform mat3x2 mat3x2_value;

in vec4 piglit_vertex;
out vec4 fs_color;

#define RED vec4(1.0, 0.0, 0.0, 1.0)
#define GREEN vec4(0.0, 1.0, 0.0, 1.0)

void main()
{
    gl_Position = piglit_vertex;
    mat3x2 converted = dmat3x2_value;
    bool match = converted == mat3x2_value;
    fs_color = match ? GREEN : RED;
}
