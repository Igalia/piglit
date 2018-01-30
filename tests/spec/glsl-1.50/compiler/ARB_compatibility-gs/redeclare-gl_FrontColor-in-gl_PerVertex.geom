// [config]
// expect_result: pass
// glsl_version: 1.50compatibility
// require_extensions: GL_ARB_compatibility
// [end config]
#version 150
#extension GL_ARB_compatibility : enable

in gl_PerVertex {
    vec4 gl_FrontColor;
    vec4 gl_FrontSecondaryColor;
} gl_in[];

out gl_PerVertex {
    flat vec4 gl_FrontColor;
    flat vec4 gl_FrontSecondaryColor;
};

void main()
{
}
