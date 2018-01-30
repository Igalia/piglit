// [config]
// expect_result: pass
// glsl_version: 1.50compatibility
// require_extensions: GL_ARB_compatibility
// [end config]
#version 150
#extension GL_ARB_compatibility : enable

in gl_PerVertex {
    vec4 gl_ClipVertex;
} gl_in[];

out gl_PerVertex {
    vec4 gl_ClipVertex;
};

void main()
{
}
