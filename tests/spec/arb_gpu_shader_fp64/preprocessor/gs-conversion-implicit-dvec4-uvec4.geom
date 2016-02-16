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

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

uniform uvec4 uvec4_value;
uniform dvec4 dvec4_value;

in vec4 vertex_to_gs[3];
out vec4 fs_color;

#define RED vec4(1.0, 0.0, 0.0, 1.0)
#define GREEN vec4(0.0, 1.0, 0.0, 1.0)

void main()
{
    uvec4 converted = dvec4_value;
    bool match = converted == uvec4_value;
    fs_color = match ? GREEN : RED;

    for (int i = 0; i < 3; i++) {
        gl_Position = vertex_to_gs[i];
        EmitVertex();
    }
}
