// [config]
// expect_result: fail
// glsl_version: 1.50
// require_extensions: GL_ARB_shader_subroutine
// [end config]

#version 150
#extension GL_ARB_shader_subroutine: require

subroutine void func_type(float x);

/* From GLSL 4.60.7 spec, section 6.1.2 "Subroutines":
 *
 * It is a compile-time error if arguments and return type don’t match
 * between the function and each associated subroutine type.
 */
subroutine (func_type) void f(int x) {}
