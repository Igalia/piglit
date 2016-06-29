// [config]
// expect_result: fail
// glsl_version: 1.30
// require_extensions: GL_MESA_shader_integer_functions
// [end config]

#version 130
#extension GL_MESA_shader_integer_functions: require

in vec4 piglit_vertex;
uniform vec4 a;
uniform vec4 b;

void main()
{
    gl_Position = fma(piglit_vertex, a, b);
}
