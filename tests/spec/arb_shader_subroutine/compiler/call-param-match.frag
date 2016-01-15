// [config]
// expect_result: pass
// glsl_version: 1.50
// require_extensions: GL_ARB_shader_subroutine
// [end config]

#version 150
#extension GL_ARB_shader_subroutine: require

uniform vec4 u;
out vec4 piglit_fragcolor;
subroutine float func_type(vec4 color);

subroutine uniform func_type f;

subroutine(func_type) float R(vec4 p)
{
    return p.r;
}

subroutine(func_type) float G(vec4 p)
{
    return p.g;
}

void main()
{
    piglit_fragcolor = vec4(f(u));
}
