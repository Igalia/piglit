// [config]
// expect_result: pass
// glsl_version: 1.50compatibility
// require_extensions: GL_ARB_compatibility
// [end config]
#version 150
#extension GL_ARB_compatibility : enable

in gl_PerVertex {
    float gl_FogFragCoord;
} gl_in[];

out gl_PerVertex {
    float gl_FogFragCoord;
};

void main()
{
}
