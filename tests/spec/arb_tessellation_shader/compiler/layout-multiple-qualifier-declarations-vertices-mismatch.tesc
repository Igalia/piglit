// [config]
// expect_result: fail
// glsl_version: 1.50
// require_extensions: GL_ARB_tessellation_shader
// check_link: true
// [end config]
//
// From the ARB_tessellation_shader spec:
//
//    "All tessellation control shader layout declarations in a program must
//     specify the same output patch vertex count."

#version 150
#extension GL_ARB_tessellation_shader: require

layout(vertices = 3) out;
layout(vertices = 4) out;

void main() {
    gl_out[gl_InvocationID].gl_Position = vec4(0.0);
    gl_TessLevelOuter = float[4](1.0, 1.0, 1.0, 1.0);
    gl_TessLevelInner = float[2](1.0, 1.0);
}
