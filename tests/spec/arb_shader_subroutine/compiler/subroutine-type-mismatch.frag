// [config]
// expect_result: fail
// glsl_version: 1.50
// require_extensions: GL_ARB_shader_subroutine
// [end config]

#version 150
#extension GL_ARB_shader_subroutine: require

uniform vec2 u;
subroutine vec4 func_type(vec2 p);


// The type of f() does not match func_type.  This should be an error.
subroutine(func_type) float f()
{
    return u.r;
}
