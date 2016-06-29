// [config]
// expect_result: fail
// glsl_version: 1.30
// require_extensions: GL_MESA_shader_integer_functions
// [end config]

#version 130
#extension GL_MESA_shader_integer_functions: require

out vec4 piglit_fragcolor;
in vec4 a;
uniform vec4 b;
uniform vec4 c;

void main()
{
    piglit_fragcolor = fma(a, b, c);
}
