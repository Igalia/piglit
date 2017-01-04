// [config]
// expect_result: pass
// glsl_version: 1.50
// require_extensions: GL_ARB_shading_language_420pack GL_ARB_explicit_attrib_location
// [end config]

#version 150 core
#extension GL_ARB_shading_language_420pack: require
#extension GL_ARB_explicit_attrib_location: require

layout(location=0) out vec4 o;
layout(binding=2) layout(binding=3, std140) uniform U {
    vec4 a;
} u;

void main() {
    o = u.a / 5.0;
}
