// [config]
// expect_result: fail
// glsl_version: 1.50
// require_extensions: GL_ARB_shader_subroutine
// [end config]

#version 150
#extension GL_ARB_shader_subroutine: require

uniform vec2 u;
out vec4 piglit_fragcolor;
subroutine vec4 func_type(vec2 p);

subroutine uniform func_type f;

subroutine(func_type) vec4 R(vec2 p)
{
    return vec4(p.r);
}

subroutine(func_type) vec4 G(vec2 p)
{
    return vec4(p.g);
}

void main()
{
    // This should be f(u).  You can't call subroutine type name.
    piglit_fragcolor = func_type(u);
}
