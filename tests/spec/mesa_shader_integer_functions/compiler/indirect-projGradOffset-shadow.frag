// [config]
// expect_result: fail
// glsl_version: 1.30
// require_extensions: GL_MESA_shader_integer_functions
// [end config]
#version 130
#extension GL_MESA_shader_integer_functions: require

uniform int i;
uniform vec4 coord;
uniform sampler2DShadow s[5];

void main()
{
  float f = textureProjGradOffset(s[i], coord, coord.xy, coord.zw,
				  ivec2(-5, 3));
  gl_FragColor = vec4(0, f, 0, 1);
}
